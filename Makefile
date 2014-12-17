CC=gcc
LFLAGS=-lusb

all:	lcdcontrol

lcdcontrol:	lcdcontrol.c
		$(CC) $(CFLAGS) lcdcontrol.c $(LFLAGS) -o lcdcontrol
clean:		
		rm -f lcdcontrol

.PHONY:	all clean
