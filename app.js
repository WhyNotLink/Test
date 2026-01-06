document.addEventListener('DOMContentLoaded',function(){

    const btn1 = document.getElementById('link');
    const text1 = document.getElementById('ip');
    const text2 = document.getElementById('port');

    const btn2 = document.getElementById('start');
    const btn3 = document.getElementById('stop');
    const btn4 = document.getElementById('last');
    const btn5 = document.getElementById('next');
    const btn6 = document.getElementById('singlecircle');
    const btn7 = document.getElementById('listcircle');
    const btn8 = document.getElementById('random');
    const btn9 = document.getElementById('songlist_get');

    const playlistData = [
            { id: 1, name: "晴天"},
            { id: 2, name: "花海" },
            { id: 3, name: "小幸运"},
            { id: 4, name: "起风了"},
            { id: 5, name: "稻香"},
            { id: 6, name: "水星记"}
        ];

    const songListElement = document.getElementById("songList");

    // playlistData.forEach((song, index) => {
    //     const li = document.createElement("li");
    //     li.textContent = `${song.name}`;
    //     li.dataset.songId = song.id;
    //     li.dataset.soneName = song.name;
    //     songListElement.appendChild(li);
    // });
    // songListElement.addEventListener("click", function(event) {
    //     if(event.target.nodeName === "LI") {
    //         const songId = event.target.dataset.songId;
    //         const songName = event.target.dataset.soneName;
    //         alert(`准备播放：${songName} (ID: ${songId})`);
    //         console.log("点击的歌曲详情：", { id: songId, name: songName });


    //         const sendMsg = 'song:' + songId;
    //         webSocket.send(sendMsg);
    //         console.log('已发送消息到中转服务器：', sendMsg);

    //     }
    // });




    function loadSongListFromJson() {
        fetch('./udp-proxy-serve/linux_messages.json')
            .then(response => {
                if (!response.ok) {
                    throw new Error(`请求失败：${response.status} ${response.statusText}`);
                }
                return response.json();
            })
            .then(playlistData => {
                songListElement.innerHTML = '';

                if (!Array.isArray(playlistData) || playlistData.length === 0) {
                    songListElement.innerHTML = '<li>暂无歌单数据</li>';
                    console.log('歌单JSON文件为空或格式错误');
                    return;
                }

                playlistData.forEach((song, index) => {
                    if (!song.id || !song.name) {
                        console.warn(`第${index+1}条歌单数据无效（缺少id/name）：`, song);
                        return;
                    }
                    const li = document.createElement("li");
                    li.textContent = `${song.name}`;
                    li.dataset.songId = song.id;
                    li.dataset.songName = song.name; 
                    songListElement.appendChild(li);
                });
            })
            .catch(error => {
                songListElement.innerHTML = '<li>加载歌单失败</li>';
                console.error('加载歌单JSON失败：', error);
            });
    }


    songListElement.addEventListener("click", function(event) {
        if(event.target.nodeName === "LI") {
            const songId = event.target.dataset.songId;
            const songName = event.target.dataset.songName; 
            if (!songId || !songName) return; 
            
            alert(`准备播放：${songName} (ID: ${songId})`);
            console.log("点击的歌曲详情：", { id: songId, name: songName });

            if (webSocket && webSocket.readyState === WebSocket.OPEN) {
                const sendMsg = 'song:' + songId;
                webSocket.send(sendMsg);
                console.log('已发送消息到中转服务器：', sendMsg);
            } else {
                alert('未连接到中转服务器，无法发送播放指令！');
            }
        }
    });






    let linkflag = 0;
    let ip = 0;
    let port = 0;
    let webSocket = null;
    const ipReg = /^((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)$/;
    const numReg = /^[0-9]+$/;


    btn1.addEventListener('click',function(){
        ip = text1.value.trim();
        port = text2.value.trim();

        const isIpValid = ip !== "" && ipReg.test(ip);
        const isPortPureNum = numReg.test(port);
        const portNum = parseInt(port, 10);
        const isPortValid = isPortPureNum && portNum >= 1 && portNum <= 65536;
        linkflag++;

        if(linkflag==2)
        {
            linkflag = 0;
            text1.value="";
            text2.value="";
            this.classList.toggle('bg_blue');
            this.classList.toggle('bg_red');
            if(webSocket)
            {
                webSocket.close();
                webSocket=null; 
            }
        }

        if ((!isIpValid || !isPortValid)&&linkflag==1) {
            linkflag = 0;
            alert('输入错误!请填写合法IP和端口号');
            text1.value="";
            text2.value="";
        }else if(linkflag==1){
            this.classList.toggle('bg_blue');
            this.classList.toggle('bg_red');
        }

        if(linkflag==1)
        {
            webSocket = new WebSocket('ws://127.0.0.1:8001');//这是本机tcp与udp传输的端口
            webSocket.onopen = () => {
                alert('已连接中转服务器');
                const linuxConfig = JSON.stringify({
                    ip: ip,
                    port: port
                });
                webSocket.send(linuxConfig);
            };

        
            webSocket.onerror = (err) => {
                linkflag = 0;
                alert('连接中转服务器失败！');
                console.error('WebSocket错误:', err);
                webSocket = null;
            };

            webSocket.onclose = () => {
                linkflag = 0;
                this.classList.toggle('bg_blue');
                this.classList.toggle('bg_red');
                alert('与中转服务器的连接已断开');
                webSocket = null;
            };

            webSocket.onmessage = (event) => {
                console.log('中转服务器反馈：', event.data);
                // alert('中转服务器反馈：' + event.data);
            };
        }
    })

    btn2.addEventListener('click',function(){
        if (linkflag !== 1 || !webSocket || webSocket.readyState !== WebSocket.OPEN) {
            alert('未建立有效连接,请先绑定Linux IP和端口!');
            return;
        }

        const sendMsg = 'a'
        webSocket.send(sendMsg);
        console.log('已发送消息到中转服务器：', sendMsg);

    })
    btn3.addEventListener('click',function(){
        if (linkflag !== 1 || !webSocket || webSocket.readyState !== WebSocket.OPEN) {
            alert('未建立有效连接,请先绑定Linux IP和端口!');
            return;
        }

        const sendMsg = 'b'
        webSocket.send(sendMsg);
        console.log('已发送消息到中转服务器：', sendMsg);

    })
    btn4.addEventListener('click',function(){
        if (linkflag !== 1 || !webSocket || webSocket.readyState !== WebSocket.OPEN) {
            alert('未建立有效连接,请先绑定Linux IP和端口!');
            return;
        }

        const sendMsg = 'c'
        webSocket.send(sendMsg);
        console.log('已发送消息到中转服务器：', sendMsg);

    })
    btn5.addEventListener('click',function(){
        if (linkflag !== 1 || !webSocket || webSocket.readyState !== WebSocket.OPEN) {
            alert('未建立有效连接,请先绑定Linux IP和端口!');
            return;
        }

        const sendMsg = 'd'
        webSocket.send(sendMsg);
        console.log('已发送消息到中转服务器：', sendMsg);

    })
    btn6.addEventListener('click',function(){
        if (linkflag !== 1 || !webSocket || webSocket.readyState !== WebSocket.OPEN) {
            alert('未建立有效连接,请先绑定Linux IP和端口!');
            return;
        }

        const sendMsg = 'e'
        webSocket.send(sendMsg);
        console.log('已发送消息到中转服务器：', sendMsg);

    })
    btn7.addEventListener('click',function(){
        if (linkflag !== 1 || !webSocket || webSocket.readyState !== WebSocket.OPEN) {
            alert('未建立有效连接,请先绑定Linux IP和端口!');
            return;
        }

        const sendMsg = 'f'
        webSocket.send(sendMsg);
        console.log('已发送消息到中转服务器：', sendMsg);

    })
    btn8.addEventListener('click',function(){
        if (linkflag !== 1 || !webSocket || webSocket.readyState !== WebSocket.OPEN) {
            alert('未建立有效连接,请先绑定Linux IP和端口!');
            return;
        }

        const sendMsg = 'g'
        webSocket.send(sendMsg);
        console.log('已发送消息到中转服务器：', sendMsg);

    })
    btn9.addEventListener('click',function(){
        const sendMsg = 'l'
        webSocket.send(sendMsg);
        console.log('获取歌单：', sendMsg);
        setTimeout(function() {
            loadSongListFromJson(); 
        }, 1000);

    })




})