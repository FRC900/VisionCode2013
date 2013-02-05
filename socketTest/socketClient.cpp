#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <iostream>

#define IPADDR "127.0.0.1"
//#define IPADDR "192.168.1.100"
#define IPPORT 5000

using namespace std;

int main(){
  int sockfd, n=0;
  char recvBuff[2048];

  struct sockaddr_in serv_addr;

  memset(recvBuff, '0', sizeof(recvBuff));
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    cout << "Fail socket initialization" << endl;
    return 1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(IPPORT);
  
  if(inet_pton(AF_INET, IPADDR, &serv_addr.sin_addr) <= 0){
    cout << "inet issue!" << endl;
    return 1;
  }

  if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
    cout << "Connection failed!" << endl;
    return 1;
  }

  while((n=read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0){
    recvBuff[n] = 0;
    if(fputs(recvBuff, stdout) == EOF){
      cout << "fputs error!" << endl;
    }
  }

  if(n<0){
    cout << "Read error" << endl;
  }

  return 0;
}
