#include <stdio.h>             
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/autoconf.h>
#include "ralink_gpio.h"

#define GPIO_DEV	"/dev/gpio"

enum {
	gpio_in,
	gpio_out,
};
enum {
	gpio2300,
	gpio3924,
	gpio7140,
	gpio72,
};

int gpio_set_dir(int r, int dir)
{
	int fd, req;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	if (dir == gpio_in) {
		if (r == gpio72)
			req = RALINK_GPIO72_SET_DIR_IN;
		else if (r == gpio7140)
			req = RALINK_GPIO7140_SET_DIR_IN;
		else if (r == gpio3924)
			req = RALINK_GPIO3924_SET_DIR_IN;
		else
			req = RALINK_GPIO_SET_DIR_IN;
	}
	else {
		if (r == gpio72)
			req = RALINK_GPIO72_SET_DIR_OUT;
		else if (r == gpio7140)
			req = RALINK_GPIO7140_SET_DIR_OUT;
		else if (r == gpio3924)
			req = RALINK_GPIO3924_SET_DIR_OUT;
		else
			req = RALINK_GPIO_SET_DIR_OUT;
	}
	if (ioctl(fd, req, 0xffffffff) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int gpio_read_int(int r, int *value)
{
	int fd, req;

	*value = 0;
	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}

	if (r == gpio72)
		req = RALINK_GPIO72_READ;
	else if (r == gpio7140)
		req = RALINK_GPIO7140_READ;
	else if (r == gpio3924)
		req = RALINK_GPIO3924_READ;
	else
		req = RALINK_GPIO_READ;
	if (ioctl(fd, req, value) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int gpio_read(int gpiog)
{
	int i, d;
	unsigned reg;
	gpio_set_dir(gpiog, gpio_in);
	gpio_read_int(gpiog, &d);
	printf("gpio 71~40 = 0x%x\n", d);
	return d;
}


extern int getApid()
{
	int d;
	int flag1, flag2;
	int mask=1;
	d = gpio_read(gpio7140);
	flag1 = d&(mask<<16);
	flag2 = d&(mask<<17);
	if( flag1==0 && flag2==0 )
		return 1;
	else if(flag1==0 && flag2)
		return 2;
	else if(flag1 && flag2==0)
		return 3;
	else
		return 0;
}
