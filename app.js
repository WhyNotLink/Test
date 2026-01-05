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


     const playlistData = [
            { id: 1, name: "晴天", singer: "周杰伦", duration: "4:29" },
            { id: 2, name: "花海", singer: "周杰伦", duration: "4:24" },
            { id: 3, name: "小幸运", singer: "田馥甄", duration: "4:25" },
            { id: 4, name: "起风了", singer: "买辣椒也用券", duration: "5:23" },
            { id: 5, name: "稻香", singer: "周杰伦", duration: "3:43" },
            { id: 6, name: "水星记", singer: "郭顶", duration: "5:25" }
        ];

    const songListEl = document.getElementById("songList");

    function renderPlaylist() {
        songListEl.innerHTML = "";
        playlistData.forEach(song => {
            const liEl = document.createElement("li");
            liEl.className = "song-item";
            liEl.dataset.songId = song.id;
            liEl.innerHTML = `
                <div class="song-info">
                    <span class="song-name">${song.name}</span>
                    <span class="song-singer">${song.singer}</span>
                </div>
                <span class="song-duration">${song.duration}</span>
            `;

            liEl.addEventListener("click", function() {
                document.querySelectorAll(".song-item").forEach(item => {
                    item.classList.remove("active");
                });
                this.classList.add("active");
                const songId = this.dataset.songId;
                const targetSong = playlistData.find(item => item.id == songId);
                showTip(`你点击了：${targetSong.name} - ${targetSong.singer}`);
                console.log("点击的歌曲详情：", targetSong);
            });
            songListEl.appendChild(liEl);
        });
    }

    







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
            webSocket = new WebSocket('ws://127.0.0.1:8080');
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
                this.classList.toggle('bg_blue');
                this.classList.toggle('bg_red');
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
    

})