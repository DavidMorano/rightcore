#!/usr/bin/ksh
# DISFPATH


BASE=pathenum
PROG=
if [[ -n "${OFF}" ]] ; then
  PROG=${BASE}.${OFF}
fi
  
if [[ -n "${PROG}" ]] && whence $PROG > /dev/null ; then

  $PROG FPATH -es "*PWD*"

else

  CUT=/bin/cut
  FGREP=/bin/fgrep

  F=0
  I=1
  print ${FPATH} | ${FGREP} ':' > /dev/null
  if [ $? -eq 0 ] ; then

    while [ $I -lt 100 ] ; do

      A=$( print $FPATH | ${CUT} -d ':' -f ${I} )
      if [ -n "${A}" ] ; then

        if [ $F -gt 0 ] ; then 
          echo "*PWD*"
        fi

        echo $A
        F=0

      else

        F=$( expr $F + 1 )
        if [ $F -gt 1 ] ; then 
          exit 0 ; 
        fi

      fi

      I=$( expr $I + 1 )

    done

  else

    echo $FPATH

  fi

fi



