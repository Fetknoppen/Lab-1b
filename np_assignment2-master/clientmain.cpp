#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <iostream>
#include<unistd.h>

#include <stdio.h>
#include <inttypes.h>
// Included to get the support library
//#include <calcLib.h>


#include "protocol.h"

#define AS_SECONDS 1000000

using namespace  std;

int sendMsg(calcMessage &msg, int &socket, sockaddr_in &addr){
  
  int nrOf = 0;
  while(nrOf < 3){
    int sendRes = sendto(socket, &msg, sizeof(msg), 0, (sockaddr*)&addr, sizeof(addr));
    
    if(sendRes == -1){
      nrOf ++;
      usleep(2*AS_SECONDS);
      if(nrOf < 3){
        cout<<"Could not send trying again...\n";
      }
      continue;
    }
    return 0;
  }
  return -1;
}
int sendProt(calcProtocol &msg, int &socket, sockaddr_in &addr){
  
  int nrOf = 0;
  while(nrOf < 3){
    int sendRes = sendto(socket, &msg, sizeof(msg), 0, (sockaddr*)&addr, sizeof(addr));
    
    if(sendRes == -1){
      nrOf ++;
      usleep(2*AS_SECONDS);
      if(nrOf < 3){
        cout<<"Could not send trying again...\n";
      }
      continue;
    }
    return 0;
  }
  return -1;
}

int checkMsg(calcMessage &msg){
  if(ntohs(msg.type) == 2 &&
     ntohl(msg.message)== 2 &&
     ntohs(msg.major_version) == 1 &&
     ntohs(msg.minor_version) == 0){
       return -1;
  }
  return 0;
}

int main(int argc, char *argv[]){
  
  //Check the initail input, whatever came before ":"
  //and whatever came after ":"
  char delim[]=":";
  char *Desthost=strtok(argv[1],delim);
  char *Destport=strtok(NULL,delim);

  int port = atoi(Destport);
  string ipAddr = Desthost;

  calcMessage msg;
  msg.type = 22; //client-to-server, binary protocol
  msg.message = 0; //Not applicable/availible (N/A or NA)
  msg.protocol = 17; //UDP
  msg.major_version = 1;
  msg.minor_version = 0;


  //make socket
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock == -1){
    cerr<<"Could not make socket\n";
    return -1;
  }
  

  struct sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(port);
  inet_pton(AF_INET, ipAddr.c_str(), &hint.sin_addr);

  if(sendMsg(msg, sock, hint) == -1){
    cout<<"closing socket.\n";
    close(sock);
    exit(1);
  }

  calcProtocol msgRcv;

  int bytesRecived = recvfrom(sock, &msgRcv, sizeof(msgRcv), 0, (sockaddr*)&hint, (socklen_t*)&hint);
  if(bytesRecived < sizeof(msgRcv)){
    //Got a calcMessage
    calcMessage *clcMsg = (calcMessage*)&msgRcv;
    if(checkMsg(*clcMsg)==-1){
      cout<<"Server sent a NOT OK message.\nClosing socket!\n";
      close(sock);
      exit(1);
    }
  }
  else{
    //got a calcProtocol
    //cout<<ntohl(msgRcv.arith)<<endl;
    printf("%"PRIu32"\n",ntohl(msgRcv.arith));
    switch (ntohl(msgRcv.arith))
    {
    case 1://add
      break;
    case 2://sub
      break;
    case 3://mul
      break;
    case 4://div
      break;
    case 5://fadd
      break;
    case 6://fsub
      break;
    case 7://fmul
      break;
    case 8://fdiv
      break;
    default:
          cout<<"Cant do that operation.\n";
      break;
    }
  }

  /*if(sendProt(msgRcv, sock, hint)==-1){
    cout<<"closing socket.\n";
    close(sock);
    exit(1);
  }
  char buf[1000];
  recvfrom(sock, buf, sizeof(buf), 0, (sockaddr*)&hint, (socklen_t*)&hint);
  printf("%s\n", buf);*/
  


  //Te emot som void*

  //eller ta emot som calcprotocol
  //hur många bytes tog jag emot? calcprotocol är större bytes casta
  


  close(sock);
  return 0;

}
