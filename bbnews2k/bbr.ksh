#!/usr/bin/ksh
# BBR

# last modified:

#	= 1994-11-18, Dave Morano
#	- enhanced the BB program to not indent the lines
#	  so this program was correspondingly modified to handle it
#
#	= 1996-10-14, Dave Morano
#	- enhanced program to check if the user's HOME
#	  file system is too full or not
#
#



TF=/tmp/tf${$}

cleanup() {
  rm -f $TF
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

P=`basename ${0}`

# is the variable BBMAIL set ?  if not use '${HOME}/mail/bbtemp'

if [ -n "${BBMAIL}" ] ; then
  MBFILE=$BBMAIL
else
  MBFILE=${HOME}/mail/bbtemp
fi

BB=bb
V=`basename ${0} | cut -c1 `
if [ "${V}" = 'n' ] ; then
  BB=nbb
fi

if [ ! -d ${HOME}/mail ] ; then
  mkdir ${HOME}/mail
  echo "${P}: creating \"mail\" directory \"${HOME}/mail\"" >&2
fi


if [ ! -f ${MBFILE} ] ; then
  cp /dev/null ${MBFILE}
fi

TMP=${MBFILE}X
if [ -f ${MBFILE} ] ; then

  cp ${MBFILE} ${TMP}
  trap 'echo "${P}: interrupt - restoring \c"; \
		cp ${TMP} ${MBFILE}; \
		echo ${MBFILE}; rm -f ${TMP}' 1 2 3

else

	trap 'echo "\n${0}: interrupt - removing ${MBFILE}"; \
	rm -f ${MBFILE}' 1 2 3

fi


# check if BB is in our PATH

echo $PATH | fgrep "${PCS}" > /dev/null
if [ $? -ne 0 ] ; then

  PATH=${PATH}:${PCS}/bin

fi


# get new articles if there are any

echo "${P}: searching for new bulletin articles ..."

${BB} $* -int -mailbox > ${TF}

RF_NEWMSGS=false
if [ -s "${TF}" ] ; then

  RF_NEWMSGS=true
  cat $MBFILE $TF > ${MBFILE}.tmp
  A=`wc -c < ${MBFILE} `
  B=`wc -c < ${MBFILE}.tmp `
  echo "${P}: A=${A} B=${B}" >&2
  if [ $B -gt $A ] ; then
    mv ${MBFILE}.tmp ${MBFILE}
  else
    echo "${P}: possibly full HOME file system"
    echo "${P}: new BB articles have been lost (use BB program to recover)"
rm -f $TF ${MBFILE}.tmp
rm -f ${TMP}
    EX=1
  fi

fi


rm -f $TF ${MBFILE}.tmp
rm -f ${TMP}

if [[ ${EX} -eq 0 ]] ; then
  if [[ ${RF_NEWMSGS} == true ]] ; then
    echo "${P}: new bulletins added to mailbox file \"${MBFILE}\""
    EX=2
  else
    echo "${P}: no new bulletins"
    EX=0
  fi
fi

return ${EX}


