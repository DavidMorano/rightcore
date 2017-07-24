/* uptspawn */

/* UNIX® POSIX Thread manipulation */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-20, David A­D­ Morano
        This is a complete rewrite of the trash that performed this function
        previously.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module is an extension of the UPT (UNIX POSIX thread) code 
	modulep.  This modeule contains the subroutine |utpspawn()|.

	Name:

	int uptspawn(rp,ptap,start,op,arg)
	pthread_t	*rp ;
	pthread_attr_t	*ptap ;
	objsub_t	start ;
	void		*op ;
	void		*arg ;

	Arguments:

	rp		pinter to hold resulting thread-id
	ptap 		pointer to pthread attributes
	tart		starting thread address
	op		thread object pointer (arg1)
	arg		argument (arg2)

	Returns:

	<0		error
	>=0		some value


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<pthread.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"upt.h"


/* local defines */

#ifndef	OURARGS
#define	OURARGS		struct ourargs
#endif

#define	TO_NOMEM	5


/* typedefs */

typedef	int	(*objsub_t)(void *,void *) ;


/* external subroutines */

extern int	cfdeci(cchar *,int,int *) ;
extern int	msleep(int) ;


/* local structures */

struct ourargs {
	int		(*start)(void *,void *) ;
	void		*op ;
	void		*ap ;
} ;


/* forward references */

static int	uptcreator(pthread_t *,pthread_attr_t *,void *) ;

static void	*uptruner(void *) ;


/* local variables */


/* exported subroutines */


int uptspawn(pthread_t *rp,pthread_attr_t *ptap,objsub_t start,
		void *op,void *arg)
{
	OURARGS		*oap ;
	const int	osize = sizeof(OURARGS) ;
	int		rs ;
	int		rs1 ;
	int		rv = 0 ;

#if	CF_DEBUGS
	debugprintf("uptcreate: ent\n") ;
#endif

	if (rp == NULL) return SR_FAULT ;
	if (start == NULL) return SR_FAULT ;

	if ((rs = uc_libmalloc(osize,&oap)) >= 0) {
	    sigset_t	nsm, osm ;
	    uc_sigsetfill(&nsm) ;
	    if ((rs = pt_sigmask(SIG_BLOCK,&nsm,&osm)) >= 0) {
		{
	            oap->start = start ;
	            oap->op = op ;
	            oap->ap = arg ;
		    if ((rs = uptcreator(rp,ptap,oap)) >= 0) {
		        rv = (*rp & INT_MAX) ;
		    }
		}
		rs1 = pt_sigmask(SIG_SETMASK,&osm,NULL) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (sigblock) */
	} /* end if (memory-allocation) */

	return (rs >= 0) ? rv : rs ;
}
/* end subroutine (uptspawn) */


/* local subroutines */


static int uptcreator(pthread_t *rp,pthread_attr_t *ptap,void *arg)
{
	int		to_nomem = TO_NOMEM ;
	int		rs ;
	int		f_exit = FALSE ;

	repeat {
	    if ((rs = pthread_create(rp,ptap,uptruner,arg)) > 0) rs = (- rs) ;
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
/* end subroutine (uptcreator) */


static void *uptruner(void *vp)
{
	OURARGS		*oap = (OURARGS *) vp ;
	void		*vrp ;
	int		rs ;

	if (oap != NULL) {
	    int		(*start)(void *,void *) = oap->start ;
	    void	*op = oap->op ;
	    void	*ap = oap->ap ;
	    uc_libfree(oap) ;
	    rs = (*start)(op,ap) ;
	} else {
	    rs = SR_NOEXEC ;
	}

	vrp = (void *) rs ;
	return vrp ;
}
/* end subroutine (uptruner) */


