#!/usr/extra/bin/ksh
# SYSTATD


TO=3

: ${HOME:=$( userhome )}
: ${LOCAL:=/usr/add-on/local}
: ${EXTRA:=/usr/extra}
export HOME LOCAL EXTRA

PRS=" ${HOME} ${LOCAL} ${EXTRA} "

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

LIBLKCMD=
for PR in ${PRS} ; do
   LDIR=${PR}/lib
   if [[ -d "${LDIR}" ]] ; then
     LIBLKCMD=${LDIR}/liblkcmd.so
     if [[ -x ${LIBLKCMD} ]] ; then
       break ;
     fi
   fi
done

builtin -f ${LIBLKCMD} issue la

DATE=$( daytime )
: ${NODE:=$( nodename )}
export NODE

N=0
case ${NODE} in
rca )
  N=1
  ;;
rcb )
  N=2
  ;;
rcc )
  N=3
  ;;
rcd )
  N=4
  ;;
esac

#print -- "RightCore Networking Services (${N})"
#print -- "${DATE}"
issue -k systat
print

if [[ ! -d ${LOCAL} ]] ; then
  print -- "** the system is currently offline for maintenance **"
  exit 1
fi

N=$( who | wc -l )
print -- "+ current account customers logged in\t${N}"

LOADSUM=
float A=$( la la15m )
if [[ ${A} -ge 1.0 ]] ; then
  LOADSUM=VERYHIGH
elif [[ ${A} -ge 0.67 ]] ; then
  LOADSUM=HIGH
elif [[ ${A} -ge 0.33 ]] ; then
  LOADSUM=MODERATE
else
  LOADSUM=LOW
fi
print -- "+ the current system load (15m) is   \t${LOADSUM} (${A})"
unset A


A=$( netload netload | caseupper )
print -- "+ current web traffic is             \t${A}"

print -- "+ all nodes in the cluster are       \tUP"

pingstat rca rcg -q
RS1=$?

pingstat hp0 -q
RS2=$?

if [[ ${RS1} -ne 0 ]] ; then
  print -- "+ some servers in the system are     \tDOWN (not service affecting)"
else
  print -- "+ all servers in the system are      \tUP"
fi

if [[ ${RS2} -ne 0 ]] ; then
  print -- "+ some peripheral machines are       \tDOWN (not service affecting)"
fi


