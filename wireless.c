#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/ioctl.h>
#include	<arpa/inet.h>
#include	<linux/types.h>
#include	<linux/socket.h>
#include	<linux/if.h>
#include	<linux/wireless.h>
#include 	<linux/sockios.h>
#include 	<linux/netlink.h>
#include 	<sys/socket.h>
#include	"wireless.h"

#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE				0x8BE0
#endif
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT	(SIOCIWFIRSTPRIV + 0x1F)

#define MAX_CONNECT_COUNTER_24G	30	
#define LM_NETLINK	17

//#define NETLINK_SENDMAC 17
#define MAX_PAYLOAD 32
#define SCTIME 700

struct req{
	struct nlmsghdr nlh;
	char buf[MAX_PAYLOAD];
};


int setlimitNum( char *num )
{
	fd_set fds;
	int sk = -1;
	struct sockaddr_nl nladdr, to_nladdr;
	struct msghdr msg;
	//struct nlmsghdr * nlh;
	struct iovec iov;
	struct req r;
	pthread_t pid;
	struct timeval now;
	FILE *pFile;

	memset(&msg , 0 ,sizeof(struct msghdr));
	sk = socket(AF_NETLINK, SOCK_RAW, LM_NETLINK);
	if (sk == -1){
		printf("open socket error!\n");
		return -1;
	}

	memset(&nladdr, 0, sizeof(nladdr));
	//设置bind用的本地 struct sockaddr
	nladdr.nl_family = AF_NETLINK;
	nladdr.nl_groups = 0; 
	nladdr.nl_pid = getpid();

	//绑定本地socket
	if (bind(sk, (struct sockaddr *)&nladdr, sizeof(struct sockaddr_nl))< 0){
		printf("bind netlink socket error!\n");
		return -1;
	}
	//send netlink message to kernel
	memset(&to_nladdr, 0, sizeof(to_nladdr));

	//设置发送用的目的sockaddr_nl
	to_nladdr.nl_family = AF_NETLINK;
	to_nladdr.nl_pid = 0;
	to_nladdr.nl_groups = 0;

	//通过iov指向要使用的nlmsghdr
	iov.iov_base = &r;
	iov.iov_len = sizeof(r);

	//设置msghdr
	msg.msg_name = &to_nladdr;
	msg.msg_namelen = sizeof(to_nladdr);
	msg.msg_iov = &iov;
	msg.msg_iovlen =1;

	//设置承载负荷数据的nlmsghdr
	//包含数据和头部长度的nlmsghdr的长度
	r.nlh.nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	r.nlh.nlmsg_pid = getpid();
	r.nlh.nlmsg_flags = 0;
	memset(r.buf, 0, MAX_PAYLOAD);
		
	strcpy(NLMSG_DATA(&(r.nlh)), "limitnum");
	strcpy(NLMSG_DATA(&(r.nlh))+8, num);
	sendmsg(sk, &msg, 0);
	close(sk);
	return 0;
}


typedef union _MACHTTRANSMIT_SETTING {
	struct  {
		unsigned short  MCS:7;  // MCS
		unsigned short  BW:1;   //channel bandwidth 20MHz or 40 MHz
		unsigned short  ShortGI:1;
		unsigned short  STBC:2; //SPACE
		unsigned short	rsv:3;
		unsigned short  MODE:2; // Use definition MODE_xxx.
	} field;
	unsigned short      word;
} MACHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
	unsigned char		ApIdx;
	unsigned char           Addr[6];
	char			Ssid[32];
	unsigned char           Aid;
	unsigned char           Psm;     // 0:PWR_ACTIVE, 1:PWR_SAVE
	unsigned char           MimoPs;  // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	char                    AvgRssi0;
	char                    AvgRssi1;
	char                    AvgRssi2;
	unsigned int            ConnectedTime;
	MACHTTRANSMIT_SETTING	TxRate;
	unsigned int		LastRxRate;
	short			StreamSnr[3];
	short			SoundingRespSnr[3];
} RT_802_11_MAC_ENTRY;


typedef struct _RT_802_11_MAC_TABLE {
	unsigned long            Num;
	RT_802_11_MAC_ENTRY      Entry[32]; //MAX_LEN_OF_MAC_TABLE = 32
} RT_802_11_MAC_TABLE;

extern int getOnlineMaclist(char *ifname, struct maclist* ml)
{
	int i, s;

	struct iwreq iwr;
	RT_802_11_MAC_TABLE table = {0};

	s = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) &table;

	if (s < 0) {
		return -1;
	}
	
	//system("dmesg -c > /dev/null");

	if (ioctl(s, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		close(s);
		return -1;
	}

	close(s);
	
	//system("dmesg -c");
	ml->macNum = table.Num;
#if 1
	for (i = 0; i < table.Num; i++) {
		printf( "%02X:%02X:%02X:%02X:%02X:%02X %s\n",
		table.Entry[i].Addr[0], table.Entry[i].Addr[1],
		table.Entry[i].Addr[2], table.Entry[i].Addr[3],
		table.Entry[i].Addr[4], table.Entry[i].Addr[5]);

		memcpy( ml->macArray[i], table.Entry[i].Addr, 6);
	}
#endif
	return 0;
}

extern int getOnlineMacNum(char *ifname)
{
	int i, s;

	struct iwreq iwr;
	RT_802_11_MAC_TABLE table = {0};

	s = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) &table;

	if (s < 0) {
		return -1;
	}
	
	//system("dmesg -c > /dev/null");

	if (ioctl(s, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		close(s);
		return -1;
	}

	close(s);
	return table.Num;
}



