 # <-- force CSH to use Bourne shell
# CAPCLEAN


if [ $# -lt 1 ] ; then

  echo "${0}: not enough arguments" >&2
  exit 1

fi

O="-type f -print"
find "${@}" $O | egrep '\.finderinfo|\.resource' | while read F ; do

  R=`dirname $F `
  RR=`dirname $R `
  if [ ! -f ${RR}/${F} ] ; then

    echo $F | egrep '\.finderinfo|\.resource' > /dev/null
    if [ $? -eq 0 ] ; then

     rm -f $F

    fi

  fi

done


