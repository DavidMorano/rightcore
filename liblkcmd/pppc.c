/* percache */

/* persistent cache */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	This subroutine was originally written.  

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int percache_xxx(PERCACHE *op,...)

	where:

	op		pointer to particular data instance (many are possible)

	Returns:

	<0		error
	>=0		OK, w/ possible useful value


	Note:

	We need to (should) use the memory allocation subroutines:

	+ uc_libmalloc(3uc)
	+ uc_libfree(3uc)

        since we deal with data that is persistent across full instances of
        command executions. That is: our data stays valid throughout the start
        and complete exit of a command program. Therefore normal conventions
        about what data was allocated and left un-freed do not apply to our
        dynamic memory usage(s). To someone looking at this code to add or
        modify it, it means that these memory allocations are not tracked like
        normal ones are. Therefore try not to mess up by creating memory leaks,
        where there are *none* right now! Get it?

	Notes:

	Just so an observer (like myself later on) won't go too
	crazy trying to understand what is going on with the 'struct
	percache' local data, it is a persistent data structure.
	This program can operate as both a regular program (is
	flushed from system memory when it exits) or it can be
	operated as sort of a terminate-stay-resident (TSR) program
	(its data is not flushed when it exists).  We detect which
	it is (which mode we are executing in) dynamically.  We do
	this by simply looking at the persistent data and seeing
	if elements of it are non-zero.  Any non-zero data indicates
	that we have already been executed in the past.  This data
	is allocated in the BSS section of our process memory map
	so it is initialized to all-zero on program-load (a UNIX
	standard now for? over twenty years!).

	Hopefully, everything else now makes sense upon inspection with
	this understanding.

	Why do this?  Because it speeds things up.  Everything in this
	program is already quite fast, but we have the chance of reducing
	some file-access work with the introduction of a persistent
	data cache.  It is hard to get faster than a single file-access
	(like a shared-memory cache), so anything worth doing has to
	be a good bit faster than that.  Hence, pretty much only TSR
	behavior can beat a single file access.

	Parallelism?  There isn't any, so we don't have to worry about
	using mutexes or semaphores.  Maybe someday we will have to
	think about parallelism, but probably not any time soon!

	OK, now for some of the unpleasantness.  We have to release
	persistent data that was dynamically allocated with the
	PERCACHE facility.  We do this by calling |percache_fini()| on
	our particular data handle.  But this should be done at module
	unload time, so we need to register a subroutine to do this
	that is called at module-unload time.  That subroutine will be
	|ourfini()|.  The registration is only done when we are
	*exiting* this command.  This is done so that we do not perform
	the registration if the PERCACHE facility was never used.  The
	cache facility keeps track within itself whether it was used or
	not (among all possible simulraneously users also).  We ask its
	opinion on that and act accordingly.  Only one "user" (command)
	within the same module will be given notice to perform a
	registration.  However if multiple users (commands) do register
	a |fini()| (needlessly) it is dealt with without problems
	(extras are ignored as expected).  Welcome to TSR management
	when in a very dynamic execution environment!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<utmpacc.h>
#include	<localmisc.h>

#include	"percache.h"


/* local defines */


/* external subroutines */

extern int	snfsflags(char *,int,ulong) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	statvfsdir(const char *,struct statvfs *) ;
extern int	getsysdomain(char *,int) ;
extern int	localgetnetload(const char *,char *,int) ;
extern int	localgetsystat(const char *,char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	isNoProcs(int rs) ;


/* local variables */

static const int	timeouts[] = {
	(1*60*60),	/* hostid */
	5,		/* nprocs */
	(5*3600),	/* btime (boot-time) */
	20,		/* run-level */
	5, 		/* nusers */
	(1*60*60),	/* sysdomain */
	(5*60),		/* netload */
	(5*60),		/* systat */
	0
} ;


/* exported subroutines */


int percache_init(PERCACHE *pcp)
{
	int		rs = SR_OK ;

	if (! pcp->f_init) {
	    pcp->f_init = TRUE ;
	    pcp->f_initdone = TRUE ;
	} /* end if (needed initialization) */

	return rs ;
}
/* end subroutine (percache_init) */


int percache_fini(PERCACHE *pcp)
{
	int		rs = SR_OK ;

	if (pcp->f_initdone) {
	    int	rs1 ;
	    if (pcp->sysdomain != NULL) {
		rs1 = uc_libfree(pcp->sysdomain) ;
		if (rs >= 0) rs = rs1 ;
		pcp->sysdomain = NULL ;
	    }
	    memset(pcp,0,sizeof(PERCACHE)) ;
	} /* end if (was inited) */

	return rs ;
}
/* end subroutine (percache_fini) */


/* is a "fini" registration required? */
int percache_finireg(PERCACHE *pcp)
{
	int		f = FALSE ;
	if (pcp->f_init) {
	    f = ((! pcp->f_finireg) && (pcp->sysdomain != NULL)) ;
	    if (f) pcp->f_finireg = TRUE ;
	}
	return f ;
}
/* end subroutine (percache_finireg) */


int percache_invalidate(PERCACHE *pcp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pcp->f_init) {
	    const int	size = (sizeof(PERCACHE_ITEM) * pertype_overlast) ;
	    memset(pcp->items,0,size) ;
	    if (pcp->sysdomain != NULL) {
	        rs1 = uc_libfree(pcp->sysdomain) ;
	        if (rs >= 0) rs = rs1 ;
	        pcp->sysdomain = NULL ;
	    }
	} /* end if (initialized) */

	return rs ;
}
/* end subroutine (percache_invalidate) */


