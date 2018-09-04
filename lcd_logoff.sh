#!/bin/bash
#!/bin/bash

interface=org.gnome.ScreenSaver
member=ActiveChanged

dbus-monitor --session "type='signal',interface='org.gnome.ScreenSaver'" | \
( while true
    do read X
    if echo $X | grep "boolean true" &> /dev/null; then
	lcdcontrol -b 0
	killall lcdproc
    elif echo $X | grep "boolean false" &> /dev/null; then
	lcdcontrol -b 50
	lcdproc -f &
    fi
    done )
