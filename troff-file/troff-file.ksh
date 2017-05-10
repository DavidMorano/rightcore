#!/bin/ksh
# TROFF-FILE


TYPE=
MP=

while getopts T:m: key ; do

  case $key in

 'T' )
   TYPE=$OPTARG
   ;;

  'm' )
    MP=$OPTARG
    ;;

  ? )
    echo "${0}: unknown argument option" >&2
    exit 2
    ;;

  esac

done

#echo TYPE=${TYPE}
#echo MP=${MP}

shift $(( $OPTIND - 1 ))

#echo FILES="$*"

if [ -n "${FILES}" ] ; then

C=0
for F in $FILES ; do
  cp $F file${C}.troff
  $(( C += 1 ))
done

else

  cat > file0.troff

fi



