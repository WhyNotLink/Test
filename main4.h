#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/ptrace.h>






void play_singlecircle_music(int sig);//单曲循环
void play_listcircle_music(int sig);//列表循环
void play_random_music(int sig);//随机播放
void last_play_music(int sig);//上一首歌
void next_play_music(int sig);//下一首歌 
void getname(void);//获取mp3文件名字
void playing_or_not(void);//测试是否正在播放歌曲
void test_connect(int sig);//测试连接

char song_buf[100];//存放将要播放歌曲的歌名
char cmd_buf[100];//播放暂停命令缓冲区

char *song[100];//指向歌名的指针数组
int all=0;//歌曲总数ed_

int played_name[100];//存放播放过的歌曲名字，最多保存100首
int now=0;//当前歌曲
int n;//存放name[]中的数量

int playing_flag=0;//是否正在播放，0为未播放，1为正在播放
int play_mode=0;//默认列表播放，0为列表播放，1为随机，2为单曲循环



/*
struct dirent
{
   long d_ino; // inode number 索引节点号 
   off_t d_off; // offset to this dirent 在目录文件中的偏移 
   unsigned short d_reclen; // length of this d_name 文件名长
   unsigned char d_type; // the type of d_name 文件类型 
   char d_name [NAME_MAX+1]; //file name (null-terminated) 文件名，最长255字符 
}
*/

//获取歌单
void getname(void)
{
	int name_len;
	static char *buf;
	DIR *dir;
	struct dirent *ptr;//两个结构体指针
	dir =opendir(".");
	while((ptr = readdir(dir))!=NULL)
	{
		name_len = strlen(ptr->d_name);
		name_len -= 4;

		if(strcmp(ptr->d_name+name_len,".mp3")==0)
		{
			buf = (char *)malloc(100); 
			memset(buf,0,sizeof(buf));
			strcat(buf,ptr->d_name);
			song[all++]=buf;
		}
	}
	for(int i=0 ; i<all ; i++)
	{
		printf("%d,%s\n",i+1,song[i]);
	}
	closedir(dir);
	memset(played_name, -1, sizeof(played_name));
	if(all>0)
	{
		now = 0; 
		n = 1;  
		played_name[0] = now; 
		playing_flag = 1; 
		sprintf(song_buf,"madplay -o wav:- /root/System_project/mp3/ %s 2> /dev/null | aplay 2>/dev/null &",song[now]);
		system(song_buf);
	}

}

//测试当前是否正在播放
void playing_or_not(void)
{
	FILE *fp;
	char *buf = (char *)malloc(10);
	memset(buf,'\0',sizeof(buf));
	fp = popen("ps -ef | grep -w madplay |grep -v grep |wc -l ","r");
	fread(buf,1,1,fp);
	pclose(fp);
	playing_flag = (int)buf[0]-48;//0代表未播放
	free(buf);
	buf = NULL;
}

//保存当前播放歌曲名称
void savename(void)
{
	// printf("savename: n=%d, now=%d, played_name[n]=%d\n", n, now, played_name[n]);

	played_name[n] = now; 
	n++;
	if (n >= 100) 
	{
		n = 0;
	}

}

void test_connect(int sig)
{
	printf("LINK!");
}

void play_singlecircle_music(int sig)
{
	play_mode=2;
	
	playing_or_not();
	if(playing_flag)
	{
		sleep(3);
		playing_or_not();
	}
	if (!playing_flag)
	{
		sprintf(song_buf,"madplay -o wav:- /root/System_project/mp3/ %s 2> /dev/null | aplay 2>/dev/null &",song[now]);
		system(song_buf);
	}
 	if (play_mode == 2) 
	{
		raise(SIGUSR2);
	}
            
}

