#CC = /opt/buildroot-gcc463/usr/bin/mipsel-linux-gcc
#ROOTDIR = /home/mosaic/workspace/Ralink/RT288x_SDK/source
#LINUXDIR = linux-2.6.36.x

EXEC = module
CFLAGS += -I$(ROOTDIR)/$(LINUXDIR)/drivers/char
CFLAGS	+= -I$(ROOTDIR)/lib/libnvram
CFLAGS += -I$(ROOTDIR)/user/goahead/src
LDLIBS	+= -lnvram

all: $(EXEC) test

$(EXEC): $(EXEC).c tool.c pingthread.c wireless.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) -lpthread

test: test.c tool.c wireless.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o test $^

romfs:
	$(ROMFSINST) /bin/$(EXEC)
	$(ROMFSINST) /bin/test

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o *~ test

