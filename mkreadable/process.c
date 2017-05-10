/* process */

/* process a name */


#define	CF_DEBUG	0


/* revision history:

	= 96/03/01, David A­D­ Morano

	The program was written from scratch to do what
	the previous program by the same name did.


*/


/******************************************************************************

	This module just provides optional expansion of directories.
	The real work is done by the 'checkname' module.


******************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<wdt.h>

#include	"localmisc.h"
#include	"paramopt.h"
#include	"config.h"
#include	"defs.h"



/* local defines */



/* external subroutines */

extern int	wdt() ;
extern int	checkname() ;


/* forward references */


/* external variables */


/* global variables */


/* local variables */






int process(pip,name,pp)
struct proginfo	*pip ;
const char	name[] ;
PARAMOPT	*pp ;
{
	struct ustat	sb ;

	struct checkparams	ck ;

	int	rs ;


	if (name == NULL) 	
		return SR_FAULT ;

	rs = u_stat(name,&sb) ;

	if (rs < 0)
		goto ret0 ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("process: name=\"%s\" mode=%0o\n",
			name,sb.st_mode) ;
#endif

	ck.gp = gp ;
	ck.pp = pp ;

	if (S_ISDIR(sb.st_mode)) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("process: calling wdt\n") ;
#endif

		rs = wdt(name,WDTM_FOLLOW,checkname,&ck) ;

	} else
		rs = checkname(name,&sb,&ck) ;

ret0:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("process: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */



