 # <-- force CSH to use Bourne shell
# PATHTO - prints path to argument (executable command) via searching PATH
# 
#

USAGE="USAGE: pathto [-s] <command-name>"

if [ $# -lt 1 ] ; then

  echo "${0}: argument required" >&2
  echo ${USAGE} >&2
  exit 1

fi

if [ -z "${1}" -o -z "${PATH}" ] ; then exit 1 ; fi

CWD=${PWD:-`pwd`}

RS=1
IFS=:
for I in ${PATH} ; do

  if [ -z "${I}" ] ; then

    if [ -x "${1}" ] ; then

      echo "${CWD}/${1}"
      RS=0

    fi

  else

    if [ -x "${I}/${1}" ] ; then

      echo "${I}/${1}"
      RS=0

    fi

  fi

done

exit $RS


