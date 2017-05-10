/* process */

/* process a key */


#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 96/03/01, David A­D­ Morano

	The subroutine was adapted from others programs that
	did similar types of functions.


*/


/******************************************************************************

	This subroutine processes one key.


	Synopsis:

	int process(pip,keyname)
	struct proginfo	*pip ;
	const char	keyname[] ;


	Arguments:

	pip		program information pointer
	printer		printer
	keyname		key name to test for


	Returns:

	>=0		good
	<0		error


******************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<svctab.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */



/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */







int process(pip,keyname)
struct proginfo	*pip ;
const char	keyname[] ;
{
	int	rs ;
	int	buflen ;
	int	f ;

	char	buf[BUFLEN + 1] ;


	if (keyname == NULL) 
		return SR_FAULT ;

#if	CF_DEBUG
	debugprintf("process: keyname=%s\n",keyname) ;
#endif

	rs = fdbfetch(pip,keyname,buf,BUFLEN) ;

	buflen = rs ;
	if (rs >= 0) {

	    f = TRUE ;
	    rs = bprintf(pip->ofp,"%t\n",buf,buflen) ;

	} else
	    rs = bprintf(pip->ofp,"\n") ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (process) */



