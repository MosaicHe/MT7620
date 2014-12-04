//#include "module.h"
#include "nvram.h"
#include "command.h"
#include "tool.h"

int checkId(int id)
{
	return 0;
}


char* getModuleIp( int id , char* ipaddr )
{
	int len;
	int i = 0;
	int s = 0;

	char lastStr[5];
	char n_lastStr[5];
	strcpy(lastStr, strrchr(ipaddr, '.' ));
	len = strlen(lastStr);
	
	for(i=0; i< len-1; i++){
		lastStr[i]=lastStr[i+1];
	}
	lastStr[len-1] = '\0';
	s = atoi(lastStr);
	s += id;
	len = strlen(ipaddr);
	
	for(i=strlen(lastStr); i>0; i--){
		ipaddr[len-i]='\0';
	}
	
	sprintf(n_lastStr, "%d", s);
	strcat(ipaddr, n_lastStr);

	printf( "ip: %s\n", ipaddr);
	return ipaddr;
}

int getServerIPbyDns( char* s)
{
	int ret;
	char buf[128];
	FILE *fp;
	char* p;
	
	fp = fopen(FILENAME, "r");
	if(fp==NULL)
		return -1;

	ret = fgets(buf, 128, fp);
	if(ret < 0)
		return ret;	
	
	strtok(buf," ");
	p = strtok(NULL, " ");
	if(p)
		strcpy(s, p);
	else
		return -1;
	
	return 0;
}



#if 0
/*
 * try get serverip through udhcpc, and reset "lan_ipaddr" and "Serverip";
 */
int anotherTry()
{
	system("udhcpc -i br0");
	sleep(2);
	serverip = (char*)malloc(16);
	ret = getServerIPbyDns( serverip );	
	deb_print("get serverip:%s\n",serverip);

	bzero( item, sizeof(item) );
	sprintf(item, "Server_ipaddr");
	nvram_bufset(RT2860_NVRAM, item, serverip);
	nvram_commit(RT2860_NVRAM);

	bzero(item, sizeof(item));
	sprintf(item, "lan_ipaddr");
	getModuleIp( moduleID, dataBuf);
	nvram_bufset(RT2860_NVRAM, item, dataBuf);
	nvram_commit(RT2860_NVRAM);

	system("killall udhcpc");

	lan_ipaddr= nvram_bufget(RT2860_NVRAM, "lan_ipaddr");
	bzero(cmd, sizeof(cmd));
	sprintf(cmd, "ifconfig br0 %s", lan_ipaddr);
	system(cmd);
	deb_print("ifconfig br0 %s\n",lan_ipaddr);
	return 0;
}
#endif



int main(int argc, char *argv[])
{
	int ret = -1;
	int val = -1;
	int cmdType;
	char dataBuf[DATASIZE];
	int dataLen;
	char cmd[256];
	char *lan_ipaddr = NULL;
//	char initip[256] = INITIP;
	char *p_id;	
	char item[256];
	
	p_id = nvram_bufget(RT2860_NVRAM, "moduleID");
	if(p_id==NULL || checkId(p_id)){
		/* FIXME*/
		nvram_bufset(RT2860_NVRAM, "moduleID", p_id);
	}
	deb_print("get moduleID:%s\n", p_id);
	moduleID = atoi(p_id);
	lan_ipaddr= nvram_bufget(RT2860_NVRAM, "lan_ipaddr");
	serverip= nvram_bufget(RT2860_NVRAM, "Server_ipaddr");
	if( (serverip == NULL) || strcmp( lan_ipaddr, getModuleIp( moduleID, dataBuf)) ||  (srv_fd=openServerSocket(PORT, serverip)) <0 ){	
		
		//anotherTry();
		system("udhcpc -i br0");
		sleep(2);
		serverip = (char*)malloc(16);
		ret = getServerIPbyDns( serverip );	
		deb_print("get serverip:%s\n",serverip);

		bzero( item, sizeof(item) );
		sprintf(item, "Server_ipaddr");
		nvram_bufset(RT2860_NVRAM, item, serverip);
		nvram_commit(RT2860_NVRAM);

		bzero(item, sizeof(item));
		sprintf(item, "lan_ipaddr");
		getModuleIp( moduleID, dataBuf);
		nvram_bufset(RT2860_NVRAM, item, dataBuf);
		nvram_commit(RT2860_NVRAM);

		system("killall udhcpc");

		lan_ipaddr= nvram_bufget(RT2860_NVRAM, "lan_ipaddr");
		bzero(cmd, sizeof(cmd));
		sprintf(cmd, "ifconfig br0 %s", lan_ipaddr);
		system(cmd);
		deb_print("ifconfig br0 %s\n",lan_ipaddr);

		srv_fd =  openServerSocket(PORT, serverip);
		if(srv_fd < 0){
			perror("can't connect to server!\n");
			exit(1);
		}		
	}
	
	val = register2Server();
	
	deb_print("wait for  server command......\n");
	while(1){
		deb_print("recvData....\n");
		ret = recvData(srv_fd, &cmdType, dataBuf, &dataLen, 0);
		if(ret < 0){
			perror("socket read error\n");
			close(srv_fd);
			exit(-1);
		}else if(ret == 0){
			// heartbeat test
			//heartbeatTest();
		}else{
			if(cmdType == CMD_CLOSE)
				exit(1);
			else{
				val = doCommand( cmdType, dataBuf, &dataLen );
				if(val < 0){
					perror("doCommand error!\n");
					continue;
				}
			}
		}
	}
	free(serverip);
	close(srv_fd);
	return 0;
}
