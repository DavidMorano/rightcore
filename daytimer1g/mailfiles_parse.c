/* mailfiles_parse */

/* subroutine to parse MAILPATH */


#define	CF_DEBUGS	0


/* revision history:

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


	Synposis:

	int mailfiles_parse(mfp,variable)
	MAILFILES	*mfp ;
	char		*variable ;


	Arguments:

	mfp		object pointer
	variable	whatever


	Returns:

	<0		error
	>=0		OK


*************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>

#include	"localmisc.h"
#include	"mailfiles.h"



/* local defines */



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */







int mailfiles_parse(lp,mailpath)
MAILFILES	*lp ;
const char	mailpath[] ;
{
	int	rs, f_done = FALSE ;

	char	*cp, *cp1 ;


	if ((lp == NULL) || (mailpath == NULL))
	    return SR_FAULT ;

	cp = (char *) mailpath ;
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



