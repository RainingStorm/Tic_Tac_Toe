#include "tic.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <exception>
#include <fstream>
#include <limits>

#include <stdlib.h>


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 80
#define SERV_PORT 9999

// main

int main(int argc, char** argv)
{
  struct sockaddr_in servaddr;
	// char buf[MAXLINE];
	int sockfd;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr.s_addr);
	servaddr.sin_port = htons(SERV_PORT);

  // 设置登陆系统



	connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  
  // 功能选择界面
  T:
  system("clear");
  printf("*****欢迎来到TIC游戏室!*****\n");
  printf("游戏目录：\n");
  printf("1、AI对战\n");
  printf("2、联机对战\n");
  printf("0、退出游戏\n");
  std::cout << "Your choice: ";
  char buf[MAXLINE];
  if (fgets(buf, MAXLINE, stdin) == NULL) {
      return -1;
  }
  int choice = buf[0] - '0';

  write(sockfd, buf, strlen(buf));

  if (choice == 1) {
    system("clear");
    sleep(0.25);
    HumanPlay humanPlay;
    humanPlay.play(sockfd);
    goto T;
  } else if (choice == 2) {
    system("clear");
    sleep(0.25);
    HumanPlay humanPlay;
    humanPlay.p2pPlay(sockfd);
    goto T;
  } else if (choice == 0) {
    system("clear");
    std::cout << "你已退出游戏" << std::endl;
  } else {
    std::cout << "Invalid Move\n";
    sleep(1);
    // system("clear");
    goto T;
  }
}