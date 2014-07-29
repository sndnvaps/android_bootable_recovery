# ! /sbin/sh

if [ ! -d /sd-ext ]
then
	mkdir /sd-ext
fi

#remove /cache/recovery/command  if exist 
if [ -f /cache/recovery/command ]
then
	rm /cache/recovery/command
fi

#remove /cache/update.zip if exist 
if [ -f /cache/update.zip ]
then
	rm /cache/update.zip
fi


#use pgrep get the process id if it exist 
#make it more easy 
if [ -f /sbin/pgrep ]
then
	echo "kill /sbin/adbd && /sbin/recovery"
	echo "use /sbin/pgrep"
	kill $(pgrep /sbin/adbd)
	kill $(pgrep /sbin/recovery)
else
	echo "kill /sbin/adbd && /sbin/recovery"
	echo "use /sbin/grep "
	kill $(ps | grep /sbin/adbd)
	kill $(ps | grep /sbin/recovery)
fi

# On the Galaxy S, the recovery comes test signed, but the
# recovery is not automatically restarted.
if [ -f /init.smdkc110.rc ]
then
    /sbin/recovery &
fi

# Droid X
if [ -f /init.mapphone_cdma.rc ]
then
    /sbin/recovery &
fi

exit 1






