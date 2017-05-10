# program to copy files to a STARLAN server RJE area

# Dave Morano, 03/20/91


# configurables

DEFSERVER=mtsvb				# default server machine
DEFSHARENAME=netspool

PCCONV=pcconv
SLPUTD=/usr/spool/sft/bin/slputd


# start of program from here on

USAGE="usage: slput [-vr] [-u user] file1 [file2 [...]]"


if [ "$#" -lt 1 ] ; then

  echo "${0}: not enough arguments given" >&2
  echo ${USAGE} >&2
  exit 1

fi

TMPDIR="/tmp/sl${$}"

F_BIN=false
UUXOPTS=""
FILES=""
SERVER=${DEFSERVER}
SHARENAME="${DEFSHARENAME}"

for A in $@ ; do

  case $A in

  '-b' )
    F_BIN=true
    ;;

  '-c' | '-C' )
    UUXOPTS="-C"
    ;;

  '-s' )
    SERVER=${A}
    ;;

  '-n' )
    SHARENAME=${A}
    ;;

  '-v' | '-V' )
    echo "${0} version: 0"
    ;;

  '-'* )
    echo "unrecognized option (ignored)" >&2
    ;;

  * )
    FILES="${FILES} $A"
    ;;

  esac

done

if [ -z "${FILES}" ] ; then exit 0 ; fi


if [ -z "${LOGNAME}" ] ; then LOGNAME="rje" ; fi


if [ "$F_BIN" = "true" ] ; then

  for FP in $FILES ; do

      F=`basename $FP `
      uux ${UUXOPTS} ${SERVER}!${SLPUTD} !${FP} ${SHARENAME} ${LOGNAME} ${F}

  done

else

  mkdir ${TMPDIR}
  NFILES=""
  for FP in $FILES ; do

    if [ -f $FP ] ; then

      F=`basename $FP `
      eval ${PCCONV} $FP > ${TMPDIR}/${F}
      NFILES="${NFILES} ${F}"

    else

      echo "${0}: file \"${FP}\" was not found" >&2
     
    fi

  done

  if [ -n "${NFILES}" ] ; then

    cd ${TMPDIR}
    for F in $NFILES ; do

      uux -C ${SERVER}!${SLPUTD} !${F} ${SHARENAME} ${LOGNAME} ${F}

    done

  fi

  cd
  rm -fr ${TMPDIR}

fi

exit 0


