
#ifndef __TOOL_H
#define __TOOL_H

#include "module.h"

int openServerSocket(int srv_port, char* serverIp);
int sendData(int fd, int dataType, void* buf, int buflen);
int recvData(int fd, msg* msgbuf, struct timeval* ptv);
moduleInfo* getModuleInfo();
int getServerIPbyDns( char* s);
char* getModuleIp( int id , char* ipaddr );
int checkId(int id);
int waitForServerBroadcast(struct sockaddr_in* p_addr);
//void client_print( client* p);
int recvFirmware( int fd);
void updateFirmware();

void setStaLimit( int num);
void sendMacList(int fd, char *ifname);

#endif //__TOOL_H
