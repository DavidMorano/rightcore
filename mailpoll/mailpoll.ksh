#!/usr/bin/ksh
# MAILPOLL


: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${PCS:=/usr/add-on/pcs}
: ${EXTRA:=/usr/extra}

export LOCAL NCMP PCS EXTRA


FPATH=${LOCAL}/fbin
export FPATH

addpath PATH ${NCMP}/bin f
addpath PATH ${LOCAL}/bin f

addpath PATH ${PCS}/bin
addpath PATH ${EXTRA}/bin


addpath LD_LIBRARY_PATH ${NCMP}/lib f
addpath LD_LIBRARY_PATH ${LOCAL}/lib f

addpath LD_LIBRARY_PATH ${PCS}/lib
addpath LD_LIBRARY_PATH ${EXTRA}/lib



pcsconf -pl mailpoll mailpoll

YESFILE=${PCS}/var/mailpoll/yes
if [ ! -r ${YESFILE} ] ; then
  exit 0
fi

if fileolder $YESFILE 50s ; then

  touch $YESFILE
  requestmail

  {
  sleep 20
  addpath PATH /usr/lib/uucp/
  uusched
  } &

fi



