#!/usr/bin/ksh
# MSGBOX (send a mesbox instant message)

#
# Dave Morano, 99//06/01
#	This program was originally written.
#


# Arguments :
#
#	msgbox [-f from] [-c comment] recepient(s) [...] [-u mailuser]
#
#


MAILUSER=dam



: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${PCS:=/usr/add-on/pcs}
export LOCAL NCMP PCS





# configurable parameters

unset ENV
unalias cd


# OK to proceed

PROG_FGREP=/bin/fgrep



haveprog() {
  ES1=1
  $PROG_WHICH $1 | ${PROG_FGREP} "no " | ${PROG_FGREP} "in " > /dev/null
  ES=$?
  if [ $ES -eq 0 ] ; then ES1=1 ; else ES1=0 ; fi
  return $ES1
}

MACH=$( uname -n )


# start of program stuff

P=$( basename ${0} )


#	msgbox [-f from] [-c comment] recepient(s) [...] [-u mailuser]

RECEPIENTS="dam@rca dam@rcb"
MAILUSER=dam
FROM=""
COMMENT=""

F_DEBUG=false

S=recepients
OS=""
for A in "${@}" ; do

  case $A in

  '-c' )
    OS=${S}
    S=comment
    ;;

  '-u' )
    OS=${S}
    S=mailuser
    ;;

  '-f' )
    OS=${S}
    S=from
    ;;

  '-D' )
    F_DEBUG=true
    ;;

  '-'* )
    echo "${P}: unknown option \"${A}\" ignored" >&2
    ;;

  * )
    case $S in

    comment )
      COMMENT=${A}
      S=${OS}
      ;;

    recepients )
      RECEPIENTS="${RECEPIENTS} ${A}"
      ;;

    from )
      FROM=${A}
      S=${OS}
      ;;
      
    mailuser )
      MAILUSER=${A}
      S=${OS}
      ;;
      
    esac
    ;;

  esac

done




PATH=${LOCAL}/bin:${PATH}:${NCMP}/bin:${PCS}/bin


keyauth

: ${LOGNAME:=$( logname )}

TF=/tmp/msgbox${$}


trap 'cleanup ; exit 1' 1 2 3 15 16 17

cleanup() {
  rm -f $TF
}





# get it local and clean it while we're getting it!
sanity | textclean > $TF

if [ -s $TF ] ; then

  # pop it to any logged-in sessions
#  rmsg $RECEPIENTS < $TF
  notice dam morano < $TF
  
  OPTS=""
  FROMLINE=""
  if [ -n "${FROM}" ] ; then
    OPTS="-f ${FROM}"
    FROMLINE="${FROMLINE} ${FROM}"
  fi
  
  # save it permanently
  {
    if [ -n "${COMMENT}" ] ; then
      FROMLINE="${FROMLINE} (${COMMENT})"
    fi
    echo "From: ${FROMLINE}"
    echo
    cat $TF
  } | deliver $OPTS $MAILUSER -M msgbox -m msgbox

  F_GO=1
  if haveprog audioavail ; then
    F_GO=0
    if audioavail ; then
      F_GO=1
    fi
  fi
  if [ $F_GO -ne 0 ] ; then
#    audial -audio rca:0 -volume 50 -spacing 300 -duration 300 12 &
    audioplay -v 10 ${PCS}/lib/sounds/shark.au
  fi
fi

cleanup



