REMSYSTEMS="lzpfc lzpfd lzpfe lzpfa lzpfg"
# calendar posting service

#  all entries have DATE and TIME fields then 
#  different formats have different additional fields.


# get default options from CALOPTS
PLACE="no"
for i in `/usr/5bin/echo $CALOPTS | sed "s/:/ /g"`
do
case $i in
+remind)	REMIND="yes";;
-remind)	REMIND="no";;
+review)	REVIEW="yes";;
-review)	REVIEW="no";;
+place)		PLACE="yes";;
-place)		PLACE="no";;
esac
done






# check command arguments for calendar names and options
CALF=""
FORMAT=""

for i in $*
do
case $i in

 # standard calendars
- )
	PIPE=yes
	;;
personal)
	FORMAT=short
	CALF="$CALF ${CAL-$HOME/.calendar}"
	;;
general)
	FORMAT=talk
	CALF="${CALF} ${CALG}"
	if	test -z "$PIPE"
	then	/usr/5bin/echo "< general calendar >"
	fi
	;;
project)
	FORMAT=project
	CALF="$CALF ${CALDIR}/project"
	if	test -z "$PIPE"
	then	/usr/5bin/echo "< project calendar >"
	fi
	;;
downtime)
	FORMAT=downtime
	CALF="$CALF ${CALDIR}/downtime"
	if	test -z "$PIPE"
	then	/usr/5bin/echo "< downtime calendar >"
	fi
	;;
citeam | des_review)
	FORMAT=special
	POSTER=`logname`
	CALF="$CALF ${CALDIR}/$i"
	if	test -z "$PIPE"
	then	/usr/5bin/echo "< $i calendar >"
	INTRO=1
	fi
	;;


 #  options
+rem* ) 	REMIND="yes";;
-rem* )		REMIND="no";;
+rev* )		REVIEW="yes";;
-rev* )		REVIEW="no";;
+p* )		PLACE="yes";;
-p* )		PLACE="no";;
+m* )	
	MORE=first
	/usr/5bin/echo "multiple calendar entries have been requested."
	/usr/5bin/echo "input is terminated by entering a '.' when prompted for date."
	;;
+* | -*)
	/usr/5bin/echo usage: calpost [+/- options] [calendar names]
	/usr/5bin/echo usage:  options are:  remind, review, place, multiple
	/usr/5bin/echo "'$i' is an unknown option"
	exit
	;;


 # new calendar
*)
	FORMAT=short
	if	test -f $i -o -f $CALDIR/$i
	then	if	test -w $i -o -w $CALDIR/$i
		then	if	test -w $i
			then	CALF="$CALF $i"
				if	test -z "$PIPE"
				then	/usr/5bin/echo "< $i >"
				fi
			else	CALF="$CALF $CALDIR/$i"
				if	test -z "$PIPE"
				then	/usr/5bin/echo "< $CALDIR/$i >"
				fi
			fi
		else	/usr/5bin/echo usage: calpost [+/- options] [calendar names]
			/usr/5bin/echo "'$i' is not writable"
			exit
		fi
	else	/usr/5bin/echo "'$i' is not an existing calendar"
		/usr/5bin/echo "do you want to create it ? [no] \c"
		read REPLY
		if	test "$REPLY" = "yes" -o "$REPLY" = "y"
		then	/usr/5bin/echo "'$i' calendar will be created when entry is posted."
			CALF="$CALF $i"
		else	/usr/5bin/echo "ok, please try again"
			/usr/5bin/echo  usage: calpost [+/- options] [calendar names]
			exit
		fi
	fi
	;;

esac
done


# if no calendar specified, default to personal calendar
if	test -z "$CALF"
then	CALF="${CAL-$HOME/.calendar}"
	if 	test -z "$FORMAT"
	then	FORMAT=short
	fi
fi








# get events


