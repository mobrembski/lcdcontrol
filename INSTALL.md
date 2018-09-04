
1. Install libusb development headers:

    sudo apt-get install libusb-dev

2. Build lcdcontrol:

    sudo make

3. Install lcdcontrol to system:

    sudo make install

OR MANUAL INSTALL:

1. Copy 71-lcd2usb.rules to /etc/udev/rules.d directory. This gives regular user priviliges to use LCD2USB device:

    sudo cp 71-lcd2usb.rules /etc/udev/rules.d

2. Copy lcd_logoff.sh somewhere in PATH, and give it executable permissions. I recommend to use /usr/local/bin:

    sudo cp lcd_logoff.sh /usr/local/bin

3. Copy lcdcontrol somewhere in PATH, and give it executable permissions. I recommend to use /usr/local/bin:

    sudo cp lcdcontrol /usr/local/bin

4. Go to Settings->Startup programs and add /usr/local/bin/lcd_logoff.sh script.


If you have a problem, that lcdproc just exits after start with no message, it may be an issue of lack of permissions to the /var/run folder when running lcdproc from regular user.
To solve it, please modify following line in lcdproc.conf:
PidFile=/tmp/lcdproc.pid
