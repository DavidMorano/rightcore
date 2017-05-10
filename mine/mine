#!/usr/bin/ksh
# MINE

# program to make a file owned by the executing user

#	This program will also arrange that the modification time
#	of the file remains the same.


# start of program

for F in $* ; do

if test -r $F ; then

	if cat $F > mine${$} 2> /dev/null ; then

		A=$( fstat -otype ttouch $F )
		rm -f $F
		mv mine${$} $F
		touch -t $A $F
	fi

else 
  echo "${0}: file ${F} was not not readable"
fi

done



