/* progname */

/* process a name */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano
        The subroutine was adapated from other programs that do similar
        types of functions.

*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine processes just one name at a time, but it can be either
        a regular file or a directory.


******************************************************************************/


#include	<envstandards.h>

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
#include	<bfile.h>
#include	<field.h>
#include	<wdt.h>
#include	<localmisc.h>

#include	"removename.h"
#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	checkname(const char *,struct ustat *,PROGINFO *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int progname(PROGINFO *pip,cchar *name)
{
	struct ustat	sb ;
	int		rs ;
	int		wopts = 0 ;
	int		f_dir = FALSE ;

	if (name == NULL) return SR_FAULT ;

	rs = u_lstat(name,&sb) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progname: name=%s mode=%0o\n",
	        name,sb.st_mode) ;
#endif

	f_dir = S_ISDIR(sb.st_mode) ;

	if ((! f_dir) && S_ISLNK(sb.st_mode) && pip->f.follow) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("progname: dymbolic link\n") ;
#endif

	    if (u_stat(name,&sb) >= 0)
	        f_dir = S_ISDIR(sb.st_mode) ;

	} /* end if */

	if (f_dir) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progname: directory=%s\n",name) ;
#endif

	    wopts |= (pip->f.follow) ? WDT_MFOLLOW : 0 ;
	    rs = wdt(name,wopts,checkname,pip) ;

	} else
	    rs = checkname(name,&sb,pip) ;

/* finish up */

	if ((rs >= 0) && (! pip->f.no)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progname: removename() name=%s\n",name) ;
#endif

	    rs = removename(pip->rvp, pip->bcount, pip->rn_opts, name) ;

	    if ((rs < 0) && isNotPresent(rs)) rs = SR_OK ;
	} /* end if (remove-all) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progname) */


