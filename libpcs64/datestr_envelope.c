/* datestr_envelope */

/* create a date-str for the UNIX mail envelope */


/* revision history:

	= 1998-01-10, David A­D­ Morano
        This subroutine was written to replace the previous one. This version
        simply calls 'timestr_edate()'.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	  Places today's date and time into the string "datestr".  
	  The format is RFC822 standard is:
		   27 Nov 1981 13:17 EST
		    1 Mar 1982  7:53 EDT

	This kind of date string is used in the DATE header of the message.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>

#include	<localmisc.h>


/* external subroutines */

extern char	*timestr_edate(time_t,char *) ;
extern char	*timestr_hdate(time_t,char *) ;


/* external variables */


/* local (static) variables */


/* exported subroutines */


char *datestr_envelope(time_t date,char *datestr)
{

	return timestr_edate(date,datestr) ;
}
/* end subroutine (datestr_envelope) */


