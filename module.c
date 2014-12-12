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

#define MAX(a,b) (a>b?a:b)

/*
 * register module to server 
 */
int register2Server()
{
	int ret = -1;
	int dataType;
	char buf[DATASIZE];
	int buflen;
	int counter=0;

	printf("register2Server\n");
	while(1){
		if(counter > MAXCOUNTER){
			g_state = STATE_IDLE;			
			return -1;
		}

		switch(g_state){
			case STATE_IDLE:
				ret = sendData(srv_fd, REQ_HELLO, p_module, sizeof(moduleInfo));
				if(ret < 0){
					/* FIXME */
					printf("%s:sendData error",__FUNCTION__);
					counter++;
					break;
				}
				ret = recvData(srv_fd, &dataType, buf, &buflen, 1);
				if(ret<0 || dataType != REQ_HELLO){
					counter++;		
					break;
				}
				g_state = STATE_HELLO;
				counter=0;
				break;

			case STATE_HELLO:
				ret = sendData(srv_fd, REQ_FIRTWARE_UPDATE, p_module->fwVersion, 
								sizeof(p_module->fwVersion));
				if(ret < 0){
					/* FIXME */
					printf("%s:sendData error\n",__FUNCTION__);
					counter++;
					break;
				}
				ret = recvData(srv_fd, &dataType, buf, &buflen, 1);
				if(ret<0 || dataType != REQ_FIRTWARE_UPDATE){
					printf("REQ_FIRTWARE_UPDATE response error\n");
					counter++;
					break;
				}
				if( *(int*)buf == 1 ){
					recvFirmware(srv_fd);	

					//this function will reboot the module!!				
					updateFirmware();
					exit(0);
				}
				g_state = STATE_FIRMWARE_CHECKED;
				counter=0;
				break;

			case STATE_FIRMWARE_CHECKED:
				ret = sendData(srv_fd, REQ_CONFIG, NULL, 0);
				if(ret < 0){
					/* FIXME */
					printf("%s:sendData error",__FUNCTION__);
					counter++;
					break;
				}
				ret = recvData(srv_fd, &dataType, buf, &buflen, 1);
				if(ret<0 || dataType != REQ_CONFIG){
					printf("REQ_FIRTWARE_UPDATE response error\n");
					counter++;
					break;
				}
				if( buflen!=0 ){
//					doConfiguration(buf, buflen);
					break;
				}else{				
					g_state = STATE_CONFIG;
					counter=0;
					break;
				}

			case STATE_CONFIG:
				ret = sendData(srv_fd, REQ_RUN, NULL, 0);
				if(ret < 0){
					/* FIXME */
					printf("%s:sendData error",__FUNCTION__);
					counter++;
					break;
				}
				ret = recvData(srv_fd, &dataType, buf, &buflen, 1);
				if(ret<0 || dataType != REQ_RUN){
					counter++;
					break;
				}
				g_state = STATE_RUN;
				counter=0;
				break;

			case STATE_RUN:
				close(srv_fd);
				srv_fd = -1;
				return 0;
		}
	}
	
}


/*
 * try get serverip through udhcpc, 
 *  and reset "lan_ipaddr" and "g_serverip";
 */
int anotherWayGetServerip()
{
	int ret;
	char item[128];
	char cmd[256];

	system("udhcpc -i br0");
	sleep(2);
	g_serverip = (char*)malloc(16);
	ret = getServerIPbyDns( g_serverip );	
	deb_print("get g_serverip:%s\n",g_serverip);

	bzero( item, sizeof(item) );
	sprintf(item, "Server_ipaddr");
	nvram_bufset(RT2860_NVRAM, item, g_serverip);
	nvram_commit(RT2860_NVRAM);

	bzero(item, sizeof(item));
	sprintf(item, "lan_ipaddr");

	g_lanip = (char*)malloc(16);
	getModuleIp( g_moduleID, g_lanip);
	nvram_bufset(RT2860_NVRAM, item, g_lanip);
	nvram_commit(RT2860_NVRAM);

	system("killall udhcpc");

	bzero(cmd, sizeof(cmd));
	sprintf(cmd, "ifconfig br0 %s", g_lanip);
	system(cmd);
	deb_print("ifconfig br0 %s\n", g_lanip);
	return 0;
}