void play_listcircle_music(int sig)
{
	play_mode=0;

	playing_or_not();
	if(playing_flag)
	{
		sleep(3);
		playing_or_not();
	}
	if(!playing_flag)
	{
		savename();
		now++;	
		if(now>all-1)
		{
			now=0;
		}
		sprintf(song_buf,"madplay -o wav:- /root/System_project/mp3/ %s 2> /dev/null | aplay 2>/dev/null &",song[now]); 
		system(song_buf); 
	}

	if (play_mode == 0)
	{
		raise(SIGRTMIN+0); 
	}

}

void play_random_music(int sig)
{
	play_mode=1;

	playing_or_not();
	if(playing_flag)
	{
		sleep(3);
		playing_or_not();
	}
	if(!playing_flag)
	{
		savename();
		srand((unsigned int)time(NULL));
		now = rand() % (all);
		sprintf(song_buf,"madplay -o wav:- /root/System_project/mp3/ %s 2> /dev/null | aplay 2>/dev/null &",song[now]); 
		system(song_buf);
	}
 	if (play_mode == 1) 
	{
		raise(SIGRTMIN+1);
	}
}


void last_play_music(int sig)
{
	savename();
	playing_or_not();
	if(playing_flag)
	{
		sprintf(song_buf,"killall -9 madplay aplay");
		system(song_buf);
	}

	int has_prev_history = 0;
	int prev_idx = n - 2;
	if (prev_idx < 0)
	{
		prev_idx = 99 + prev_idx + 1; 
	}
	if (played_name[prev_idx] >= 0 && played_name[prev_idx] < all)
	{
		now = played_name[prev_idx];
		has_prev_history = 1;
	}
	if(!has_prev_history)
	{
		now--;
		if(now < 0) 
		{
			now = all - 1;
		}
	}

	
	sprintf(song_buf,"madplay -o wav:- /root/System_project/mp3/ %s 2> /dev/null | aplay 2>/dev/null &",song[now]); 
	system(song_buf);
	sleep(1);
}

void next_play_music(int sig)
{	printf("test");
	savename();
	playing_or_not();
	if(playing_flag)
	{
		sprintf(song_buf,"killall -9 madplay aplay");
		system(song_buf);
	}
	
	int has_next_history = 0;
	int next_idx = n;
	if (next_idx >= 100)
	{
		next_idx = 0;
	}
	if (played_name[next_idx] != -1 && played_name[next_idx] < all) 
	{
		now = played_name[next_idx];
		has_next_history = 1;
	}
	if(!has_next_history)
	{
		now++;
		if(now >= all) 
		{
			now = 0;
		}
	}


	
	sprintf(song_buf,"madplay -o wav:- /root/System_project/mp3/ %s 2> /dev/null | aplay 2>/dev/null &",song[now]); 
	system(song_buf);
	sleep(1);                   
}



// #ifndef MUSIC_PLAYER_H
// #define MUSIC_PLAYER_H

// #include<stdio.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include<unistd.h>
// #include <signal.h>
// #include <string.h>
// #include <stdlib.h>
// #include <dirent.h>
// #include <sys/wait.h>
// #include <time.h>
// #include <sys/ptrace.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <pthread.h>
// #include <errno.h>

// // UDP 配置参数
// #define UDP_PORT 8000  // UDP接收端口
// #define BUF_SIZE 32    // UDP接收缓冲区大小
// extern pid_t child_pid; // 保存子进程PID

// // 函数声明
// void play_singlecircle_music(int sig);//单曲循环
// void play_listcircle_music(int sig);//列表循环
// void play_random_music(int sig);//随机播放
// void last_play_music(int sig);//上一首歌
// void next_play_music(int sig);//下一首歌 
// void getname(void);//获取mp3文件名字
// void playing_or_not(void);//测试是否正在播放歌曲
// void test_connect(int sig);//测试连接
// void savename(void);//保存播放历史

