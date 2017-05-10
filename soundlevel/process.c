/* process */

/* process a name */


#define	CF_DEBUG	0


/* revision history:

	= 96/03/01, David A­D­ Morano

	The subroutine was adapted from others programs that
	did similar types of functions.


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

#include	<bfile.h>
#include	<baops.h>
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

extern char	*strbasename() ;


/* local forward references */


/* external variables */


/* global variables */


/* local variables */






int process(pip,name,pp)
struct proginfo	*pip ;
char		name[] ;
PARAMOPT	*pp ;
{
	struct ustat	sb ;

	struct checkparams	ck ;

	int	rs ;


	if (name == NULL) 
		return BAD ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("process: entered name=\"%s\"\n",name) ;
#endif

	if (u_stat(name,&sb) < 0) 
		return BAD ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("process: name=\"%s\" mode=%0o\n",
			name,sb.st_mode) ;
#endif

	ck.pip = pip ;
	ck.pp = pp ;

	if (S_ISDIR(sb.st_mode)) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("process: calling wdt\n") ;
#endif

		rs = wdt(name,WDTM_FOLLOW,checkname,&ck) ;

	} else
		rs = checkname(name,&sb,&ck) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("process: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */



