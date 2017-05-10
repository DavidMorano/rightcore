#!/usr/bin/ksh
# BBPOST

# 1984-09-30 JIS
#	The delete feature has been removed, and all posting is now done 
#	by using the new PCSMAIL.  Only the user interface is provided 
#	by this shell script.

# 1989-07-24 LEEPER
#	All posting now done via PDU.

# 1989-11-13 JIS
#	Use NFS to do posting from Suns.

# 1994-11-29 DAM
#	Fixed some "unknown command" bugs and got rid of special treatment
#	for machine 'mtsol'

# 1995-07-13 DAM
#	Fixed it so that hierarchical postings are posted properly
#	in the newsgroup spool area.
#
# 1995-07-14 DAM
#	Enhanced it so that the user can give a subject on the program
#	invocation and also to take an attachment file from the
#	invocation.  Also, added an option to display the program
#	version so that future changes can be tracked more easily.
#
# 1997-05-28 DAM
#	This program has been changed to simply pass a file which is
#	ready to enter into the spool area to a program named RBBPOST.
#	We now let the RBBPOST program copy the article file
#	into the spool area and also distribute to any other hosts
#	that are configured.  Supposedly the PCSMAIL program served
#	this same purpose in the past, but somewhere along the line
#	that feature of the PCSMAIL program was silently dropped
#	and the same function was ported back to here.
#


VERSION="2b"


if [ -d /usr/sbin ] ; then
  : ${NODE:=$( uname -n )}
  PROG_ECHO=echo
else
  : ${NODE:=$( hostname )}
  PATH=/usr/5bin:${PATH}
  PROG_ECHO=/usr/5bin/echo
fi


# ADMINISTRATOR take note: 
# please make sure the varaibles below are set correctly

# PCS directory
: ${PCS:=/usr/add-on/pcs}

# newsgroups spool area directory
: ${BBDIR:=${PCS}/spool/boards} 


# END OF SETABLE VARIABLES


PATH=${PATH}:${PCS}/bin
export PATH


# make a log entry for us

P=$( basename ${0} )


if test -z "${BBDIR}" ; then 	

  ${PROG_ECHO} "${P}: could not find PCS newsgroup spool area directory" >&2
	exit 1

fi


FROMNODE=$( clustername )

: ${USERNAME:=$( username )}
export USERNAME

ORG=$( pcsorg )


# choose how we select the user's editor

if [[ -z "${ED}" ]] ; then
  ED=${EDITOR}
fi


: ${PAGER:=more}


TMPFILE=/tmp/bbpost${$}


cleanup() {
  rm -f ${TMPFILE}
}

# trap 'cleanup ; exit 1' 1 2 3 15 16 17


SUBJECT=""
ATTACHMENT=""
NEWSGROUPS=""

RF_DEBUG=false
RF_VERSION=false
RF_USAGE=false
RF_EDIT=false
RF_MAIL=false
RF_SUBJECT=false
RF_ATTACHMENT=false
RF_NEWSGROUPS=false

S=boards
OS=""
for A in "${@}" ; do

  case ${A} in

  '-e'* | '+e'* )
    RF_EDIT=true
    ;;

  '-m'* | '+m'* )
    RF_MAIL=true
    ;;

  '-s' )
    OS=${S}
    S=subject
    ;;

  '-a' )
    OS=${S}
    S=attachment
    ;;

  '-V' )
    RF_VERSION=true
    ;;

  '-D' )
    RF_DEBUG=true
    ;;

  '-?' )
    RF_USAGE=true
    ;;

  '-'* )
    ${PROG_ECHO} "${0}: unknown option \"${A}\"" >&2
    exit 1
    ;;

  * )
    case ${S} in

    subject )
      SUBJECT="${A}"
      RF_SUBJECT=true
      S=${OS}
      ;;

    attachment )
      ATTACHMENT="${A}"
      RF_ATTACHMENT=true
      S=${OS}
      ;;
      
    boards | newsgroups )
      NEWSGROUPS="${NEWSGROUPS} ${A}"
      ;;

    esac
    ;;

  esac

done


# check or process the arguments so far

if ${RF_VERSION} ; then

  ${PROG_ECHO} "${P}: version ${VERSION}" >&2
  exit 0

fi

if ${RF_USAGE} ; then

  USAGE="${P}: USAGE> bbpost [<newsgroup>] [-V] [-e] [-m] [-s <subject>]"
  USAGE="${USAGE} [-a <attachment>]"
  ${PROG_ECHO} "${USAGE}" >&2
  cleanup
  exit 0

