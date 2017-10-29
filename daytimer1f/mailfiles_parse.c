/* mailfiles_parse */

/* subroutine to parse MAILPATH */


#define	F_DEBUGS	0
#define	F_DEBUG		1



/* revision history :

	= 88/02/01, David A­D­ Morano

	This subroutine was originally written.


	= 88/02/01, David A­D­ Morano

	This subroutine was modified to not write out anything
	to standard output if the access time of the associated
	terminal has not been changed in 10 minutes.


*/



/************************************************************************

	This subroutine looks at the environment variables MAILPATH
	and parses it into its component mailfiles.  Each mailfile
	is added to the 'mailfiles' object.

	int mailfiles_parse(mfp,variable)
	MAILFILES	*mfp ;
	char		*variable ;



*************************************************************************/



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>

#include	"misc.h"
#include	"mailfiles.h"



/* local defines */



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;


/* external variables */


/* local structures */


/* forward references */


/* global data */


/* local data */





int mailfiles_parse(lp,mailpath)
MAILFILES	*lp ;
char		*mailpath ;
{
	int	rs, f_done = FALSE ;

	char	*cp, *cp1 ;


	if ((lp == NULL) || (mailpath == NULL))
	    return SR_FAULT ;

	cp = mailpath ;
	while ((cp1 = strpbrk(cp,":?")) != NULL) {

	    rs = mailfiles_add(lp,cp,(cp1 - cp)) ;

	    if (rs < 0)
	        return rs ;

	    if (*cp1 == '?')
	        f_done = TRUE ;

	    cp = cp1 + 1 ;

	} /* end while */

	if (! f_done)
	    rs = mailfiles_add(lp,cp,-1) ;

	return rs ;
}
/* end subroutine (mailfiles_parse) */



