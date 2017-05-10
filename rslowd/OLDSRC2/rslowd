#!/bin/ksh
# RSLOWD (Remote Slow Daemon)


if [ $# -ne 1 -o -z "${1}" ] ; then exit 1 ; fi

LOG=logfile
LOG2=errfile

F_DEBUG=false


# go !

cd ${1}
CWD=`pwd`

#exec > ${LOG} 2> ${LOG}2

touch $LOG $LOG2
chmod 666 $LOG $LOG2
rm -f tmp/*

TF1=tmp/spa${$}
TF2=tmp/spb${$}
TF3=tmp/spc${$}



      DATE=` date '+%y%m%d %T' `
      echo "${JOBNAME}\tstart ${DATE}" >> $LOG


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

          echo "${JOBNAME}\to=${O} u=${U} f=${M} s=${SIZE}" >> $LOG

          if [ -z "${CMD}" ] ; then

            CMD="*null*"
            RS=-1

          else

            eval ${CMD}
            RS=$?
	    cd $CWD

          fi
          echo "${JOBNAME}\tsrv \"${CMD}\"" >> $LOG
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

          echo "${JOBNAME}\tend - rs (${RS})" >> ${LOG}
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

      rm -fr $TF2 $TF3

    done

exit 0


