	while
	:
	do
		/usr/5bin/echo "DATE: \c"
		read DATE
		if	test "$DATE" = "?"
		then	/usr/5bin/echo "
	Please enter a date in one of the following forms:

		11/11/80     October 10, 1980     10feb80     jan 12
		today        tomorrow              monday

	The closest year will be assumed if	omitted.

	"
			continue
		fi
		# check for '.' in date
		if	test "$DATE" = '.'
		then	if	test -z "$MORE" -o "$MORE" = "first"
			then	exit
			else	break
			fi
		fi
		DATE=`caldate $DATE 2>/dev/null`
		if	test -n "$DATE"
		then	DAY=`calday $DATE 2>/dev/null`
			if	test -n "$DAY"
			then	break
			fi
		fi
		/usr/5bin/echo "Incorrect date, please try again."
	done