// // 全局变量声明
// extern char song_buf[100];//存放将要播放歌曲的歌名
// extern char cmd_buf[100];//播放暂停命令缓冲区
// extern char *song[100];//指向歌名的指针数组
// extern int all;//歌曲总数
// extern int played_name[100];//存放播放过的歌曲索引
// extern int now;//当前播放歌曲索引
// extern int n;//播放历史计数
// extern int playing_flag;//是否正在播放（0=未播放，1=正在播放）
// extern int play_mode;//播放模式（0=列表，1=随机，2=单曲循环）

// // 全局变量定义
// char song_buf[100];
// char cmd_buf[100];
// char *song[100];
// int all=0;
// int played_name[100];
// int now=0;
// int n=0;
// int playing_flag=0;
// int play_mode=0;

// // 保存当前播放歌曲索引
// void savename(void)
// {
//     printf("【测试】savename 执行前：n=%d，当前歌曲索引now=%d，played_name[%d] 即将赋值\n", n, now, n);
// 	played_name[n] = now; 
// 	n++;
// 	if (n >= 100) 
// 	{
// 		n = 0;
//         printf("【测试】播放历史已满100条，n重置为0\n");
// 	}
//     printf("【测试】savename 执行后：n=%d，played_name[%d]=%d（已保存当前歌曲索引）\n", n, n-1, now);
// }

// // 测试连接
// void test_connect(int sig)
// {
// 	printf("【测试】收到连接测试信号 sig=%d，LINK!\n", sig);
// }

// // 获取歌单
// void getname(void)
// {
// 	int name_len;
// 	static char *buf;
// 	DIR *dir;
// 	struct dirent *ptr;
//     printf("【测试】开始遍历当前目录，查找.mp3文件...\n");
// 	dir = opendir(".");
// 	if (dir == NULL) {
// 		perror("【测试】opendir 失败");
// 		return;
// 	}
// 	while((ptr = readdir(dir))!=NULL)
// 	{
// 		name_len = strlen(ptr->d_name);
// 		if (name_len < 4) {
// 			continue;
// 		}
// 		name_len -= 4;

// 		if(strcmp(ptr->d_name+name_len,".mp3")==0)
// 		{
// 			buf = (char *)malloc(100); 
// 			memset(buf,0,sizeof(buf));
// 			strcat(buf,ptr->d_name);
// 			song[all]=buf;
//             printf("【测试】找到MP3文件：第%d首 -> %s\n", all+1, song[all]);
// 			all++;
// 		}
// 	}
//     printf("【测试】遍历结束，共找到 %d 首MP3文件\n", all);
// 	for(int i=0 ; i<all ; i++)
// 	{
// 		printf("%d,%s\n",i+1,song[i]);
// 	}
// 	closedir(dir);
// 	memset(played_name, -1, sizeof(played_name));
// 	if(all>0)
// 	{
// 		now = 0; 
// 		n = 1;  
// 		played_name[0] = now; 
// 		playing_flag = 1; 
//         printf("【测试】默认播放第1首：%s（索引now=%d）\n", song[now], now);
// 		sprintf(song_buf,"madplay -o wav:- /root/System_project/mp3/ %s 2> /dev/null | aplay 2>/dev/null &",song[now]);
//         printf("【测试】播放命令：%s\n", song_buf);
// 		system(song_buf);
// 	} else {
//         printf("【测试】未找到任何MP3文件，不启动默认播放\n");
//     }
// }

// // 测试当前是否正在播放
// void playing_or_not(void)
// {
// 	FILE *fp;
// 	char *buf = (char *)malloc(10);
// 	memset(buf,'\0',sizeof(buf));
//     printf("【测试】正在检测播放状态：执行 ps -ef | grep -w madplay | grep -v grep | wc -l\n");
// 	fp = popen("ps -ef | grep -w madplay |grep -v grep |wc -l ","r");
// 	if (fp == NULL) {
// 		perror("【测试】popen 失败");
// 		playing_flag = 0;
// 		free(buf);
// 		return;
// 	}
// 	fread(buf,1,1,fp);
// 	pclose(fp);
// 	playing_flag = (int)buf[0]-48;
//     printf("【测试】播放状态检测结果：buf=[%c]，playing_flag=%d（0=未播放，1=正在播放）\n", buf[0], playing_flag);
// 	free(buf);
// 	buf = NULL;
// }

