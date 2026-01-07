#include "main4.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>


// UDP 配置参数
#define UDP_PORT 8000  // UDP接收端口，可自定义
#define BUF_SIZE 32    // UDP接收缓冲区大小
pid_t child_pid = -1; // 保存子进程PID

static const char *DEST_IP = "172.16.46.155";
static const int DEST_PORT = 8889;

void* udp_receive_thread(void* arg);
void send_song_json_udp();

int main()
{
    int ch;
    pid_t music_pid = -1; // 记录madplay进程PID（如果能获取）
    pthread_t udp_thread;

    getname(); 
    printf("共找到以上本地MP3文件\n");

    child_pid = fork();
    if (child_pid < 0)
    {
        perror("fork");
        return -1;
    }
    else if (child_pid == 0)
    {
            
        signal(SIGUSR1, test_connect);                // 测试连接
        signal(SIGUSR2, play_singlecircle_music);     // 单曲循环
        signal(SIGRTMIN+0, play_listcircle_music);    // 列表循环
        signal(SIGRTMIN+1, play_random_music);        // 随机播放
        signal(SIGRTMIN+2, last_play_music);          // 上一首歌
        signal(SIGRTMIN+3, next_play_music);          // 下一首歌
        signal(SIGINT, SIG_IGN);                      // 忽略Ctrl+C中断信号
        signal(SIGTERM, SIG_IGN);
        
        while (1)
        {
            pause(); 
        }
        exit(0);
    }


    int thread_ret = pthread_create(&udp_thread, NULL, udp_receive_thread, NULL);
    if (thread_ret != 0)
        {
            perror("pthread_create udp thread failed");
            printf("UDP线程创建失败，继续运行本地键盘控制功能...\n");
        }
    else
        {
            pthread_detach(udp_thread);
            printf("UDP接收线程创建成功，监听端口 %d（等待a/g指令）...\n", UDP_PORT);
        }

    sleep(2);
    kill(child_pid, SIGUSR1);


    while (1) 
    {
        // 播放界面
        printf("*********************************\n");
        printf("            按1暂停播放            \n");
        printf("            按2继续播放            \n");
        printf("            按3单曲循环            \n");
        printf("            按4列表循环            \n");
        printf("            按5随机播放            \n");
        printf("            按6上一首歌            \n");
        printf("            按7下一首歌            \n");
        printf("            按0退出系统            \n");
        printf("*********************************\n");

        printf("请输入你需要进行的操作：\n");
        while (scanf("%d", &ch) != 1)
        {
            printf("输入有误！请输入0-7之间的数字：\n");
            while (getchar() != '\n');
        }

        if (ch < 0 || ch > 7)
        {
            printf("输入有误！请输入0-7之间的数字\n");
            continue;
        }

        switch (ch)
        {
            case 1:
                printf("暂停\n");
                sprintf(cmd_buf, "killall -SIGSTOP madplay 2>/dev/null");
                system(cmd_buf);
                break;
            case 2:
                printf("播放\n");
                sprintf(cmd_buf, "killall -SIGCONT madplay 2>/dev/null");
                system(cmd_buf);
                break;
            case 3:
                printf("单曲循环\n");
                kill(child_pid, SIGUSR2);
                break;
            case 4:
                printf("列表循环\n");
                kill(child_pid, SIGRTMIN+0);
                break;
            case 5:
                printf("随机播放\n");
                kill(child_pid, SIGRTMIN+1);
                break;
            case 6:
                kill(child_pid, SIGRTMIN+2);
                break;
            case 7:
                kill(child_pid, SIGRTMIN+3);
                break;
            case 0:
                printf("正在退出系统，停止音乐播放...\n");

                sprintf(song_buf, "killall -9 madplay 2>/dev/null");
                system(song_buf);
                usleep(200000); 

                if (child_pid > 0)
                {
                    kill(child_pid, 9);
                    waitpid(child_pid, NULL, 0); 
                }
                
                int ret = system("pgrep madplay > /dev/null");
                if (ret == 0) 
                {
                    printf("检测到残留的madplay进程，再次强制终止...\n");
                    system("killall -9 madplay 2>/dev/null");
                }
                
                system("amixer set Master mute > /dev/null 2>&1");
                usleep(100000);
                system("amixer set Master unmute > /dev/null 2>&1");

                printf("退出成功！\n");
                exit(EXIT_SUCCESS); 
                break;
            default:
                break;
        }
    }

    waitpid(child_pid, NULL, 0);
    return 0;
}


