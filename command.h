#include "nvram.h"
#include "tool.h"
#include "module.h"

typedef int ( *CommandFunction )( char* buf, int *len);

typedef struct{
	int commandNum;
	CommandFunction cmdfunc;
}commandNode;


#define SET_MODULE 	 23
#define GET_MODULE	 24
#define RESET_MODULE 25
#define CLOSE		 0

//RT2860_NVRAM
//RTDEV_NVRAM
int response_getModule(char* buf, int *len)
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
		memcpy(&mn, buf+i*sizeof(moduleNvram), sizeof(mn));
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
	ret = sendData(srv_fd, GET_MODULE,  &ret, sizeof(ret));
	
	system("internet.sh");
	return ret;
}

int response_setModule(char* buf, int *len)
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
	memcpy(&mn, buf, sizeof(mn));

	if(!strcmp( mn.nvramDev, "2860"))
		ndev = RT2860_NVRAM;
	else
		ndev = RTDEV_NVRAM;

	value = nvram_bufget( ndev, mn.item);
	strcpy(mn.value, value);
	ret = sendData(srv_fd, GET_MODULE,  &mn, sizeof(moduleNvram));
	return ret;
}


int response_heartbeat(char* buf, int *len){
	int ret;
	ret = sendData(srv_fd, HEARTBEAT,  NULL, 0);
	return ret;
}


int response_resetModule(char* buf, int *len){
	int ret;
	ret = sendData(srv_fd, RESET_MODULE,  NULL, 0);
	/* FIXME to reset module*/
	return ret;
}

commandNode cmdTable[]={
	{HEARTBEAT,  response_heartbeat},
	{SET_MODULE, response_setModule},
	{GET_MODULE, response_getModule},
	{RESET_MODULE, response_resetModule},
};

#define CMDTABLESIZE  sizeof(cmdTable)/sizeof(commandNode)

int doCommand(int cNum, msg* buf)
{
	int i = 0;
	int ret = -1;
#if 0
	deb_print("cmd number is: %d\n", cNum);
	for(i = 0; i< CMDTABLESIZE; i++){
		if( cmdTable[i].commandNum == cNum ){
			ret = cmdTable[i].cmdfunc(buf, len);
			return ret;
		}
	}
#endif
	printf("CommadNum is not in cmdTable\n");
	return ret;
}

