 # <-- force CSH to use Bourne shell
# NEWMAIL


: ${LOGNAME:=${USER}}
: ${HOME:=/home/${LOGNAME}}


MAILBOX=${HOME}/mail/new


if [ -n "${1}" ] ; then

  while [ -n "${1}" ] ; do

    if [ -r "${1}" ] ; then

      cat $1 >> $MAILBOX

    else

      echo "${0}: could not read file \"${1}\"" >&2

    fi

    shift

  done

else

  cat >> $MAILBOX

fi



