#!/bin/ksh


VALS=$( loginfo - uid euid gid egid )

(( c = 0 ))
for V in $VALS ; do

  case $c in

  0 )
    echo "uid=${V}"
    ;;

  1 )
    echo "euid=${V}"
    ;;

  2 ) 
    echo "gid=${V}"
    ;;

  3 )
    echo "egid=${V}"
    ;;

  esac

  (( c += 1 ))

done



