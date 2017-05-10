# daemon program for the 'slput' command

# Dave Morano, 02/22/91


# called as :
#
#	slputd [-b] infile sharename user filename
#

# configurables

SFTBIN=/usr/spool/sft/bin
PATH=${PATH}:${SFTBIN}

DEFSHAREDIR=/sv10/netmisc/netspool
LOG=/usr/spool/sft/log

PCCONV=${SFTBIN}/pcconv


# start of program from here on

if [ ! -w $LOG ] ; then LOG=/dev/null ; fi

if [ "$#" -lt 3 ] ; then

  exit 1

fi

TMPDIR="/tmp/sl${$}"
TMPFILE="/usr/tmp/sl${$}"

F_BIN=false
FILENAME=""
SHARENAME=""

OS=""
S=infile
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

  * )
    case $S in

    infile )
      INFILE=${A}
      OS=user
      S=sname
      ;;

    user )
      USER=${A}
      S=file
      ;;

    sname )
      SHARENAME=${A}
      S=${OS}
      ;;

    file )
      FILENAME=${A}
      ;;

    esac
    ;;

  esac
  N=`expr $N - 1 `

done

if [ -z "${USER}" ] ; then USER=rje ; fi

if [ -z "${FILENAME}" ] ; then exit 0 ; fi


# find the SPOOL directory associated with the share_name given

if [ -n "${SHARENAME}" ] ; then

  case $SHARENAME in

  rje | netspool )
    SHAREDIR=/sv10/netmisc/netspool
    ;;

  * )
    DATE=`date '+%m/%d %T'`
    echo "${DATE} ${0}: unrecognized sharename given (fatal)" >> $LOG
    exit 1
    ;;

  esac

else

  SHAREDIR=${DEFSHAREDIR}

fi


if [ -d "${SHAREDIR}" ] ; then

  USERDIR=${SHAREDIR}/${USER}

  if [ ! -d ${USERDIR} ] ; then

    mkdir ${USERDIR}
    chmod +rwx ${USERDIR}
    chgrp DOS---- ${USERDIR}
    chown msnet ${USERDIR}

  fi

  if [ ! -w ${USERDIR} ] ; then

    DATE=`date '+%m/%d %T'`
    echo "${DATE} ${0}: user's spool directory is not writable " >> $LOG
    exit 1

  fi

  if [ ! -w ${USERDIR}/${FILENAME} ] ; then

    if [ -f ${USERDIR}/${FILENAME} ] ; then rm -f ${USERDIR}/${FILENAME} ; fi

    > ${USERDIR}/${FILENAME}
    chmod +rwx ${USERDIR}/${FILENAME}
    chgrp DOS---- ${USERDIR}/${FILENAME}
    chown msnet ${USERDIR}/${FILENAME}

  fi

  cp ${INFILE} ${USERDIR}/${FILENAME}

else

    DATE=`date '+%m/%d %T'`
    echo "${DATE} ${0}: share directory does not exist" >> $LOG
    exit 1

fi

DATE=`date '+%m/%d %T'`
echo "${DATE} UUCP file transfered in - ${@}" >> $LOG

exit 0


