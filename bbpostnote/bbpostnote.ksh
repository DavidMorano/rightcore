#!/usr/bin/ksh
# BBPOSTNOTE


P=postnote


if [ -d /usr/sbin ] ; then
  MACH=`uname -n`
else
  MACH=`hostname`
fi

case $MACH in

hocp[a-d] | nucleus | logicgate | nitrogen )
  : ${PCS:=/home/gwbb/add-on/pcs}
  : ${LOCAL:=/home/gwbb/add-on/local}
  : ${NCMP:=/home/gwbb/add-on/ncmp}
  : ${TOOLS:=/opt/exptools}
  ;;

esac

: ${PCS:=/usr/add-on/pcs}
: ${LOCAL:=/usr/add-on/local}
: ${NCMP:=/usr/add-on/ncmp}
: ${TOOLS:=/usr/add-on/exptools}
export PCS LOCAL NCMP TOOLS


: ${BBNEWSDIR:=${PCS}/spool/boards}


# should not have to make any modifications beyond this point

usage() {
  echo "${P}: USAGE> ${P} [msggroups]" >&2
}


# add the package area BIN to the user's PATH

echo $PATH | fgrep ${PCS}/bin > /dev/null
if [ $? -ne 0 ] ; then

  PATH=${PATH}:${PCS}/bin
  export PATH

fi

if [ ! -d /usr/sbin ] ; then PATH=/usr/5bin:${PATH} ; fi

: ${TMPDIR:=/tmp}


TMPFILE=${TMPDIR}/pn${$}

cleanup() {
  rm -fr $TMPFILE
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17


NEWSGROUPS=""
FROM=""

RF_EXIT=false
RF_DEBUG=false
RF_FROM=false

S=newsgroups
for A in "${@}" ; do

# echo "${0}: A=\"${A}\"" >&2

  case "${A}" in

  '-D' )
    RF_DEBUG=true
    ;;

  '-?' )
# echo "${0}: question=\"${A}\"" >&2
    usage
    RF_EXIT=true
    ;;

  '-'* )
    echo "${P}: unknown option \"${A}\" ignored" >&2
    usage
    RF_EXIT=true
    ;;

  * )

# echo "${0}: S=${S}" >&2

    case $S in

    newsgroups )
      NEWSGROUPS="${NEWSGROUPS} ${A}"
      ;;

    esac
    ;;

  esac

# echo "${0}: bottom of for loop" >&2

done


if [ $RF_EXIT = true ] ; then exit 1 ; fi


# make a log entry
pcsconf -p $P -l $P


OPTS=""


countem() {
  C=0
  for U in ${NEWSGROUPS} ; do

	C=`expr $C + 1 `

  done
  echo $C
}
# end function (countem)


# echo "${0}: NEWSGROUPS=${NEWSGROUPS}" >&2

NNEWSGROUPS=0
if [ -n "${NEWSGROUPS}" ] ; then

  NNEWSGROUPS=`countem`

fi


donewsgroup() {
  NEWSGROUPDIR=`pcsngdir $1 `
  export MSGSDIR=${BBNEWSDIR}/${NEWSGROUPDIR}
  msgs -s
  unset MSGSDIR
}
# end function (donewsgroup)


if [ "${NNEWSGROUPS}" -gt 1 ] ; then

  cat > $TMPFILE

  for NG in $NEWSGROUPS ; do

    donewsgroup $NG < $TMPFILE

  done

else

  if [ -n "${NEWSGROUPS}" ] ; then

    donewsgroup $NEWSGROUPS

  else

    msgs -s

  fi

fi

cleanup
exit 0



