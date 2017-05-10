/* date_envelope */


#define	CF_DEBUG	0


/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
 *		T.S.Kennedy						
 *									
 

	  Places today's date and time into the string "datestr".  
	  The format is RFC822 standard, e.g. :
		   27 Nov 1981 13:17 EST
		    1 Mar 1982  7:53 EDT

	This kind of date string is used in the DATE header of the message.


**************************************************************************/



#include	<sys/types.h>
#include	<time.h>

#include	"localmisc.h"



extern struct tm	*localtime() ;

extern char		*tzname[] ;			


static char	*month[] = {
	"Jan","Feb","Mar","Apr","May","Jun",
	"Jul","Aug","Sep","Oct","Nov","Dec",
} ;

static char	*day[] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
} ;



void date_envelope(clock,datestr)
time_t	clock ;
char	datestr[] ;
{
	struct tm	*timeinfo ;


	timeinfo = localtime(&clock) ;	

	sprintf(datestr,"%3s %3s %02d %02d:%02d %s %04d",
		day[timeinfo->tm_wday],
		month[timeinfo->tm_mon],
		timeinfo->tm_mday,
		timeinfo->tm_hour,
		timeinfo->tm_min,
		((timeinfo->tm_isdst != 0) ? tzname[1] : tzname[0]),
		timeinfo->tm_year + 1900) ;

}
/* end subroutine (date_envelope) */


