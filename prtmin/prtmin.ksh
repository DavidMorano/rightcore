#!/usr/bin/ksh
# PRTMIN - program to print out files using special 'mini' type font

#
#	= 89/09/01, Dave Morano
#	This program was originally written but borrowed totally
#	from the idea of previous programs like it, like 'mroff'
#	and previous mini-print programs.
#
#	= 92/07/30, Dave Morano
#	Added support for the HO DiSCO printer as a destination
#


#
# Synopsis:
#	prtmin [ -d dst ] [ -s sides ] file [ file{s) ]
#
#

# configurable parameters

CONCATENATE=false
DEVLINES=94
OFFSET=10

# fix some bad stuff

unset ENV
#unalias cd

# OK to proceed

if [ ! -d /usr/sbin ] ; then
  ARCH=`/bin/arch`
  MACH=`hostname`
else
  ARCH=att
  MACH=`uname -n`
fi

sh_logdir() {
  TF=/tmp/ld${$}
  ypcat passwd 2> /dev/null > ${TF}
  if [ $? -ne 0 ] ; then
    grep "^${1}:" < /etc/passwd | line | cut -d: -f6
  else
    grep "^${1}:" < ${TF} | line | cut -d: -f6
  fi
  rm -f ${TF}
  return ${RS}
}

DEFDST=${LPDEST:-hp0}
PRT=prt
MROFF=mroff

case ${MACH} in
rc* | hodi* | hosb* )
  DEFDST=hp0
  ;;
ho* | nucleus | logicgate | nitrogen )
  DEFDST=gwbb0
  ;;
mthost* | mtsol | mtgbs* | mtgzfs* | mtgz[0-9]* )
  DEFDST=di0
  ;;
octet | dds | disco | intrigue | spin )
  DEFDST=di0
  PRT=/usr/add-on/local/bin/prt
  if [ ! -x $PRT ] ; then
    PRT=/proj/starbase/tools/bin/prt
  fi
  if [ ! -x $PRT ] ; then
    PRT=/home/dam/discobin/prt
  fi
  if [ ! -x $PRT ] ; then
    PRT=`sh_logdir dam`/discobin/prt
  fi
  ;;
allegra | bebop )
  DEFDST=di0
  ;;
esac


# fix up some PATH stuff

: ${DWBHOME:=/usr/add-on/dwb}
export DWBHOME

if [ "${ARCH}" != "att" -a -d /usr/5bin ] ; then
  PATH=/usr/5bin:${PATH}
fi

case ${MACH} in
hodi* | disco | octet )
  PATH=/usr/add-on/local/bin:${DWBHOME}/bin:${PATH}
  ;;
mt* )
  PATH=/mt/mtgzfs8/hw/starbase/tools/bin:${PATH}
  ;;
esac


# start of program stuff

P=`basename ${0}`

PR=pr
if [ ${ARCH} != 'att' -a -d /usr/5bin ] ; then
  PR=/usr/5bin/pr
fi

TMP=/tmp

TMPFILE1=$TMP/mpr${$}A
TMPFILE2=$TMP/mpr${$}B
TMPFILE3=$TMP/mpr${$}C

cleanup() {
  rm -f $TMPFILE1 $TMPFILE2 $TMPFILE3
}

trap 'cleanup ; exit 1' 1 2 3 15 16 17


DST=${PRINTER:-${DEFDST}}
FILES=""
NS=2
COPIES=1
PMODE="portrait"
FORM=nohole

RF_RP=false
RF_DEBUG=false
RF_Z=false
RF_SIDES=false
RF_COPIES=false
RF_PMODE=false
RF_FORM=false

S=files
OS=""
for A in "${@}" ; do
  case ${A} in
  '-c' )
    OS=${S}
    S=copies
    ;;
  '-d' )
    OS=${S}
    S=dst
    ;;
  '-f' )
    OS=${S}
    S=form
    ;;
  '-nr' )
    RF_RP=false
    ;;
  '-p' )
    OS=${S}
    S=pmode
    ;;
  '-r' )
    RF_RP=true
    ;;
  '-s' )
    OS=${S}
    S=sides
    ;;
  '-Z' )
    RF_Z=true
    ;;
  '-D' )
    RF_DEBUG=true
    ;;
  '-'* )
    echo "${P}: unknown option \"${A}\" ignored" >&2
    ;;
  * )
    case ${S} in
    copies )
      COPIES=${A}
      RF_COPIES=true
      S=${OS}
      ;;
    files )
      FILES="${FILES} ${A}"
      ;;
    dst )
      DST=${A}
      S=${OS}
      ;;
    form )
      FORM=${A}
      RF_FORM=true
      S=${OS}
      ;;
    pmode )
      PMODE=${A}
      RF_PMODE=true
      S=${OS}
      ;;
    sides )
      NS=${A}
      RF_SIDES=true
      S=${OS}
      ;;
    esac
    ;;
  esac
done


PRTOPTS=""
OPT=

# minimum sanity check on the printer destination

