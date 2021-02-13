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

template <typename T>
int sendMsg(T &msg, int &socket, sockaddr *addr)
{

  int nrOf = 0;
  while (nrOf < 3)
  {
    if (sendto(socket, &msg, sizeof(msg), 0, addr, sizeof(*addr)) < 0)
    {
      if (errno == EWOULDBLOCK)
      {
        fprintf(stderr, "socket timeout\n");
        nrOf++;
        continue;
      }
      else
      {
        cout << "sendto error.\n";
        nrOf++;
        continue;
      }
    }
    //success!
    return 0;
  }
  cout << "Could not send.\n";
  return -1;
}

int checkMsg(calcMessage &msg)
{
  if (ntohs(msg.type) == 2 &&
      ntohl(msg.message) == 2 &&
      ntohs(msg.major_version) == 1 &&
      ntohs(msg.minor_version) == 0)
  {
    return -1;
  }
  return 0;
}

int main(int argc, char *argv[])
{
  //Check the initail input, whatever came before ":"
  //and whatever came after ":"
  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);

  int port = atoi(Destport);
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
      timeout.tv_sec = 2000; //2 sec
      timeout.tv_usec = 2000;
      if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
      {
        perror("setsockopt failed\n");
      }

      if (sendMsg(msg, sock, p->ai_addr) == 1)
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
  calcMessage testM;
  calcProtocol testP;
  memset(&msgRcv, 0, sizeof(msgRcv));

  int bytesRecived = recvfrom(sock, &msgRcv, sizeof(msgRcv), 0, p->ai_addr, &p->ai_addrlen);
  if (bytesRecived == -1)
  {
    cout << "Error: recvfrom\n";
    close(sock);
    exit(1);
  }

  cout << "size of calcProtocol: " << sizeof(testP) << ", Sizeof calcMessage: " << sizeof(testM) << ", Bytes recived: " << bytesRecived << "\n";

  if (bytesRecived < sizeof(testP))
  {
    //Recived a calcMessage
    if (checkMsg(testM) == -1)
    {
      close(sock);
      exit(1);
    }
  }
  //Recived a calcProtocol
  int i1 = ntohl(msgRcv.inValue1), i2 = ntohl(msgRcv.inValue2), iRes = ntohl(msgRcv.inResult);
  float f1 = msgRcv.flValue1, f2 = msgRcv.flValue2, fRes = msgRcv.flResult;

  switch (ntohl(msgRcv.arith))
  {
  case 1: //add
    cout << "Add\n";
    iRes = i1 + i2;
    msgRcv.inResult = htonl(iRes);
    break;
  case 2: //sub
    cout << "Sud\n";
    iRes = i1 - i2;
    msgRcv.inResult = htonl(iRes);
    break;
  case 3: //mul
    cout << "Mul\n";
    iRes = i1 * i2;
    msgRcv.inResult = htonl(iRes);
    break;
  case 4: //div
    cout << "Div\n";
    iRes = i1 / i2;
    msgRcv.inResult = htonl(iRes);
    break;
  case 5: //fadd
    cout << "Fadd\n";
    fRes = f1 + f2;
    msgRcv.flResult = fRes;
    break;
  case 6: //fsub
    cout << "Fsub\n";
    fRes = f1 - f2;
    msgRcv.flResult = fRes;
    break;
  case 7: //fmul
    cout << "Fmul\n";
    fRes = f1 * f2;
    msgRcv.flResult = fRes;
    break;
  case 8: //fdiv
    cout << "Fdiv\n";
    fRes = f1 / f2;
    msgRcv.flResult = fRes;
    break;
  default:
    cout << "Cant do that operation.\n";
    break;
  }

  /*if (sendto(sock, &msgRcv, sizeof(msgRcv), 0, p->ai_addr, p->ai_addrlen) == -1)
  {
    cout << "closing socket.\n";
    close(sock);
    exit(1);
  }*/
  if (sendMsg(msgRcv, sock, p->ai_addr) == 1)
  {
    cout << "Could not send message\n";
    close(sock);
    exit(1);
  }
  cout << "Message sent.\n";

  calcMessage resp;

  bytesRecived = recvfrom(sock, &resp, sizeof(resp), 0, p->ai_addr, &p->ai_addrlen);
  if (bytesRecived == -1)
  {
    cout << "Error: recvfrom\n";
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

  //Te emot som void*

  //eller ta emot som calcprotocol
  //hur många bytes tog jag emot? calcprotocol är större bytes casta

  close(sock);
  return 0;
}
