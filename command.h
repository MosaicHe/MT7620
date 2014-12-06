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
	int dataType;
	char buf[DATASIZE];
	int buflen;

	printf("register2Server\n");
	moduleInfo *p_module;
	p_module = getModuleInfo();
	
	ret = sendData(srv_fd, REGISTER, p_module, sizeof(moduleInfo));
	if(ret < 0){
		/* FIXME */
		printf("%s:sendData error",__FUNCTION__);
	}

	ret = recvData(srv_fd, &dataType, buf, &buflen, 1);
	if(ret<0){
		printf("%s:recvData error,__FUNCTION__");
		exit(1);
	}
	
	close(srv_fd);	

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
	ret = sendData(srv_fd, GET_MODULE,  &ret, sizeof(ret));
	
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
	ret = sendData(srv_fd, GET_MODULE,  &mn, sizeof(moduleNvram));
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

