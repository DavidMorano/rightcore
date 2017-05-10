#!/usr/bin/ksh
# DELIVER-ALIAS


MU=${1}
if [[ -z "${MU}" ]] ; then
  MU=unknown
fi


: ${LOCAL:=/usr/add-on/local}
: ${GNU:=/usr/add-on/gnu}
: ${PCS:=/usr/add-on/pcs}
: ${EXTRA:=/usr/extra}
export LOCAL GNU PCS EXTRA


  FPATH=${LOCAL}/fbin
if [[ ! -f ${FPATH} ]] ; then
  FPATH=${PCS}/fbin
fi
if [[ ! -f ${FPATH} ]] ; then
  FPATH=${EXTRA}/fbin
fi
export FPATH


pathadd PATH ${LOCAL}/bin
pathadd LD_LIBRARY_PATH ${LOCAL}/lib

pathadd PATH ${GNU}/bin
pathadd LD_LIBRARY_PATH ${GNU}/lib

pathadd PATH ${PCS}/bin
pathadd PATH ${PCS}/sbin
pathadd LD_LIBRARY_PATH ${PCS}/lib

pathadd PATH ${EXTRA}/bin
pathadd LD_LIBRARY_PATH ${EXTRA}/lib


BOGOFILTER_DIR=${GNU}/var/bogofilter

PROTO=MAILALIAS
if pt -e -q bogofilter && [[ -d ${BOGOFILTER_DIR} ]] ; then
  bogofilter -p | dmail -p $({PROTO} ${MU}
  EX=$?
else
  dmail -p ${PROTO} ${MU}
  EX=$?
fi

return $EX



