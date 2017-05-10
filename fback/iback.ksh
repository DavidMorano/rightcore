#!/usr/bin/ksh
# {FI}BACK
# script to perform backups

# calling synopsis
#
#	Xback bk_table_file
#
# If no backup table is specified then the back table defined by the 
# DEFBKTAB variable below is used instead.
#


# configurable defaults

PM=i
RFSMOUNTCHECK=false
RF_FILEFILTER=true
RF_DEBUG=false

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
RF_BUILTIN=false
DN=/dev/null
if whence builtin > ${DN} ; then
  RF_BUILTIN=true
  if [[ ! -r "${ENV}" ]] ; then
    ENV=${HOME}/.kshrc
    if [[ -r "${ENV}" ]] ; then
      . ${ENV}
      export ENV
    fi
  fi
fi


P_RMNTTRY=/usr/nserve/rmnttry
P_RFSCHECK=/usr/admin/bin/rfscheck

if ${RF_BUILTIN} ; then
  builtin -f libcmd.so rm
fi


: ${LOCAL:=/usr/add-on/local}
: ${GNU:=/usr/add-on/gnu}
export LOCAL GNU


PATH=${LOCAL}/bin:${PATH}:${GNU}/bin
export PATH


if [[ ! -d /usr/sbin ]] ; then
  RFSMOUNTCHECK=false
fi


DATESTAMP=$( date '+%m%d%H%M' )
umask 000

case ${USERNAME:=$( username )} in

root )
  HOME=$( userhome admin )
  ;;

* )
  RFSMOUNTCHECK=false
  ;;

esac

DEFBKTAB=${HOME}/etc/bktab/default

DATE=$( date '+%m/%d %H:%M' )
if [[ $# -ge 1 ]] && [[ -n "${1}" ]] ; then 
  BKTAB=${1}
else 
  BKTAB=${DEFBKTAB}
fi

if [[ ! -f ${BKTAB} ]] ; then

  print -u2 "${PN}: ${DATE} backup table file not found"
  exit 1

fi

EXCLUDE=
if [[ -s "${BACK_EXCLUDE}" ]] ; then
  EXCLUDE=${BACK_EXCLUDE}
fi

FFILES=
if [[ -r "${EXCLUDE}" ]] ; then
  FFILES=$( < ${EXCLUDE} )
fi

FSIZE=1500000c
FOPTS=
FSA="c,cc,h,s,map,txt,help,mm,ms,man"
FSR="o,x,a,so,i,lm,ls,ln"

FILEFILTER_OPTS="noprog,nosock,sa=+,sr=+"
FILEFILTER_SUFACC=${FSA}
FILEFILTER_SUFREJ=${FSR}
export FILEFILTER_OPTS FILEFILTER_SUFACC FILEFILTER_SUFREJ

if [[ $# -ge 2 ]] && [[ -n "${2}" ]] ; then
  RATE=$2
fi

case "${RATE}" in

h* )
  RATE=h
  ;;

d* )
  RATE=d
  ;;

w* )
  RATE=w
  ;;

m* )
  RATE=m
  ;;

esac


TFILE=/tmp/bk${$}
print "${DATE} backups started (${PM}) \"${BKTAB}\""

while read FROM TO NAME J ; do

#print FROM="${FROM}"
#print TO="${TO}"
#print NAME=${NAME}

  case ${FROM} in

  '#'* | '-' | '' )
    continue
    ;;	

  esac 

  case "${TO}" in

  '#'* | '-' | '' )
    continue
    ;;

  esac

  case "${NAME}" in

  '#'* | '-' )
    NAME=""
    ;;

  esac

# truncate name to 12 characters

  NAME=$( print -- ${NAME} | cut -c 1-12 )

