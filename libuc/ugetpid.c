/* ugetpid */

/* get the current process PID (quickly and fork-safely) */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_UCGETOPID	1		/* compile in |ungetpid(3uc)| */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We get (and possibly set) our PID.

	Symopsis:

	pid_t ugetpid(void)

	Arguments:

	-

	Returns:

	-		the current process PID

	Notes:

	Q. Lock problems?
	A. No.  Deal with it!

        We did not really need any mutexes or condition-variables in this code.
        If they are not needed for the regular service-methods, then they were
        not really ever needed. They are needed when more than one atomic data
        element needs to conhere together as an atomic unit. We have no such
        data requirements in this service function. The type |pid_t| is already
        atomic on any platform this code will ever run on.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ptm.h>

#include	"ugetpid.h"


/* local defines */

#ifndef	UGETPID	
#define	UGETPID		struct ugetpid
#endif

#define	NDF		"ugetpid.deb"


/* external subroutines */

extern int	msleep(int) ;


/* local structures */

struct ugetpid {
	PTM		m ;		/* data mutex */
	pid_t		pid ;
	volatile int	f_init ; 	/* race-condition, blah, blah */
	volatile int	f_initdone ;
} ;


/* forward references */

int		ugetpid_init() ;
void		ugetpid_fini() ;

static void	ugetpid_atforkbefore() ;
static void	ugetpid_atforkparent() ;
static void	ugetpid_atforkchild() ;


/* lcoal variables */

static UGETPID		ugetpid_data ; /* zero-initialized */


/* exported subroutines */


int ugetpid_init()
{
	UGETPID		*uip = &ugetpid_data ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (! uip->f_init) {
	    uip->f_init = TRUE ;
	    if ((rs = ptm_create(&uip->m,NULL)) >= 0) {
	        void	(*b)() = ugetpid_atforkbefore ;
	        void	(*ap)() = ugetpid_atforkparent ;
	        void	(*ac)() = ugetpid_atforkchild ;
	        if ((rs = uc_atfork(b,ap,ac)) >= 0) {
	            if ((rs = uc_atexit(ugetpid_fini)) >= 0) {
	                uip->f_initdone = TRUE ;
	                f = TRUE ;
	            }
	            if (rs < 0)
	                uc_atforkrelease(b,ap,ac) ;
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
/* end subroutine (ugetpid_init) */


void ugetpid_fini()
{
	UGETPID		*uip = &ugetpid_data ;
	if (uip->f_initdone) {
	    uip->f_initdone = FALSE ;
	    {
	        void	(*b)() = ugetpid_atforkbefore ;
	        void	(*ap)() = ugetpid_atforkparent ;
	        void	(*ac)() = ugetpid_atforkchild ;
	        uc_atforkrelease(b,ap,ac) ;
	    }
	    ptm_destroy(&uip->m) ;
	    memset(uip,0,sizeof(UGETPID	)) ;
	} /* end if (was initialized) */
}
/* end subroutine (ugetpid_fini) */


int ugetpid(void)
{
	UGETPID		*dip = &ugetpid_data ;
	int		rs = SR_OK ;
	if (! dip->f_init) rs = ugetpid_init() ;
	if (rs >= 0) {
	    if (dip->pid == 0) dip->pid = getpid() ;
	    rs = dip->pid ;
	}
	return rs ;
}
/* end subroutine (ugetpid) */


#if	CF_UCGETPID
int ucgetpid(void)
{
	return ugetpid() ;
}
/* end subroutine (ucgetpid) */
#endif /* CF_UCGETPID */


void usetpid(pid_t pid)
{
	UGETPID		*dip = &ugetpid_data ;
	if (pid < 0) pid = getpid() ;
	dip->pid = pid ;
}
/* end subroutine (usetpid) */


/* local subroutines */


static void ugetpid_atforkbefore()
{
	UGETPID		*uip = &ugetpid_data ;
	ptm_lock(&uip->m) ;
}
/* end subroutine (ugetpid_atforkbefore) */


static void ugetpid_atforkparent()
{
	UGETPID		*uip = &ugetpid_data ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (ugetpid_atforkparent) */


static void ugetpid_atforkchild()
{
	UGETPID		*uip = &ugetpid_data ;
	uip->pid = 0 ;
	ptm_unlock(&uip->m) ;
}
/* end subroutine (ugetpid_atforkchild) */