// // 单曲循环（无死循环，非阻塞）
// void play_singlecircle_music(int sig)
// {
//     printf("【测试】进入单曲循环模式，sig=%d，play_mode设置为2\n", sig);
// 	play_mode=2;

//     playing_or_not();
//     if (playing_flag)
//     {
//         printf("【测试】单曲循环中：当前正在播放，休眠3秒后再次检测\n");
//         sleep(3);
//         playing_or_not();
//     }
//     if (!playing_flag)
//     {
//         printf("【测试】单曲循环中：当前歌曲播放完毕，准备重新播放第%d首：%s\n", now+1, song[now]);
//         sprintf(song_buf,"madplay -o wav:- /root/System_project/mp3/ %s 2> /dev/null | aplay 2>/dev/null &",song[now]);
//         printf("【测试】单曲循环播放命令：%s\n", song_buf);
//         system(song_buf);

//         if (play_mode == 2)
//             raise(SIGUSR2);
//     }
// }

// // 列表循环（无死循环，非阻塞）
// void play_listcircle_music(int sig)
// {
//     printf("【测试】进入列表循环模式，sig=%d，play_mode设置为0\n", sig);
// 	play_mode=0;

//     playing_or_not();
//     if (playing_flag)
//     {
//         printf("【测试】列表循环中：当前正在播放，休眠3秒后再次检测\n");
//         sleep(3);
//         playing_or_not();
//     }
//     if (!playing_flag)
//     {
//         savename();
//         now++;	
//         printf("【测试】列表循环：当前歌曲播放完毕，now自增为%d\n", now);
//         if(now>all-1)
//         {
//             now=0;
//             printf("【测试】列表循环：now超过最大索引%d，重置为0\n", all-1);
//         }
//         printf("【测试】列表循环：准备播放第%d首：%s（索引now=%d）\n", now+1, song[now], now);
//         sprintf(song_buf,"madplay -o wav:- /root/System_project/mp3/ %s 2> /dev/null | aplay 2>/dev/null &",song[now]); 
//         printf("【测试】列表循环播放命令：%s\n", song_buf);
//         system(song_buf); 

//         if (play_mode == 0)
//             raise(SIGRTMIN+0);
//     }
// }

// // 随机播放（无死循环，非阻塞）
// void play_random_music(int sig)
// {
//     printf("【测试】进入随机播放模式，sig=%d，play_mode设置为1\n", sig);
// 	play_mode=1;

//     playing_or_not();
//     if (playing_flag)
//     {
//         printf("【测试】随机播放中：当前正在播放，休眠3秒后再次检测\n");
//         sleep(3);
//         playing_or_not();
//     }
//     if (!playing_flag)
//     {
//         savename();
//         srand((unsigned int)time(NULL));
//         now = rand() % (all);
//         printf("【测试】随机播放：生成随机索引now=%d，对应歌曲%s\n", now, song[now]);
//         sprintf(song_buf,"madplay -o wav:- /root/System_project/mp3/ %s 2> /dev/null | aplay 2>/dev/null &",song[now]); 
//         printf("【测试】随机播放命令：%s\n", song_buf);
//         system(song_buf);

//         if (play_mode == 1)
//             raise(SIGRTMIN+1);
//     }
// }

// // 上一首歌（移除阻塞性raise逻辑）
// void last_play_music(int sig)
// {
//     printf("【测试】收到上一首歌信号 sig=%d，开始执行上一首逻辑\n", sig);
// 	savename();
// 	playing_or_not();
// 	if(playing_flag)
// 	{
//         printf("【测试】当前正在播放，执行强制终止命令：killall -9 madplay aplay\n");
// 		sprintf(song_buf,"killall -9 madplay aplay");
// 		system(song_buf);
// 	} else {
//         printf("【测试】当前未播放，无需终止进程\n");
//     }

