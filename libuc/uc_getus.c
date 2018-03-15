/* uc_getus */

/* interface component for UNIX® library-3c */
/* retrieve system-configured user-shells */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines manage the retrieval of system-configured user-shells.
	These subroutines are multi-thread safe, but if multiple threads use
	them simultaneously, each thread will enumerate a separate set of the
	database (as eeems to be the standard as decreed by the UNIX® gods).

	Notes:

        There is no header file which declares the underlying UNIX® subroutines!
        I guess they(?) forgot to make one up!

	Q. Do these subroutines need to be multi-thread safe?
	A. What do you think?

	Q. Where is the data we are protecting with our mutex lock?
	A. It is the private static data that is located inside the
	   |getusershell(3c)| subroutine.

	Q. All of this (locking) just to protect the private static data inside
	   of the |getusershell(3c)| subroutine?
	A. Yes. Pretty much.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<sncpy.h>
#include	<localmisc.h>


/* local defines */

#define	GETUS		struct getus


/* external subroutines */

extern int	msleep(int) ;


/* external variables */


/* local structures */

struct getus {
	PTM		m ;		/* data mutex */
	volatile	f_active ;	/* "set" or "get" was done */
	volatile int	f_init ;	/* race-condition, blah, blah */
	volatile int	f_initdone ;
} ;


/* forward references */

int 		getus_init() ;
void		getus_fini() ;

static void	getus_atforkbefore() ;
static void	getus_atforkafter() ;


/* local variables */

static GETUS	getus_data ; /* zero-initialized */


/* exported subroutines */


int getus_init()
{
	GETUS		*uip = &getus_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        void	(*b)() = getus_atforkbefore ;
	        void	(*a)() = getus_atforkafter ;
	        if ((rs = uc_atfork(b,a,a)) >= 0) {
	            if ((rs = uc_atexit(getus_fini)) >= 0) {
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
		if (rs == SR_INTR) break ;
	    }
	    if ((rs >= 0) && (! uip->f_init)) rs = SR_LOCKLOST ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (getus_init) */


void getus_fini()
{
	GETUS		*uip = &getus_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    if (uip->f_active) {
		uip->f_active = FALSE ;
		endusershell() ;
	    }
	    {
	        void	(*b)() = getus_atforkbefore ;
	        void	(*a)() = getus_atforkafter ;
	        uc_atforkrelease(b,a,a) ;
	    }
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(struct getus)) ;
	} /* end if (was initialized) */
}
/* end subroutine (getus_fini) */


int uc_setus()
{
	GETUS		*uip = &getus_data ;
	int		rs ;
	int		rs1 ;

	if ((rs = getus_init()) >= 0) {
	    if ((rs = uc_forklockbegin(-1)) >= 0) { /* multi */
	 	if ((rs = ptm_lock(&uip->m)) >= 0) { /* single */
		    uip->f_active = TRUE ;
		    setusershell() ;
		    rs1 = ptm_unlock(&uip->m) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (mutex) */
	        rs1 = uc_forklockend() ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (forklock) */
	} /* end if (init) */

	return rs ;
}
/* end subroutine (uc_setus) */


int uc_endus()
{
	GETUS		*uip = &getus_data ;
	int		rs ;
	int		rs1 ;

	if ((rs = getus_init()) >= 0) {
	    if ((rs = uc_forklockbegin(-1)) >= 0) { /* multi */
	 	if ((rs = ptm_lock(&uip->m)) >= 0) { /* single */
		    uip->f_active = FALSE ;
		    endusershell() ;
		    rs1 = ptm_unlock(&uip->m) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (mutex) */
	        rs1 = uc_forklockend() ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (forklock) */
	} /* end if (init) */

	return rs ;
}
/* end subroutine (uc_endus) */


int uc_getus(char *rbuf,int rlen)
{
	GETUS		*uip = &getus_data ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (rbuf == NULL) return SR_FAULT ;

	if (rlen <= 0) return SR_OVERFLOW ;

	if ((rs = getus_init()) >= 0) {
	    if ((rs = uc_forklockbegin(-1)) >= 0) { /* multi */
	 	if ((rs = ptm_lock(&uip->m)) >= 0) { /* single */
		    cchar	*rp ;
		    uip->f_active = TRUE ;
		    errno = 0 ;
		    if ((rp = (cchar *) getusershell()) != NULL) {
	    	        rs = sncpy1(rbuf,rlen,rp) ;
			len = rs ;
		    } else { /* this is really extra safety */
			if (errno != 0) rs = (- errno) ;
		    }
		    rs1 = ptm_unlock(&uip->m) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (mutex) */
	        rs1 = uc_forklockend() ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (forklock) */
	} /* end if (getus_init) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uc_getus) */


/* local subroutines */


static void getus_atforkbefore()
{
	struct getus	*uip = &getus_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (getus_atforkbefore) */


static void getus_atforkafter()
{
	struct getus	*uip = &getus_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (getus_atforkafter) */


