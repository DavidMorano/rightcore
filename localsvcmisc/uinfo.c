/* uinfo */

/* UNIX® information (a cache for 'uname(2)' and sisters) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module serves as a cache (of sorts) for UNIX® information that is
	related to the underlying machine and which does not (easily) change
	during program exection.

	Since we are basically dealing with global data, we need to make the
	establishment of it multi-thread safe.  We also want fork safety.  Yes,
	we want everything, including cleanup on module unloading (since, yes,
	we could all be inside a loadable and unloadble module!).  For these
	purposes we employ the basic (and not so basic) means of accomplishing
	this.  See the code for our various machinations.

	¥ Note: The type 'int' is assumed to be atomic for multithreaded
	synchronization purposes.  The atomic type |sig_atomic_t" is (just) an
	'int', so we do not feel too guilty ourselves about using an 'int' as
	an interlock.

	Q. Do these subroutines (the public ones) need to be 
	multi-thread-safe?
	A. What do you think?

	Q. Did these subroutines need to be async-signal-safe?
	A. No.  I do not think that these needed to be async-signal-safe.

	Q. Is it a "waste" to make these subroutines async-signal-safe?
	A. Yes. Probably.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<sys/systeminfo.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"uinfo.h"


/* local defines */

#define	UINFO		struct uinfo
#define	UINFO_TMPAUX	struct uinfo_tmpaux
#define	UINFO_ALLOC	struct uinfo_alloc


/* external subroutines */

extern int	msleep(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct uinfo_tmpaux {
	char		architecture[NODENAMELEN+1] ;
	char		platform[NODENAMELEN+1] ;
	char		provider[NODENAMELEN+1] ;
	char		hwserial[NODENAMELEN+1] ;
	char		nisdomain[NODENAMELEN+1] ;
} ;

struct uinfo_alloc {
	volatile cchar	*name ;	/* string allocation for "name" */
	volatile cchar	*aux ;	/* string allocation for "aux" */
} ;

struct uinfo {
	PTM		m ;		/* data mutex */
	UINFO_ALLOC	a ;		/* memory allocations */
	UINFO_NAME	name ;
	UINFO_AUX	aux ;
	volatile int	f_init ; 	/* race-condition, blah, blah */
	volatile int	f_initdone ;
} ;


/* forward references */

int		uinfo_init() ;
void		uinfo_fini() ;

static void	uinfo_atforkbefore() ;
static void	uinfo_atforkafter() ;

static int	uinfo_getaux(UINFO_TMPAUX *) ;


/* local variables */

static UINFO	uinfo_data ; /* zero-initialized */


/* exported subroutines */


int uinfo_init()
{
	UINFO		*uip = &uinfo_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        void	(*b)() = uinfo_atforkbefore ;
	        void	(*a)() = uinfo_atforkafter ;
	        if ((rs = uc_atfork(b,a,a)) >= 0) {
	            if ((rs = uc_atexit(uinfo_fini)) >= 0) {
	                uip->f_initdone = TRUE ;
	                f = TRUE ;
	            }
	            if (rs < 0)
	                uc_atforkrelease(b,a,a) ;
	        } /* end if (uc_atfork) */
	 	if (rs < 0)
		    ptm_destroy(&uip->m) ;
	    } /* end if (ptm_create) */
	    if (rs < 0)
	        uip->f_init = FALSE ;
	} else {
	    while ((rs >= 0) && uip->f_init && (! uip->f_initdone)) {
		rs = msleep(1) ;
		if (rs == SR_INTR) rs = SR_OK ;
	    }
	    if ((rs >= 0) && (! uip->f_init)) rs = SR_LOCKLOST ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uinfo_init) */


void uinfo_fini()
{
	UINFO		*uip = &uinfo_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    {
	        UINFO_ALLOC	*uap = &uip->a ;
	        if (uap->name != NULL) {
	            const void	*p = (const void *) uap->name ;
	            uc_libfree(p) ;
	            uap->name = NULL ;
	        }
	        if (uap->aux != NULL) {
	            const void	*p = (const void *) uap->aux ;
	            uc_libfree(p) ;
	            uap->aux = NULL ;
	        }
	    }
	    {
	        void	(*b)() = uinfo_atforkbefore ;
	        void	(*a)() = uinfo_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(UINFO)) ;
	} /* end if (was initialized) */
}
/* end subroutine (uinfo_fini) */