fi

ANSWER=""
if ${RF_EDIT} ; then
	ANSWER=edit
fi

MAILTO=""
if ${RF_MAIL} ; then
	MAILTO=yes
fi


# get board name and bulletin title

BOARD=""
if [ -n "${NEWSGROUPS}" ] ; then
  ${PROG_ECHO} $NEWSGROUPS | read BOARD J
fi

if [ -n "${BOARD}" ] ; then 
  RF_NEWSGROUPS=true
fi


RF_GOODBOARD=false
while [[ ${RF_GOODBOARD} == false ]] ; do

  if [[ ${RF_NEWSGROUPS} != true ]] ; then

	${PROG_ECHO} "newsgroup: \c"
	read BOARD

  else

    BOARD=$( ${PROG_ECHO} ${NEWSGROUPS} | cut -d ',' -f 1 )

  fi

  if test -z "${BOARD}" ; then	

    ${PROG_ECHO} "enter the name of a newsgroup"
    ${PROG_ECHO} "enter \"?\" to obtain names of existing newsgroups"
		BOARD=""
    		RF_NEWSGROUPS=false
		continue
	fi

	if test "${BOARD}" = "?" ; then 	

		${PROG_ECHO} "enter the name of a newsgroup"
		${PROG_ECHO} "the exiting newsgroups are :"
		bb -newsgroups -e | ${PAGER}
		BOARD=""
    		RF_NEWSGROUPS=false
		continue
	fi

	if test "${BOARD}" = "." ; then 	

		ANSWER=edit
		BOARD=""
		continue
	fi

        NGDIR=$( pcsngdir -N ${BBDIR} ${BOARD} )
	if [ -z "${NGDIR}" -o ! -d ${BBDIR}/${NGDIR} ] ; then 	

          ${PROG_ECHO} "newsgroup \"${BOARD}\" does not exist"
          ${PROG_ECHO} "enter \"?\" to obtain names of existing newsgroups"
		BOARD=""
    		RF_NEWSGROUPS=false
	else
  		RF_GOODBOARD=true
	fi

	if [ "x$( ${PROG_ECHO} ${BOARD} | grep general )" != "x" ] ; then

        TEXT="The \"${BOARD}\" newsgroup is for work-related items only."
	${PROG_ECHO} "${TEXT}"
	${PROG_ECHO} "For-sale and other non-work-related items should go"
	${PROG_ECHO} "on the \"misc\" newsgroup. "
	${PROG_ECHO} "Is this item work-related [y/n] ? "
		read ANS
		test "x${ANS}" = xy || continue

	fi

done

# end while


# check for a good subject

if ${RF_SUBJECT} ; then
  TITLE="${SUBJECT}"
fi

while test -z "${TITLE}" -a -z "${ANSWER}" ; do

	${PROG_ECHO} "Subject: \c"
	TITLE=`line`

	if test -z "${TITLE}" ; then	

	${PROG_ECHO} "enter the title for the article"
	${PROG_ECHO} "enter \"?\" to obtain names of existing artile titles"
		continue
	fi

	# check for ? (help)

  if test "${TITLE}" = "?" ; then 	

	${PROG_ECHO} "enter a subject for the article"
	${PROG_ECHO} "the title may contain spaces"
	${PROG_ECHO} "existing article subjects on this newsgroup are :"
	bbnews -subject -a ${BOARD} | ${PAGER}
		TITLE=""
		continue
	fi

	if test "${TITLE}" = "." ; then	

		ANSWER=edit
		TITLE=""
		continue
	fi

	OWNER=""

done

# end while


trap "
cat <<EOF
You must have hit break.  That kills ${0}.
Your recovery file is in ${TMPFILE}.
Please try to post again and pull ${TMPFILE} into the editor.
EOF
exit
" 2

trap "rm -f ${TMPFILE} ; exit 1" 1 15


# create an initial article heading

{
  ${PROG_ECHO} "Newsgroups: ${BOARD}"
  ${PROG_ECHO} "Subject:    ${TITLE}" 
  ${PROG_ECHO}
} > ${TMPFILE}


# have the user enter some text

if test ! "${ANSWER}" = "edit" ; then 	

  ${PROG_ECHO} "enter text for bulletin terminated by a '.' character"
  while
    TEXT=$( line )
    test ! "${TEXT}" = "."
  do
    ${PROG_ECHO} "${TEXT}" >> ${TMPFILE}
  done
