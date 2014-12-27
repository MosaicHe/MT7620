
struct maclist{
	int macNum;
	char macArray[50][6];
};


int getOnlineMaclist(char *ifname, struct maclist* ml);
int getOnlineMacNum(char *ifname);
int setlimitNum( char *num );
