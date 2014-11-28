#include "client.h"
#include "nvram.h"

extern int getIfLive(char *ifname);
extern int getIfMac(char *ifname, char *if_hw);
extern 	int initInternet(void);


typedef int ( *CommandFunction )( char* buf, int *len);

typedef struct{
	int commandNum;
	CommandFunction cmdfunc;
}commandNode;


#define GET_INFO 	 22
#define SET_SSID 	 23
#define SET_SSID5G	 24
#define CLOSE		 0

int respose_getInfo(char* buf, int *len)
{
	int ret = -1;
	int channel, channel_5g;
	printf("hello cf_moduleon\n");
	client cli_info;
	bzero(&cli_info, sizeof(client));
	cli_info.id = ID;
	
	const char* ssid = nvram_bufget(RT2860_NVRAM, "SSID1");
	memcpy(cli_info.ssid, ssid, strlen(ssid));
	
	const char* ch = nvram_bufget(RT2860_NVRAM, "Channel");
	channel = atoi(ch);
	cli_info.channel = channel;
	
	getIfMac("ra0", cli_info.mac);
	
	if( !getIfLive("rai0") ){
		cli_info.have5g = 1;
		char *ssid_5g = nvram_bufget(RTDEV_NVRAM, "SSID1");
		memcpy(cli_info.ssid_5g, ssid_5g, strlen(ssid_5g));
		
		const char* ch_5g = nvram_bufget(RTDEV_NVRAM, "Channel");
		channel_5g = atoi(ch_5g);
		
		cli_info.channel_5g = channel_5g;
		getIfMac("rai0", cli_info.mac_5g);
	}else{
		cli_info.have5g = 0;
	}
	
	client_print( &cli_info);
	
	ret = sendData(srv_fd, ID, GET_INFO,  &cli_info, sizeof(client));
	return ret;
}

int respose_setSsid( char* buf, int *len)
{
	client cli_info;
	char cmd[128];
	bzero(cmd, sizeof(cmd));
	int ret;
		
	memcpy( &cli_info, buf, sizeof(client));
	nvram_bufset(RT2860_NVRAM, "SSID1", cli_info.ssid);
	nvram_commit(RT2860_NVRAM);

	sprintf( cmd,"iwpriv ra0 set SSID=%s", cli_info.ssid);	
	system( cmd );
	
	ret = 1;
	sendData(srv_fd, ID, SET_SSID,  &ret, sizeof(int));
	return 0;
}

int respose_setSsid5g( char* buf, int *len)
{
	client cli_info;
	char cmd[128];
	bzero(cmd, sizeof(cmd));
	int ret;
	
	memcpy( &cli_info, buf, sizeof(client));
	nvram_bufset(RTDEV_NVRAM, "SSID1", cli_info.ssid_5g);
	nvram_commit(RTDEV_NVRAM);
	
	sprintf( cmd,"iwpriv rai0 set SSID=%s", cli_info.ssid);	
	system( cmd );
	
	ret = 1;
	sendData(srv_fd, ID, SET_SSID5G,  &ret, sizeof(int));
	return 0;
}


commandNode cmdTable[]={
	{GET_INFO, respose_getInfo},
	{SET_SSID, respose_setSsid},
	{SET_SSID5G, respose_setSsid5g},
};

#define CMDTABLESIZE  sizeof(cmdTable)/sizeof(commandNode)

int doCommand(int cNum, char* buf, int *len )
{
	int i = 0;
	int ret = -1;
	deb_print("cmd number is: %d\n", cNum);
	for(i = 0; i< CMDTABLESIZE; i++){
		if( cmdTable[i].commandNum == cNum ){
			ret = cmdTable[i].cmdfunc(buf, len);
			return ret;
		}
	}
	printf("CommadNum is not in cmdTable\n");
	return ret;
}

