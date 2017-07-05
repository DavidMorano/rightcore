/* removename */

/* remove a named file-system object (and its descendants) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano
        The subroutine was adapated from other programs that do similar types of
        functions.

*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This function removes a named UNIX file-system object along with
	all of its descendents (if any).

	Synopsis:

	int removename(name,rvp,opts,bcount)
	const char	name[] ;
	randomvar	*rvp ;
	int		opts ;
	int		bcount ;

	Arguments:

	name		name of FS object to remove
	rvp		pointer to RADNDOMVAR object
	opts		options
	bcount		burn-count (number of times to burn files)

	Returns:

	>=0		count of objects (files or directories) removed
	<0		error


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<wdt.h>
#include	<vecpstr.h>
#include	<randomvar.h>
#include	<localmisc.h>

#include	"removename.h"


/* local defines */


/* external subroutines */

extern int	burn(randomvar	*,int,const char *) ;


/* external variables */


/* global variables */


/* local structures */

struct removeinfo_flags {
	uint		burn:1 ;
	uint		follow:1 ;
} ;

struct removeinfo {
	randomvar		*rvp ;
	struct removeinfo_flags	f ;
	VECPSTR			dirs ;
	uint			c_removed ;
	int			bcount ;
} ;


/* forward references */

static int	removeit(const char *,struct ustat *,struct removeinfo *) ;
static int	rmdirs(struct removeinfo *) ;


/* local variables */


/* exported subroutines */


int removename(rvp,bcount,opts,name)
randomvar	*rvp ;
int		bcount ;
int		opts ;
const char	name[] ;
{
	struct ustat	sb, sb2 ;

	struct removeinfo	ri ;

	randomvar	x ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	wopts = 0 ;
	int	f_rv = FALSE ;


#if	CF_DEBUGS
	debugprintf("removename: opts=%04x bcount=%d\n",opts,bcount) ;
#endif

	if (name == NULL)
	    return SR_FAULT ;

	rs = u_lstat(name,&sb) ;
	if (rs < 0)
	    goto ret0 ;

/* initialize the "removeinfo" object */

	memset(&ri,0,sizeof(struct removeinfo)) ;

	ri.f.burn = (opts & REMOVENAME_MBURN) ? 1 : 0 ;
	ri.f.follow = (opts & REMOVENAME_MFOLLOW) ? 1 : 0 ;

#if	CF_DEBUGS
	debugprintf("removename: burn=%d follow=%d\n",
	    ri.f.burn,ri.f.follow) ;
#endif

	ri.rvp = rvp ;
	ri.bcount = bcount ;
	ri.c_removed = 0 ;

	if (ri.f.burn && (rvp == NULL)) {

	    rs = randomvar_start(&x,FALSE,opts) ;
	    if (rs < 0)
	        goto ret0 ;

	    f_rv = TRUE ;
	    ri.rvp = &x ;

	} /* end if (we need our own random variable) */

	if ((rs = vecpstr_start(&ri.dirs,10,0,0)) >= 0) {

/* continue with our processing */

#if	CF_DEBUGS
	    debugprintf("removename: name=\"%s\" mode=%0o\n",
	        name,sb.st_mode) ;
#endif

	    if (S_ISLNK(sb.st_mode)) {

	        if (ri.f.follow &&
	            (u_stat(name,&sb2) >= 0) && S_ISDIR(sb2.st_mode)) {

#if	CF_DEBUGS
	            debugprintf("removename: wdt() name=%s\n",name) ;
#endif

	            wopts |= ((ri.f.follow) ? WDT_MFOLLOW : 0) ;
	            rs = wdt(name,wopts,removeit,&ri) ;

	            if (rs >= 0)
	                rs = vecpstr_add(&ri.dirs,name,-1) ;

	        } else {

	            rs = removeit(name,&sb,&ri) ;

	        } /* end if */

	    } else if (S_ISDIR(sb.st_mode)) {

	        wopts |= ((ri.f.follow) ? WDT_MFOLLOW : 0) ;
	        rs = wdt(name,wopts,removeit,&ri) ;

	        if (rs >= 0)
	            rs = vecpstr_add(&ri.dirs,name,-1) ;

	    } else {

	        rs = removeit(name,&sb,&ri) ;

	    } /* end if */

/* remove the directories */

#if	CF_DEBUGS
	    debugprintf("removename: remove the directories\n") ;
#endif

	    if (rs >= 0)
	        rs = rmdirs(&ri) ;

	    rs1 = vecpstr_finish(&ri.dirs) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecpstr) */

	if (f_rv) {
	    rs1 = randomvar_finish(&x) ;
	    if (rs >= 0) rs = rs1 ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("removename: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? ri.c_removed : rs ;
}
/* end subroutine (removename) */


/* local subroutines */


static int removeit(name,sbp,rip)
const char		name[] ;
struct ustat		*sbp ;
struct removeinfo	*rip ;
{
	struct ustat	sb2 ;

	int	rs = SR_OK ;


#if	CF_DEBUGS
	debugprintf("removeit: name=%s\n",name) ;
#endif

	if (S_ISLNK(sbp->st_mode)) {

#if	CF_DEBUGS
	    debugprintf("removeit: got symlink=%s\n",name) ;
#endif

	    rs = (rip->f.follow) ? 0 : 1 ;
	    if (rip->f.follow &&
	        (u_stat(name,&sb2) >= 0) && S_ISDIR(sb2.st_mode)) {

#if	CF_DEBUGS
	        debugprintf("removeit: adding=%s\n",name) ;
#endif

	        rs = vecpstr_add(&rip->dirs,name,-1) ;

	    } else {

	        rs = u_unlink(name) ;
	        if (rs >= 0)
	            rip->c_removed += 1 ;

	    } /* end if */

	} else if (S_ISDIR(sbp->st_mode)) {

	    rs = vecpstr_add(&rip->dirs,name,-1) ;

	} else {

	    if (rip->f.burn && S_ISREG(sbp->st_mode))
	        rs = burn(rip->rvp,rip->bcount,name) ;

	    if (rs >= 0) {

	        rs = u_unlink(name) ;
	        if (rs >= 0)
	            rip->c_removed += 1 ;

	    } /* end if */

	} /* end if */

	return rs ;
}
/* end subroutine (removeit) */


static int rmdirs(rip)
struct removeinfo	*rip ;
{
	VECPSTR	*dlp = &rip->dirs ;

	int	rs ;
	int	n = 0 ;

	const char	*cp ;


	if ((rs = vecpstr_count(dlp)) > 0) {
	    n = rs ;
	    if ((rs = vecpstr_sort(dlp,NULL)) >= 0) {
	        int	i ;
	        for (i = (n - 1) ; i >= 0 ; i -= 1) {
	            if (vecpstr_get(dlp,i,&cp) >= 0) {
	                if (cp != NULL) {
	                    rs = u_rmdir(cp) ;
	                    if (rs >= 0)
	                        rip->c_removed += 1 ;
	                }
	            }
	        } /* end for */
	    } /* end if */
	} /* end if */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (rmdirs) */



