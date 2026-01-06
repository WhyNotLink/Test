const WebSocket = require('ws');
const dgram = require('dgram');

const fs = require('fs').promises; // 异步fs（支持await）
const fsSync = require('fs');      // 同步fs（用于判断文件是否存在）
const path = require('path');
const tempJsonPath = path.join(__dirname, 'temp_linux_messages.json');

const udpProxyClient = dgram.createSocket('udp4');
const jsonFilePath = path.join(__dirname, 'linux_messages.json');

let linuxUdpMessages = [];
const RX_UDP_PORT = 8889;//linux端发送到中间服务器的端口

const wss = new WebSocket.Server({ port: 8001 }); //这是本机tcp与udp传输的端口
console.log('中间中转服务器已启动，监听WebSocket 端口');

// 3. 存储Linux主机的UDP地址（由网页传入）
let linuxUdpIp = '';
let linuxUdpPort = 0;





async function writeMessagesToJson() {
    // try {
    //     const jsonContent = JSON.stringify(linuxUdpMessages, null, 2);
    //     fs.truncate(jsonFilePath, 0, (truncateErr) => {
    //         if (truncateErr) {
    //             console.error('清空JSON文件失败：', truncateErr);
    //             return;
    //         }
    //     fs.writeFile(jsonFilePath, jsonContent, (err) => {
    //         if (err) {
    //             console.error('写入JSON文件失败：', err);
    //         } else {
    //             console.log(`inux消息已保存到JSON文件：${jsonFilePath}`);
    //         }
    //     });
    // });
    // } catch (e) {
    //     console.error('JSON格式化失败：', e);
    // }
    try {
        const validData = Array.isArray(linuxUdpMessages) ? linuxUdpMessages : [];
        const jsonContent = JSON.stringify(validData, null, 2);

        await fs.writeFile(tempJsonPath, jsonContent, 'utf8');
        await fs.rename(tempJsonPath, jsonFilePath);

        console.log(`Linux消息已保存到JSON文件：${jsonFilePath}`);
    } catch (err) {
        console.error('写入JSON文件失败：', err);
        if (fsSync.existsSync(tempJsonPath)) {
            await fs.unlink(tempJsonPath);
        }
    }
     
}



// function initJsonFile() {
//     if (!fs.existsSync(jsonFilePath)) {
//         fs.writeFile(jsonFilePath, '[]', (err) => {
//             if (err) {
//                 console.error('初始化JSON文件失败：', err);
//             } else {
//                 console.log(`初始化JSON文件成功：${jsonFilePath}`);
//             }
//         });
//     } else {
//         fs.readFile(jsonFilePath, 'utf8', (err, data) => {
//             if (err) {
//                 console.error('读取现有JSON文件失败：', err);
//                 return;
//             }
//             try {
//                 linuxUdpMessages = [];
//                 console.log(`已加载现有Linux消息，共${linuxUdpMessages.length}条`);
//             } catch (e) {
//                 console.error('JSON文件格式错误，重置为空数组：', e);
//                 linuxUdpMessages = [];
//             }
//         });
//     }
// }
async function initJsonFile() {
    try {
        if (!fsSync.existsSync(jsonFilePath)) {
            await fs.writeFile(jsonFilePath, '[]', 'utf8');
            console.log(`初始化JSON文件成功：${jsonFilePath}`);
        } else {
            const data = await fs.readFile(jsonFilePath, 'utf8');
            const parsedData = JSON.parse(data || '[]');
            linuxUdpMessages = Array.isArray(parsedData) ? parsedData : [];
            console.log(`已加载现有Linux消息，共${linuxUdpMessages.length}条`);
        }
    } catch (e) {
        console.error('JSON文件初始化/解析失败，重置为空数组：', e);
        linuxUdpMessages = [];
        await fs.writeFile(jsonFilePath, '[]', 'utf8');
    }
}


initJsonFile();

udpProxyClient.on('message', (msg, rinfo) => {
    const linuxMsg = msg.toString('utf8').trim();
    let messageList = []; 

    try {
        const parsedData = JSON.parse(linuxMsg);
        const dataArray = Array.isArray(parsedData) ? parsedData : [parsedData];
        dataArray.forEach((item, index) => {
            if (item.id && item.name) {
                messageList.push({ id: item.id, name: item.name });
            } else {
                console.error(`第${index+1}条数据无效（缺少id/name）：`, item);
            }
        });
    } catch (e) {
        console.error('接收到的Linux消息非合法JSON格式：', linuxMsg);
        return;
    }

    if (messageList.length > 0) {
        linuxUdpMessages = messageList;
        console.log(`接收Linux歌单数据，共${messageList.length}条有效：`, messageList);
        writeMessagesToJson();
    } else {
        console.log('未接收到有效Linux歌单数据：', linuxMsg);
    }
});



udpProxyClient.bind(RX_UDP_PORT, () => {
    const address = udpProxyClient.address();
    console.log(`UDP客户端已绑定固定本地端口：${address.address}:${address.port}（Linux请往这个地址发UDP回复）`);
});



// 监听网页客户端的WebSocket连接
wss.on('connection', (ws) => {
    console.log('有网页客户端建立连接');

    // 接收网页发送的消息
    ws.on('message', (data) => {
        const msgStr = data.toString();
        try {
            // 第一步：网页先传入Linux主机的UDP IP和端口（JSON格式）
            const linuxUdpConfig = JSON.parse(msgStr);
            if (linuxUdpConfig.ip && linuxUdpConfig.port) {
                linuxUdpIp = linuxUdpConfig.ip; 
                linuxUdpPort = linuxUdpConfig.port; 
                ws.send(`已绑定Linux UDP地址：${linuxUdpIp}:${linuxUdpPort}`);
                console.log(`已绑定Linux UDP地址：${linuxUdpIp}:${linuxUdpPort}`);
                return;
            }
        } catch (e) {
            // 非JSON格式 = 业务消息，转发给Linux主机
            if (!linuxUdpIp || !linuxUdpPort) {
                ws.send('请先配置Linux主机的UDP IP和端口');
                return;
            }

            // 将网页消息转为Buffer，通过UDP转发给Linux主机
            const msgBuffer = Buffer.from(msgStr, 'utf8');
            udpProxyClient.send(
                msgBuffer,
                0,
                msgBuffer.length,
                linuxUdpPort,
                linuxUdpIp,
                (err) => {
                    if (err) {
                        console.error('转发UDP消息到Linux失败：', err);
                        ws.send('消息转发失败：' + err.message);
                    } else {
                        console.log(`已转发消息到Linux ${linuxUdpIp}:${linuxUdpPort}：${msgStr}`);
                        ws.send('消息转发成功');
                    }
                }
            );
        }
    });

    // 监听网页客户端断开连接
    ws.on('close', () => {
        console.log('网页客户端断开连接');
    });

    // WebSocket错误处理
    ws.on('error', (err) => {
        console.error('WebSocket错误：', err);
    });
});

// UDP客户端错误处理
udpProxyClient.on('error', (err) => {
    console.error('UDP代理客户端错误：', err);
    udpProxyClient.close();
});


