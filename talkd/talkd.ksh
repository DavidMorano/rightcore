#!/bin/ksh
# TALKD



cd /tmp

echo "hello from inside" > /tmp/talkd.hello

DEBUGFD=3 
export DEBUGFD

/etc/bin/talkd.s5 3> /tmp/talkd.deb 2> /tmp/talkd.err 1> /tmp/talkd.out



