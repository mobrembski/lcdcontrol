CC=gcc
LFLAGS=-lusb

all:	lcdcontrol

lcdcontrol:	lcdcontrol.c
		$(CC) $(CFLAGS) lcdcontrol.c $(LFLAGS) -o lcdcontrol
clean:		
		rm -f lcdcontrol
install:
		cp lcdcontrol /usr/local/bin
		chmod 0755 /usr/local/bin/lcdcontrol
		cp lcd_logoff.sh /usr/local/bin
		chmod 0755 /usr/local/bin/lcd_logoff.sh
		cp 71-lcd2usb.rules /etc/udev/rules.d

.PHONY:	all clean
