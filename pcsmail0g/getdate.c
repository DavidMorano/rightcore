static char sccsid[] = "@(#)getdate.c	PCS 3.0";

/************************************************************************
 *									*
 * The information contained herein is for the use of AT&T Information	*
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	*
 *									*
 *	(c) 1984 AT&T Information Systems				*
 *									*
 * Authors of the contents of this file:				*
 *									*
 *		T.S.Kennedy						*
 *									*
 

	getdate    
	  places today's date and time into the string "datestr".  
	  format is ARPAnet standard, e.g.
		   27 Nov 1981 13:17 EST
		    1 Mar 1982  7:53 EDT


**************************************************************************/



#include	<time.h>



extern char	*tzname[] ;			


static char	*month[] = {
	"Jan","Feb","Mar","Apr","May","Jun",
	"Jul","Aug","Sep","Oct","Nov","Dec",
} ;



getdate(datestr)
char	*datestr ;
{
	struct tm	*timeinfo, *localtime() ;

	long		clock ;


	time(&clock) ;

	timeinfo = localtime(&clock) ;	

	sprintf(datestr,"%2d %3s %4d %2d:%02d %s",
		timeinfo->tm_mday,
		month[timeinfo->tm_mon],
		timeinfo->tm_year + 1900,
		timeinfo->tm_hour,
		timeinfo->tm_min,
		((timeinfo->tm_isdst != 0) ? tzname[1] : tzname[0])) ;

}


