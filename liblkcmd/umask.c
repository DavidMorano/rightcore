/* umask */

/* UNIX® UMASK (file-creation-mask) management */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module serves to provide two functions that manipulate the UNIX®
        UMASK.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"umask.h"


/* external subroutines */

extern int	msleep(int) ;


/* local structures */

struct umask {
	PTM		m ;		/* data mutex */
	volatile int	f_init ;	/* race-condition, blah, blah */
	volatile int	f_initdone ;
} ;


/* forward references */

void		umask_fini() ;

static void	umask_atforkbefore() ;
static void	umask_atforkafter() ;


/* local variables */

static struct umask	umask_data ; /* zero-initialized */


/* exported subroutines */


int umask_init()
{
	struct umask	*uip = &umask_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        void	(*b)() = umask_atforkbefore ;
	        void	(*a)() = umask_atforkafter ;
	        if ((rs = uc_atfork(b,a,a)) >= 0) {
	            if ((rs = uc_atexit(umask_fini)) >= 0) {
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
/* end subroutine (umask_init) */


void umask_fini()
{
	struct umask	*uip = &umask_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    {
	        void	(*b)() = umask_atforkbefore ;
	        void	(*a)() = umask_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(struct umask)) ;
	} /* end if (atexit registered) */
}
/* end subroutine (umask_fini) */


int getumask()
{
	struct umask	*uip = &umask_data ;
	SIGBLOCK	b ;
	int		rs ;
	int		cmask = 0 ;

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = umask_init()) >= 0) {
	        if ((rs = uc_forklockbegin(-1)) >= 0) { /* multi */
	            if ((rs = ptm_lock(&uip->m)) >= 0) { /* single */

			cmask = umask(0) ; /* in case of race! */
			umask(cmask) ;

	                ptm_unlock(&uip->m) ;
	            } /* end if (mutex) */
	            uc_forklockend() ;
	        } /* end if (forklock) */
	    } /* end if (init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

	cmask &= INT_MAX ;
	return (rs >= 0) ? cmask : rs ;
}
/* end subroutine (getumask) */


int setumask(mode_t cmask)
{
	struct umask	*uip = &umask_data ;
	SIGBLOCK	b ;
	int		rs ;
	int		omask = 0 ;

	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = umask_init()) >= 0) {
	        if ((rs = uc_forklockbegin(-1)) >= 0) { /* multi */
	            if ((rs = ptm_lock(&uip->m)) >= 0) { /* single */

			omask = umask(cmask) ;

	                ptm_unlock(&uip->m) ;
	            } /* end if (mutex) */
	            uc_forklockend() ;
	        } /* end if (forklock) */
	    } /* end if (init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */

	omask &= INT_MAX ;
	return (rs >= 0) ? omask : rs ;
}
/* end subroutine (setumask) */


int umaskset(mode_t cmask) 
{
	return setumask(cmask) ;
}
/* end subroutine (umaskset) */


int umaskget() 
{
	return getumask() ;
}
/* end subroutine (umaskget) */


/* local subroutines */


static void umask_atforkbefore()
{
	struct umask	*uip = &umask_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (umask_atforkbefore) */


static void umask_atforkafter()
{
	struct umask	*uip = &umask_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (umask_atforkafter) */


