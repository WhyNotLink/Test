const WebSocket = require('ws');
const dgram = require('dgram');

const udpProxyClient = dgram.createSocket('udp4');

const wss = new WebSocket.Server({ port: 8080 }); // 监听端口
console.log('中间中转服务器已启动，监听WebSocket 端口');

// 3. 存储Linux主机的UDP地址（由网页传入）
let linuxUdpIp = '';
let linuxUdpPort = 0;

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