#!/usr/bin/ksh
# PCSGETMAIL


set -x
exec 2> /tmp/pcsgetmail.sherr

: ${HOME:=$( userhome )}
: ${PCS:=/home/pcs}
: ${LOCAL:=/usr/add-on/local}
: ${EXTRA:=/usr/extra}
export HOME PCS LOCAL EXTRA


for A in ${HOME}/fbin ${PCS}/fbin ${LOCAL}/fbin ${EXTRA}/fbin ; do
  if [[ -d "${A}" ]] ; then
    FPATH="${FPATH}:${A}"
  fi
done
export FPATH


pathadd PATH ${HOME}/bin
pathadd LD_LIBRARY_PATH ${HOME}/lib

pathadd PATH ${PCS}/bin
pathadd LD_LIBRARY_PATH ${PCS}/lib

pathadd PATH ${LOCAL}/bin
pathadd LD_LIBRARY_PATH ${LOCAL}/lib

pathadd PATH ${EXTRA}/bin
pathadd LD_LIBRARY_PATH ${EXTRA}/lib


P=pcsgetmail

z /tmp/${P}.err /tmp/${P}.deb

export PCSGETMAIL_DEBUGFILE=/tmp/${P}.deb
execname ${PCS}/bin/pcsgetmail.x pcsgetmail -D=5 2> /tmp/${P}.err "${@}"


