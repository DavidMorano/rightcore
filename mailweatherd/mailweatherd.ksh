#!/bin/ksh
# MAILMETARD


: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${PCS:=/usr/add-on/pcs}
: ${DWBHOME:=/usr/add-on/dwb}
: ${GNU:=/usr/add-on/gnu}

export LOCAL NCMP PCS DWBHOME


FPATH=${LOCAL}/fbin
export FPATH


addpath PATH ${NCMP}/bin f
addpath PATH ${LOCAL}/bin f

addpath PATH /usr/bin
addpath PATH ${PCS}/bin
addpath PATH ${DWBHOME}/bin
addpath PATH ${GNU}/bin

export PATH


TF1=/tmp/lnd0${$}
TF2=/tmp/lnd1${$}
TF3=/tmp/lnd2${$}
LND=/tmp/numbered${$}.txt

cleanup() {
  rm -f $TF1 $TF2 $TF3 $LND
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17


cat > $TF1

TO=$( ema $TF1 -h from )
FROM="(weather observation server) metar@rightcore.com"
SUBJ=$( mailhead < $TF1 | fgrep -i subject | line | cut -d : -f 2 )

textclean $TF1 | mailbody > $LND

while read STATION J ; do

  if [ -n "${STATION}" ] ; then
    break
  fi

done < $LND

{
if [ -n "${STATION}" ] ; then
  weather $STATION
else
  print "no station identifier was given"
fi
} > $TF3


DATE=$( date '+%y/%m/%d %T' )
SUBJ="observation from ${STATION}"

{
  print "weather observation job processed ${DATE}"
  print "requested from > ${TO} <"
  echo
  print "your observation query was \"${STATION}\""
  print "your results are below"
  echo
  print "have a nice day !"
  echo
} | mkmsg -s "${SUBJ}" $TO -f "${FROM}" -a txt=${TF3} > $TF2

rmail $TO < $TF2

cleanup