int uinfo_name(UINFO_NAME *unp)
{
	UINFO		*uip = &uinfo_data ;
	SIGBLOCK	b ;
	int		rs ;
	int		f = TRUE ;

	if (unp == NULL) return SR_FAULT ;

	memset(unp,0,sizeof(UINFO_NAME)) ;

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = uinfo_init()) >= 0) {
	        UINFO_ALLOC	*uap = &uip->a ;

	        if (uap->name == NULL) {
	            UINFO_NAME	name ;
	            const int	size = sizeof(struct utsname) ;
	            const char	*nnamep = NULL ;
	            void	*p ;
	            if ((rs = uc_libmalloc(size,&p)) >= 0) {
	                struct utsname	*unp = p ;

	                if ((rs = u_uname(unp)) >= 0) {
	                    const int	nlen = NODENAMELEN ;
	                    int		size = 0 ;
	                    char	*bp ;
	                    size += (strnlen(unp->sysname,nlen) + 1) ;
	                    size += (strnlen(unp->nodename,nlen) + 1) ;
	                    size += (strnlen(unp->release,nlen) + 1) ;
	                    size += (strnlen(unp->version,nlen) + 1) ;
	                    size += (strnlen(unp->machine,nlen) + 1) ;
	                    if ((rs = uc_libmalloc(size,&bp)) >= 0) {
	                        nnamep = bp ;
	                        name.sysname = bp ;
	                        bp = (strwcpy(bp,unp->sysname,nlen) + 1) ;
	                        name.nodename = bp ;
	                        bp = (strwcpy(bp,unp->nodename,nlen) + 1) ;
	                        name.release = bp ;
	                        bp = (strwcpy(bp,unp->release,nlen) + 1) ;
	                        name.version = bp ;
	                        bp = (strwcpy(bp,unp->version,nlen) + 1) ;
	                        name.machine = bp ;
	                        bp = (strwcpy(bp,unp->machine,nlen) + 1) ;
	                    } /* end if (memory-allocation) */
	                } /* end if (uname) */

	                if (rs >= 0) {
	                    if ((rs = uc_forklockbegin(-1)) >= 0) {
	                        if ((rs = ptm_lock(&uip->m)) >= 0) {

	                            if (uap->name == NULL) {
	                                uap->name = nnamep ;
	                                uip->name = name ;
	                                nnamep = NULL ;
	                                f = FALSE ;
	                            }

	                            ptm_unlock(&uip->m) ;
	                        } /* end if (mutex) */
	                        uc_forklockend() ;
	                    } /* end if (forklock) */
	                } /* end if (ok) */

	                if (nnamep != NULL) uc_libfree(nnamep) ;

	                uc_libfree(unp) ;
	            } /* end if (memory-allocation-free) */
	        } /* end if (need to get information) */
	        if (rs >= 0) *unp = uip->name ;

	    } /* end if (init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uinfo_name) */


