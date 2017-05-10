# program to copy files from a STARLAN server RJE area to a local RJE

# Dave Morano, 02/22/91


# called as :
#
#	slget [-b] [-r] [-v] [-n share_name] [file(s)]
#

# configurables

DEFSERVER=mtsvb				# default server machine

PCCONV=pcconv
SLGETD=/usr/spool/sft/bin/slgetd


# start of program from here on

USAGE="${0} usage: ${0} [-b] [-r] [-v] [file1(s)]"

if [ -x /bin/arch ] ; then

  MACH=`hostname `

else

  MACH=`uname -n `

fi

if [ "$#" -lt 1 ] ; then

  echo "${0}: not enough arguments given" >&2
  echo ${USAGE} >&2
  exit 1

fi

TMPDIR="/tmp/sl${$}"

F_BIN=false
f_RM=false
FILES=""
SERVER=${DEFSERVER}
SHARENAME=""

for A in $@ ; do

  case $A in

  '-b' )
    F_BIN="true"
    ;;

  '-n' )
    SHARENAME=${A}
    ;;

  '-s' )
    SERVER=${A}
    ;;

  '-r' )
    F_RM=true
    ;;

  '-v' | '-V' )
    echo "${0} version: 0" >&2
    ;;

  '-'* )
    echo "${0}: unrecognized option (ignored)" >&2
    ;;

  * )
    FILES="${FILES} ${A}"
    ;;

  esac

done

if [ -z "${FILES}" ] ; then exit 0 ; fi


OPT=""
if [ "${F_BIN}" = "true" ] ; then OPT="${OPT} -b" ; fi

if [ "${F_RM}" = "true" ] ; then OPT="${OPT} -r" ; fi

XFILES=""
for FP in $FILES ; do

  F=`basename $FP `
  XFILES="${XFILES} ${F}"

done

#uux ${SERVER}!${SLGETD} ${OPT} -n ${SHARENAME} ${MACH} ${LOGNAME} ${XFILES}
uux ${SERVER}!${SLGETD} ${OPT} ${MACH} ${LOGNAME} ${XFILES}

exit 0


