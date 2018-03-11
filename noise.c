/* noise */

/* noise related operations */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-05-01, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These routines provide an interface to a noise generator.  Noise can be
	read or written.  Reading noise is what user's really want.  Writing
	noise, just helps mix it up!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<localmisc.h>

#include	"noise.h"


/* local defines */


/* external subroutines */

void		*bsearch() ;


/* local variables */


/* exported subroutines */


int noise_open(np)
NOISE		*np ;
{
	int	rs = SR_OK ;
	int	size ;


	if (n <= 1) 
		n = VECHAND_DEFENTS ;

	memset(vhp,0,sizeof(NOISE)) ;

	vhp->policy = policy ;
	vhp->i = 0 ;
	vhp->c = 0 ;

	size = (n+1) * sizeof(void **) ;
	rs = uc_malloc(size,&p) ;
	if (rs < 0) goto ret0 ;

	vhp->va = (void **) p ;
	vhp->va[0] = NULL ;
	vhp->e = n ;
	vhp->f_sorted = FALSE ;

ret0:
	return rs ;
}
/* end subroutine (noise_open) */


/* close down the noise channel */
int noise_close(np)
NOISE		*np ;
{
	int	i, nn ;

	char	*sp ;

	void	**ep ;


	if (vhp == NULL) 
		return -1 ;

/* do we have to grow the array? */

	if ((vhp->i + 1) > vhp->e) {

	    if (vhp->e == 0) {

	        nn = VECHAND_DEFENTS ;
	        ep = (void **)
	            malloc(sizeof(char **) * (nn + 1)) ;

	    } else {

	        nn = vhp->e * 2 ;
	        ep = (void **)
	            realloc(vhp->va,sizeof(char **) * (nn + 1)) ;

	    }

	    if (ep == NULL) return -1 ;

	    vhp->va = ep ;
	    vhp->e = nn ;

	} /* end if */

/* do the regular thing */

	vhp->c += 1 ;			/* increment list count */

/* link into the list structure */

	i = vhp->i ;
	(vhp->va)[(vhp->i)++] = p ;
	(vhp->va)[vhp->i] = NULL ;
	vhp->f_sorted = FALSE ;
	return i ;
}
/* end subroutine (noise_close) */


int noise_read(np,buf,buflen)
NOISE		*np ;
char		buf[] ;
int		buflen ;
{
	int	i ;


	if (vhp == NULL) 
		return BAD ;

	if (vhp->va != NULL) {

	    for (i = 0 ; i < vhp->i ; i += 1) {

	        if ((vhp->va)[i] != NULL)
	            uc_free((vhp->va)[i]) ;

	    } /* end while */

/* free the vechand array itself */

	    uc_free(vhp->va) ;

	    vhp->va = NULL ;
	}

	vhp->i = 0 ;
	vhp->e = 0 ;
	return OK ;
}
/* end subroutine (noise_read) */


/* write some noise */
int noise_write(np,buf,buflen)
NOISE		*np ;
const char	buf[] ;
int		buflen ;
{


#if	CF_DEBUGS
	debugprintf("vechandget: ent\n") ;
#endif

	if (vhp == NULL) 
		return BAD ;

#if	CF_DEBUGS
	debugprintf("vechandget: 1\n") ;
#endif

	*pp = NULL ;
	if ((i < 0) || (i >= vhp->i)) 
		return BAD ;

#if	CF_DEBUGS
	debugprintf("vechandget: 2\n") ;
#endif

	if (vhp->va == NULL) 
		return BAD ;

#if	CF_DEBUGS
	debugprintf("vechandget: 3\n") ;
#endif

	*pp = (vhp->va)[i] ;

#if	CF_DEBUGS
	debugprintf("vechandget: 4\n") ;
#endif

	return OK ;
}
/* end subroutine (noise_write) */