# try to insure that the backup medium is online before continuing

  if [[ ${RFSMOUNTCHECK} == true ]] && whence ${P_RMNTTRY} > ${DN} ; then
    if whence ${P_RFSCHECK} > ${DN} ; then
    C=0
    while [[ ! -d ${TO} ]] ; do

      if [[ $C -eq 0 ]] ; then ${P_RMNTTRY} ; fi

      if [[ $C -eq 5 ]] ; then ${P_RMNTTRY} ; fi

      ${P_RFSCHECK}
      sleep 90
      C=$( expr $C + 1 )
      if [[ $C -ge 10 ]] ; then break ; fi

    done
    fi
  fi

  if [ ! -d "${TO}" ] ; then

    print -u2 "${PN}: TO directory \"${TO}\" does not exist"
    continue

  else

    cd ${TO}
    if [[ ! -f README  ]] ; then {

      print "All files in the directories under here are "
      print "CPIO archives ('name')\c"
      print " or compressed CPIO archives ('name.Z' or 'name.gz')."

    } >> README ; fi

    BDIR=${PM}${DATESTAMP}
    if [[ ! -d ${BDIR} ]] ; then
      mkdir ${BDIR}
    fi

        FINDOPT=
        #FINDOPT="-follow"
        RF_KEYREMOVE=false
        if [[ "${PM}" == "i" ]] ; then

# do we have a key file?

DS=${DATESTAMP}
A=$( ls -t ${TO} 2> ${DN} | grep "[if][0-9]" | fgrep -v ${DS} | line )

            KEYFILE=/tmp/kf${$}
            RF_KEYREMOVE=true
          if [[ -n "${A}" ]] && [[ -d "${A}" ]] ; then

if ${RF_DEBUG} ; then
print -u2 -- A=${A}
ls -ld $A >&2
fi

		B=$( print -- ${A##*/} | cut -c 2-14 )
if ${RF_DEBUG} ; then
	    print -u2 -- B=${B}
fi
		touch $B ${KEYFILE}

	  else

            touch 0101000070 ${KEYFILE}

          fi
          FINDOPT="-newer ${KEYFILE}"

if ${RF_DEBUG} ; then
print -u2 -- KEYFILE=${KEYFILE}
ls -l ${KEYFILE} >&2
fi

        fi

    for F in ${FROM} ; do

# does the directory (or file !!) exist?

      if [[ ! -d "${F}" ]] && [[ ! -f "${F}" ]] ; then continue ; fi

      FNODE=${F##*/}
      if [[ "${FNODE}" == "lost+found" ]] ; then continue ; fi

      FROMDIR=${F%/*}
      if [[ ! -d "${FROMDIR}" ]] ; then

#        print "FROM directory \"${FROMDIR}\" does not exist"
         continue

      else

        CPIOFILE=${FNODE}
        if [[ -n "${NAME}" ]] ; then
          CPIOFILE=${NAME}
        fi

        if cd ${FROMDIR} ; then

#print -u2 -- PWD=${PWD} FNODE=${FNODE}
#        print -- `pwd` $F node is $FNODE $TO $BDIR $CPIOFILE 

	if [[ ${PM} == 'i' ]] ; then
	  find ${FNODE} -type f ${FINDOPT} -size -${FSIZE} -print
	else
          find ${FNODE} -type f ${FINDOPT} -print
	fi 2> ${DN} | filefilter ${FOPTS} ${FFILES} > ${TFILE}

	fi

if ${RF_DEBUG} ; then
print -u2 -- TFILE=${TFILE}
while read AA ; do print -u2 -- AA=${AA} ; done < ${TFILE}
fi

	WRITEFILE=${TO}/${BDIR}/${CPIOFILE}
        if [[ -s ${TFILE} ]] ; then

          rm -f ${WRITEFILE}*

	  cpio -oc < ${TFILE} | bzip2 > ${WRITEFILE}.bz2

        fi 2>&1

        rm ${TFILE}

      fi

    done

        if [[ ${RF_KEYREMOVE} == true ]] ; then
          rm -f ${KEYFILE}
        fi

  fi

done < $BKTAB

DATE=$( date '+%m/%d %H:%M' )
print -- "${DATE} backups completed (${PM}) \"${BKTAB}\""