// 	int has_prev_history = 0;
// 	int prev_idx = n - 2;
// 	if (prev_idx < 0)
// 	{
// 		prev_idx = 99 + prev_idx + 1; 
//         printf("【测试】上一首历史索引prev_idx<0，重置为%d\n", prev_idx);
// 	}
//     printf("【测试】尝试从历史中获取上一首：prev_idx=%d，played_name[%d]=%d\n", prev_idx, prev_idx, played_name[prev_idx]);
// 	if (played_name[prev_idx] >= 0 && played_name[prev_idx] < all)
// 	{
// 		now = played_name[prev_idx];
// 		has_prev_history = 1;
//         printf("【测试】找到上一首历史记录，now设置为%d（对应歌曲%s）\n", now, song[now]);
// 	}
// 	if(!has_prev_history)
// 	{
// 		now--;
//         printf("【测试】无历史上一首记录，now自减为%d\n", now);
// 		if(now < 0) 
// 		{
// 			now = all - 1;
//             printf("【测试】now<0，重置为最大索引%d（对应最后一首歌曲）\n", now);
// 		}
//         printf("【测试】上一首歌曲索引now=%d，对应歌曲%s\n", now, song[now]);
// 	}

//     printf("【测试】准备播放上一首：第%d首 -> %s\n", now+1, song[now]);
// 	sprintf(song_buf,"madplay -o wav:- /root/System_project/mp3/ %s 2> /dev/null | aplay 2>/dev/null &",song[now]); 
//     printf("【测试】上一首播放命令：%s\n", song_buf);
// 	system(song_buf);
// 	sleep(1);
// }

// // 下一首歌（移除阻塞性raise逻辑，修复多次切歌无响应）
// void next_play_music(int sig)
// {
//     printf("【测试】收到下一首歌信号 sig=%d，开始执行下一首逻辑\n", sig);
// 	printf("test");
// 	savename();
// 	playing_or_not();
// 	if(playing_flag)
// 	{
//         printf("【测试】当前正在播放，执行强制终止命令：killall -9 madplay aplay\n");
// 		sprintf(song_buf,"killall -9 madplay aplay");
// 		system(song_buf);
// 	} else {
//         printf("【测试】当前未播放，无需终止进程\n");
//     }
	
// 	int has_next_history = 0;
// 	int next_idx = n;
// 	if (next_idx >= 100)
// 	{
// 		next_idx = 0;
//         printf("【测试】下一首历史索引next_idx>=100，重置为0\n");
// 	}
//     printf("【测试】尝试从历史中获取下一首：next_idx=%d，played_name[%d]=%d\n", next_idx, next_idx, played_name[next_idx]);
// 	if (played_name[next_idx] != -1 && played_name[next_idx] < all) 
// 	{
// 		now = played_name[next_idx];
// 		has_next_history = 1;
//         printf("【测试】找到下一首历史记录，now设置为%d（对应歌曲%s）\n", now, song[now]);
// 	}
// 	if(!has_next_history)
// 	{
// 		now++;
//         printf("【测试】无历史下一首记录，now自增为%d\n", now);
// 		if(now >= all) 
// 		{
// 			now = 0;
//             printf("【测试】now>=%d（歌曲总数），重置为0\n", all);
// 		}
//         printf("【测试】下一首歌曲索引now=%d，对应歌曲%s\n", now, song[now]);
// 	}

//     printf("【测试】准备播放下一首：第%d首 -> %s\n", now+1, song[now]);
// 	sprintf(song_buf,"madplay -o wav:- /root/System_project/mp3/ %s 2> /dev/null | aplay 2>/dev/null &",song[now]); 
//     printf("【测试】下一首播放命令：%s\n", song_buf);
// 	system(song_buf);
// 	sleep(1);
// }


// #endif // MUSIC_PLAYER_H