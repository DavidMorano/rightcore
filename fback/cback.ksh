#!/usr/extra/bin/ksh
# CBACK
# script to clean up (remove) old backup save set files

# calling synopsis
#
#	cback bk_table_file
#
# If no back table is specified then the back table defined by the DEFBKTAB
# variable below is used instead.
#


P_BURN=filerm

DN=/dev/null
N=/dev/null
if whence builtin > ${DN} && [[ ! -r "${ENV}" ]] ; then
  ENV=${HOME}/.kshrc
  if [[ -r "${ENV}" ]] ; then
    . ${ENV}
    export ENV
  fi
fi


# configurable defaults

NFDAYS=10
NIDAYS=10


: ${LOCAL:=/usr/add-on/local}
export LOCAL


# backup table file format
#
#	The input file consists of lines of two columns each.  The first
#	column contains the files to be backed up.  The second column contains
#	a directory name that is used to place the backup archive file in.
#
#	file-to-be-backed-up		directory-to-put-in
#
#	Lines beginning with a "#" character are treated as comments.
#	Lines which have a "-" character as a file specification are
#	treated as no-operation lines.
#

# start of program

BN=${0##*/}
PN=${BN%.*}
RF_RFS=true
if [[ ! -d /usr/sbin ]] || [[ ! -d /etc/nserve ]] ; then
  RF_RFS=false
fi

if [ -d /usr/5bin -a ! -d /usr/sbin ] ; then

  echo ${PATH} | fgrep '/usr/5bin' > ${DN}
  if [[ $? -ne 0 ]] ; then PATH=/usr/5bin:${PATH} ; fi

fi


  echo ${PATH} | fgrep ${LOCAL}/bin > ${DN}
  if [[ $? -ne 0 ]] ; then
    PATH=${PATH}:${LOCAL}/bin
  fi


TMPFILE=/tmp/cb_${$}
: ${USERNAME=$( username )}
export USERNAME

RF_MOUNTCHECK=false
TABHOME=${HOME}
case ${USERNAME} in

root )
  TABHOME=$( userhome admin )
  RF_MOUNTCHECK=${RF_RFS}
  ;;

esac

function mountcheck {
  typeset C P_RMNTTRY P_RFSCHECK
  P_RMNTTRY=/usr/nserve/rmnttry 
  P_RFSCHECK=/usr/admin/bin/rfscheck
  if [[ -x "${P_RMNTTRY}" ]] && [[ -x "${P_RFSCHECK}" ]] ; then
    C=0
    while [[ ! -d ${TO} ]] ; do

      if [[ $C -eq 0 ]] ; then /usr/nserve/rmnttry ; fi

      if [[ $C -eq 5 ]] ; then /usr/nserve/rmnttry ; fi

      /usr/admin/bin/rfscheck
      sleep 90
      C=$( expr $C + 1 )
      if [[ $C -ge 10 ]] ; then break ; fi

    done
  fi
  return 0
}

cleanup() {
  rm -f ${TMPFILE}
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

DEFBKTAB=${TABHOME}/etc/bktab/default

DATE=$( date '+%m/%d %H:%M' ) 
if [[ $# -eq 1 ]] && [[ -n "${1}" ]] ; then 
  BKTAB=${1} 
else 
  BKTAB=$DEFBKTAB 
fi

if [[ ! -f ${BKTAB} ]] ; then

  print -u2 -- "${PN}: ${DATE} backup table file not found"
  exit 1

fi

print -- "${DATE} cleaning up old backups \"${BKTAB}\""

while read FROM TO J ; do

  case ${FROM} in

  '#'* | '' | '-' )
    continue
    ;;	

  esac 

  if [[ -z "${TO}" ]] || [[ "${TO}" == "-" ]] ; then continue ; fi

# try to insure that the backup medium is online before continuing

  if ${RF_MOUNTCHECK} ; then
    mountcheck
  fi

  if [[ -d "${TO}" ]] && cd ${TO} ; then

    fileop -pm filerm -im -o older=${NIDAYS}d -t f i*
    fileop -pm filerm -im -o older=${NIDAYS}d -t d i*

    fileop -pm filerm -im -o older=${NFDAYS}d -t f f*
    fileop -pm filerm -im -o older=${NFDAYS}d -t d f*

  else

    print -u2 -- "${PN}: directory ${TO} does not exist or is unaccessable"

  fi

done < ${BKTAB}

cleanup



