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
#include <unistd.h>

#include <stdio.h>
#include <inttypes.h>
// Included to get the support library
//#include <calcLib.h>

#include "protocol.h"

using namespace std;

int checkMsg(calcMessage *clcMsg)
{
  int ret = 0;
  if (ntohs(clcMsg->type) == 2 && ntohs(clcMsg->message) == 2 && ntohs(clcMsg->major_version) == 1 && ntohs(clcMsg->minor_version) == 0)
  {
    cout << "Abort";
    ret = -1;
  }
  return ret;
}

int main(int argc, char *argv[])
{
  int nrOfSent = 0;
  int bytesRecived = 0;
  //Check the initail input, whatever came before ":"
  //and whatever came after ":"
  if (argc != 2)
  {
    printf("Invalid input\n");
    exit(1);
  }
  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);
  if (Desthost == NULL || Destport == NULL)
  {
    cout << "Invalid input.\n";
    exit(1);
  }

  //int port = atoi(Destport);
  string ipAddr = Desthost;

  calcMessage msg;
  msg.type = htons(22);     //client-to-server, binary protocol
  msg.message = htonl(0);   //Not applicable/availible (N/A or NA)
  msg.protocol = htons(17); //UDP
  msg.major_version = htons(1);
  msg.minor_version = htons(0);

  struct addrinfo hint, *servinfo, *p;
  memset(&hint, 0, sizeof(hint));
  hint.ai_family = AF_INET;
  hint.ai_socktype = SOCK_DGRAM;

  int rv = getaddrinfo(Desthost, Destport, &hint, &servinfo);
  if (rv != 0)
  {
    fprintf(stderr, "get address info: %s\n", gai_strerror(rv));
    exit(1);
  }
  int sock = 0;
  for (p = servinfo; p != NULL; p = p->ai_next)
  {
    sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sock == -1)
    {
      continue;
    }
    else
    {
      freeaddrinfo(servinfo);

      struct timeval timeout;
      timeout.tv_sec = 2; //2 sec
      timeout.tv_usec = 0;

      if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
      {
        perror("setsockopt failed\n");
      }
      if (sendto(sock, &msg, sizeof(msg), 0, p->ai_addr, p->ai_addrlen) == -1)
      {
        cout << "Could not send message\n";
        close(sock);
        exit(1);
      }

      break;
    }
  }

  //calcProtocol msgRcv;
  calcProtocol msgRcv;

  while (nrOfSent < 3)
  {
    memset(&msgRcv, 0, sizeof(msgRcv));
    bytesRecived = recvfrom(sock, &msgRcv, sizeof(msgRcv), 0, p->ai_addr, &p->ai_addrlen);
    if (bytesRecived < 0)
    {

      cout << "Recive timeout, sending again.\n";
      nrOfSent++;
      if (sendto(sock, &msg, sizeof(msg), 0, p->ai_addr, p->ai_addrlen) == -1)
      {
        cout << "Could not send message\n";
        close(sock);
        exit(1);
      }
      if (nrOfSent < 3)
      {
        continue;
      }
      else
      {
        cout << "Could not send message\n";
        close(sock);
        exit(1);
      }
    }
    else
    {
      nrOfSent = 0;
      break;
    }
  }

  if (bytesRecived < (int)sizeof(msgRcv))
  {
    //Recived a calcMessage (omvandla till clacMessage)
    calcMessage *clcMsg = (calcMessage *)&msgRcv;
    if (checkMsg(clcMsg))
    {
      cout << "Got a NOT OK message.\nAbort!\n";
      close(sock);
      exit(1);
    }
  }
  //Recived a calcProtocol
  int i1 = ntohl(msgRcv.inValue1), i2 = ntohl(msgRcv.inValue2), iRes = ntohl(msgRcv.inResult);
  float f1 = msgRcv.flValue1, f2 = msgRcv.flValue2, fRes = msgRcv.flResult;
  cout<<"Task: ";
  string op = "";
  switch (ntohl(msgRcv.arith))
  {
  case 1: //add
    cout << "Add "<< i1 <<" "<<i2 << endl;
    iRes = i1 + i2;
    msgRcv.inResult = htonl(iRes);
    op = "i";
    break;
  case 2: //sub
    cout << "Sud "<< i1 << " "<<i2 << endl;
    iRes = i1 - i2;
    msgRcv.inResult = htonl(iRes);
    op = "i";
    break;
  case 3: //mul
    cout << "Mul "<< i1 << " "<<i2 << endl;
    iRes = i1 * i2;
    msgRcv.inResult = htonl(iRes);
    op = "i";
    break;
  case 4: //div
    cout << "Div "<< i1 << " "<<i2 << endl;
    iRes = i1 / i2;
    msgRcv.inResult = htonl(iRes);
    op = "i";
    break;
  case 5: //fadd
    cout << "Fadd "<< f1 << " "<<f2 << endl;
    fRes = f1 + f2;
    msgRcv.flResult = fRes;
    op = "f";
    break;
  case 6: //fsub
    cout << "Fsub "<< f1 << " "<<f2 << endl;
    fRes = f1 - f2;
    msgRcv.flResult = fRes;
    op = "f";
    break;
  case 7: //fmul
    cout << "Fmul "<< f1 << " "<<f2 << endl;
    fRes = f1 * f2;
    msgRcv.flResult = fRes;
    op = "f";
    break;
  case 8: //fdiv
    cout << "Fdiv "<< f1 << " "<<f2 << endl;
    fRes = f1 / f2;
    msgRcv.flResult = fRes;
    op = "f";
    break;
  default:
    cout << "Cant do that operation.\n";
    break;
  }

  if (sendto(sock, &msgRcv, sizeof(msgRcv), 0, p->ai_addr, p->ai_addrlen) == -1)
  {
    cout << "Could not send message\n";
    close(sock);
    exit(1);
  }
  if(op == "i"){
    cout << "Sent result: "<< ntohl(msgRcv.inResult)<<"\n";
  }
  else{
    cout << "Sent result: "<< msgRcv.flResult<<"\n";
  }
  

  calcMessage resp;

  while (nrOfSent < 3)
  {
    memset(&resp, 0, sizeof(resp));
    bytesRecived = recvfrom(sock, &resp, sizeof(resp), 0, p->ai_addr, &p->ai_addrlen);
    if (bytesRecived < 0)
    {

      cout << "Recive timeout, sending again.\n";
      nrOfSent++;
      if (sendto(sock, &msgRcv, sizeof(msgRcv), 0, p->ai_addr, p->ai_addrlen) == -1)
      {
        cout << "Could not send message\n";
        close(sock);
        exit(1);
      }
      if (nrOfSent < 3)
      {
        continue;
      }
      else
      {
        cout << "Could not send message\n";
        close(sock);
        exit(1);
      }
    }
    else
    {
      nrOfSent = 0;
      break;
    }
  }
  if (checkMsg(&resp) == -1)
  {
    cout << "Got a NOT OK message.\nAbort!\n";
    close(sock);
    exit(1);
  }

  switch (ntohl(resp.message))
  {
  case 0:
    cout << "Not availible\n";
    break;
  case 1:
    cout << "OK\n";
    break;
  case 2:
    cout << "NOT OK\n";
    break;
  }

  close(sock);
  return 0;
}
