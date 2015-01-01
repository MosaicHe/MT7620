#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<sys/ioctl.h>
#include	<net/if.h>
#include	<net/route.h>
#include    <dirent.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<fcntl.h>
#include 	"nvram.h"
#include 	"command.h"
#include 	"tool.h"
#include	"pingthread.h"
#include	"wireless.h"

#define MAX(a,b) (a>b?a:b)

struct sockaddr_in server_addr;

/*
 * register module to server 
 */
int register2Server()
{
	int ret = -1;
	msg msgbuf;
	int counter=0;
	int srv_fd;
	struct timeval tv;
	tv.tv_sec=1;
	tv.tv_usec=0;

	srv_fd = socket(PF_INET,SOCK_STREAM,0);
	if(srv_fd < 0){
		perror("can't open socket\n");
		exit(1);
	}
	server_addr.sin_port = htons(SERVER_PORT);
	
	ret = connect(srv_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));
	if(ret < 0){
		printf("can not connect to server\n");
		close(srv_fd);
		return -1;
	}
	printf("connect to server\n");

	/*****************************/
	/******** REQ_REGISTER *******/
	/*****************************/
	printModuleInfo();
	ret = sendData(srv_fd, REQ_REGISTER, &g_moduleInfo, sizeof(moduleInfo));
	if(ret < 0){
		/* FIXME */
		printf("%s:sendData error",__FUNCTION__);
		return -1;
	}
	ret = recvData(srv_fd, &msgbuf, &tv);
	if(ret<0 || ret==0 || msgbuf.dataType != RESP_SUCCESS){
		return -1;
	}

	/***********************************/
	/******* REQ_FIRTWARE_UPDATE *******/
	/***********************************/
#ifndef DEBUG_PC
	ret = sendData(srv_fd, REQ_FIRTWARE_UPDATE, g_moduleInfo.fwVersion, 
					sizeof(g_moduleInfo.fwVersion));
#else

	ret = sendData(srv_fd, REQ_FIRTWARE_UPDATE, NULL, 0);
#endif
	if(ret < 0){
		/* FIXME */
		printf("%s:sendData error\n",__FUNCTION__);
		return -1;
	}
	tv.tv_sec=1;
	tv.tv_usec=0;
	ret = recvData(srv_fd, &msgbuf, &tv);
	if(ret<0 || ret==0  || msgbuf.dataType != REQ_FIRTWARE_UPDATE){
		printf("REQ_FIRTWARE_UPDATE response error\n");
		return -1;
	}
	if( *(int*)(msgbuf.dataBuf) == 1 ){
		recvFirmware(srv_fd);
	    //this function will reboot the module!!				
		updateFirmware();
		exit(0);
	}

	/**********************/
	/***** REQ_CONFIG *****/
	/**********************/
	ret = sendData(srv_fd, REQ_CONFIG, NULL, 0);
	if(ret < 0){
		/* FIXME */
		printf("%s:sendData error",__FUNCTION__);
		return -1;
	}
	tv.tv_sec=1;
	tv.tv_usec=0;
	ret = recvData(srv_fd, &msgbuf, &tv);
	if(ret<0 ||ret==0 || msgbuf.dataType != REQ_CONFIG){
		printf("REQ_FIRTWARE_UPDATE response error\n");
		return -1;
	}
	if( msgbuf.dataSize!=0 ){
		//doConfiguration(buf, buflen);
		return -1;
	}

	/***********************/
	/******* REQ_RUN ******/
	/**********************/
	ret = sendData(srv_fd, REQ_RUN, NULL, 0);
	if(ret < 0){
		/* FIXME */
		printf("%s:sendData error",__FUNCTION__);
		return -1;
	}
	tv.tv_sec=1;
	tv.tv_usec=0;
	ret = recvData(srv_fd, &msgbuf, &tv);
	if(ret<0 ||ret==0 || msgbuf.dataType != RESP_SUCCESS){
		printf(" REQ_RUN error\n");
		return -1;
	}

	close(srv_fd);
	srv_fd = -1;
	return 0;

}

