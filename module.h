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

// send2server flag
#define RETURN 1


pthread_mutex_t mutex;

typedef struct{
	unsigned char moduleID;
	unsigned int dataType;
	unsigned int dataSize;
	char dataBuf[DATASIZE];
}msg;

typedef struct{
	char SN[12];
	char fwVersion;

	char state_24g;
	char ssid_24g[36];
	char mac_24g[18];
	int channel_24g;
	
	char state_5g;
	char ssid_5g[36];
	char mac_5g[18];
	int channel_5g;
}moduleInfo;

#if 0
typedef struct{
	char radioOnoff_24g;
	char wifiOnoff_24g;
	char ssid_24g[128];
	char mac_24g[18];
	int channel_24g;
	char radioOnoff_5g;
	char wifiOnoff_5g;
	char ssid_5g[128];
	char mac_5g[18];
	int channel_5g;
}setModule;


typedef struct{
	int 24g_stacliNum;
	char 24g_stacli[20][18];
	int 5g_stacliNum;
	char 5g_stacli[50][18];
}stacli;


typedef struct{
	long int 24g_statistics;
	long int 5g_statistics;
}statistics;
#endif


typedef struct{
	char nvramDev[6];  // "2860" or "rtdev"
	char item[128];
	char value[128];
}moduleNvram;


#define CMD_CLOSE 0
#define CMD_HEARTBEAT 1

#define RSP_RETURN   200
#define RQ_IP  201
#define RSP_IP 202
#define RSP_HEARTBEAT  203


int srv_ip;
int srv_fd;

static char* g_serverip;
static char* g_lanip;
static int g_moduleID;

#endif  // __CLIENT_H

