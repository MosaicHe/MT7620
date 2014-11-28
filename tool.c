#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<sys/ioctl.h>
#include	<net/if.h>
#include	<net/route.h>
#include    <dirent.h>

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
        perror("getIfLive: device not found");
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