TMPFILE=/tmp/calpost$$
TMPFILE2=/tmp/cal$$
trap "rm -f ${TMPFILE} >/dev/null 2>/dev/null;exit" 1 2 3 15
trap "rm -f ${TMPFILE2} >/dev/null 2>/dev/null;exit" 1 2 3 15


#if	pipe, filter event from standard input
if	test -n "$PIPE"
then	grep "^[0-9][0-9]/[0-9][0-9]/[0-9][0-9]" > ${TMPFILE}
	if	test ! -s ${TMPFILE}
	then	/usr/5bin/echo "error: calendar entry is empty"
		rm -f ${TMPFILE} >/dev/null 2>/dev/null
		exit
	fi
fi

 # if not pipe, format event from standard input
while
test -z "$PIPE"
do

	# get date for all calendars
	DATE=`prmpt_date`
# Build a file to display all calendar events for possible conflicts
# before posting the new one
	if test -z "$MORE" -a -z "$INTRO"
	then	(pcscal +all $DATE >> $TMPFILE2;)&
	fi

	# check for end of entries (NULL date)
	if	test -z  "$DATE"
	then	if	test -z "$MORE" -o "$MORE" = "first"
		then	exit
		else	break
		fi
	fi

	# get day of week for all cals
	DAY=`calday $DATE 2>/dev/null`

	# get time for all calendars
	/usr/5bin/echo "TIME: \c"
	read TIME
	


	# rest of fields depend on the format
	case $FORMAT in
	
	short )		#personal and user-defined
		/usr/5bin/echo "EVENT: \c"
		read EVENT
		if	test ! "$PLACE" = "no"
		then	/usr/5bin/echo "PLACE: \c"
			read PLACE
			if	test -n "$PLACE"
			then	EVENT="$PLACE - $EVENT"
				PLACE="yes"
			fi
		fi
		if 	test -z "$TIME"
		then	/usr/5bin/echo "$DATE $DAY.	$EVENT" >> ${TMPFILE}
		else	/usr/5bin/echo "$DATE $DAY.	$TIME $EVENT" >> ${TMPFILE}
		fi
		;;

	talk )		#public copied to events board
		/usr/5bin/echo "TITLE: \c"
		read TITLE
		/usr/5bin/echo "SPEAKER: \c"
		read NAME
		/usr/5bin/echo "AFFILIATION: \c"
		read AFFILIATION
		/usr/5bin/echo "PLACE: \c"
		read TPLACE

		/usr/5bin/echo "$DATE $DAY.	$TIME TITLE: $TITLE" >> ${TMPFILE}
		/usr/5bin/echo "$DATE    	PLACE: $TPLACE" >> ${TMPFILE}
		/usr/5bin/echo "$DATE    	SPEAKER: $NAME - $AFFILIATION" >> ${TMPFILE}
		/usr/5bin/echo "$DATE" >> ${TMPFILE}

		/usr/5bin/echo "Enter description terminated with '.':"
		while
			read DESCRIPTION
			[ "$DESCRIPTION" != "." ]
		do
			/usr/5bin/echo "$DATE    	$DESCRIPTION" >> ${TMPFILE}
		done
		;;

	project )
		while
		:
		do
			if	test -n "$TIME" -a ! "$TIME" = "?"
			then	break
			fi
			/usr/5bin/echo "You must enter a time (eg, \"1:00\", \"2:00 - 3:30\")"
			/usr/5bin/echo "TIME: \c"
			read TIME
		done
		
		while
		:
		do
			/usr/5bin/echo "PLACE: \c"
			read PLACE
			if	test -n "$PLACE" -a ! "$PLACE" = "?"
			then	break;
			fi
			/usr/5bin/echo "Enter the place for the meeting.\nEx:  HO 1A216"
		done
		
		NAME=`info - +me | upname`
		/usr/5bin/echo "CONTACT:  [$NAME] \c"
		read CONTACT
		if	test -n "$CONTACT"
		then	NAME=`info - "$CONTACT"`
			if	test -z "$NAME"
			then	NAME=`/usr/5bin/echo $CONTACT | upname`
			else	NAME=`/usr/5bin/echo $NAME | sed "s/ .*//" | upname`
			fi
		fi

		DA=`/usr/5bin/echo $NAME | da - | cut -f3`
		EXT=`/usr/5bin/echo $DA | cut -f2 -d" " `
		LOC=`/usr/5bin/echo $DA | cut -f1 -d" " `
		if	test ! "$LOC" = HO
		then	EXT="$LOC x$EXT"
		else	EXT=x${EXT}
		fi

		/usr/5bin/echo "EXTENSION:  [$EXT] \c"
		read EXTENSION
		if	test -n "$EXTENSION"
		then	EXT=$EXTENSION
		fi

		/usr/5bin/echo "$DATE $DAY.	$TIME $PLACE - $NAME - $EXT" >> ${TMPFILE}
		/usr/5bin/echo "$DATE	.fi" >> ${TMPFILE}
		/usr/5bin/echo "$DATE	.TL" >> ${TMPFILE}

		while
		:
		do
			/usr/5bin/echo "TITLE: \c"
			read TITLE
			if	test -n "$TITLE" -a ! "$TITLE" = "?"
			then	break;
			fi
			/usr/5bin/echo "Enter the type of the meeting.\nEx: design review, requirements review, status meeting, department meeting"
		done

		/usr/5bin/echo "$DATE	$TITLE -" >> ${TMPFILE}

		/usr/5bin/echo "Enter description terminated with '.':"
		while
			read DESCRIPTION
			[ "$DESCRIPTION" != "." ]
		do
			/usr/5bin/echo "$DATE    	$DESCRIPTION" >> ${TMPFILE}
		done
		;;
		

	downtime )	#for systems
		/usr/5bin/echo "SYSTEM:	\c"
		read SYSTEM
		/usr/5bin/echo "REASON:	\c"
		read REASON
		/usr/5bin/echo "$DATE $DAY	$TIME	"$SYSTEM - "	$REASON" >> ${TMPFILE}
		;;
	special)
 		/usr/5bin/echo "LOCATION: \c"
		read LOCATION
		/usr/5bin/echo "SUMMARY: \c"
		read SUMMARY
		/usr/5bin/echo "$DATE $DAY	$TIME	$LOCATION	$SUMMARY ($POSTER)" >> ${TMPFILE}
		;;
		
	
	esac	#end of FORMAT-dependent field case
	
	
	if	test -z "$MORE"
	then	break
	fi
	MORE=next

