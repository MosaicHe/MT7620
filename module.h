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

#define DATASIZE    	512
#define SERVER_PORT 	8000
#define BROADCAST_PORT 	8001
#define LISTEN_PORT		8002
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
	char fwVersion[16];

	char state_24g;
	char ssid_24g[36];
	char mac_24g[18];
	int channel_24g;
	
	char state_5g;
	char ssid_5g[36];
	char mac_5g[18];
	int channel_5g;
}moduleInfo;

typedef struct{
	unsigned int dataType;
	unsigned int dataSize;
	char dataBuf[DATASIZE];
}UDPMessage;

typedef struct{
	char nvramDev[8];  // "rt2860" or "rtdev"
	char item[128];
	char value[128];
}moduleNvram;


#define REQ_REGISTER		1
#define REQ_FIRTWARE_UPDATE 2
#define REQ_CONFIG		    3
#define REQ_RUN 			4
#define REQ_REPORT			6

#define RESP_ERROR			-1			
#define RESP_SUCCESS		 0

#define STATE_IDLE 					0
#define STATE_HELLO					1
#define STATE_FIRMWARE_CHECKED 		2
#define STATE_CONFIG 				3
#define STATE_RUN 					4
#define STATE_DISCONNECTED			5

#define BROADCAST			100	
#define HEARTBEAT 			101
#define HEARTBEAT_ACK		102
#define COMMAND				103
#define COMMAND_SUCCEED		104
#define COMMAND_FAIL		105

#define SET_NVRAM			106
#define GET_NVRAM			107
#define SET_NVRAM_COMMIT	111
#define INIT_INTERNET		108
#define SET_STALIMIT		109
#define GET_MACLIST			110
#define GET_MACNUM			112
#define SYSTEM_CMD			113

int srv_ip;
static int srv_fd;

static char* g_serverip;
static char* g_lanip;
int g_moduleID;
char g_state;
moduleInfo g_moduleInfo;

#define DEBUG_PC

#define MAXCOUNTER 5
#define TIMEOUT 10

#endif  // __CLIENT_H

