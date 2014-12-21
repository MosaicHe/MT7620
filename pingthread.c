
#include "module.h"


#define PING_TIMEOUT 10
#define TIMEOUTLIMIT 5
#define PING_PORT	8003
void * pingThread(void* arg)
{
	int ret=-1;
	int udpFd=-1;
	fd_set rdfds;
	struct timeval tv;
	int timeout = 0;
	msg umsg;

	struct sockaddr_in local_addr, server_addr;
	socklen_t addrlen;
	udpFd = socket(AF_INET, SOCK_DGRAM, 0); 
	if (udpFd < 0){
		perror("udpFd error");
		return;
	}

	memset((void*) &local_addr, 0, sizeof(struct sockaddr_in));
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htons(INADDR_ANY );
	local_addr.sin_port = htons(PING_PORT);

	ret = bind(udpFd, (struct sockaddr*)&local_addr, sizeof(local_addr));
	if (ret < 0){
		perror("bind error");
		return ;
	}

	while(1){
		FD_ZERO(&rdfds);
		FD_SET(udpFd, &rdfds);
		tv.tv_sec = PING_TIMEOUT;
		tv.tv_usec= 0;
		ret = select(udpFd+1, &rdfds, NULL, NULL, &tv);
		if(ret<0){
			return;		
		}else if(ret==0){
			timeout++;
			if(timeout> TIMEOUTLIMIT)
				g_state = STATE_DISCONNECTED;
		}else{
			ret = recvfrom(udpFd, &umsg, sizeof(msg), 0,
						(struct sockaddr*)&server_addr, &addrlen);
			if(umsg.dataType == HEARTBEAT){
				deb_print("recvfrom server heartbeat\n");
				timeout=0;
				umsg.dataType = HEARTBEAT_ACK;
				sendto(udpFd, &umsg, sizeof(msg), 0,
						(struct sockaddr*)&server_addr, sizeof(struct sockaddr));
			}
			
		}
	}
}
