#!/usr/extra/bin/ksh
# WEATHER

PV=0
SF=KBOS.TXT
RF_PINGSTAT=0

PDIR=${0%/*}
A=${0##*/}
PFILE=${A%.*}
PN=${PFILE#-}
DN=/dev/null
SN=${PN}

: ${HOME:=$( userhome )}
: ${LOCAL:=/usr/add-on/local}
: ${GNU:=/usr/add-on/gnu}
: ${EXTRA:=/usr/extra}
export HOME LOCAL GNU EXTRA

PRS=" ${HOME} ${LOCAL} ${GNU} ${EXTRA} "

if [[ "${FPATH:0:1}" == ":" ]] ; then
  FPATH=${FPATH:1:200}
fi

for PR in ${PRS} ; do
  if [[ -d ${PR} ]] ; then
    FBIN=${PR}/fbin
    if [[ -d ${FBIN} ]] ; then
      if [[ -n "${FPATH}" ]] ; then
        FPATH="${FPATH}:${FBIN}"
      else
        FPATH="${FBIN}"
      fi
    fi
  fi
done
export FPATH

for PR in ${PRS} ; do
  pathadd PATH ${PR}/bin
  pathadd LD_LIBRARY_PATH ${PR}/lib
done

for PR in ${PRS} ; do
  pathadd PATH ${PR}/sbin
done


#print query >&3

Q=
if [[ -n "${1}" ]] ; then
  Q=$( print ${1} | cut -d . -f 1 | tr '[A-Z]' '[a-z]' )
fi

#print Q=${Q} >&3

SF=KBOS.TXT
if [[ -n "${Q}" ]] ; then
  SF=$( print -- ${Q} | tr '[a-z]' '[A-Z]' ).TXT
fi

#print SF=${SF} >&3

WEATHERHOST=tgftp.nws.noaa.gov
URL=http://${WEATHERHOST}/data/observations/metar/decoded/${SF}


SPOOLDIR=${LOCAL}/var/spool/${SN}

if [[ ! -d ${SPOOLDIR} ]] ; then
  mkdir ${SPOOLDIR}
  chmod 777 ${SPOOLDIR}
fi

LOGFILE=${LOCAL}/log/${SN}

: ${NODE:=$( uname -n )}

JID=$( print "${NODE}${$}             " | cut -c 1-14 )
DATE=$( date '+%y%m%d_%T' )

function logfile {
  print -- "${JID}\t${*}" >> ${LOGFILE}
}


TF=${SPOOLDIR}/tf${NODE}${$}
TF2=${SPOOLDIR}/tf2${NODE}${$}

function cleanup {
  rm -f ${TF} ${TF2}
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

logfile "${DATE} ${SN} ${PV}"

logfile "query=${Q}"

#print query=${Q} >&3


F=1
if [[ "${RF_PINGSTAT}" != 0 ]] ; then
  F=0
  if pingstat ${WEATHERHOST} -n ; then
    F=1
  fi
fi


if fileolder ${SPOOLDIR}/${SF} -60m && [[ "${F}" != 0 ]] ; then
  wget -q -T 30 -O ${TF2} ${URL}
  RS=${?}
  if [ $RS -eq 0 ] ; then
#    print executed_OK >&3
    if [ -s $TF2 ] ; then
#      print non_zero_wget >&3
      linefold $TF2 > $TF

#      weathertime ${TF}
      TT=$( date '+%y%m%d%H05' )
      touch -t $TT ${TF}

      chmod ugo+rw ${TF}
      mv ${TF} ${SPOOLDIR}/${SF}
      logfile "refreshed"
    else :
#      print zero_wget >&3
    fi
  fi
#  print executed_rs=${RS} >&3
fi

if [[ -s ${SPOOLDIR}/${SF} ]] ; then
#      print non_zero_cat >&3
  cat ${SPOOLDIR}/${SF}
else
  print "invalid station code or network is busy"
fi

cleanup


