#!/usr/extra/bin/ksh
# CB


: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: {SUNWSPRO:=/opt/SUNWspro}
export LOCAL NCMP SUNWSPRO


#
# All CB programs have problems!!
# The problem is picking the CB that has the least problems
# that we are all willing to live with!!  The older SunOS 4.1.x
# version creates incorrect code and is really intolerable.
# The newer SunOS 5.x version messes up some of the formatting as
# well as messing up code by breaking comment boundaries.
# I think that the least of the problems is currently with the
# newer Sun OS 5.x version so we will use that for now!
# Enjoy!!
#


MAXLINELEN=120

A=${0##*/}
PN=${A%.*}

CB=${LOCAL}/bin/ncb.s5
if [[ -x ${CB} ]] ; then
  ${CB} -l ${MAXLINELEN} "${@}"
else
  CB=${SUNWSPRO}/bin/cb
  if [[ -x ${CB} ]] ; then
    ${CB} -l ${MAXLINELEN} "${@}"
  else
    CB=${LOCAL}/lib/cb/cb.aout
    if [[ -x ${CB} ]] ; then
      ${CB} -l ${MAXLINELEN} "${@}"
    else
      CB=${NCMP}/bin/cb
      if [[ -x ${CB} ]] ; then
        ${CB} -l ${MAXLINELEN} "${@}"
      else
        print -u2 -- "${PN}: could not find software component"
      fi
    fi
  fi
fi


