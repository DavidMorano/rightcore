/* varsub_dumpfd */

/* dump the substitution variables of the object */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-05-23, David A­D­ Morano
        This subroutine was written to provide some helper support for
        substitution setup.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vsystem.h>
#include	<varsub.h>
#include	<localmisc.h>


/* external subroutines */

extern int	fdprintf(int,...) ;


/* exported subroutines */


#if	CF_DEBUGS

int varsub_dumpfd(varsub *vshp,int fd)
{
	if (vshp->vsa != NULL) {
	    varsub_ent	*vsa = vshp->vsa ;
	    int	i ;
	    if (fd < 0) fd = egetfd() ;
	    for (i = 0 ; i < vshp->i ; i += 1) {
	        if (vsa[i].kp == NULL) {
	            fdprintf(fd,"varsub_dumpfd NULL\n") ;
	        } else {
	            fdprintf(fd,"varsub_dumpfd key=%t val=%t\n",
	            vsa[i].kp,vsa[i].klen,
	            vsa[i].vp,vsa[i].vlen) ;
	        }
	    } /* end for */
	} /* end if (non-null) */

	return 0 ;
}
/* end subroutine (varsub_dumpfd) */

#else /* CF_DEBUGS */

/* ARGSUSED */
int varsub_dumpfd(varsub *vsp,int fd)
{
	if (vsp == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (varsub_dumpfd) */

#endif /* CF_DEBUGS */


