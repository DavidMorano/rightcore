# daemon program for the 'slget' command

# Dave Morano, 02/22/91


# called as :
#
#	slgetd [-b] [-r] mach user [-n share_name] [file(s) ...]
#

# configurables

SFTBIN=/usr/spool/sft/bin
PATH=${PATH}:${SFTBIN}

DEFSHAREDIR=/sv10/netspool/rje
LOG=/usr/spool/sft/log
ERRHELLO=/usr/spool/sft/errhello

PCCONV=${SFTBIN}/pcconv


# start of program from here on

if [ "$#" -lt 3 ] ; then

  exit 1

fi

TMPDIR="/tmp/sl${$}"
TMPFILE="/usr/tmp/sl${$}"

F_BIN=false
F_RM=false
FILES=""
SHARENAME=""

OS=""
S=mach
N=${#}
for A in $@ ; do

  case $A in

  '-b' )
    F_BIN=true
    ;;

  '-n' )
    OS=${S}
    S=sname
    ;;

  '-r' )
    F_RM=true
    ;;

  * )
    case $S in

    mach )
      DSTMACH=${A}
      S=user
      ;;

    user )
      DSTUSER=${A}
      S=files
      ;;

    sname )
      SHARENAME=${A}
      S=${OS}
      ;;

    files )
      FILES="${FILES} $A"
      ;;

    esac
    ;;

  esac
  N=`expr $N - 1 `

done


if [ -z "${FILES}" ] ; then exit 0 ; fi


# find the RJE directory associated with the share_name given

if [ -n "${SHARENAME}" ] ; then

  case $SHARENAME in

  netspool )
    SHAREDIR=/sv10/netspool/rje
    ;;

  net )
    SHAREDIR=/sv10/netspool/rje
    ;;

  * )
    echo "${0}: unrecognized sharename given (fatal)" >&2
    exit 1
    ;;

  esac

else

  SHAREDIR=${DEFSHAREDIR}

fi

UUOPT=""
if [ $F_RM = true ] ; then UUOPT="-C" ; fi

if [ -d "${SHAREDIR}" ] ; then

  cd ${SHAREDIR}
  if [ "$F_BIN" = true ] ; then

    for FP in $FILES ; do

      if [ -r $FP ] ; then

        F=`basename $FP `
        uucp $UUOPT ${FP} ${DSTMACH}!~${DSTUSER}/rje/${F}

      else

        BADFILES="${BADFILES} ${FP}"

      fi

    done

    if [ $F_RM = true ] ; then rm -fr $FILES ; fi

  else

    if [ -x "${PCCONV}" ] ; then

      mkdir ${TMPDIR}
      NFILES=""
      for FP in $FILES ; do

        if [ -f $FP ] ; then

          F=`basename $FP `
          eval ${PCCONV} -f $FP > ${TMPDIR}/${F}
          NFILES="${NFILES} ${F}"

        else

          BADFILES="${BADFILES} ${FP}"

        fi

      done

      if [ $F_RM = true ] ; then rm -fr $FILES ; fi

      if [ -n "${NFILES}" ] ; then

        cd ${TMPDIR}
        uucp -C ${NFILES} ${DSTMACH}!~${DSTUSER}/rje

      fi

      cd
      rm -fr ${TMPDIR}

    else

      DATE=`date '+%m/%d %T'`
      echo "${DATE} slgetd: could not find 'pcconv' program" >> $LOG
      LOG=junk

    fi

  fi

fi


if [ -w $LOG ] ; then

  DATE=`date '+%m/%d %T'`
  echo "${DATE} UUCP file(s) transfered out - ${@}" >> $LOG

fi


if [ -n "${BADFILES}" ] ; then

        echo "FROM:       STARLAN" > ${TMPFILE}
        echo "DATE:       `date` " >> ${TMPFILE}
        echo "SUBJECT:    file not found" >> ${TMPFILE}

	echo "\n- file transfer request failure -" >> ${TMPFILE}

  if [ -r $ERRHELLO ] ; then

    cat ${ERRHELLO} >> ${TMPFILE}

  fi

        echo "\toriginating machine:\t${DSTMACH}" >> ${TMPFILE}
        echo "\toriginating user:\t${DSTUSER}" >> ${TMPFILE}

        for FP in ${BADFILES} ; do

          echo "\tfile requested:\t\t${FP}" >> ${TMPFILE}

        done

        echo >> ${TMPFILE}

        mail ${DSTMACH}!${DSTUSER} < ${TMPFILE}
        rm ${TMPFILE}

fi

exit 0


