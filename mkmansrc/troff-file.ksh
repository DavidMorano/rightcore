#!/usr/bin/ksh
# TROFF-FILE

RF_DEBUG=false


if $RF_DEBUG ; then
  exec 3> troff-file.deb
fi


function dprint {
  if $RF_DEBUG ; then
    print -u3 -- $*
  fi
}


dprint PWD=${PWD}
dprint pwd=$( pwd )

TYPE=
MP=

while getopts T:m: key ; do

  case $key in

  'T' )
    TYPE=${OPTARG}
    ;;

  'm' )
    MP=${OPTARG}
    ;;

  '?' )
    echo "${0}: unknown argument option" >&2
    exit 2
    ;;

  esac

done

dprint TYPE=${TYPE}
dprint MP=${MP}

shift $(( $OPTIND - 1 ))

dprint OPTIND=${OPTIND}

FILES="${*}"
dprint "FILES=>${FILES}<"

if [[ -n "${FILES}" ]] ; then

  integer c=0
  for F in $FILES ; do
    if [[ "${F}" == '-' ]] ; then
      cat > file${c}.troff
    else
      cp $F file${c}.troff
    fi
    (( c += 1 ))
  done

else

  cat > file0.troff

fi



