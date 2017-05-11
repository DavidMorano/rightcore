/* uc_resetuid */

/* reset our UID if we are running SETUID */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module alows for SETUID programs to reset their EUID to their UID
        (using the saved-set-UID). And then to set it back again afterwards.


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


/* local defines */

#define	RESETUID	struct uresetuid


/* external subroutines */

extern int	msleep(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct uresetuid {
	PTM		m ;		/* data mutex */
	volatile int	f_init ; 	/* race-condition, blah, blah */
	volatile int	f_initdone ;
} ;


/* forward references */

int		uresetuid_init() ;
void		uresetuid_fini() ;

static void	uresetuid_atforkbefore() ;
static void	uresetuid_atforkafter() ;


/* local variables */

static struct uresetuid		uresetuid_data ; /* zero-initialized */


/* exported subroutines */


int uresetuid_init()
{
	RESETUID	*uip = &uresetuid_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        void	(*b)() = uresetuid_atforkbefore ;
	        void	(*a)() = uresetuid_atforkafter ;
	        if ((rs = uc_atfork(b,a,a)) >= 0) {
	            if ((rs = uc_atexit(uresetuid_fini)) >= 0) {
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
	    }
	    if ((rs >= 0) && (! uip->f_init)) rs = SR_LOCKLOST ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uresetuid_init) */


void uresetuid_fini()
{
	RESETUID	*uip = &uresetuid_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    {
	        struct uresetuid_alloc	*uap = &uip->a ;
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
	        void	(*b)() = uresetuid_atforkbefore ;
	        void	(*a)() = uresetuid_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(struct uresetuid)) ;
	} /* end if (was initialized) */
}
/* end subroutine (uresetuid_fini) */


int uc_resetuid_name(struct uc_resetuid_name *unp)
{
	RESETUID	*uip = &uresetuid_data ;
	SIGBLOCK	b ;
	int		rs ;
	int		f = TRUE ;

	if (unp == NULL) return SR_FAULT ;

	memset(unp,0,sizeof(struct uc_resetuid_name)) ;

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = uc_resetuid_init()) >= 0) {
	        struct uc_resetuid_alloc	*uap = &uip->a ;

	        if (uap->name == NULL) {
	            struct uc_resetuid_name	name ;
	            const char		*nnamep = NULL ;
	            const int		size = sizeof(struct utsname) ;
	            void		*p ;
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
	            } /* end if (m-a) */
	        } /* end if (need to get information) */
	        if (rs >= 0) *unp = uip->name ;

	    } /* end if (init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (uresetuid_name) */


/* local subroutines */


static void uresetuid_atforkbefore()
{
	RESETUID	*uip = &uresetuid_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (uresetuid_atforkbefore) */


static void uresetuid_atforkafter()
{
	RESETUID	*uip = &uresetuid_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (uresetuid_atforkafter) */


