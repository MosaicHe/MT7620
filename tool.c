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
//#include	<linux/in.h>
#include	"module.h"

#include   "nvram.h"
#include	"wireless.h"
#define MODULEID 1

/*
 * arguments: incompatible
 * description: 
 * return: 
 */
int openServerSocket(struct sockaddr_in server_addr)
{
	int fd = -1;
	int i = 3;
	int ct = 3;
	int ret = -1;
	
	fd = socket(PF_INET,SOCK_STREAM,0);
	if(fd < 0){
		perror("can't open socket\n");
		exit(1);
	}
	deb_print("i=%d\n", i);
	
	return fd;
}


int openListenTcpSocket()
{
	struct sockaddr_in local_addr;
	socklen_t addrlen;
	int server_fd;
	int ret;

	addrlen = sizeof(struct sockaddr);

	// listen socket
	server_fd = socket(PF_INET,SOCK_STREAM,0);
	if(server_fd < 0)
	{
		perror("socket");
		exit(1);
	}
	int on = 1;
	ret = setsockopt( server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

	bzero(&local_addr ,sizeof(local_addr));
	local_addr.sin_family = PF_INET;
	local_addr.sin_port = htons(LISTEN_PORT); 
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if( bind(server_fd, (struct sockaddr*)&local_addr, addrlen) < 0 )
	{
		perror("bind");
		exit(1);
	}

	if( listen(server_fd,N) < 0)
	{
		perror("listen");
		exit(1);
	}

	return server_fd;

}


int openBroadcastRecieveSocket()
{
	int ret=-1;
	int udpFd=-1;
	struct sockaddr_in local_addr;
	udpFd = socket(AF_INET, SOCK_DGRAM, 0); 
	if (udpFd < 0){
		perror("udpFd error");
		return -1;
	}
	int on = 1;
	ret = setsockopt( udpFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

	memset((void*) &local_addr, 0, sizeof(struct sockaddr_in));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htons(INADDR_ANY );
	local_addr.sin_port = htons(BROADCAST_PORT);

	ret = bind(udpFd, (struct sockaddr*)&local_addr, sizeof(local_addr));
	if (ret < 0){
		perror("bind error");
		return -1;
	}
	return udpFd;
}


int sendData(int fd, int dataType, void* buf, int buflen)
{
	int ret  = -1;
	msg *p_responseBuf;
	p_responseBuf = (msg*)malloc(sizeof(msg));
	bzero( p_responseBuf, sizeof(msg));
	
	p_responseBuf->moduleID = g_moduleID;
	p_responseBuf->dataType = dataType;
	if(buflen > 0){
		p_responseBuf->dataSize = buflen;
		memcpy( p_responseBuf->dataBuf, buf, buflen);
	}
	deb_print("msg length:%ld, send data Type:%d, length:%d\n", sizeof(msg), dataType, buflen);
	ret = write(fd, p_responseBuf, sizeof(msg));
	if(ret< 0){
		perror("socket write error\n");
		free(p_responseBuf);
		return -1;
	}
	free(p_responseBuf);
	return 0;
}

/*
 * function: read data from socket
 */
extern int recvData(int fd, msg* msgbuf, struct timeval* ptv)
{

	int ret =-1;
	fd_set rdfds;
	FD_ZERO(&rdfds);
	FD_SET(fd, &rdfds);

	ret = select(fd+1,&rdfds, NULL, NULL, ptv);
	if(ret<0||ret==0){
		return ret;
	}
	ret = read(fd, msgbuf, sizeof(msg));
	return ret;
}


extern int recvFirmware(int fd)
{
	return 0;
} 

extern void updateFirmware()
{
}

/*
 * arguments: ifname  - interface name
 * description: test the existence of interface through /proc/net/dev
 * return: -1 = fopen error, 1 = not found, 0 = found
 */
extern int getIfLive(char *ifname)
{
        FILE *fp;
        char buf[256], *p;
        int i;

        if (NULL == (fp = fopen("/proc/net/dev", "r"))) {
                perror("getIfLive: open /proc/net/dev error");
                return -1;
        }

        fgets(buf, 256, fp);
        fgets(buf, 256, fp);
        while (NULL != fgets(buf, 256, fp)) {
                i = 0;
                while (isspace(buf[i++]))
                        ;
                p = buf + i - 1;
                while (':' != buf[i++])
                        ;
                buf[i-1] = '\0';
                if (!strcmp(p, ifname)) {
                        fclose(fp);
                        return 0;
                }
        }
        fclose(fp);
        return 1;
}

/*
 * arguments: ifname  - interface name
 *            if_addr - a 18-byte buffer to store mac address
 * description: fetch mac address according to given interface name
 */
extern int getIfMac(char *ifname, char *if_hw)
{
        struct ifreq ifr;
        char *ptr;
        int skfd;

        if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("getIfMac: open socket error");
                return -1;
        }

        strncpy(ifr.ifr_name, ifname, IF_NAMESIZE);
        if(ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) {
                close(skfd);
                //error(E_L, E_LOG, T("getIfMac: ioctl SIOCGIFHWADDR error for %s"), ifname);
                return -1;
        }

        ptr = (char *)&ifr.ifr_addr.sa_data;
        sprintf(if_hw, "%02X:%02X:%02X:%02X:%02X:%02X",
                        (ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
                        (ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));

        close(skfd);
        return 0;
}

/*
 * arguments: ifname  - interface name
 *            if_addr - a 16-byte buffer to store ip address
 * description: fetch ip address, netmask associated to given interface name
 */
int getIfIp(char *ifname, char *if_addr)
{
	struct ifreq ifr;
	int skfd = 0;

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("getIfIp: open socket error");
		return -1;
	}

	strncpy(ifr.ifr_name, ifname, IF_NAMESIZE);
	if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) {
		close(skfd);
		//perror("getIfIp: ioctl SIOCGIFADDR error ");
		return -1;
	}
	strcpy(if_addr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	close(skfd);
	return 0;
}


extern int initInternet(void)
{
	system("internet.sh");
}


extern moduleInfo* getModuleInfo()
{
	int channel, channel_5g;

	bzero(&g_moduleInfo, sizeof(moduleInfo));	
	const char* ssid = nvram_bufget(RT2860_NVRAM, "SSID1");
	memcpy(g_moduleInfo.ssid_24g, ssid, strlen(ssid));
	
	const char* ch = nvram_bufget(RT2860_NVRAM, "Channel");
	channel = atoi(ch);
	g_moduleInfo.channel_24g = channel;
	
	getIfMac("ra0", g_moduleInfo.mac_24g);
	
	if( !getIfLive("rai0") ){
		g_moduleInfo.state_5g = 1;
		char *ssid_5g = nvram_bufget(RTDEV_NVRAM, "SSID1");
		memcpy(g_moduleInfo.ssid_5g, ssid_5g, strlen(ssid_5g));
		
		const char* ch_5g = nvram_bufget(RTDEV_NVRAM, "Channel");
		channel_5g = atoi(ch_5g);
		g_moduleInfo.channel_5g = channel_5g;
		getIfMac("rai0", g_moduleInfo.mac_5g);
	}else{
		g_moduleInfo.state_5g = -1;
	}
	return 0;
}


extern int initiateModule()
{	
	//g_moduleID = MODULEID;
	g_moduleID = getApid();
	if(g_moduleID<1 || g_moduleID>3){
		g_moduleID=0;
		printf("get module ID error\n");
		exit(1);
	}
	
	g_state = STATE_IDLE;
	getModuleInfo();
}

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

/*
 *
 */
int getServerIPbyDns( char* s)
{
	char* ret;
	char buf[128];
	FILE *fp;
	char* p;
	
	fp = fopen(FILENAME, "r");
	if(fp==NULL)
		return -1;

	ret = fgets(buf, 128, fp);
	if(ret == NULL)
		return -1;	
	
	strtok(buf," ");
	p = strtok(NULL, " ");
	if(p)
		strcpy(s, p);
	else
		return -1;
	
	return 0;
}


void setStaLimit( int num)
{
	char s[5];
	sprintf(s,"%d", num);
	printf("set limitnum:%s\n", s);
	setlimitNum(s);
}


void sendMacList(int fd, char *ifname)
{
	struct maclist ml;
	msg msgbuf;
	getOnlineMaclist( ifname, &ml);
	memcpy(msgbuf.dataBuf, &ml, sizeof(ml));
	msgbuf.dataSize = sizeof(ml);	
	write(fd, &msgbuf, sizeof(msg));	
}







