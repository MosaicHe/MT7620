#include	<stdlib.h>
#include	<sys/ioctl.h>
#include	<arpa/inet.h>
#include	<linux/wireless.h>
#include	<linux/types.h>
#include	<linux/socket.h>
#include	<linux/if.h>
#include	"nvram.h"
#include	"utils.h"
#include	"webs.h"
#include	"wireless.h"
#include	"oid.h"
#include	"stapriv.h"			//for statistics

#include	"linux/autoconf.h"  //kernel config

typedef struct _RT_802_11_MAC_ENTRY {
	unsigned char			ApIdx;
	unsigned char           Addr[6];
	char					Ssid[32];
	unsigned char           Aid;
	unsigned char           Psm;     // 0:PWR_ACTIVE, 1:PWR_SAVE
	unsigned char           MimoPs;  // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	char                    AvgRssi0;
	char                    AvgRssi1;
	char                    AvgRssi2;
	unsigned int            ConnectedTime;
	MACHTTRANSMIT_SETTING	TxRate;
	unsigned int			LastRxRate;
	short					StreamSnr[3];
	short					SoundingRespSnr[3];
	//int					StreamSnr[3];
	//int					SoundingRespSnr[3];
#if 0
	short					TxPER;
	short					reserved;
#endif
#if defined (RT2860_VOW_SUPPORT) || defined (RTDEV_VOW_SUPPORT)
	char					Tx_Per;
#endif
} RT_802_11_MAC_ENTRY;

#define MAX_NUMBER_OF_MAC               116

typedef struct _RT_802_11_MAC_TABLE {
	unsigned long            Num;
	RT_802_11_MAC_ENTRY      Entry[MAX_NUMBER_OF_MAC]; //MAX_LEN_OF_MAC_TABLE = 32
} RT_802_11_MAC_TABLE;


// every mac string lenth is 18-bytes
static int getWlanStaInfo(char *ifname, char** maclist)
{
	int i, s;
	struct iwreq iwr;
	RT_802_11_MAC_TABLE table = {0};

	s = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(iwr.ifr_name, ifname, IFNAMSIZ);
	iwr.u.data.pointer = (caddr_t) &table;

	if (s < 0) {
		perror("ioctl sock failed!");
		return -1;
	}

	if (ioctl(s, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		close(s);
		return -1;
	}

	close(s);

	for (i = 0; i < table.Num; i++) {
		sprintf( maclist[i], "%02X:%02X:%02X:%02X:%02X:%02X",
				table.Entry[i].Addr[0], table.Entry[i].Addr[1],
				table.Entry[i].Addr[2], table.Entry[i].Addr[3],
				table.Entry[i].Addr[4], table.Entry[i].Addr[5]);
	}

	return 0;
}
