#!/usr/bin/sh
# UUPOLLD
#
#ident	"@(#)uudemon.poll	1.5	92/07/14 SMI"	
# /* from SVR4 bnu:uudemon.poll 2.6 */

# This shell should be run out of crontab once an hour,
#  a little before  uudemon.hour, since this one
#  does not start the scheduler.



ADDON_ECE=/home/research/dmorano/add-on
ADDON_URI=/marlin/morano/add-on


PROG_WHICH=/bin/which
PROG_DOMAINNAME=/bin/domainname
PROG_FGREP=/bin/fgrep
PROG_CUT=/bin/cut
PROG_UNAME=/bin/uname


getinetdomain() {
  if [ -n "${LOCALDOMAIN}" ] ; then
    for D in ${LOCALDOMAIN} ; do
      echo $D
      break
    done
  else
    if [ -r /etc/resolv.conf ] ; then
      ${PROG_FGREP} domain /etc/resolv.conf | while read J1 D J2 ; do
        echo $D
      done
    else
      ${PROG_DOMAINNAME}
    fi
  fi
}


OS_SYSTEM=`${PROG_UNAME} -s`
OS_RELEASE=`${PROG_UNAME} -r`
ARCH=`${PROG_UNAME} -p`

MACH=`${PROG_UNAME} -n | ${PROG_CUT} -d . -f 1 `

case $MACH in

hammett* )
  DOMAIN=ece.neu.edu
  ;;

* )
  DOMAIN=`getinetdomain`
  ;;

esac


case ${MACH}.${DOMAIN} in

*.uri.edu )
  : ${LOCAL:=${ADDON_URI}/local}
  : ${NCMP:=${ADDON_URI}/ncmp}
  : ${PCS:=${ADDON_URI}/pcs}
  ;;

*.coe.neu.edu )
  : ${LOCAL:=${HOME}}
  : ${NCMP:=${HOME}}
  : ${PCS:=${HOME}}
  ;;

*.ece.neu.edu )
  : ${LOCAL:=${ADDON_ECE}/local}
  : ${NCMP:=${ADDON_ECE}/ncmp}
  : ${PCS:=${ADDON_ECE}/pcs}
  ;;

sparc*.ece.neu.edu )
  ;;

esac

: ${PCS:=/usr/add-on/pcs}
: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
export PCS LOCAL NCMP


case ${OS_SYSTEM}:${OS_RELEASE}:${ARCH} in

SubOS:5.[789]*:sparcu )
  OFD=S5U
  ;;

SunOS:5*:sparc )
  OFD=S5
  ;;

SunOS:4*:sparc )
  OFD=S4
  ;;

OSF*:*:alpha )
  OFD=OSF
  ;;

IRIX*:*:mips )
  OFD=IRIX
  ;;

esac



OFF=`echo $OFD | tr '[A-Z]' '[a-z]' `


addpath() {
  VARNAME=$1
  shift
  if [ $# -ge 1 -a -d "${1}" ] ; then
    eval AA=\${${VARNAME}}
    echo ${AA} | ${PROG_FGREP} "${1}" > /dev/null
    if [ $? -ne 0 ] ; then
      if [ -z "${AA}" ] ; then
          AA=${1}
      else
        if [ $# -eq 1 ] ; then
          AA=${AA}:${1}
        else
          case "${2}" in
          f* | h* )
            AA=${1}:${AA}
            ;;

          * )
            AA=${AA}:${1}
            ;;

          esac
        fi
      fi
      eval ${VARNAME}=${AA}
      export ${VARNAME}
    fi
  fi
  unset VARNAME
}


# add the package area BIN to the user's PATH

addpath PATH ${LOCAL}/bin f
addpath PATH ${NCMP}/bin
addpath PATH ${PCS}/bin
addpath PATH ${HOME}/bin

if [ ! -d /usr/sbin ] ; then PATH=/usr/5bin:${PATH} ; fi


addpath LD_LIBRARY_PATH ${LOCAL}/lib/${OFD}
addpath LD_LIBRARY_PATH ${NCMP}/lib/${OFD}
addpath LD_LIBRARY_PATH ${PCS}/lib/${OFD}
addpath LD_LIBRARY_PATH ${HOME}/lib/${OFD}





# UUPOLLD specific


POLLFILE=${PCS}/etc/uucp/poll
if [ ! -r ${POLLFILE} ] ; then
  exit 1
fi



# This is the sub directory that the 'C.' file will be queued in
DEFQUEUE=Z

# POLLFILE is a list of "system <tab> hour1 hour2 hour3 ..." for polling
# For example 
#	raven	2  6  10
# without the # at the beginning.  Lines starting with # are ignored.

umask 022
set +e

HOUR="`date '+%H'`"
# HOUR="`date | sed -e 's/:.*//' -e 's/^.*\(..\)$/\1/'"

while read site poll ; do

	case $site in
	\#*)	continue;;
	esac

	for i in $poll ; do

		if [ $i -eq $HOUR ] ; then

			pcsuupoll ${site}

			continue 2
		fi

	done

done < ${POLLFILE}



