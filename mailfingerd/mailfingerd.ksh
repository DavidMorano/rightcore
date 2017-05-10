#!/bin/ksh
# MAILFINGERD


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
FROM="(fingerer) finger@rightcore.com"
SUBJ=$( mailhead < $TF1 | fgrep -i subject | line | cut -d : -f 2 )

textclean $TF1 | mailbody > $LND

while read FT J ; do

  if [ -n "${FT}" ] ; then
    break
  fi

done < $LND

if [ -n "${FT}" ] ; then
  rfinger $FT > $TF3
else
  echo "no finger recipient was given" > $TF3
fi


DATE=$( date '+%y/%m/%d %T' )

{
  echo "finger job processed ${DATE}"
  echo "requested from > ${TO} <"
  echo
  echo "your finger query was \"${FT}\""
  echo "your results are below"
  echo
  echo "have a nice day !"
  echo
} | mkmsg -s "fingered" $TO -f "${FROM}" -a txt=${TF3} > $TF2

rmail $TO < $TF2

cleanup



