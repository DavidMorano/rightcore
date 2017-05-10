#!/bin/ksh
# LINENUMBERD


: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${PCS:=/usr/add-on/pcs}
: ${DWBHOME:=/usr/add-on/dwb}
: ${GNU:=/usr/add-on/gnu}

export LOCAL NCMP PCS DWBHOME


FPATH=${LOCAL}/fbin
export FPATH


addpath PATH /usr/spg4/bin f
addpath PATH ${NCMP}/bin f
addpath PATH ${LOCAL}/bin f

addpath PATH /bin
addpath PATH ${PCS}/bin
addpath PATH ${DWBHOME}/bin
addpath PATH ${GNU}/bin

export PATH


TF1=/tmp/lnd0${$}
TF2=/tmp/lnd1${$}
TF3=/tmp/lnd2${$}
TF4=/tmp/lnd3${$}
LND=/tmp/numbered${$}.txt
PDF=/tmp/numbered${$}.pdf

cleanup() {
  rm -f $TF1 $TF2 $TF3 $TF4 $LND $PDF
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17


cat > $TF1

TO=$( ema $TF1 -h from )
FROM="(numberer) linenumber@rightcore.com"
SUBJ=$( head -30 $TF1 | fgrep -i subject | line | cut -d : -f 2 )

textclean $TF1 | mailbody | nl -b a > $LND

pr -f -h "${SUBJ}" $LND | textset | troff | dpost -x +0.25 > $TF3

ps2pdf $TF3 $PDF

DATE=$( date '+%y/%m/%d %T' )

{
  echo "line numbering job"
  echo "date processed ${DATE}"
  echo "requested from > ${TO} <"
  echo
  echo "have a nice day !"
  echo
} | mkmsg -s "numbered" $TO -f "${FROM}" -a txt=${LND} -a ${PDF} > $TF2

rmail $TO < $TF2

cleanup



