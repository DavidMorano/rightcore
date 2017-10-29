/* removes */

/* removes (recursively as necessary) a directory tree */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2002-07-13, David A­D­ Morano

	This was originally written.


*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine deletes (removes) a directory tree.

	Synopsis:

	int removes(const char *dname)

	Arguments:

	dname	directory name (or regular file) to unlink

	Returns:

	>=0	bytes of noise returned
	<0	error


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<fsdir.h>
#include	<vecpstr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;


/* local structures */


/* forward references */

static int	removedir(const char *) ;
static int	loadnames(VECPSTR *,const char *) ;
static int	isdoubledot(const char *,int) ;


/* local variables */


/* exported subroutines */


int removes(const char *tardname)
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	c = 0 ;

	if (tardname == NULL)
	    return SR_FAULT ;

	if (tardname[0] == '\0')
	    return SR_INVALID ;

	if ((rs = uc_lstat(tardname,&sb)) >= 0) {
	    if (S_ISDIR(sb.st_mode)) {
	        rs = removedir(tardname) ;
	        c = rs ;
	    } else {
	        rs = u_unlink(tardname) ;
	        c = 1 ;
	    }
	} /* end if (lstat) */

#if	CF_DEBUGS
	debugprintf("removes: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs > 0) ? c : rs ;
}
/* end subroutine (removes) */


/* local subroutines */


static int removedir(const char *tardname)
{
	struct ustat	sb ;

	VECPSTR	names ;

	int	rs ;
	int	c = 0 ;

	if ((rs = vecpstr_start(&names,0,0,0)) >= 0) {

	    if ((rs = loadnames(&names,tardname)) > 0) {
		const int	size = (MAXPATHLEN+1) ;
		char		*tarfname ;

		if ((rs = uc_malloc(size,&tarfname)) >= 0) {
		    int		i ;
		    const char	*np ;

	            for (i = 0 ; vecpstr_get(&names,i,&np) >= 0 ; i += 1) {
	                if (np == NULL) continue ;

	                if ((rs = mkpath2(tarfname,tardname,np)) >= 0) {
	                    if ((rs = uc_lstat(tarfname,&sb)) >= 0) {
	                        if (S_ISDIR(sb.st_mode)) {
	                            rs = removedir(tarfname) ;
	                            c += rs ;
	                        } else {
	                            rs = u_unlink(tarfname) ;
	                            c += 1 ;
	                        }
	                    } /* end if (stat) */
	                } /* end if (mkpath2) */

	                if (rs < 0) break ;
	            } /* end for */

		    uc_free(tarfname) ;
		} /* end if (memory-allocation) */

	    } /* end if (loadnames) */

	        if (rs >= 0) {
	            rs = u_rmdir(tardname) ;
	            c += 1 ;
	        }

	    vecpstr_finish(&names) ;
	} /* end if (names) */

	return (rs > 0) ? c : rs ;
}
/* end subroutine (removedir) */


static int loadnames(nlp,dname)
VECPSTR		*nlp ;
const char	*dname ;
{
	fsdir		d ;
	fsdir_ent	ds ;

	int	rs ;
	int	c = 0 ;


	if ((rs = fsdir_open(&d,dname)) >= 0) {
	    int		nl ;
	    const char	*np ;

	    while ((rs = fsdir_read(&d,&ds)) > 0) {
		nl = rs ;
		np = ds.name ;

	        if ((np[0] != '.') || (! isdoubledot(np,nl))) {
	            c += 1 ;
	            rs = vecpstr_add(nlp,np,nl) ;
	        }

	        if (rs < 0) break ;
	    } /* end while */

	    fsdir_close(&d) ;
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadnames) */


static int isdoubledot(np,nl)
const char	*np ;
int		nl ;
{
	int	f = ((nl == 2) && (np[1] == '.')) ;
	return f ;
}
/* end subroutine (isdoubledot) */



