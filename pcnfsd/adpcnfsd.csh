#!/bin/csh
#
# adpcnfsd.csh
#
# This script will make rpc.pcnfsd, move it to the correct place on your
# system, then edit the appropriate file to allow the daemon to start up
# whenever you reboot your system.
#
# The prefered method of starting rpc.pcnfsd is from inetd.  This script will
# modify either your /etc/servers file (for SunOS 3.X) or /etc/inetd.conf
# file (for SunOS 4.X).  If you wish to instead start rpc.pcnfsd from 
# rc.local as a normal daemon, then you should refer to the section in the
# Installing PC-NFS guide that discusses installing server software.  Also
# make sure you read the READ THIS FIRST guide.
#
#
# To use this script you must first have copied the following files from the
# PC-NFS "Server 1" diskette:
#		adpcnfsd.csh   ( this file )
#		makefile
#		r_pcnfsd.c
# to some place on the system you are going to perform the install.  Usually
# /usr/tmp is a good place.
#
# Use then must become root on the machine you are intending to add the
# rpc.pcnfsd to.  Then run this script.  There are no arguments.
#


if ( $#argv > 0 ) then
	echo " "
	echo "   Usage: adpcnfsd"
	echo " "
	exit (1)
endif

echo " "
echo "Add RPC.PCNFSD shell script"
echo " "
echo

echo "Making RPC.PCNFSD"
mv r_pcnfsd.c rpc.pcnfsd.c
make

echo " "
echo "Make complete."
echo "Moving RPC.PCNFSD to /usr/etc directory."

mv rpc.pcnfsd /usr/etc

# delete files?

echo " "

if ( -f /etc/servers ) then
echo "Editing servers file."
ed  - /etc/servers  <<EOFSTRING
a
rpc	udp	/usr/etc/rpc.pcnfsd	150001	1
.
w
q
EOFSTRING
#

else if ( -f /etc/inetd.conf ) then
echo "Editing inetd.conf file."
ed  - /etc/inetd.conf  <<EOFSTRING
a
pcnfsd/1 dgram	rpc/udp	wait	root	/usr/etc/rpc.pcnfsd	rpc.pcnfsd
.
w
q
EOFSTRING
#
else
echo "No /etc/servers or /etc/inetd.conf file."
echo "Add RPC.PCNFSD script failed."
echo " "
exit(1)
endif

echo " "
echo "Add RPC.PCNFSD script completed."
echo "You can start the daemon now by typing /usr/etc/rpc.pcnfsd."
echo " "
echo " "


