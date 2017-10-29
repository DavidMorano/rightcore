/* date_header */


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
	  The format is RFC822 standard, e.g.:
		   27 Nov 1981 13:17 EST
		    1 Mar 1982  7:53 EDT

	This kind of date string is used in the DATE header of the message.


**************************************************************************/



#include	<sys/types.h>
#include	<time.h>
#include	<tzfile.h>

#include	"localmisc.h"



/* local data */

static char	*month[] = {
	"Jan","Feb","Mar","Apr","May","Jun",
	"Jul","Aug","Sep","Oct","Nov","Dec",
} ;




void date_header(clock,datestr)
time_t	clock ;
char	datestr[] ;
{
	struct tm	*timeinfo ;


	timeinfo = localtime(&clock) ;	

	sprintf(datestr,"%2d %3s %4d %02d:%02d %s",
		timeinfo->tm_mday,
		month[timeinfo->tm_mon],
		timeinfo->tm_year + 1900,
		timeinfo->tm_hour,
		timeinfo->tm_min,
		((timeinfo->tm_isdst != 0) ? tzname[1] : tzname[0])) ;

}
/* end subroutine (date_header) */


