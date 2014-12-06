//#include "module.h"
#include "nvram.h"
#include "command.h"
#include "tool.h"


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
	p_idStr = nvram_bufget(RT2860_NVRAM, "moduleID");
	g_moduleID = atoi(p_idStr);
	if(p_idStr==NULL || checkId(g_moduleID)){
		/* FIXME*/
		nvram_bufset(RT2860_NVRAM, "moduleID", p_idStr);
	}
	deb_print("get moduleID:%s\n", p_idStr);
	g_lanip= nvram_bufget(RT2860_NVRAM, "lan_ipaddr");
	g_serverip= nvram_bufget(RT2860_NVRAM, "Server_ipaddr");
	if( (g_serverip == NULL) || strcmp( g_lanip, getModuleIp( g_moduleID, dataBuf)) || (srv_fd=openServerSocket(PORT, g_serverip)) <0 )
	{	
		
		//get g_serverip through dhcp
		anotherWayGetServerip();
		srv_fd =  openServerSocket(PORT, g_serverip);
		if(srv_fd < 0){
			perror("can't connect to server!\n");
			exit(1);
		}	
	}
	val = register2Server();

	free(g_serverip);
	free(g_lanip);
	return 0;
}