DSTOPT=""
if [ -n "${DST}" ] ; then
  DSTOPT="-d ${DST}"
fi

case ${PMODE} in
port | portrait )
  PMODE=portrait
  ;;
land | landscape )
  PMODE=landscape
  ;;
2on1 )
  ;;
* )
  echo "${P}: unsupported printer mode specified \"${PMODE}\"" >&2
  ;;
esac


if [ ${RF_COPIES} = true ] ; then
  case "${COPIES}" in
  [0-9] | [0-9][0-9] )
    ;;
  * )
    echo "${P}: invalid number of copies parameter" >&2
    exit 1
    ;;
  esac
fi

if [ ${RF_SIDES} = true ] ; then
  if [ "${NS}" -gt 2 -o "${NS}" -lt 0 ] ; then NS=2 ; fi
fi

if [ ${RF_RP} = false ] ; then
  OPT=""
  if [ ${RF_SIDES} = true ] ; then
    PRTOPTS="-s ${NS}"
  fi
else
  OPT="-r"
  PRTOPTS="-s 2"
fi

if [ ${RF_COPIES} = true ] ; then
  PRTOPTS="${PRTOPTS} -c ${COPIES}"
fi

if [ ${RF_PMODE} = true ] ; then
  PRTOPTS="${PRTOPTS} -p ${PMODE}"
fi

if [ $RF_FORM = true ] ; then
  PRTOPTS="${PRTOPTS} -f ${FORM}"
fi


# handle for the 'pr' command

PRLINES=${DEVLINES}


if [ -n "${FILES}" ] ; then

  for F in ${FILES} ; do

    if [ -s "${F}" ] ; then

      if [ ${RF_DEBUG} = true ] ; then 
        echo "${P}: sides ${NS} - dst ${DST} - opt ${OPT}"
      fi

      HEADER="  ${F}  "
      if [ ${RF_RP} = true ] ; then

#        CMD="fixbreak $F | ${PR} -h ${HEADER} -f -e -o${OFFSET} -l${PRLINES}"
        CMD="${PR} -h ${HEADER} -f -e -o${OFFSET} -l${PRLINES} ${F}"
        CMD="${CMD} | ${MROFF} ${OPT} | troff"

      else

#        CMD="fixbreak $F | ${PR} -h ${HEADER} -f -e -l${PRLINES}"
# or
#        CMD="${PR} -h ${HEADER} -f -e -l${PRLINES} ${F}"
# more
#        CMD="${CMD} | postprint -s 6 -f CB -l ${DEVLINES} -x 1"
# or
#        CMD="${CMD} | textset -p 6 -f CW -${DEVLINES} -offset ${OFFSET}"
#        CMD="${CMD} | troff"
# alternative
        CMD="textset -h ${F} -p 6 -f CW -${DEVLINES} -offset ${OFFSET}"
        CMD="${CMD} | troff"

      fi

      if [ ${RF_Z} = true ] ; then

        if [ $RF_DEBUG != true ] ; then
          eval ${CMD}
        else
          echo "${CMD}"
        fi

      else

        if [ $RF_DEBUG != true ] ; then
	  eval ${CMD} | ${PRT} ${DSTOPT} -l troff ${PRTOPTS}
        else
	  echo "${CMD} | ${PRT} ${DSTOPT} -l troff ${PRTOPTS}"
        fi

      fi

    else

      echo "${P}: file \"${F}\" is empty or does not exist" >&2

    fi

  done

else

      HEADER="standard input"
      if [ ${RF_RP} = true ] ; then

#        CMD="fixbreak | ${PR} -h \"${HEADER}\" -f -e -o${OFFSET} -l${PRLINES}"
        CMD="${PR} -h \"${HEADER}\" -f -e -o${OFFSET} -l${PRLINES}"
        CMD="${CMD} | ${MROFF} -f CW ${OPT} | troff"

      else

#        CMD="fixbreak | ${PR} -h \"${HEADER}\" -f -e -l${PRLINES}"
# or
#        CMD="${PR} -h \"${HEADER}\" -f -e -l${PRLINES}"
# more
#        CMD="${CMD} | postprint -s 6 -f CB -l ${DEVLINES} -x 1"
# or
#        CMD="${CMD} | textset -p 6 -f CW -${DEVLINES} -offset ${OFFSET}"
#        CMD="${CMD} | troff"
# alternative
	CMD="textset -h -p 6 -f CW -${DEVLINES} -offset ${OFFSET}"
        CMD="${CMD} | troff"

      fi

      if [ ${RF_Z} = true ] ; then

        if [ $RF_DEBUG != true ] ; then
          eval ${CMD}
        else
          echo "${CMD}"
        fi

      else

        if [ ${RF_DEBUG} != true ] ; then
	  eval ${CMD} | ${PRT} ${DSTOPT} -l troff ${PRTOPTS}
        else
	  echo "${CMD} | ${PRT} ${DSTOPT} -l troff ${PRTOPTS}"
        fi

      fi

fi

cleanup


