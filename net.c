
#define DEBUG
#ifndef DEBUG
int SetLocalIp( const char *ipaddr )  
{  

	int sock_set_ip;  

	struct sockaddr_in sin_set_ip;  
	struct ifreq ifr_set_ip;  

	bzero( &ifr_set_ip,sizeof(ifr_set_ip));  

	if( ipaddr == NULL )  
		return -1;  

	if(sock_set_ip = socket( AF_INET, SOCK_STREAM, 0 ) == -1);  
	{  
		perror("socket create failse...SetLocalIp!/n");  
		return -1;  
	}  

	memset( &sin_set_ip, 0, sizeof(sin_set_ip));  
	strncpy(ifr_set_ip.ifr_name, "eth0", sizeof(ifr_set_ip.ifr_name)-1);     

	sin_set_ip.sin_family = AF_INET;  
	sin_set_ip.sin_addr.s_addr = inet_addr(ipaddr);  
	memcpy( &ifr_set_ip.ifr_addr, &sin_set_ip, sizeof(sin_set_ip));  

	if( ioctl( sock_set_ip, SIOCSIFADDR, &ifr_set_ip) < 0 )  
	{  
		perror( "Not setup interface/n");  
		return -1;  
	}  

	//设置激活标志  
	ifr_set_ip.ifr_flags |= IFF_UP |IFF_RUNNING;  

	//get the status of the device  
	if( ioctl( sock_set_ip, SIOCSIFFLAGS, &ifr_set_ip ) < 0 )  
	{  
		perror("SIOCSIFFLAGS");  
		return -1;  
	}  

	close( sock_set_ip );  
	return 0;  
} 



int GetLocalIp()  
{  

	int sock_get_ip;  
	char ipaddr[50];  

	struct   sockaddr_in *sin;  
	struct   ifreq ifr_ip;     

	if ((sock_get_ip=socket(AF_INET, SOCK_STREAM, 0)) == -1)  
	{  
		printf("socket create failse...GetLocalIp!/n");  
		return "";  
	}  

	memset(&ifr_ip, 0, sizeof(ifr_ip));     
	strncpy(ifr_ip.ifr_name, "eth0", sizeof(ifr_ip.ifr_name) - 1);     

	if( ioctl( sock_get_ip, SIOCGIFADDR, &ifr_ip) < 0 )     
	{     
		return "";     
	}       
	sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;     
	strcpy(ipaddr,inet_ntoa(sin->sin_addr));         

	printf("local ip:%s /n",ipaddr);      
	close( sock_get_ip );  

	return 0;  
} 

#else

extern int SetLocalIp( const char *ipaddr ) 
{
	return 0;
}
#endif

