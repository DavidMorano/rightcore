/* uc_libmemalloc (3uc) */
/* lang=C89 */

/* interface component for UNIX® library-3c */
/* memory allocation facility (for library use) */


#define	CF_DEBUGN	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-85, David A­D­ Morano
	This subroutine was originally written.

	= 2001-02-01, David A­D­ Morano
        I expanded the storage table size somewhat for larger programs. We are
        handling larger amounts of data now-a-days!

*/

/* Copyright © 1998,2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is the low-level component of the MEMALLOC facility. These
        subroutines need to be able to be interposed upon, so they have to be in
        their own compilation (object) image.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	NDEBFNAME	"ucmemalloc.deb"

#define	TO_AGAIN	(5 * 60)


/* external subroutines */

extern int	msleep(int) ;

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* local vaiables */


/* exported subroutines */


int uc_libmallocstrw(cchar *sp,int sl,cchar **rpp)
{
	int		rs ;
	int		size ;
	char		*bp ;

	if (rpp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	*rpp = NULL ;
	if (sl < 0) sl = strlen(sp) ;

	size = (sl + 1) ;
	if ((rs = uc_libmalloc(size,&bp)) >= 0) {
	    *rpp = bp ;
	    strncpy(bp,sp,sl) ;
	    bp[sl] = '\0' ;
	}

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (uc_libmallocstrw) */


int uc_libmalloc(int size,void *vp)
{
	const size_t	msize = size ;
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;
	void		**rpp = (void **) vp ;
	void		*rp ;

	if (vp == NULL) return SR_FAULT ;

	*rpp = NULL ;
	if (size <= 0) return SR_INVALID ;

	repeat {
	    rs = SR_OK ;
	    if ((rp = malloc(msize)) == NULL) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
	                msleep(1000) ;
		    } else {
	                f_exit = TRUE ;
		    }
		    break ;
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } else {
	        *rpp = rp ;
	    }
	} until ((rs >= 0) || f_exit) ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"uc_libmalloc: ret a=%p s=%u rs=%d\n",
	    (*rpp),size,rs) ;
#endif

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (uc_libmalloc) */


int uc_libcalloc(int nelem,int esize,void *vp)
{
	const int	size = (nelem*esize) ;
	return uc_libmalloc(size,vp) ;
}
/* end subroutine (uc_libcalloc) */


int uc_libvalloc(int size,void *vp)
{
	const size_t	msize = size ;
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;
	void		**rpp = (void **) vp ;
	void		*rp ;

	if (vp == NULL) return SR_FAULT ;

	*rpp = NULL ;
	if (size <= 0) return SR_INVALID ;

	repeat {
	    rs = SR_OK ;
	    if ((rp = valloc(msize)) == NULL) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
	                msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
		    break ;
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } else {
	        *rpp = rp ;
	    }
	} until ((rs >= 0) || f_exit) ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"uc_libvalloc: ret a=%p s=%u rs=%d\n",
	    (*rpp),size,rs) ;
#endif

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (uc_libvalloc) */


int uc_librealloc(const void *cp,int size,void *vp)
{
	const size_t	msize = size ;
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;
	void		*argp = (void *) cp ;
	void		**rpp = (void **) vp ;
	void		*rp ;

	if (cp == NULL) return SR_FAULT ;
	if (vp == NULL) return SR_FAULT ;

	*rpp = NULL ;
	if (size <= 0) return SR_INVALID ;

/* main function */

	repeat {
	    rs = SR_OK ;
	    if ((rp = realloc(argp,msize)) == NULL) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
	                msleep(1000) ;
		    } else {
	                f_exit = TRUE ;
		    }
		    break ;
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } else {
	        *rpp = rp ;
	    }
	} until ((rs >= 0) || f_exit) ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"uc_librealloc: ret a=%p s=%u oa=%p rs=%d\n",
	    (*rpp),size,cp,rs) ;
#endif

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (uc_librealloc) */


int uc_libfree(const void *vp)
{
	int		rs = SR_OK ;

	if (vp != NULL) {
	    ulong	v = (long) vp ;
	    if ((v & 3) == 0) {
	        void	*fvp = (void *) vp ;
	        free(fvp) ;
	    } else {
		rs = SR_BADFMT ;
	    }
	} else {
	    rs = SR_FAULT ;
	}

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"uc_libfree: ret a=%p rs=%d\n",vp,rs) ;
#endif /* CF_DEBUGN */

	return rs ;
}
/* end subroutine (uc_libfree) */


