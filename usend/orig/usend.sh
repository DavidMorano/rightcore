 # <-- force CSH to use Bourne shell
# USEND

# revision history:
#
# February 1992
# Dave Morano
#	This program was written from some previous USEND programs
#	that operated in a different networking encironment.
#
#
# May 2000
# Dave Morano
#	Oh has this program been hacked to pieces !
#	And also, how the time flies by !
#	I hacked almost everything in this program out (all of
#	the gateway stuff) and forced it to basically only
#	use the PCS version of the UUCP facility.
#
#



PROG_BASENAME=/bin/basename
PROG_CAT=/bin/cat

PROG_UUCP=${PCS}/bin/pcsuucp
PROG_UUNAME=${PCS}/bin/pcsuuname




UUCP_PATH=/etc/uucp/uupath


P=`${PROG_BASENAME} ${0} `

if [ -x /bin/arch ] ; then
  MACH=`hostname`
else
  MACH=`uname -n`
fi


# program starts here

if [ ! -r $UUCP_PATH ] ; then
  F_RELAY=false
else
  RELAY=`${PROG_CAT} $UUCP_PATH `
  F_RELAY=true
fi


F_UUCP=true


FILES=""
DSTUSER=""
DST=""
F_DEBUG=false
F_NOTE=false
F_MAIL=false
S=files
OS=""
for A in $@ ; do

  case $A in

  '-d' )
    OS=${S}
    S=dst
    ;;

  '-n' )
    F_NOTE=true
    ;;

  '-m' )
    F_MAIL=true
    ;;

  '-u' )
    OS=${S}
    S=user
    ;;

  '-D' )
    F_DEBUG=true
    ;;

  '-'* )
    OPT="${OPT} ${A}"
    ;;

  * )
    case $S in

    files )
      FILES="${FILES} ${A}"
      ;;

    dst )
      DST=${A}
      S=${OS}
      ;;

    user )
      DSTUSER=${A}
      S=${OS}
      ;;

    esac
    ;;

  esac

done


if [ -z "${DSTUSER}" ] ; then

  echo "${P}: user must be specified with the \"-u user\" construct"
  exit 1

fi

if [ -z "${DST}" ] ; then

  echo "${{}: machine destination must be specified \c"
  echo "with the \"-d machine\" construct"
  exit 1

fi



ANS=`${PROG_UUNAME} | while read N J ; do

  if [ $DST = $N ] ; then echo $DST ; fi

done`

OPTS=""
if [ $F_MAIL = true ] ; then OPTS="-m" ; fi

if [ $F_NOTE = true ] ; then OPTS="${OPTS} -n ${DSTUSER}" ; fi

if [ -z "${ANS}" ] ; then

  if [ $F_RELAY = false ] ; then

    echo "${P}: no route to remote host - contact machine administrator"
    exit 1

  fi

  echo $RELAY | fgrep $DST > /dev/null
  if [ $? -eq 0 ] ; then
    ${PROG_UUCP} $OPTS $FILES ${DST}!~${DSTUSER}/rje
  else
    ${PROG_UUCP} $OPTS $FILES ${RELAY}!${DST}!~${DSTUSER}/rje
  fi

else

  ${PROG_UUCP} $OPTS $FILES ${DST}!~${DSTUSER}/rje

fi

exit 0


