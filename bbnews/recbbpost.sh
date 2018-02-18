# RECBBPOST
# 1989-06-26 JIS
#
# rbbpost address [ address ]* [s=subject]

PCSBIN=/usr/add-on/pcs/bin
SMAILNAMES=
SMAILLISTS=
SMAILOPTS=+standard:-ed
export SMAILNAMES SMAILLISTS SMAILOPTS

ADDRESS=""
TITLE=""
while [ "X$1" != "X" ] ; do

	case $1 in

	s=) 	TITLE="`echo $1  | sed -e "s/_/ /"`"
		;;

	-)	;;

	*)	ADDRESS=$ADDRESS" $1"
		;;

	esac

	shift
done

${PCSBIN}/smail - $ADDRESS "$TITLE"


