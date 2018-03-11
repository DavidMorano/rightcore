/* lockmemalloc */
/* lang=C89 */

/* memory allocation facility w/ locking */


#define	CF_DEBUGN	0		/* non-switchable debug print-outs */
#define	CF_MALLOCSTRW	0		/* compile in |uc_libmallocstrw()| */


/* revision history:

	= 2015-04-06, David A­D­ Morano
	These subroutines were originally written to get around the lack of a
	mutex lock around the KSH (AST) memory allocation subroutines.  I have
	put this off for a long time now.  I had tried to avoid this little
	scheme for some time.  And, YES, I am pissed off that KSH sucks cock
	meat by not locking its memory allocation subroutines!

*/

/* Copyright © 2015 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a hack to get around the fact that the KSH program does not
	have a mutex around its memory allocation subroutines.

	Although the KSH program wants to be sigle-threaded, the problem comes
	when complicated built-in commands are dynamically loaded.  These
	built-ins can call subroutines that are multi-threaded.  This usually
	leads to a program crash due to no mutex lock being around the memory
	allocation facility (using the familiar subroutine names that we all
	know and love: |malloc(3c)| and friends) used by KSH.

	The only real (semi-user visible) subroutine of this module is
	|lockmemalloc_set(3uc)|.

	Synopsis:

	int lockmemalloc_set(int cmd)

	Arguments:

	cmd		command to facility

	Returns:

	<0		fail
	>=0		OK

	Notes:

	= Commands are one of:

	0		turn locking OFF
	1		turn locking ON

	By default, locking is OFF.  This is done so that when software is
	linked with this module (by mistake, confusion, or otherwise), it will
	not suffer the performance overhead of the locking.

	When KSH loads a built-in and does so from a new shared library, it
	looks for a subroutine named |lib_init()| in the associated shared
	library and calls it.  This above subroutine
	(|lockmemalloc_set(3lkcmd)|) is called from the |lib_init(3lkcmd)|
	subroutine to turn locking ON within the module that is loaded by KSH
	(usually the shared-memory object that this subroutine is linked with
	as a bundle).

	= How to use:

	Put this module somewhere so that it interposes itself upon the "real"
	subroutines with these same names in the lower-level UNIX® library
	adaptation layer |libuc(3uc)|.

	= Compilation options:

        + CF_MALLOCSTRW - This switch requests that the subroutine
        |uc_libmallocstrw()| be compiled into this module. This might be wanted
        if the module which otherwise contains the subroutine |uc_libmalloc()|
        is linked with the Solaris® "symbolic" mode. That mode ("symbolic")
        makes references from a given module be linked to the symbols in that
        same module if those sumbols are present. Since the subroutine
        |uc_libmallocstrw()| calls |uc_libmalloc()|, this compile-time switch
        might be wanted if the module containing |uc_libmallocstrw()| is linked
        with the "symbolic" mode. We want the subroutine |uc_libmallocstrw()|
        (where ever it is located) to always call us (this module) if this
        module is linked into a program.


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
#include	<ptm.h>
#include	<localmisc.h>

#include	"lockmemalloc.h"


/* local defines */

#define	LOCKMEMALLOC	struct lockmemalloc

#define	NDF		"lockmemalloc.deb"

#define	TO_AGAIN	(5 * 60)


/* external subroutines */

extern int	msleep(int) ;

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
#endif


/* local structures */

struct lockmemalloc {
	PTM		m ;		/* data mutex */
	volatile uint	f_init ;
	volatile uint	f_initdone ;
	volatile uint	f_lock ;
} ;


/* forward references */

int		lockmemalloc_init() ;
void		lockmemalloc_fini() ;

int		uc_libmalloc(int,void *) ;

static int	lockmemalloc_basemalloc(int,void *) ;
static int	lockmemalloc_basevalloc(int,void *) ;
static int	lockmemalloc_baserealloc(const void *,int,void *) ;
static int	lockmemalloc_basefree(const void *) ;

static int	lockmemalloc_lockmalloc(int,void *) ;
static int	lockmemalloc_lockvalloc(int,void *) ;
static int	lockmemalloc_lockrealloc(const void *,int,void *) ;
static int	lockmemalloc_lockfree(const void *) ;

