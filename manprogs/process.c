/* process */

/* process a name */


#define	CF_DEBUG	1


/* revision history:

	= David A.D. Morano, March 1996
	The program was written from scratch to do what
	the previous program by the same name did.

*/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<errno.h>

#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<wdt.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */



/* external subroutines */

extern char	*strbasename() ;


/* local forward references */


/* external variables */


/* global variables */


/* local variables */




int process(gp,name)
struct global	*gp ;
char		name[] ;
{
	struct ustat	sb ;

	int	rs ;


	if (name == NULL) return BAD ;

	if (u_stat(name,&sb) < 0) return BAD ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
		debugprintf("process: name=\"%s\" mode=%0o\n",
			name,sb.st_mode) ;
#endif

	if (S_ISDIR(sb.st_mode)) {

#if	CF_DEBUG
	if (gp->debuglevel > 1)
		debugprintf("process: calling wdt\n") ;
#endif

		rs = wdt(name,WDTM_FOLLOW,checkname,gp) ;

	} else
		rs = checkname(name,&sb,gp) ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
		debugprintf("process: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */



