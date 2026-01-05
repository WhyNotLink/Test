## 项目简介

基于Linux操作系统的一个简易音乐播放器。能够在终端控制也可使用web进行可视化控制

Web通过tcp与本地js连接，本地js再通过udp向linux传输数据

## 软件环境与依赖

### Ubuntu 20.04 LTS

### SSH

### VSCODE

### NODE

## 项目结构

项目根目录 

├── udp-proxy-serve/          # TCP_UDP代理服务的子目录（存储代理相关子模块、配置） 

├── 28f2870bd8ae1155cb092c6f1f71f98b...  # 项目配套图片资源（JPG格式） 

├── app.js                    # 前端交互逻辑脚本（负责页面与服务的交互） 

├── index.html                # 项目前端展示页面（用户操作的界面） 

├── main.css                  # 前端页面样式文件（美化index.html的布局/样式） 

├── main4.h                   #播放器所需库函数

├── mainudp.c                 # 播放器主函数 

└── README.md                 # 项目说明文档（记录项目介绍、使用方法）