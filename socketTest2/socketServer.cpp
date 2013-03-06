#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <iostream>

// Code from: www.thegeekstuff.com/2011/12/c-socket-programming
#define IPPORT 5000

int main(){
  int connfd = 0;
  struct sockaddr_in serv_addr;
  char sendBuff[1025];
  time_t ticks;

  int listenfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&serv_addr, '0', sizeof(serv_addr));
  memset(sendBuff, '0', sizeof(sendBuff));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(IPPORT);
  
  bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  
  listen(listenfd, 10);
  
  while(1){
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

    ticks = time(NULL);
    snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
    //write(connfd, sendBuff, strlen(sendBuff));
    write(connfd, "A", 1);
    close(connfd);
    sleep(1);
  }
}


