#!/usr/extra/bin/ksh
# POSTDELIVER


  RF_RUNBF=true

#
# environment variables passed from PostFix:
#
# SHELL		recipient user shell
# HOME		recipient user home directory
# USER		bare recipient user name
# LOGNAME	bare recipient user name
# EXTENSION	optional extension on recipinent address
# DOMAIN	domain of recipient
# LOCAL		entire recipient user address (bare + extension)
# RECIPIENT	entire recipinent address (bare + extension @ domain)
# SENDER	entire sender address (also in mail message envelope?)
#

#
# The 'HOME' variable is that of the sending user, but the rest are
# of the recipient (which in theory should be the same as the sending
# user) which is differentiated from the sender within POSTFIX for
# whatever strange reason anyone can think of.  So we differentiate
# the two entities here also.
#

PF_SHELL=${SHELL}
PF_HOME=${HOME}
PF_USER=${USER}
PF_LOGNAME=${LOGNAME}
PF_EXTENSION=${EXTENSION}
PF_DOMAIN=${DOMAIN}
PF_LOCAL=${LOCAL}
PF_RECIPIENT=${RECIPIENT}
PF_SENDER=${SENDER}

unset SHELL USER LOGNAME EXTENSION DOMAIN LOCAL RECIPIENT SENDER


: ${POSTFIX:=/usr/postfix}
: ${LOCAL:=/usr/add-on/local}
: ${PCS:=/usr/add-on/pcs}
: ${EXTRA:=/usr/extra}
export POSTFIX LOCAL PCS EXTRA

PRS=" ${POSTFIX} ${LOCAL} ${PCS} ${EXTRA} "

if [[ "${FPATH:0:1}" == ":" ]] ; then
  FPATH=${FPATH:1:200}
fi

for PR in ${PRS} ; do
  if [[ -d ${PR} ]] ; then
    B=${PR}/fbin
    if [[ -d ${B} ]] ; then
      if [[ -n "${FPATH}" ]] ; then
        FPATH="${FPATH}:${B}"
      else
        FPATH=${B}
      fi
    fi
  fi
done
export FPATH

if [[ -d "${POSTFIX}" ]] ; then
  PR=${POSTFIX}
  pathadd PATH ${PR}/bin
  pathadd PATH ${PR}/sbin
  pathadd LD_LIBRARY_PATH ${PR}/lib
fi

for PR in ${PRS} ; do
  pathadd PATH ${PR}/bin
  pathadd LD_LIBRARY_PATH ${PR}/lib
done

for PR in ${PRS} ; do
  pathadd PATH ${PR}/sbin
done


if [[ -d ${PCS} ]] ; then
  DMAIL_PROGRAMROOT=${PCS}
fi
: ${DMAIL_PROGRAMROOT:=${EXTRA}}

if [[ -d ${LOCAL} ]] ; then
  ISMAILADDR_PROGRAMROOT=${LOCAL}
fi
: ${ISMAILADDR_PROGRAMROOT:=${EXTRA}}

export DMAIL_PROGRAMROOT ISMAILADDR_PROGRAMROOT

PN=postdeliver
BF=bogofilter
PV=0
TF=/tmp/pd${$}

DN=/dev/null
LOGFILE=${POSTFIX}/log/${PN}

logfile -c ${LOGFILE} -n ${PN}:${PV} -s 10m |&

function logprint {
  typeset V="${1}"
  if [[ -n "${V}" ]] ; then
    print -p -- "${V}"
  fi
}

function cleanup {
  rm -f ${TF}
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17

logprint "user=>${PF_USER}<"
logprint "sender=>${PF_SENDER}<"
logprint "recipient=>${PF_RECIPIENT}<"

CF=${POSTFIX}/etc/${BF}.cf
BOGOFILTER_DIR=${POSTFIX}/var/${BF}
export BOGOFILTER_DIR

EX=67
if [[ -n "${PF_USER}" ]] ; then

  P_IMA=ismailaddr
  logprint "sender=>${PF_SENDER}<"

  if [[ -n "${PF_SENDER}" ]] && haveprogram ${P_IMA} ; then
    logprint "checking ISMSG"
    ERR=/tmp/ismailaddr.err
    chmod ugo+rw ${ERR} 2> ${DN}
    if [[ ${RF_RUNBF} ]] ; then
      if [[ -w ${ERR} ]] ; then
        if ${P_IMA} -o log -D 2> ${ERR} "${PF_SENDER}" ; then
          RF_RUNBF=false
        fi
      else
        if ${P_IMA} -o log "${PF_SENDER}" ; then
          RF_RUNBF=false
        fi
      fi
    fi
  fi
    logprint "runbogo=${RF_RUNBF}"

  if ${RF_RUNBF} && pt -qe ${BF} && [[ -d ${BOGOFILTER_DIR} ]] ; then
    logprint "bogofilter"
    ${BF} -c ${CF} -d ${BOGOFILTER_DIR} -p > ${TF}
    EX=$?
    logprint "bogofilter ex=${EX}"
    logprint "dmail ${PF_USER}"
    dmail < ${TF} -p POSTFIX "${PF_USER}"
    EX=$?
    logprint "dmail ex=${EX}"
  else
    logprint "dmail ${PF_USER}"
    dmail -p POSTFIX "${PF_USER}"
    EX=$?
    logprint "dmail ex=${EX}"
  fi

fi # end if (had a user)

    logprint "%T exiting ex=${EX}"
cleanup

return ${EX}


