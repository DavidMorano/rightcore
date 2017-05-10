#!/bin/ksh
# RUNADVICE


ADVHIST=.advhist


SF=/tmp/xadvsf${$}
TF1=/tmp/xadvtfa${$}
TF2=/tmp/xadvtfb${$}
TF3=/tmp/xadvtfc${$}
TF4=/tmp/xadvtfd${$}
TF5=/tmp/xadvtfe${$}
RUN_F=/tmp/xadvrf${$}
MAINCKT_F=/tmp/xadvmcf${$}
CONTROLCKT_F=/tmp/xadvccf${$}
OUTFILE_F=/tmp/xadvof${$}

LK=LK_runadvice
PROC=an
TEMP=85
STEP=0.05n
LENGTH=10u
LK_PLOT=LK_plot


# create the lock file quickly
: > ${LK}

cleanup() {
  rm -f $LK $SF $RUN_F $CONTROLCKT_F $MAILCKT_F $TF1 $TF2 $TF3 $TF4
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17


# now we can relax a little

RUN=run.adv
if [ -n "${1}" ] ; then

  RUN=$1

fi

if [ ! -r $RUN ] ; then

  echo "${0}: could not find run file \"${RUN}\"" >&2
  cleanup
  exit 1

fi


# prepare the variables for the Advice run script

case ${PROC} in

slow | SLOW )
  PNAME=SLOW
  PROC=s
  ;;

norm* | NORM* )
  PNAME=NORM
  PROC=n
  ;;

fast | FAST )
  PNAME=FAST
  PROC=f
  ;;

* )
  PNAME=${PROC}
  ;;

esac

MAINCKT=main.adv
CONTROLCKT=control.adv
OUTFILE=main.out


# make the variable substitution script

cat <<-EOF > $SF
	TEMP		${TEMP}
	PROC		${PROC}
	STEP		${STEP}
	LENGTH		${LENGTH}
	OUTFILE		${OUTFILE_F}
	MAINCKT		${MAINCKT_F}
	CONTROLCKT	${CONTROLCKT_F}
EOF

# filter the user's run file (currently 'run_xadv.adv')

varsub -f $SF < $RUN > $RUN_F

# filter the user's control file (currently only 'control.adv')

if [ -r "${CONTROLCKT}" ] ; then 

  varsub -f $SF < $CONTROLCKT > $CONTROLCKT_F

fi


# filter the "control" and "main" ADVICE circuit files, if present

if [ -r "${CONTROLCKT}" ] ; then 

  varsub -f $SF < $CONTROLCKT_F > $TF3 
  mv $TF3 $CONTROLCKT_F 

fi

if [ -r "${MAINCKT}" ] ; then 

  varsub -f $SF < $MAINCKT > $MAINCKT_F

else

  cp $MAINCKT $MAINCKT_F

fi


# remote the SED filter script

rm -f $SF


# OK, ready for ADVICE

rm -f ${ADVHIST}
cp /dev/null ${ADVHIST}
chmod 666 ${ADVHIST}
HISTFILE=${ADVHIST} advice -t 0 < $RUN_F | tee xadv.log
#nice -5 advice -t 0 < $RUN_F | tee xadv.log
#nice -5 tsyscad advice -t 0 < $RUN_F | tee xadv.log
rm -f ${ADVHIST}

if [ -r ${OUTFILE_F} ] ; then cp ${OUTFILE_F} ${OUTFILE} ; fi

if [ -s ${OUTFILE} -a -w all.out ] ; then

  lockfile -t 60 LK_allout
  if [ $? -ne 0 ] ; then

    echo "${0}: could not get lock on ADVICE output file" >&2

  fi

  cat ${OUTFILE} >> all.out

  rm -f LK_allout

fi

if [ -n "${LK_PLOT}" -a -s "${LK_PLOT}" ] ; then :

  PID=`cat ${LK_PLOT}`
  kill -USR1 ${PID} 2> /dev/null
  if [ $? -ne 0 ] ; then rm -f ${LK_PLOT} ; fi

fi


cleanup

echo "\007RUNADVICE run completed"


