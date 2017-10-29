/* uc_malloc */


#define	F_DEBUGS	0


/* revision history :

	= 01/03/85, David A­D­ Morano

	This subroutine was originally written.


*/


/******************************************************************************

	This is the friendly version of the standrd 'malloc(3c)'
	subroutine.



******************************************************************************/


#include	<sys/types.h>
#include	<stdlib.h>
#include	<errno.h>

#include	<vsystem.h>

#ifdef	DMALLOC
#include	<dmalloc.h>
#endif



/* time outs */

#define	TO_AGAIN	(5 * 60)




int uc_malloc(size,rpp)
int	size ;
void	**rpp ;
{
	int	rs, to_again = 0 ;

	void	*p ;


#if	F_DEBUGS
	eprintf("uc_malloc: entered, size=%d\n",size) ;
#endif

	if (rpp == NULL)
	    return SR_FAULT ;

	if (size < 0)
	    return SR_INVALID ;

	*rpp = NULL ;

again:
	rs = SR_OK ;
	errno = (- SR_NOMEM) ;
	if ((p = malloc(size)) == NULL) rs = (- errno) ;

	if (rs < 0) {

#if	F_DEBUGS
	    eprintf("uc_malloc: failed rs=%d\n",rs) ;
#endif

	    switch (rs) {

	    case SR_AGAIN:
	        if (to_again++ > TO_AGAIN)
	            break ;

	        sleep(1) ;

	        goto again ;

	    case SR_INTR:
	        goto again ;

	    } /* end switch */

	} else
	    *rpp = p ;

#if	F_DEBUGS
	eprintf("uc_malloc: exiting, rs=%d\n",
	    ((rs < 0) ? rs : size)) ;
#endif

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (uc_malloc) */



