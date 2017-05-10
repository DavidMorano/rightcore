#!/usr/bin/ksh
# MAILSHOW


#
# Synopsis:
#
#	mailshow
#
#


PN=mailshow


: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${PCS:=/usr/add-on/pcs}
: ${TOOLS:=/usr/add-on/exptools}
export LOCAL NCMP PCS TOOLS


FPATH=${LOCAL}/fbin
if [[ ! -d "${FPATH}" ]] ; then
  FPATH=${NCMP}/fbin
fi
if [[ ! -d "${FPATH}" ]] ; then
  FPATH=${PCS}/fbin
fi
export FPATH


pathadd PATH ${LOCAL}/bin
pathadd LD_LIBRARY_PATH ${LOCAL}/lib

pathadd PATH ${NCMP}/bin
pathadd LD_LIBRARY_PATH ${NCMP}/lib

pathadd PATH ${PCS}/bin
pathadd LD_LIBRARY_PATH ${PCS}/lib


: ${USERNAME:=$( username )}
export USERNAME

: ${MAILDIR:=/var/mail}
export MAILDIR

: ${MAIL:=${MAILDIR}/${USERNAME}}
export MAIL


DN=/dev/null
if [[ -s "${MAIL}" ]] ; then
  if whence frm > ${DN} ; then
    frm | cut -c 1-80
  fi
fi


