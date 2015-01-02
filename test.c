#include "tool.h"

int main(int argc, char** argv)
{
	char mac[20];
	getIfMac( argv[1], mac);
	printf("%s mac:%s\n", argv[1], mac);
}