int main(int argc, char *argv[])
{
	int ret = -1;
	int val = -1;
	int cmdType;
	msg msgBuf;
	int dataLen;
	char cmd[256];
	char *p_idStr;
	char item[256];
	int timeoutCounter=0;
	int connFd=-1, listenFd;
	struct timeval tv;
	fd_set rdfds;
	int maxfd =0;
	struct sockaddr_in local_addr;
	socklen_t addrlen;

	//get Module information
	p_module = getModuleInfo();

	p_idStr = nvram_bufget(RT2860_NVRAM, "moduleID");
	g_moduleID = atoi(p_idStr);
	if(p_idStr==NULL || checkId(g_moduleID)){
		/* FIXME*/
		nvram_bufset(RT2860_NVRAM, "moduleID", p_idStr);
	}
	deb_print("get moduleID:%s\n", p_idStr);
	g_lanip= nvram_bufget(RT2860_NVRAM, "lan_ipaddr");

	g_serverip= nvram_bufget(RT2860_NVRAM, "Server_ipaddr");

	// wait for server send broadcast
	if( waitForServerBroadcast( &g_servaddr ) < 0 ){
		
		//get g_serverip through dhcp
		anotherWayGetServerip();
		g_servaddr.sin_family = PF_INET;
		g_servaddr.sin_port = htons(PORT); 
		g_servaddr.sin_addr.s_addr = inet_addr(g_serverip); 
	}
	deb_print("server ip:%s, port:%d\n", inet_ntoa(g_servaddr.sin_addr),
				ntohs(g_servaddr.sin_port));

	srv_fd = socket(PF_INET,SOCK_STREAM,0);
	if(srv_fd < 0){
		perror("can't open socket\n");
		exit(1);
	}
	g_servaddr.sin_port = htons(PORT);
	ret = connect(srv_fd, (struct sockaddr*)&g_servaddr, sizeof(struct sockaddr));
	if(ret < 0){
		printf("cannt connect to server\n");
		close(srv_fd);
		exit(1);
	}

	val = register2Server();
	if(val < 0){
		printf("register to server error\n");
		close(srv_fd);
		exit(1);
	}


	//create listen socket
	listenFd = socket(PF_INET,SOCK_STREAM,0);
	if(listenFd < 0)
	{
		perror("socket");
		exit(1);
	}
	bzero(&local_addr ,sizeof(local_addr));
	local_addr.sin_family = PF_INET;
	local_addr.sin_port = htons(LISTEN_PORT); 
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if( bind(listenFd, (struct sockaddr*)&local_addr, addrlen) < 0 )
	{
		perror("bind");
		exit(1);
	}

	if( listen(listenFd, N) < 0)
	{
		perror("listen");
		exit(1);
	}

	while(1){
		maxfd = 0;
		tv.tv_sec = 5;
		FD_ZERO(&rdfds);
		FD_SET(listenFd, &rdfds);
		maxfd = MAX(maxfd, listenFd);
		if(connFd>0){
			FD_SET(connFd, &rdfds);	
			maxfd = MAX(maxfd, connFd);	
		}
		ret = select( maxfd+1, &rdfds, NULL, NULL, &tv);
		switch(ret){
		case -1:
			perror("recvData from error socket\n");
			close(connFd);
			connFd = -1;
			break;
		case 0:
			printf("select timeout\n");
			timeoutCounter++;
			if( timeoutCounter>MAXCOUNTER ){
				deb_print("lose connecting to server\n");
				exit(1);			
			}
			break;
		default:
			if(FD_ISSET(listenFd, &rdfds)){
				connFd = accept(listenFd, NULL, NULL);
			}else if(FD_ISSET(connFd, &rdfds)){
				ret = read(connFd, &msgBuf, sizeof(msg));
				if(ret<0 || ret==0){
					perror("read socket data error\n");
					close(connFd);
					connFd = -1;			
				}else{
					doCommand(cmdType, &msgBuf);
					timeoutCounter = 0;
				}
			}
			break;
		}
	}

	close(listenFd);
	free(p_module);
	free(g_serverip);
	free(g_lanip);
	return 0;
}