int executeCommad(int fd)
{
	int ret =-1;
	msg *pmsg;
	pmsg = (msg*)malloc(sizeof(msg));
	moduleNvram mNvram;
	char nvramStr[1024];
	char ifname[16];
	const char* valueStr;
	int ndev;
	int num;
	struct timeval tv;
	tv.tv_sec =1;
	tv.tv_usec=0;
	ret = recvData(fd, pmsg, &tv);
	if(ret!=sizeof(msg)){
		printf("read data error\n");
		close(fd);
		return -1;			
	}
	switch(pmsg->dataType){
		case SET_NVRAM:
			deb_print("SET_NVRAM\n");
			memcpy(&mNvram, pmsg->dataBuf, sizeof(moduleNvram));
			if(!strcmp(mNvram.nvramDev, "rtdev"))
				ndev = RTDEV_NVRAM;
			else
				ndev = RT2860_NVRAM;
			nvram_bufset(ndev, mNvram.item, mNvram.value);
			
			strcpy(pmsg->dataBuf,"Success");
			pmsg->dataSize = strlen(pmsg->dataBuf)+1;
			write(fd, pmsg, sizeof(msg));
			break;

		case SET_NVRAM_COMMIT:
			deb_print("SET_NVRAM_COMMIT\n");
			memcpy(&mNvram, pmsg->dataBuf, sizeof(moduleNvram));
			if(!strcmp(mNvram.nvramDev, "rtdev"))
				ndev = RTDEV_NVRAM;
			else
				ndev = RT2860_NVRAM;
			nvram_commit(ndev);
			strcpy(pmsg->dataBuf,"Success");
			pmsg->dataSize = strlen(pmsg->dataBuf)+1;
			write(fd, pmsg, sizeof(msg));			
			break;

		case GET_NVRAM:
			deb_print("GET_NVRAM\n");
			memcpy(&mNvram, pmsg->dataBuf, sizeof(moduleNvram));
			if(!strcmp(mNvram.nvramDev, "rtdev"))
				ndev = RTDEV_NVRAM;
			else
				ndev = RT2860_NVRAM;
			valueStr = nvram_bufget( ndev, mNvram.item);
			if(valueStr==NULL)
				strcpy(pmsg->dataBuf,"NULL");
			else
				strcpy(pmsg->dataBuf, valueStr);
			pmsg->dataSize = strlen(pmsg->dataBuf)+1;
			deb_print("return: %s\n",pmsg->dataBuf);
			write(fd, pmsg, sizeof(msg));
			break;

		case INIT_INTERNET:
			initInternet();
			break;

		case GET_MACLIST:
			memcpy(ifname, pmsg->dataBuf, pmsg->dataSize);
			sendMacList(fd, ifname);
			break;

		case GET_MACNUM:
			memcpy(ifname, pmsg->dataBuf, pmsg->dataSize);
			num = getOnlineMacNum(ifname);
			memcpy(pmsg->dataBuf, num, sizeof(num));
			pmsg->dataSize=sizeof(num);
			write(fd, pmsg, sizeof(msg));			
			break;

		case SET_STALIMIT:
			setStaLimit( *(int*)(pmsg->dataBuf));
			break;
		default:
			break;
	}

	return 0;
}

int waitForServerCommand()
{
	int listenFd=-1;
	int connectFd=-1;
	int count=0;
	int ret=-1;
	fd_set readfd;
	struct timeval tv;
	
	listenFd = openListenTcpSocket();
	if(listenFd<0){
		printf("openListenTcpSocket error\n");
		exit(-1);
	}
	while( 1 ){
		FD_ZERO(&readfd);
		FD_SET(listenFd, &readfd);
		tv.tv_sec = 10;
		tv.tv_usec = 0;
		ret = select(listenFd + 1, &readfd, NULL, NULL, &tv); 
		if(ret<0){
			perror("select error\n");
			close(listenFd);
			return -1;
		}else if(ret==0){
			if(g_state==STATE_DISCONNECTED)
				return 0;	
		}
		else{
			 connectFd = accept(listenFd, NULL, NULL);
			if(connectFd<0){
				perror("accept error\n");
				close(listenFd);
				return -1;
			}
			executeCommad(connectFd);
		}
	}
	
}


int main(int argc, char *argv[])
{
	int ret = -1;
	int udpFd;
	struct sockaddr_in local_addr; 
	int server_len = sizeof(struct sockaddr_in);
	int count = -1;
	fd_set readfd; 
	msg umsg;
	pthread_t ptd;
	struct timeval timeout;
	g_state = STATE_IDLE;
	int timeoutCounter = 0;

	//before waiting for server broadcast, need to initateModule  
	initiateModule();
	while (1)
	{	
		udpFd = openBroadcastRecieveSocket();
		if(udpFd<0){
			printf("openBroadcastRecieveSocket error\n");
			exit(-1);
		}

		timeout.tv_sec = 30;
		timeout.tv_usec = 0;
	
		FD_ZERO(&readfd);
		FD_SET(udpFd, &readfd);

		ret = select(udpFd + 1, &readfd, NULL, NULL, &timeout); 
		switch (ret)
		{
			case -1: 
				perror("select error:");
				break;
			case 0: 
//				printf("select timeout\n");
//				timeoutCounter++;
//				if(timeoutCounter>3){
//					printf("disconnected to server\n");
//					exit(-1);
//				}
				break;
			default:
				count = recvfrom(udpFd, &umsg, sizeof(msg), 0,
						(struct sockaddr*)&server_addr, &server_len); 
			
				if( umsg.dataType == BROADCAST){
					printf("\nrecvfrom server broadcast:\n\t IP: %s, port: %d\n",
						(char *)inet_ntoa(server_addr.sin_addr),ntohs(server_addr.sin_port));

					// register to server
					if(g_state==STATE_IDLE)					
						ret=register2Server();
					if(ret<0)
						break;
					
					ret =  pthread_create(&ptd, NULL, pingThread, NULL);
					if(ret<0)
						exit(-1);

					// this function will never return until catch error 
					// or lost connect to server!
					waitForServerCommand();
				}
				break;
		}
		pthread_cancel(ptd);
		close(udpFd);
	}
	return 0;
}
