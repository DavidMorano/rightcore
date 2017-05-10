#!/bin/ksh
# RSLOWD (Remote Slow Daemon)

POLLTIME=300

if [ $# -ne 1 -o -z "${1}" ] ; then exit 1 ; fi

LOG=logfile
LOG2=errfile

F_DEBUG=false


# go !

cd ${1}
CWD=`pwd`

#exec > ${LOG} 2> ${LOG}2

chmod 777 q
rm i
mkpipe i
chmod 666 i
touch $LOG $LOG2
chmod 666 $LOG $LOG2
rm -f tmp/*

TF1=tmp/spa${$}
TF2=tmp/spb${$}
TF3=tmp/spc${$}

#while sleep $POLLTIME ; do echo ; done > i &
PATH=${CWD}/bin:${PATH} rslowd_aux 300 > i &

while read I ; do

  if [ $F_DEBUG = true ] ; then

    echo "${0}: top of loop" >&2

  fi

  ls q > $TF1
  if [ -s $TF1 ] ; then

    ls -l q | while read F1 F2 F3 F4 SIZE1 F6 F7 F8 JOBNAME J ; do

      if [ "${F1}" = "total" -o -z "${JOBNAME}" ] ; then continue ; fi

      DATE=` date '+%y%m%d %T' `
      echo "${JOBNAME}\t: start ${DATE}" >> $LOG
      C=0
      while [ $C -lt 4 ] ; do

        sleep 5
        SIZE2=${SIZE1}
        SIZE1=`wc -c q/${JOBNAME} | awk '{ print $1 }' `
        if [ ${SIZE1} -eq ${SIZE2} ] ; then

          C=`expr $C + 1 `

        fi

      done

      SIZE=`wc -c q/${JOBNAME} | awk '{ print $1 }' `

      rm -f $TF2
      mkpipe $TF2
      cp q/${JOBNAME} $TF2 &

      U='*unknown*'
      O='*unknown*'
      M=""
      CMD=""
      F_EXIT=false
      while L=`line` ; do

        HEADER=`echo ${L} | cut -d: -f1 `
        A=`echo ${L} | cut -d: -f2 `

        case "${HEADER}" in

        '#'* )
          ;;

        S* )
          ;;

        A* )
          ;;

        O* | x-orighost* )
          O=`echo ${A}`
          ;;

	U* | x-user* )
          U=`echo ${A}`
          ;;

	M* | from* )
          M=`echo ${A}`
          ;;

	C* | x-service* )
          CMD="${A}"
          ;;

	'' )
          echo $M | fgrep '<' > /dev/null
          if [ $? -eq 0 ] ; then

            A=`echo ${M} | cut -d'<' -f2 `
            M=`echo ${A} | cut -d'>' -f1 `

	  fi

          echo "${JOBNAME}\t: o=${O} - u=${U} - f=${M} - s=${SIZE}" >> $LOG

          if [ -z "${CMD}" ] ; then

            CMD="*null*"
            RS=-1

          else

            eval ${CMD}
            RS=$?
	    cd $CWD

          fi
          echo "${JOBNAME}\t: srv \"${CMD}\"" >> $LOG
          DATE=` date '+%a, %e %b %Y %T %Z' `
          if [ ${RS} -ne 0 -a -n "${M}" ] ; then {

            echo "From:       RSLOW <mtgbcs!dam>"
            echo "To:         ${M}"
            echo "Date:       ${DATE}"
            echo "Subject:    remote execution failure"
            echo
            cat <<-EOF
	Your remotely submitted command :
	    "${CMD}"
	has failed with an exit status of : ${RS}
	Sorry !

	signed,
	"your friendly remote service daemon"
	EOF
            echo
 
          } | rmail ${M} ; fi

          echo "${JOBNAME}\t: end - rs (${RS})" >> ${LOG}
          F_EXIT=true
          ;;

        * )
          ;;

        esac

        if [ $F_EXIT = true ] ; then break ; fi

      done < $TF2 > /dev/null 2> $TF3

      if [ -s $TF3 ] ; then {

        echo "JOB ${JOBNAME} ${DATE}"
        cat $TF3 
        echo "END ${JOBNAME}"

      } >> $LOG2 ; fi

      rm -fr q/${JOBNAME} $TF2 $TF3

    done

  fi

done < i