fi

if [ ${RF_ATTACHMENT} = true -a -r "${ATTACHMENT}" ] ; then {

  ${PROG_ECHO}
  cat $ATTACHMENT

} >> $TMPFILE ; fi


# prompt the user for the disposition so far

while true ; do

	if test -z "${ANSWER}" ; then	

	${PROG_ECHO} "post, review, edit, or quit ? [post] \c"
		read ANSWER
	fi

	if test -z "${ANSWER}" ; then 
		ANSWER=post
	fi

	case "${ANSWER}" in

# edit the message so far
	e* )

		ANSWER=""
		${PROG_ECHO} "editing bulletin ..."
		${ED-ed} $TMPFILE

		# check for title & board change
		TITLE=`grep -i '^.ubject:' $TMPFILE | sed "s/.*:[ 	]*//"`

BOARD=`grep -i '^.ewsgroups:' ${TMPFILE} | sed "s/.*:[ 	]*//" | sed "s/[	 ]*$//"`

        NGDIR=$( pcsngdir -N ${BBDIR} ${BOARD} )
		if [ -z "${NGDIR}" -o -z "${BOARD}" ] ; then 	

		${PROG_ECHO} "warning - newsgroup \"${BOARD}\" does not exit"
		${PROG_ECHO} "please edit the name of the newsgroup"
			ERROR=board
			continue
		fi

		if test ! -d "${BBDIR}/${NGDIR}" ; then 	

	${PROG_ECHO} "warning - newsgroup \"${BOARD}\" does not exist"
	${PROG_ECHO} "please edit the name of the newsgroup"
	${PROG_ECHO} "the following are available newsgroups :"
			bb -newsgroups -e
			ERROR=board
			continue
		fi

	TITLE=`grep -i 'ubject:' $TMPFILE | sed "s/.*:[ 	]*//" | sed "s/[	 ]*$//"`
		if test -z "${TITLE}" ; then 	

			${PROG_ECHO} "warning - the article subject is defined"
			${PROG_ECHO} "please edit the title of the bulletin"
			ERROR=title
			continue
		fi

		ERROR=""
		;;

# review
  r* )
    ANSWER=""
    trap "continue" 1 2 3 15
    if test -s $TMPFILE ; then 

      ${PROG_ECHO}
      ${PAGER} $TMPFILE

    else 

      ${PROG_ECHO} "the article body is is empty"

    fi
    ;;

# post
	p* )
		ANSWER=""
		if test "${ERROR}" = board ; then 	

			if test -z "${BOARD}" ; then 	

		${PROG_ECHO} "warning - bulletin board not defined"

			else 	

	${PROG_ECHO} "warning - bulletin board \"${BOARD}\" does not exist"

			fi
			${PROG_ECHO} "please edit the name of the board"
			continue
		fi

		if test "${ERROR}" = title ; then 	

			if test -z "${TITLE}" ; then 	

		${PROG_ECHO} "warning -- title not given"

			fi
			${PROG_ECHO} "please edit the title of the bulletin"
			continue
		fi
		ANSWER=""

		# post on local system
		if test ! -s "${TMPFILE}" ; then

		${PROG_ECHO} "cannot post, article is empty"
		     continue
		fi

		AFILE=/tmp/bbpost${$}

		if [ -z "${NAME}" ] ; then
		  NAME=$( ui - fullname )
		fi

	    	{
                  cat <<-EOF
	x-mailer: ${ORG} PCS BBPOST (version ${VERSION})
	FROM:       ${NAME} <${FROMNODE}!${USERNAME}>
	DATE:       $( date "+%d %h %Y %T %Z" )
EOF
#		  newform -i-8 ${TMPFILE}
	          cat ${TMPFILE}
	   	} | rbbpost ${BOARD}

		cleanup
		exit 0
		;;

# quit
	q* )	
		rm -f ${TMPFILE} > /dev/null 2> /dev/null
		cleanup
		exit 0
		;;

# help
	\? )
		ANSWER=""
		${PROG_ECHO} "\n	post	post bulletin"
		${PROG_ECHO} "	review	print bulletin for review"
		${PROG_ECHO} "	edit	edit bulletin"
		${PROG_ECHO} "	quit	quit without posting bulletin\n"
		;;

# unknown
	* )	
	${PROG_ECHO} "unknown command \"${ANSWER}\", please try again"
		ANSWER=""
		;;

	esac

done

cleanup


