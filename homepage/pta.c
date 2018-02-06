/* pta */

/* POSIX® Thread Attribute manipulation */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object provides the initialization add-on for a PYM object.  Enjoy!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<pthread.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"pta.h"


/* local defines */

#define	TO_NOMEM	5


/* external subroutines */

extern int	msleep(int) ;


/* forward references */

int		pta_create(PTA *) ;


/* exported subroutines */


int pta_create(PTA *op)
{
	int		rs ;
	int		to_nomem = TO_NOMEM ;
	int		f_exit = FALSE ;

	if (op == NULL) return SR_FAULT ;

	repeat {
	    rs = pthread_attr_init(op) ;
	    if (rs > 0) rs = (- rs) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_NOMEM:
	            if (to_nomem-- > 0) {
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
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (pta_create) */


int pta_destroy(PTA *op)
{

	if (op == NULL) return SR_FAULT ;

	(void) pthread_attr_destroy(op) ;

	return SR_OK ;
}
/* end subroutine (pta_destroy) */


int pta_setstacksize(PTA *op,size_t v)
{
	int		rs ;

	rs = pthread_attr_setstacksize(op,v) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_setstacksize) */


int pta_getstacksize(PTA *op,size_t *vp)
{
	int		rs ;

	rs = pthread_attr_getstacksize(op,vp) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_getstacksize) */


int pta_setguardsize(PTA *op,size_t v)
{
	int		rs ;

	rs = pthread_attr_setguardsize(op,v) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_setguardsize) */


int pta_getguardsize(PTA *op,size_t *vp)
{
	int		rs ;

	rs = pthread_attr_getguardsize(op,vp) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_getguardsize) */


int pta_setstackaddr(PTA *op,void *vp)
{
	int		rs ;

	rs = pthread_attr_setstackaddr(op,vp) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_setstackaddr) */


int pta_getstackaddr(PTA *op,void **vpp)
{
	int		rs ;

	rs = pthread_attr_getstackaddr(op,vpp) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_getstackaddr) */


int pta_setdetachstate(PTA *op,int v)
{
	int		rs ;

	rs = pthread_attr_setdetachstate(op,v) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_setdetachstate) */


int pta_getdetachstate(PTA *op,int *vp)
{
	int		rs ;

	rs = pthread_attr_getdetachstate(op,vp) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_getdetachstate) */


int pta_setscope(PTA *op,int v)
{
	int		rs ;

	rs = pthread_attr_setscope(op,v) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_setscope) */


int pta_getscope(PTA *op,int *vp)
{
	int		rs ;

	rs = pthread_attr_getscope(op,vp) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_getscope) */


int pta_setinheritsched(PTA *op,int v)
{
	int		rs ;

	rs = pthread_attr_setinheritsched(op,v) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_setinheritsched) */


int pta_getinheritsched(PTA *op,int *vp)
{
	int		rs ;

	rs = pthread_attr_getinheritsched(op,vp) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_getinheritsched) */


int pta_setschedpolicy(PTA *op,int v)
{
	int		rs ;

	rs = pthread_attr_setschedpolicy(op,v) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_setschedpolicy) */


int pta_getschedpolicy(PTA *op,int *vp)
{
	int		rs ;

	rs = pthread_attr_getschedpolicy(op,vp) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_getschedpolicy) */


int pta_setschedparam(PTA *op,const struct sched_param *vp)
{
	int		rs ;

	rs = pthread_attr_setschedparam(op,vp) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_setschedparam) */


int pta_getschedparam(PTA *op,struct sched_param *vp)
{
	int		rs ;

	rs = pthread_attr_getschedparam(op,vp) ;
	if (rs > 0) rs = (- rs) ;

	return rs ;
}
/* end subroutine (pta_getschedparam) */


int pta_setstack(PTA *op,void *saddr,size_t ssize)
{
	int		rs ;
	if (op == NULL) return SR_FAULT ;
	if (saddr == NULL) return SR_FAULT ;
	if ((rs = pta_setstackaddr(op,saddr)) >= 0) {
	    rs = pta_setstacksize(op,ssize) ;
	}
	return rs ;
}
/* end subroutine (pta_setstack) */