int uinfo_aux(UINFO_AUX *uxp)
{
	UINFO		*uip = &uinfo_data ;
	SIGBLOCK	b ;
	int		rs ;
	int		f = TRUE ;

	if (uxp == NULL) return SR_FAULT ;

	memset(uxp,0,sizeof(UINFO_AUX)) ;

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = uinfo_init()) >= 0) {
	        UINFO_ALLOC	*uap = &uip->a ;

	        if (uap->aux == NULL) { /* allow race here */
	            UINFO_AUX	aux ;
	            const int	size = sizeof(UINFO_TMPAUX) ;
	            const char	*nauxp = NULL ;
	            void	*p ;
	            if ((rs = uc_libmalloc(size,&p)) >= 0) {
	                UINFO_TMPAUX	*tap = p ;

	                if ((rs = uinfo_getaux(tap)) >= 0) {
	                    const int	nlen = NODENAMELEN ;
	                    int		size = 0 ;
	                    char	*bp ;
	                    size += (strnlen(tap->architecture,nlen) + 1) ;
	                    size += (strnlen(tap->platform,nlen) + 1) ;
	                    size += (strnlen(tap->provider,nlen) + 1) ;
	                    size += (strnlen(tap->hwserial,nlen) + 1) ;
	                    size += (strnlen(tap->nisdomain,nlen) + 1) ;
	                    if ((rs = uc_libmalloc(size,&bp)) >= 0) {
	                        nauxp = bp ;
	                        aux.architecture = bp ;
	                        bp = (strwcpy(bp,tap->architecture,nlen) + 1) ;
	                        aux.platform = bp ;
	                        bp = (strwcpy(bp,tap->platform,nlen) + 1) ;
	                        aux.provider = bp ;
	                        bp = (strwcpy(bp,tap->provider,nlen) + 1) ;
	                        aux.hwserial = bp ;
	                        bp = (strwcpy(bp,tap->hwserial,nlen) + 1) ;
	                        aux.nisdomain = bp ;
	                        bp = (strwcpy(bp,tap->nisdomain,nlen) + 1) ;
	                    } /* end if (memory-allocation) */
	                } /* end if (uaux) */

	                if (rs >= 0) { /* resolve possible race conditions */
	                    if ((rs = uc_forklockbegin(-1)) >= 0) {
	                        if ((rs = ptm_lock(&uip->m)) >= 0) {

	                            if (uap->aux == NULL) {
	                                uap->aux = nauxp ;
	                                uip->aux = aux ;
	                                nauxp = NULL ;
	                                f = FALSE ;
	                            }

	                            ptm_unlock(&uip->m) ;
	                        } /* end if (mutex) */
	                        uc_forklockend() ;
	                    } /* end if (forklock) */
	                } /* end if (ok) */

	                if (nauxp != NULL) uc_libfree(nauxp) ;

	                uc_libfree(tap) ;
	            } /* end if (memory-allocation-free) */
	        } /* end if (need to get information) */
	        if (rs >= 0) *uxp = uip->aux ;

	    } /* end if (init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uinfo_aux) */


/* local subroutines */


static int uinfo_getaux(UINFO_TMPAUX *tap)
{
	const int	nlen = NODENAMELEN ;
	int		rs = SR_OK ;
	int		rs1 ;

	tap->architecture[0] = '\0' ;
	tap->platform[0] = '\0' ;
	tap->provider[0] = '\0' ;
	tap->hwserial[0] = '\0' ;
	tap->nisdomain[0] = '\0' ;

#ifdef	SI_ARCHITECTURE
	rs1 = u_sysinfo(SI_ARCHITECTURE,tap->architecture,nlen) ;
	if (rs1 < 0) tap->architecture[0] = '\0' ;
#endif

#ifdef	SI_PLATFORM
	rs1 = u_sysinfo(SI_PLATFORM,tap->platform,nlen) ;
	if (rs1 < 0) tap->platform[0] = '\0' ;
#endif

#ifdef	SI_HW_PROVIDER
	rs1 = u_sysinfo(SI_HW_PROVIDER,tap->provider,nlen) ;
	if (rs1 < 0) tap->provider[0] = '\0' ;
#endif

#ifdef	SI_HW_SERIAL
	rs1 = u_sysinfo(SI_HW_SERIAL,tap->hwserial,nlen) ;
	if (rs1 < 0) tap->hwserial[0] = '\0' ;
#endif

#ifdef	SI_SRPC_DOMAIN
	rs1 = u_sysinfo(SI_SRPC_DOMAIN,tap->nisdomain,nlen) ;
	if (rs1 < 0) tap->nisdomain[0] = '\0' ;
#endif

	return rs ;
}
/* end subroutine (uinfo_getaux) */


static void uinfo_atforkbefore()
{
	UINFO		*uip = &uinfo_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (uinfo_atforkbefore) */


static void uinfo_atforkafter()
{
	UINFO		*uip = &uinfo_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (uinfo_atforkafter) */


