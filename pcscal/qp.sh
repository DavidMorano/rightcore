
if	test -z "$1" -o -z "$2" -o -z "$3"
then	echo usage: "qp date time event"
	exit
fi

DATE=`caldate $1 2>/dev/null`
if	test -z "$DATE"
then	echo "error: '$1' is a bad date"
	exit
fi
DAY=`calday $DATE`

shift
TIME=$1

shift
EVENT="$*"

echo "${DATE} ${DAY}.	${TIME} ${EVENT}" >>${CAL-$HOME/.calendar}
if	test "$DATE" = `caldate today`
then
	for i in `echo $CALOPTS | sed "s/:/ /gp"`
	do
		if 	test "$i" = +remind
		then	rem $TIME $EVENT
		fi
	done
fi
echo "event posted on personal calendar"


