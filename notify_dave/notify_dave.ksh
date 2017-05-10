#!/usr/bin/ksh
# NOTIFY_DAVE


PROG_WHICH=/bin/which
PROG_FGREP=/bin/fgrep
PROG_BASENAME=/bin/basename


haveprog() {
  ES1=1
  $PROG_WHICH $1 | ${PROG_FGREP} "no " | ${PROG_FGREP} "in " > /dev/null
  ES=$?
  if [ $ES -eq 0 ] ; then ES1=1 ; else ES1=0 ; fi
  return $ES1
}


MACH=$( uname -n )

: ${NAME:=$( logname - name )}

: ${TMPDIR:=/tmp}

TF=${TMPDIR}/nd${RANDOM}${$}

cleanup() {
  rm -f $TF
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17


DATE=$( date '+%y%m%d_%T_%Z' )

{
  echo "Message ${DATE} from ${MACH}!${LOGNAME} (${NAME})"
  cat

} > $TF

chmod go+r $TF
BTF=$( ${PROG_BASENAME} ${TF} )
pcsuucp -g M -C $TF rca!~ftp/msgbox/${BTF}.txt


# take this opportunity to do some popping (good for places like ECE)

if haveprog pcspoll ; then
  pcspoll &
fi


cleanup


