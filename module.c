//#include "module.h"
#include "nvram.h"
#include "command.h"
#include "tool.h"



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
	char dataBuf[DATASIZE];
	int dataLen;
	char cmd[256];
	char *p_idStr;
	char item[256];
	int timeoutCounter=0;

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
	
	srv_fd = socket(PF_INET,SOCK_STREAM,0);
	if(srv_fd < 0){
		perror("can't open socket\n");
		exit(1);
	}
	ret = connect(srv_fd, (struct sockaddr*)&g_servaddr, sizeof(struct sockaddr));
	if(ret < 0){
		printf("cannt connect to server\n");
		exit(1);
	}

	val = register2Server();
	if(val < 0){
		printf("register to server error\n");
		exit(1);	
	}

	while(1){
		ret = recvData( srv_fd, &cmdType, dataBuf, &dataLen, TIMEOUT);
		if(ret = -1){
			perror("recvData from socket error\n");		
			exit(1);
		}else if(ret==0){
			printf("timeout\n");
			timeoutCounter++;
			if( timeoutCounter>MAXCOUNTER ){
				perror("lose connecting to server\n");				
				exit(1);
			}
		}else{
			doCommand(cmdType, dataBuf, &dataLen);
		}
	}
	
	free(p_module);
	free(g_serverip);
	free(g_lanip);
	return 0;
}
