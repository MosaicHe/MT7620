#include "nvram.h"
#include "tool.h"
#include "module.h"

typedef int ( *CommandFunction )( char* buf, int *len);

typedef struct{
	int commandNum;
	CommandFunction cmdfunc;
}commandNode;


#define REGISTER 	 22
#define SET_MODULE 	 23
#define GET_MODULE	 24
#define CLOSE		 0

int register2Server()
{
	int ret = -1;
	int channel, channel_5g;
	printf("hello cf_moduleon\n");
	module_info module;
	bzero(&module, sizeof(module_info));
	module.moduleID = moduleID;
	
	const char* ssid = nvram_bufget(RT2860_NVRAM, "SSID1");
	memcpy(module.ssid_24g, ssid, strlen(ssid));
	
	const char* ch = nvram_bufget(RT2860_NVRAM, "Channel");
	channel = atoi(ch);
	module.channel_24g = channel;
	
	getIfMac("ra0", module.mac_24g);
	
	if( !getIfLive("rai0") ){
		module.have5g = 1;
		char *ssid_5g = nvram_bufget(RTDEV_NVRAM, "SSID1");
		memcpy(module.ssid_5g, ssid_5g, strlen(ssid_5g));
		
		const char* ch_5g = nvram_bufget(RTDEV_NVRAM, "Channel");
		channel_5g = atoi(ch_5g);
		module.channel_5g = channel_5g;
		getIfMac("rai0", module.mac_5g);
	}else{
		module.have5g = 0;
	}
	
//	module_info_print( &module);
	
	ret = sendData(srv_fd, moduleID, REGISTER,  &module, sizeof(module_info));
	return ret;
	
}

//RT2860_NVRAM
//RTDEV_NVRAM

int respose_setModule(char* buf, int *len)
{
	int i, ret, mc;
	moduleNvram mn;
	char nvramDev[6];
	int  ndev;	
	char item[128];
	char value[128];

	mc = (*len)/sizeof(moduleNvram);
	for(i = 0; i<mc; i++){
		bzero(&mn, sizeof(mn));			
		bzero(nvramDev, 6);
		bzero(item, 128);
		bzero(value, 128);
		strcpy(&mn, buf+i*sizeof(moduleNvram));
		strcpy(nvramDev, mn.nvramDev);
		strcpy(item, mn.item);
		strcpy(value, mn.value);

		if(!strcmp(nvramDev, "2860"))
			ndev = RT2860_NVRAM;
		else
			ndev = RTDEV_NVRAM;

		nvram_bufset( ndev, item, value);
	}
	ret = nvram_commit( RTDEV_NVRAM );
	ret = sendData(srv_fd, moduleID, GET_MODULE,  &ret, sizeof(ret));
	
	system("internet.sh");
	return ret;
}

int respose_getModule(char* buf, int *len)
{
	int i, ret;
	moduleNvram mn;
	char nvramDev[6];
	int  ndev;	
	char item[128];
	char *value;

	bzero(&mn, sizeof(mn));			
	bzero(nvramDev, 6);
	bzero(item, 128);
	bzero(value, 128);
	strcpy(&mn, buf);

	if(!strcmp( &(mn.nvramDev), "2860"))
		ndev = RT2860_NVRAM;
	else
		ndev = RTDEV_NVRAM;

	value = nvram_bufget( ndev, mn.item);
	strcpy(mn.value, value);
	ret = sendData(srv_fd, moduleID, GET_MODULE,  &mn, sizeof(moduleNvram));
	return ret;
}


commandNode cmdTable[]={
	{SET_MODULE, respose_setModule},
	{GET_MODULE, respose_getModule},
	

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