done









# check for automatic review
if	test "$REVIEW" = "yes"
then	/usr/5bin/echo ""
	pcscal 1/1/70 - 12/31/99 ${TMPFILE}
	/usr/5bin/echo ""
fi

# prompt for post, etc.
while
:
do
	if 	test -z "$PIPE" -a -z "$MORE" -a -z "$INTRO"
	then
		while   #display user's items for the date of this  entry
		:
		do
			if 	test -s "$TMPFILE2"
			then	cat ${TMPFILE2}
				break
			else
				sleep 5
			fi
		done
	fi
	/usr/5bin/echo "\npost, review, edit, quit ? [post] \c"

		read REPLY
	if	test -z "$REPLY"
	then	REPLY=post
	fi

	case "$REPLY" in

	edit | e)
		/usr/5bin/echo "now editing calendar entry"
		${ED-ed}  ${TMPFILE}
		;;

	quit | q)
		rm ${TMPFILE}
		rm -f ${TMPFILE2}
		exit
		;;

		
	post | p)

		# post entry  and submit to bulletin board (for talks)

		# post entry on calendar[s]
		for i in $CALF
		do

			case "$i" in

			"${CAL-${HOME}/.calendar}" )
				if	
					cat ${TMPFILE} >> $i
				then	
					/usr/5bin/echo "entry posted on personal calendar."
					# check for possible conflicts
				else	
					/usr/5bin/echo "cannot post entry on personal calendar."
				fi
				if	test  "$REMIND" = "yes"
				then	(cal2rem $i) &
				fi
				;;

			${CALDIR}* )
				CALX=`/usr/5bin/echo $i | sed "s+${CALDIR}/++"`
				ADMIN=`info - ${CALX}.admin`
				if	test -n "$ADMIN"
				then	ME=`info - +me`
				else	ME=$ADMIN
				fi
				if	test ! "$ADMIN" = "$ME"
				then	

				# submit to calendar administrator
					sendmail - $ADMIN subject="$CALX" file=${TMPFILE}
					/usr/5bin/echo entry submitted to \'$CALX\' calendar administrator
				else	
					if	
						cat ${TMPFILE} >> $i
					then	
						 ( post $i) &
						 if test "$FORMAT" = special
						 then
							 for j in $REMSYSTEMS
								do
								nusend -d $j -s -e -f $i $i
								done
						 fi
						/usr/5bin/echo "entry posted on '$CALX' calendar."
					else	
						/usr/5bin/echo "cannot post entry on '$CALX' calendar."
					fi
				fi
				;;

			* )
				if	
					cat ${TMPFILE} >> $i
				then	
					/usr/5bin/echo "entry posted on '$i' calendar."
				else	
					/usr/5bin/echo "cannot post entry on '$i' calendar."
				fi
				;;
			esac
		done

		# if is a talk, post entry onto "events" bulletin board
		if test "$FORMAT" = "talk"
		then
			BBNAME=`bbfile "$TITLE"`
			if test -f "$BBDIR/events/$BBNAME"
			then RESPECIFY="yes"		#duplicate name
			else
				/usr/5bin/echo
				/usr/5bin/echo "the bulletin board entry will be \c"
				/usr/5bin/echo "titled '$BBNAME'."
				/usr/5bin/echo "do you wish to respecify this? [no] \c"
				read ANS
				if test "$ANS" = "yes" -o  "$ANS" = "y"
				then	RESPECIFY="yes"
				fi
			fi
			if test "$RESPECIFY" = "yes"
			then
			while
			:
			do
				/usr/5bin/echo "BULLETIN TITLE: \c"
				read TMP
				BBNAME=`bbfile $TMP`
				if test -f "$BBDIR/events/$BBNAME"
				then 	/usr/5bin/echo "a bulletin named '$BBNAME'\c"
					/usr/5bin/echo " already exists,\c"
					/usr/5bin/echo " please select another."
				else	break
				fi
			done
			fi
	
			# post entry on events board
			if	test -n "$BBNAME"
			then

				pcscal 1/1/70 - 12/31/99 ${TMPFILE} > ${BBDIR}/events/$BBNAME
				if	
					test -s ${BBDIR}/events/$BBNAME
				then	
					chmod 0644 ${BBDIR}/events/$BBNAME
					(post ${BBDIR}/events/$BBNAME )  &
					/usr/5bin/echo "entry posted on 'events' bulletin board."
				else	
					/usr/5bin/echo "cannot post entry on 'events' bulletin board."
				fi
			fi
		fi	# end of "events" bulletin board posting
		rm ${TMPFILE2} >/dev/null 2>/dev/null
		rm ${TMPFILE} >/dev/null 2>/dev/null
		exit
		;;   # end of calendar posting


		
	review | r)
		/usr/5bin/echo ""
		pcscal 1/1/70 - 12/31/99 ${TMPFILE}
		;;

	?)
		/usr/5bin/echo
		/usr/5bin/echo "post      post calendar entry on 'calendar'"
		/usr/5bin/echo "review    print calendar entry for review"
		/usr/5bin/echo "edit      edit calendar entry"
		/usr/5bin/echo "quit      terminate session without posting entry\n"
		;;
	*)
		/usr/5bin/echo "unknown command.\nplease try again.\n"
		;;
	esac
done
# Name the LZ (HO) Systems to share the design review calendar with
REMSYSTEMS="lzpfd lzpfc lzpfa lzpfe lzpfg"
LZCALDIR="/tools/pcs/lib/calendars"