void* udp_receive_thread(void* arg)
{
    int udp_socket_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char recv_buf[BUF_SIZE] = {0};
    ssize_t recv_len;

    udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket_fd < 0)
    {
        perror("udp socket create failed");
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_addr.sin_port = htons(UDP_PORT);

    if (bind(udp_socket_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("udp bind failed");
        close(udp_socket_fd);
        pthread_exit(NULL);
    }

    while (1)
    {
        memset(recv_buf, 0, BUF_SIZE); 
        recv_len = recvfrom(udp_socket_fd, recv_buf, BUF_SIZE - 1, 0,
                            (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len < 0)
        {
            perror("udp recvfrom failed");
            continue;
        }

        printf("\nUDP接收来自 %s:%d 的指令：%c\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port),
               recv_buf[0]);

        switch (recv_buf[0])
        {
            case 'a': // 对应 开始播放（与键盘2功能一致）
                printf("UDP指令：开始播放\n");
                sprintf(cmd_buf, "killall -SIGCONT madplay 2>/dev/null");
                system(cmd_buf);
                break;
            case 'b': // 对应 暂停播放（与键盘1功能一致）
                printf("UDP指令：暂停播放\n");
                sprintf(cmd_buf, "killall -SIGSTOP madplay 2>/dev/null");
                system(cmd_buf);
                break;
            case 'c': // 对应 上一首歌（与键盘6功能一致）
                printf("UDP指令：上一首歌\n");
                if (child_pid > 0)
                {
                    kill(child_pid, SIGRTMIN+2);
                }
                break;
            case 'd': // 对应 下一首歌（与键盘7功能一致）
                printf("UDP指令：下一首歌\n");
                if (child_pid > 0)
                {
                    kill(child_pid, SIGRTMIN+3);
                }
                break;
            case 'e': // 对应 单曲循环（与键盘3功能一致）
                printf("UDP指令：单曲循环\n");
                if (child_pid > 0)
                {
                    kill(child_pid, SIGUSR2);
                }
                break;
            case 'f': // 对应 列表循环（与键盘4功能一致）
                printf("UDP指令：列表循环\n");
                if (child_pid > 0)
                {
                    kill(child_pid, SIGRTMIN+0);
                }
                break;
            case 'g': // 对应 随机播放（与键盘5功能一致）
                printf("UDP指令：随机播放\n");
                if (child_pid > 0)
                {
                    kill(child_pid, SIGRTMIN+1);
                }
                break;
            case 'l': // 对应 获取歌单
                printf("UDP指令：获取歌单\n");
                send_song_json_udp();
                break;
            case 's':
                savename();
                playing_or_not();
                if(playing_flag)
                {
                    sprintf(song_buf,"killall -9 madplay aplay");
                    system(song_buf);
                }
                char *colon_pos = strchr(recv_buf,':');
                if(colon_pos == NULL){
                    now=0;
                }else{
                    now = atoi(colon_pos + 1);
                    if(now < 0 || now >= all){
                        now = 0;
                    }
                }
                sprintf(song_buf,"madplay -o wav:- /root/System_project/mp3/ %s 2> /dev/null | aplay 2>/dev/null &",song[now]); 
	            system(song_buf);
                break;
            default:
                printf("UDP无效指令：%c，仅支持a/g/s/l\n", recv_buf[0]);
                break;
        }
    }

    // 实际循环不会退出，此处为语法完整性
    close(udp_socket_fd);
    pthread_exit(NULL);
}

void send_song_json_udp(){

    int sockfd;
    struct sockaddr_in dest_addr;

    char* json = (char*)malloc(1024 * sizeof(char));
    strcpy(json, "[");
    for (int i=0; i< all; i++) 
    {
        char obj[128]; 
        if(song[i]!=NULL){
            snprintf(obj, sizeof(obj), "{\"id\":%d,\"name\":\"%s\"}", i, song[i]);
            strcat(json, obj);
            if (i != all - 1) {
                strcat(json, ",");
            }else{
                strcat(json, "]");
            }
        }
    }
    // const char *json = "[{\"id\":1,\"name\":\"晴天\"},{\"id\":2,\"name\":\"花海吗\"},{\"id\":99}]";
    
    // 创建 UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(DEST_PORT);
    dest_addr.sin_addr.s_addr = inet_addr(DEST_IP);

    printf("Sending JSON to %s:%d\n", DEST_IP, DEST_PORT);

    ssize_t sent = sendto(sockfd,
                          json,
                          strlen(json),
                          0,
                          (struct sockaddr *)&dest_addr,
                          sizeof(dest_addr));
    if (sent < 0) {
        perror("sendto");
        close(sockfd);
        return -1;
    }

    printf("Sent %zd bytes: %s\n", sent, json);

    close(sockfd);
    return 0;

}