static void	lockmemalloc_atforkbefore() ;
static void	lockmemalloc_atforkafter() ;


/* local vaiables */

static LOCKMEMALLOC	lockmemalloc_data = { PTHREAD_MUTEX_INITIALIZER } ;


/* exported subroutines */


int lockmemalloc_init()
{
	LOCKMEMALLOC	*uip = &lockmemalloc_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    {
	        void	(*b)() = lockmemalloc_atforkbefore ;
	        void	(*a)() = lockmemalloc_atforkafter ;
	        if ((rs = uc_atfork(b,a,a)) >= 0) {
	            if ((rs = uc_atexit(lockmemalloc_fini)) >= 0) {
	                uip->f_initdone = TRUE ;
			f = TRUE ;
	            }
	            if (rs < 0)
	                uc_atforkrelease(b,a,a) ;
		}
	    } /* end block */
	} else {
	    while ((rs >= 0) && uip->f_init && (! uip->f_initdone)) {
		rs = msleep(1) ;
		if (rs == SR_INTR) break ;
	    }
	    if ((rs >= 0) && (! uip->f_init)) rs = SR_LOCKLOST ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (lockmemalloc_init) */


void lockmemalloc_fini()
{
	LOCKMEMALLOC	*uip = &lockmemalloc_data ;

	if (uip->f_initdone) {
	    {
	        void	(*b)() = lockmemalloc_atforkbefore ;
	        void	(*a)() = lockmemalloc_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(lockmemalloc_data)) ;
	} /* end if (atexit registered) */

}
/* end subroutine (lockmemalloc_fini) */


int lockmemalloc_set(int cmd)
{
	LOCKMEMALLOC	*uip = &lockmemalloc_data ;
	int		rs ;

	if ((rs = lockmemalloc_init()) >= 0) {
	    switch (cmd) {
	    case lockmemallocset_end:
	        uip->f_lock = FALSE ;
	        break ;
	    case lockmemallocset_begin:
	        uip->f_lock = TRUE ;
	        break ;
	    default:
	        rs = SR_NOTSUP ;
	        break ;
	    } /* end switch */
	} /* end if (init) */

	return rs ;
}
/* end subroutine (lockmemalloc_set) */


#if	CF_MALLOCSTRW
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
#endif /* CF_MALLOCSTRW */


int uc_libmalloc(int size,void *vp)
{
	LOCKMEMALLOC	*uip = &lockmemalloc_data ;
	int		rs ;

	if (uip->f_lock) {
	    rs = lockmemalloc_lockmalloc(size,vp) ;
	} else {
	    rs = lockmemalloc_basemalloc(size,vp) ;
	}

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
	LOCKMEMALLOC	*uip = &lockmemalloc_data ;
	int		rs ;

	if (uip->f_lock) {
	    rs = lockmemalloc_lockvalloc(size,vp) ;
	} else {
	    rs = lockmemalloc_basevalloc(size,vp) ;
	}

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (uc_libvalloc) */


int uc_librealloc(const void *cp,int size,void *vp)
{
	LOCKMEMALLOC	*uip = &lockmemalloc_data ;
	int		rs ;

	if (uip->f_lock) {
	    rs = lockmemalloc_lockrealloc(cp,size,vp) ;
	} else {
	    rs = lockmemalloc_baserealloc(cp,size,vp) ;
	}

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (uc_librealloc) */


int uc_libfree(const void *vp)
{
	LOCKMEMALLOC	*uip = &lockmemalloc_data ;
	int		rs ;

	if (uip->f_lock) {
	    rs = lockmemalloc_lockfree(vp) ;
	} else {
	    rs = lockmemalloc_basefree(vp) ;
	}

	return rs ;
}
/* end subroutine (uc_libfree) */


/* local subroutines */


static int lockmemalloc_basemalloc(int size,void *vp)
{
	const size_t	msize = size ;
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;
	void		**rpp = (void **) vp ;
	void		*rp ;

	if (vp == NULL) return SR_FAULT ;

	*rpp = NULL ;
	if (size <= 0)
	    return SR_INVALID ;

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
	nprintf(NDF,"lockmemalloc_basemalloc: ret a=%p s=%u rs=%d\n",
	    (*rpp),size,rs) ;
#endif

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (lockmemalloc_basemalloc) */


static int lockmemalloc_basevalloc(int size,void *vp)
{
	const size_t	msize = size ;
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;
	void		**rpp = (void **) vp ;
	void		*rp ;

	if (vp == NULL) return SR_FAULT ;

	*rpp = NULL ;
	if (size <= 0)
	    return SR_INVALID ;

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
	nprintf(NDF,"lockmemalloc_basevalloc: ret a=%p s=%u rs=%d\n",
	    (*rpp),size,rs) ;
#endif

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (lockmemalloc_basevalloc) */


static int lockmemalloc_baserealloc(const void *cp,int size,void *vp)
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
	if (size <= 0)
	    return SR_INVALID ;

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

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (lockmemalloc_baserealloc) */


static int lockmemalloc_basefree(const void *vp)
{
	int		rs = SR_OK ;

	if (vp != NULL) {
	    ulong	v = (ulong) vp ;
	    if ((v & 3) == 0) {
	        void	*fvp = (void *) vp ;
	        free(fvp) ;
	    } else
	        rs = SR_BADFMT ;
	} else
	    rs = SR_FAULT ;

#if	CF_DEBUGN
	nprintf(NDF,"lockmemalloc_basefree: ret a=%p rs=%d\n",vp,rs) ;
#endif /* CF_DEBUGN */

	return rs ;
}
/* end subroutine (lockmemalloc_basefree) */


static int lockmemalloc_lockmalloc(int size,void *vp)
{
	LOCKMEMALLOC	*uip = &lockmemalloc_data ;
	int		rs ;
	int		rs1 ;

	if ((rs = lockmemalloc_init()) >= 0) {
	    if ((rs = ptm_lock(&uip->m)) >= 0) {
		{
	            rs = lockmemalloc_basemalloc(size,vp) ;
		}
	        rs1 = ptm_unlock(&uip->m) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	} /* end if (init) */

	return rs ;
}
/* end subroutine (lockmemalloc_lockmalloc) */


static int lockmemalloc_lockvalloc(int size,void *vp)
{
	LOCKMEMALLOC	*uip = &lockmemalloc_data ;
	int		rs ;
	int		rs1 ;

	if ((rs = lockmemalloc_init()) >= 0) {
	    if ((rs = ptm_lock(&uip->m)) >= 0) {
		{
	            rs = lockmemalloc_basevalloc(size,vp) ;
		}
	        rs1 = ptm_unlock(&uip->m) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	} /* end if (init) */

	return rs ;
}
/* end subroutine (lockmemalloc_lockvalloc) */


static int lockmemalloc_lockrealloc(const void *cp,int size,void *vp)
{
	LOCKMEMALLOC	*uip = &lockmemalloc_data ;
	int		rs ;
	int		rs1 ;

	if ((rs = lockmemalloc_init()) >= 0) {
	    if ((rs = ptm_lock(&uip->m)) >= 0) {
		{
	            rs = lockmemalloc_baserealloc(cp,size,vp) ;
		}
	        rs1 = ptm_unlock(&uip->m) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	} /* end if (init) */

	return rs ;
}
/* end subroutine (lockmemalloc_lockrealloc) */


static int lockmemalloc_lockfree(const void *vp)
{
	LOCKMEMALLOC	*uip = &lockmemalloc_data ;
	int		rs ;
	int		rs1 ;

	if ((rs = lockmemalloc_init()) >= 0) {
	    if ((rs = ptm_lock(&uip->m)) >= 0) {
		{
	            rs = lockmemalloc_basefree(vp) ;
		}
	        rs1 = ptm_unlock(&uip->m) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mutex) */
	} /* end if (init) */

	return rs ;
}
/* end subroutine (lockmemalloc_lockfree) */


static void lockmemalloc_atforkbefore()
{
	LOCKMEMALLOC	*uip = &lockmemalloc_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (lockmemalloc_atforkbefore) */


static void lockmemalloc_atforkafter()
{
	LOCKMEMALLOC	*uip = &lockmemalloc_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (lockmemalloc_atforkafter) */