int percache_gethostid(PERCACHE *pcp,time_t dt,uint *hip)
{
	const int	pt = pertype_hostid ;
	int		rs ;

	if (pcp == NULL) return SR_FAULT ;

	if ((rs = percache_init(pcp)) >= 0) {
	    time_t		vt ;
	    int	to ;
	    vt = pcp->items[pt].t ;
	    to = timeouts[pt] ;
	    if ((dt - vt) > to) {
	        uint	uv = gethostid() ;
	        pcp->items[pt].t = dt ;
	        pcp->items[pt].v = uv ;
	    } /* end if */
	    *hip = pcp->items[pt].v ;
	} /* end if (init) */

	return rs ;
}
/* end subroutine (percache_gethostid) */


int percache_getnprocs(PERCACHE *pcp,time_t dt)
{
	const int	pt = pertype_nprocs ;
	int		rs ;
	int		n = 0 ;

	if (pcp == NULL) return SR_FAULT ;

	if ((rs = percache_init(pcp)) >= 0) {
	    time_t	vt ;
	    int		to ;
	    vt = pcp->items[pt].t ;
	    n = pcp->items[pt].v ;
	    to = timeouts[pt] ;
	    if ((dt - vt) > to) {
	        if ((rs = uc_nprocs(0)) >= 0) {
	            n = rs ;
	        } else if (isNoProcs(rs)) {
		    rs = SR_OK ;
		}
	        pcp->items[pt].t = dt ;
	        pcp->items[pt].v = n ;
	    } /* end if */
	} /* end if (init) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (percache_getnprocs) */


int percache_getbtime(PERCACHE *pcp,time_t dt,time_t *btp)
{
	const int	pt = pertype_btime ;
	int		rs ;

	if (pcp == NULL) return SR_FAULT ;

	if ((rs = percache_init(pcp)) >= 0) {
	    time_t	vt ;
	    int		to ;
	    vt = pcp->items[pt].t ;
	    to = timeouts[pt] ;
	    if ((dt - vt) > to) {
	        time_t	bt ;
	        if ((rs = utmpacc_boottime(&bt)) >= 0) {
	            pcp->items[pt].t = dt ;
	            pcp->items[pt].v = bt ;
	        }
	    } /* end if */
	    *btp = pcp->items[pt].v ;
	} /* end if (init) */

	return rs ;
}
/* end subroutine (percache_getbtime) */


int percache_getrunlevel(PERCACHE *pcp,time_t dt)
{
	const int	pt = pertype_runlevel ;
	int		rs ;
	int		n = 0 ;

	if (pcp == NULL) return SR_FAULT ;

	if ((rs = percache_init(pcp)) >= 0) {
	    time_t	vt ;
	    int		to ;
	    vt = pcp->items[pt].t ;
	    n = pcp->items[pt].v ;
	    to = timeouts[pt] ;
	    if ((dt - vt) > to) {
	        if ((rs = utmpacc_runlevel()) >= 0) {
	            n = rs ;
	            pcp->items[pt].t = dt ;
	            pcp->items[pt].v = n ;
	        }
	    } /* end if */
	} /* end if (init) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (percache_getrunlevel) */


int percache_getnusers(PERCACHE *pcp,time_t dt)
{
	const int	pt = pertype_nusers ;
	int		rs ;
	int		n = 0 ;

	if (pcp == NULL) return SR_FAULT ;

	if ((rs = percache_init(pcp)) >= 0) {
	    time_t	vt ;
	    int		to ;
	    vt = pcp->items[pt].t ;
	    n = pcp->items[pt].v ;
	    to = timeouts[pt] ;
	    if ((dt - vt) > to) {
	        if ((rs = utmpacc_users(0)) >= 0) {
	            n = rs ;
	            pcp->items[pt].t = dt ;
	            pcp->items[pt].v = n ;
	        }
	    } /* end if */
	} /* end if (init) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (percache_getnusers) */


int percache_getsysdomain(PERCACHE *pcp,time_t dt,const char **rpp)
{
	const int	pt = pertype_sysdomain ;
	int		rs ;
	int		len = 0 ;

	if (pcp == NULL) return SR_FAULT ;

	if ((rs = percache_init(pcp)) >= 0) {
	    time_t	vt ;
	    int		to ;
	    vt = pcp->items[pt].t ;
	    len = pcp->items[pt].v ;
	    to = timeouts[pt] ;
	    if (((dt - vt) > to) || (pcp->sysdomain == NULL)) {
	        const int	dlen = MAXHOSTNAMELEN ;
	        char		dbuf[MAXHOSTNAMELEN+1] ;
		if (pcp->sysdomain != NULL) {
		    uc_libfree(pcp->sysdomain) ;
		    pcp->sysdomain = NULL ;
		}
	        if ((rs = getsysdomain(dbuf,dlen)) >= 0) {
		    char	*bp ;
		    if ((rs = uc_libmalloc((rs+1),&bp)) >= 0) {
		        len = (rs-1) ;
			strwcpy(bp,dbuf,len) ;
		        pcp->sysdomain = bp ;
	                pcp->items[pt].t = dt ;
	    	        pcp->items[pt].v = len ;
	            } /* end if (memory-allocation) */
		} /* end if (getsysdomain) */
	    } else {
		len = pcp->items[pt].v ;
	    }
	    if (rpp != NULL) {
		*rpp = (rs >= 0) ? pcp->sysdomain : NULL ;
	    }
	} /* end if (init) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (percache_getsysdomain) */


int percache_netload(PERCACHE *pcp,time_t dt,const char *pr,const char **rpp)
{
	const int	pt = pertype_netload ;
	int		rs ;
	int		len = 0 ;
	if (pcp == NULL) return SR_FAULT ;
	if ((rs = percache_init(pcp)) >= 0) {
	    time_t	vt ;
	    int		to ;
	    vt = pcp->items[pt].t ;
	    len = pcp->items[pt].v ;
	    to = timeouts[pt] ;
	    if (((dt - vt) > to) || (pcp->netload == NULL)) {
	        const int	dlen = MAXHOSTNAMELEN ;
	        char		dbuf[MAXHOSTNAMELEN+1] ;
		if (pcp->netload != NULL) {
		    uc_libfree(pcp->netload) ;
		    pcp->netload = NULL ;
		}
	        if ((rs = localgetnetload(pr,dbuf,dlen)) >= 0) {
		    char	*bp ;
		    if ((rs = uc_libmalloc((rs+1),&bp)) >= 0) {
		        len = (rs-1) ;
			strwcpy(bp,dbuf,len) ;
		        pcp->netload = bp ;
	                pcp->items[pt].t = dt ;
	    	        pcp->items[pt].v = len ;
	            } /* end if (memory-allocation) */
		} /* end if (localgetnetload) */
	    } else {
		len = pcp->items[pt].v ;
	    }
	    if (rpp != NULL) {
		*rpp = (rs >= 0) ? pcp->netload : NULL ;
	    }
	} /* end if (init) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (percache_netload) */


int percache_systat(PERCACHE *pcp,time_t dt,const char *pr,const char **rpp)
{
	const int	pt = pertype_systat ;
	int		rs ;
	int		len = 0 ;
	if (pcp == NULL) return SR_FAULT ;
	if ((rs = percache_init(pcp)) >= 0) {
	    time_t	vt ;
	    int		to ;
	    vt = pcp->items[pt].t ;
	    len = pcp->items[pt].v ;
	    to = timeouts[pt] ;
	    if (((dt - vt) > to) || (pcp->systat == NULL)) {
	        const int	dlen = MAXHOSTNAMELEN ;
	        char		dbuf[MAXHOSTNAMELEN+1] ;
		if (pcp->systat != NULL) {
		    uc_libfree(pcp->systat) ;
		    pcp->systat = NULL ;
		}
	        if ((rs = localgetsystat(pr,dbuf,dlen)) >= 0) {
		    char	*bp ;
		    if ((rs = uc_libmalloc((rs+1),&bp)) >= 0) {
		        len = (rs-1) ;
			strwcpy(bp,dbuf,len) ;
		        pcp->systat = bp ;
	                pcp->items[pt].t = dt ;
	    	        pcp->items[pt].v = len ;
	            } /* end if (memory-allocation) */
		} /* end if (localgetsystat) */
	    } else {
		len = pcp->items[pt].v ;
	    }
	    if (rpp != NULL) {
		*rpp = (rs >= 0) ? pcp->systat : NULL ;
	    }
	} /* end if (init) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (percache_systat) */


static int isNoProcs(int rs)
{
	int		f = FALSE ;
	f = f || isNotPresent(rs) ;
	f = f || (rs == SR_NOSYS) ;
	return f ;
}
/* end subroutine (isNoProcs) */


