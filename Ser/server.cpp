#include "tic.h"
#include "threadPool.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <exception>
#include <fstream>
#include <limits>

#include <stdlib.h>




#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char** argv)
{
    SelfPlay selfPlay;
    selfPlay.play();

    threadpool<int> pool;


    int connfd;
    // 创建监听 socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        // 处理错误
        perror("lfd create error");
        exit(-1);
    }
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定地址和端口
    struct sockaddr_in addr, cliaddr;
    socklen_t cliaddr_len;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9999);

    int ret = bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        // 处理错误
        perror("bind error");
        exit(-1);
    }

    // 设置 socket 为监听状态
    ret = listen(sockfd, 5);
    if (ret < 0) {
        // 处理错误
        perror("listen error");
        exit(-1);
    }

    printf("Accepting client connect ...\n");

    int i = 0;
    

    while (1) {
        cliaddr_len = sizeof(cliaddr);
        connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &cliaddr_len);   //阻塞监听客户端链接请求
        printf("The %dth client connected\n", i + 1);
        printf("-------------------------%d\n", connfd);
        pool.append_ser(connfd);
    }

    close(sockfd);    // 关闭主进程监听套接字


    
}