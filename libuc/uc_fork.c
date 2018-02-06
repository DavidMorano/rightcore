/* uc_fork */

/* interface component for UNIX® library-3c */
/* special handling for 'fork(2)' */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These are fake subroutines waiting for the day that some bright-ass
        UNIX® guru-ass-hole guy invents them!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<lockrw.h>
#include	<ugetpid.h>
#include	<localmisc.h>


/* external subroutines */

extern int	msleep(int) ;


/* local structures */

struct ucfork {
	LOCKRW		lock ;		/* critical-section lock */
	volatile int	f_init ;	/* race-condition, blah, blah, blah */
	volatile int	f_initdone ;
} ;


/* forward references */

void		ucfork_fini() ;


/* local variables */

static struct ucfork	ucfork_data ;	/* zero-initialized */


/* exported subroutines */


int ucfork_init()
{
	struct ucfork	*uip = &ucfork_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = lockrw_create(&uip->lock,FALSE)) >= 0) {
	        if ((rs = uc_atexit(ucfork_fini)) >= 0) {
	            f = TRUE ;
	            uip->f_initdone = TRUE ;
	        }
	        if (rs < 0) {
	            lockrw_destroy(&uip->lock) ;
	        }
	    } /* end if (lockrw-create) */
	    if (rs < 0)
	        uip->f_init = FALSE ;
	} else {
	    while ((rs >= 0) && uip->f_init && (! uip->f_initdone)) {
		rs = msleep(1) ;
		if (rs == SR_INTR) break ;
	    }
	    if ((rs >= 0) && (! uip->f_init)) rs = SR_LOCKLOST ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (ucfork_init) */


void ucfork_fini()
{
	struct ucfork	*uip = &ucfork_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    lockrw_destroy(&uip->lock) ;
	    memset(uip,0,sizeof(ucfork_data)) ;
	} /* end if (atexit registered) */
}
/* end subroutine (ucfork_fini) */


int uc_fork()
{
	struct ucfork	*uip = &ucfork_data ;
	SIGBLOCK	b ;
	int		rs ;
	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = ucfork_init()) >= 0) {
	        if ((rs = lockrw_wrlock(&uip->lock,-1)) >= 0) {
	            rs = u_fork() ;
	            if (rs == 0) { /* child */
	                ucfork_fini() ;
			usetpid(0) ;
	                if ((rs = ucfork_init()) > 0) {
	                    rs = 0 ;
	                }
	            } else if (rs > 0) {
	                lockrw_unlock(&uip->lock) ;
	            }
	        } /* end if (lockrw) */
	    } /* end if (ucfork_init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */
	return rs ;
}
/* end subroutine (uc_fork) */


int uc_forklockbegin(int to)
{
	struct ucfork	*uip = &ucfork_data ;
	SIGBLOCK	b ;
	int		rs ;
	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if ((rs = ucfork_init()) >= 0) {
	        rs = lockrw_rdlock(&uip->lock,to) ;
	    } /* end if (ucfork_init) */
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */
	return rs ;
}
/* end subroutine (uc_forklockbegin) */


int uc_forklockend()
{
	struct ucfork	*uip = &ucfork_data ;
	SIGBLOCK	b ;
	int		rs ;
	if ((rs = sigblock_start(&b,NULL)) >= 0) {
	    if (uip->f_init) {
	        rs = lockrw_unlock(&uip->lock) ;
	    } else {
	        rs = SR_NOANODE ;
	    }
	    sigblock_finish(&b) ;
	} /* end if (sigblock) */
	return rs ;
}
/* end subroutine (uc_forklockend) */


