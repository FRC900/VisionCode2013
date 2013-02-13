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
#include <opencv2/opencv.hpp>

//#define IPADDR "127.0.0.1"
//#define IPADDR "192.168.1.100"
//#define IPPORT 5000

#define IPADDR "10.9.0.40"
#define IPPORT 1130

using namespace std;
using namespace cv;

int main(){
	cout << SSIZE_MAX << endl;
  int sockfd, n=0;
  //char recvBuff[32000-1];
  //char recvBuff[921600];
  //int sz = 480*640*3;
  //char recvBuff[sz];

  char recvBuff[26];

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

  // Max readable is 1436
  int i = 0;
  while((n=read(sockfd, &recvBuff[i], sizeof(recvBuff)-1)) > 0){
    i+=n;
    //cout << "I read something! " << n;
    //recvBuff[n] = 0;
    //if(fputs(recvBuff, stdout) == EOF){
    //  cout << "fputs error!" << endl;
    //}
  }
  //recvBuff[921600]=0;
  cout << recvBuff << endl;
  cout << "Displaying: " << i << endl;

  //Mat cDisplay;
  //cDisplay = Mat(480, 640, CV_32FC3, (void*)recvBuff);
  //namedWindow("hello", CV_WINDOW_AUTOSIZE);
  //imshow("hello", cDisplay);

  if(n<0){
    cout << "Read error" << endl;
  }

  return 0;
}
