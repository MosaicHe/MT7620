#ifndef __CLIENT_H
#define __CLIENT_H

#include <sys/types.h>     
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/syscall.h>
#include <sys/select.h>
#include <sys/times.h>
#include <sys/types.h>
#include <fcntl.h> 
#include <assert.h> 

#define DEBUG

#ifdef DEBUG 
#define F_OUT	printf("%s(): ",__FUNCTION__);fflush(stdout);
#define deb_print(fmt, arg...)  F_OUT printf(fmt, ##arg);fflush(stdout)
#else
#define F_OUT	
#define deb_print(fmt, arg...)
#endif	//DEBUG

#define DATASIZE    256
#define PORT 8000
#define PORT2 8001
#define SRV_IP "192.168.1.130"
#define N 5

#define FILENAME "/etc/resolv.conf"

int ID = 0;

// send2server flag
#define RETURN 1


pthread_mutex_t mutex;

typedef struct{
	unsigned int cmd;
	unsigned char id;
	unsigned int bufsize;
	char buf[DATASIZE];
}msg;

typedef struct client_info{
	int id;
	int fd;
	char ssid[36];
	char mac[18];
	int channel;
	
	int have5g;
	char ssid_5g[36];
	char mac_5g[18];
	int channel_5g;
}client;

#define SERVERIP 101
#define INITIP	"192.168.111.1"

#define CMD_CLOSE 0
#define CMD_HEARTBEAT 1

#define RSP_RETURN   200
#define RQ_IP  201
#define RSP_IP 202
#define RSP_HEARTBEAT  203
#define REQUEST_SERVERIP 204

int srv_ip;
int srv_fd;

char* serverip =NULL;

static int openServerSocket(int srv_port, char* serverIp)
{
	int fd = -1;
	struct sockaddr_in peer_addr;
	int i = 3;
	int ct = 3;
	int ret = -1;
	
	peer_addr.sin_family = PF_INET;
	peer_addr.sin_port = htons(srv_port); 
	peer_addr.sin_addr.s_addr =  inet_addr(serverIp);  
	
	deb_print("before while loop\n");
	
	for(i=0; i<3; i++){
		deb_print("in while loop\n");
		fd = socket(PF_INET,SOCK_STREAM,0);
		if(fd < 0){
			deb_print("fd=%d\n",fd);
			perror("can't open socket\n");
			exit(1);
		}
		deb_print("i=%d\n", i);
		ret = connect( fd, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
		if(ret < 0){
			sleep(1);
			close(fd);
		}else
			return fd;
	}
	return fd;
}


static int sendData(int fd, int id, int dataType, void* buf, int buflen)
{
	int ret  = -1;
	msg *p_responseBuf;
	p_responseBuf = (msg*)malloc(sizeof(msg));
	bzero( p_responseBuf, sizeof(msg));
	
	p_responseBuf->id = id;
	p_responseBuf->cmd = dataType;
	if(buflen > 0){
		p_responseBuf->bufsize = buflen;
		memcpy( p_responseBuf->buf, buf, buflen);
	}
	deb_print("send data Type:%d,length:%d\n", dataType, buflen);
	ret = write(fd, p_responseBuf, sizeof(msg));
	if(ret< 0){
		perror("socket write error\n");
		free(p_responseBuf);
		return -1;
	}
	free(p_responseBuf);
	return 0;
}

static int recvData(int fd, int *dataType, void* buf, int* buflen, int time)
{
	int ret =-1;
	msg msgbuf;
	fd_set rdfds;
	struct timeval *p_tv;

	FD_ZERO(&rdfds);
	FD_SET(fd, &rdfds);

	if(time <= 0 ){
		p_tv = NULL;
	}else{
		p_tv = (struct timeval*)malloc(sizeof(struct timeval));
		p_tv->tv_sec = time;
		p_tv->tv_usec = 0;
	}
	ret = select(fd+1, &rdfds, NULL, NULL, p_tv);
	if(ret < 0){
		perror("select error!\n");
	}else if(ret == 0 ){
//		deb_print("select timeout\n");
	}else{
		deb_print("read data\n");
		ret = read(fd, &msgbuf, sizeof(msg));
		if(ret<0){
			perror("Socket read error\n");
			return -1;
		}
		*dataType = msgbuf.cmd;
		deb_print("recv data: %d\n",*dataType);
		*buflen = msgbuf.bufsize;
		memcpy(buf, msgbuf.buf, msgbuf.bufsize);
		return ret;
	}
	
	if(time > 0)
		free(p_tv);

	return ret;

}

void client_print( client* p)
{
	printf("      id: %d\n", p->id);
	printf("      fd: %d\n", p->fd);
	
	printf("    ssid: %s\n", p->ssid);
	printf("     mac: %s\n", p->mac);
	printf(" channel: %d\n", p->channel);
	
	if(p->have5g){
		printf("5G\n");
		printf("    ssid: %s\n", p->ssid_5g);
		printf("     mac: %s\n", p->mac_5g);
		printf(" channel: %d\n", p->channel_5g);
	}
}

#endif  // __CLIENT_H

