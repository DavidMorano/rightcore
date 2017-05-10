/* process */

/* process a name */


#define	CF_DEBUG	1


/* revision history:

	= 96/03/01, David A­D­ Morano

	The program was written from scratch to do what
	the previous program by the same name did.


*/


/******************************************************************************

	This subroutine processes one name at a time.  The name can
	be either a file or a directory.



******************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<wdt.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif



/* external subroutines */

extern int	checkname(char *, struct ustat *, struct proginfo *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */






int process(pip,name)
struct proginfo	*pip ;
char		name[] ;
{
	struct ustat	sb, sb2 ;

	int	rs ;
	int	wopts ;


	if (name == NULL)
	    return SR_FAULT ;

	rs = u_lstat(name,&sb) ;

	if (rs < 0)
	    return rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process: name=\"%s\" mode=%0o\n",
	        name,sb.st_mode) ;
#endif

	if (S_ISLNK(sb.st_mode)) {

	    if (pip->f.follow &&
		(u_stat(name,&sb2) >= 0) && S_ISDIR(sb2.st_mode)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	        debugprintf("process: calling wdt\n") ;
#endif

	        wopts = (pip->f.follow) ? WDT_MFOLLOW : 0 ;
	        rs = wdt(name,wopts,checkname,pip) ;

	    } else
	        rs = checkname(name,&sb,pip) ;

	} else if (S_ISDIR(sb.st_mode)) {

	        wopts = (pip->f.follow) ? WDT_MFOLLOW : 0 ;
	        rs = wdt(name,wopts,checkname,pip) ;

	} else
	    rs = checkname(name,&sb,pip) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */



