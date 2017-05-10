#!/bin/ksh
# DMAIL (emergency)


LOGFILE=/tmp/dmail.env
E=/tmp/dmail.err
D=/tmp/dmail.deb

logprint() {
  print $* >> $LOGFILE
}

DATE=$( date '+%y%m%d %T' )

logprint "${DATE} dmail"
chmod ugo+rw $LOGFILE

logprint path=${PATH}


: ${LOCAL:=/usr/add-on/local}
: ${PCS:=/usr/add-on/pcs}
export LOCAL PCS


FPATH=${LOCAL}/fbin
export FPATH


PATH=/bin
export PATH


addpath PATH /bin f

addpath PATH ${PCS}/bin
addpath PATH ${LOCAL}/bin f


addpath LD_LIBRARY_PATH ${PCS}/lib
addpath LD_LIBRARY_PATH ${LOCAL}/lib f

export PATH LD_LIBRARY_PATH

logprint path=${PATH}
logprint lib=${LD_LIBRARY_PATH}

echo "dmail called" | logger -p mail.notice


PROG_DMAIL=${PCS}/bin/dmail
if [[ ! -x ${PROG_DMAIL} ]] ; then
  PROG_DMAIL=/usr/extra/sbin/dmail.s5
  logprint using other version
fi


export DMAIL_DEBUGFD=3
exec execname $PROG_DMAIL dmail -D=5 2> $E 3> $D "${@}"



