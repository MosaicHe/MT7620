
#ifndef __TOOL_H
#define __TOOL_H

int openServerSocket(int srv_port, char* serverIp);
int sendData(int fd, int id, int dataType, void* buf, int buflen);
int recvData(int fd, int *dataType, void* buf, int* buflen, int time);
//void client_print( client* p);

#endif __TOOL_H
