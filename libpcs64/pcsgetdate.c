/* pcsgetdate */


/* important note: ABANDONED!! */


#define	CF_DEBUG	0		/* compile-time debugging */


/* revision history:

	= 1994-02-01, David A­D­ Morano
        This subroutine was started to be written, from scratch! Then I
        discovered 'getabsdate()' and I abandoned this work but left it here
        dangling anyway.

*/

/* Copyright © 1994 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
 
	This subroutine parses a UNIX® time value out of a date string.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>

#include	<localmisc.h>


/* local defines */

#define	PCS_DATEMSK	"lib/datemsk"


/* external subroutines */


/* external variables */

extern char		*tzname[] ;			


/* local (static) variables */

#ifdef	COMMENT
static char	*month[] = {
	"Jan","Feb","Mar","Apr","May","Jun",
	"Jul","Aug","Sep","Oct","Nov","Dec",
} ;
#endif /* COMMENT */


/* exported subroutines */


int pcsgetdate(pcs,datemsk,datestr,timep)
char		pcs[] ;
char		datemsk[] ;
char		datestr[] ;
struct tm	*timep ;
{
	struct tm	ts ;

	char	*datemskp ;


	if (timep == NULL) return BAD ;

	if ((datemsk == NULL) || (datemsk[0] == '/')) {
		if ((pcs == NULL) || (pcs[0] == '\0'))
			pcs = getenv("PCS") ;
	}



	return OK ;
}
/* end subroutine (pcsgetdate) */


