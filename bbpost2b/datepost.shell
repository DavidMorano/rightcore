#!/bin/ksh
# MTGPOST


: ${PCS:=/usr/add-on/pcs}

VERSION="0a"

F_DEBUG=false


if [ ! -d /usr/sbin ] ; then

  RSH=/usr/ucb/rsh
  MACH=`hostname`
  echo $PATH | fgrep /usr/5bin > /dev/null
  if [ $? -ne 0 ] ; then PATH=/usr/5bin:${PATH} ; fi

else

  RSH=/bin/rsh
  MACH=`uname -n`

fi

P=`basename ${0}`

ORG=`pcsconf organization`


TFA=/tmp/spa${$}
TFB=/tmp/spb${$}

cleanup() {
  rm -f $TFA $TFB
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17


gecosname() {
  U=${LOGNAME}
  if [ $# -gt 0 ] ; then U=${1} ; fi

  L=`grep "^${U}:" /etc/passwd `
  if [ $? -ne 0 ] ; then

    L=`ypmatch $U passwd ` 2> /dev/null
    if [ $? -ne 0 ] ; then return 1 ; fi

  fi

  N=`${ECHO} $L | cut -d: -f5 `

  case "${N}" in

  *'-'*'('* )
    N=`${ECHO} $N | cut -d'-' -f2 `
    N=`${ECHO} $N | cut -d'(' -f1 `
    ;;

  esac

  ${ECHO} $N
  return 0
}
# end function (gecosname)


bbnotus() {
  if [ "${1}" = $MACH ] ; then return 1 ; fi

  if [ -r ${PCS}/etc/bb/bbnames ] ; then

    while read L J ; do

      case "${L}" in

      '#'* | ' '* | '	'* )
        ;;

      * )
        if [ -n "${L}" ] ; then

          if [ $L = $1 ] ; then

            return 1

          fi

        fi
        ;;

      esac

    done < ${PCS}/etc/bb/bbnames

  fi

  return 0
}
# end function (bbnotus)


bbdistribute() {

  DEPOSIT=/usr/add-on/pcs/etc/bin/deposit
  if [ -n "${AFILE}" -a -r "${AFILE}" ] ; then

    if [ -r ${PCS}/etc/bb/bbhosts ] ; then

      while read M J ; do 

        case "${M}" in

        '#'* | ' '* | '	'* )
          ;;

        * )
          if [ -n "${M}" ] ; then

            if bbnotus $M ; then

          QS=/usr/add-on/pcs/spool/rslow
          rslow ${M}!${QS} $DEPOSIT $AFILE < $AFILE

            fi

          fi
	  ;;

        esac

      done < ${PCS}/etc/bb/bbhosts

    fi

  fi

}
# end function (bbdistribute)



OPT_DEBUG=""
if [ $F_DEBUG = true ] ; then

  OPT_DEBUG="-D"

fi

# get newsgroups names from invocation

NGS="starbase.misc"
if [ $# -ge 1 ] ; then

  NGS="${*}"

fi 

# get a meeting notice date from where ever !

MTGDATE="-"



# go with the rest of the program

NEWSMAILER="${ORG} PCS DATEPOST (version ${VERSION})"

C=0
for NG in $NGS ; do

  NGDIR=`pcsngdir $NG `
  NGPATH="${PCS}/spool/boards/${NGDIR}"
  if [ -d $NGPATH -a -w $NGPATH ] ; then
  
    if [ $F_DEBUG = true ] ; then

      echo "${P}: good directory path \"${NGPATH}\"" >&2

    fi
 
    JOBFILE=`pcsjobfile -M ${MTGDATE} $OPT_DEBUG ${NGPATH}`
    if [ $F_DEBUG = true ] ; then

      echo "${P}: jobfile \"${JOBFILE}\"" >&2

    fi
 
    AFILE=${NGPATH}/${JOBFILE}
    if [ $# -eq 1 ] ; then

      if [ $F_DEBUG = true ] ; then

        echo "${P}: about to do it w/ \"${AFILE}\"" >&2
        echo "pcscl -a $JOBFILE -M ${NEWSMAILER} -n ${NG} > $AFILE "

      fi
 
      pcscl -a $JOBFILE -M "${NEWSMAILER}" -n ${NG} > $AFILE

    else

      if [ $C -eq 0 ] ; then

        tee $TFB | pcscl -a $JOBFILE -M "${NEWSMAILER}" -n $NG > $AFILE

      else

        pcscl -a $JOBFILE < $TFB -M "${NEWSMAILER}" -n $NG > $AFILE

      fi

    fi

    bbdistribute ${AFILE}

  else

    if [ $C -eq 0 ] ; then

      cat > $TFB

    fi
    echo "${P}: could not post to newsgroup \"${NG}\"" >&2

  fi

  C=`expr $C + 1 `

done


# clean up

cleanup



