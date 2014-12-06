
#ifndef __TOOL_H
#define __TOOL_H

#include "module.h"

int openServerSocket(int srv_port, char* serverIp);
int sendData(int fd, int dataType, void* buf, int buflen);
int recvData(int fd, int *dataType, void* buf, int* buflen, int time);
moduleInfo* getModuleInfo();
int getServerIPbyDns( char* s);
char* getModuleIp( int id , char* ipaddr );
int checkId(int id);
//void client_print( client* p);

#endif __TOOL_H
