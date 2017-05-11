/* vecpstr_envadd */

/* vector-packed-string object (add environment variable) */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-24, David A­D­ Morano
        This object module was morphed from some previous one. I do not remember
        what the previous one was.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a little add-on to the VECPSTR object to add an environment
	variable in the form of a key-value pair.

	Synopsis:

	int vecpstr_envadd(op,kp,vl,vl)
	VECPSTR		*op ;
	const char	*kp ;
	const char	*vp ;
	int		vl ;

	Arguments:

	op		object pointer
	kp		key pointer
	vp		value pointer
	vl		value length

	Returns:

	>=0		the total length of the filled up vecpstr so far!
	<0		error


*******************************************************************************/


#define	VECPSTR_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"vecpstr.h"


/* local defines */


/* external subroutines */

extern int	vstrkeycmp(cchar **,cchar **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int vecpstr_envadd(VECPSTR *op,cchar *kp,cchar *vp,int vl)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	int		kl ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	kl = strlen(kp) ;

	if ((rs = vecstr_finder(op,kp,vstrkeycmp,NULL)) == rsn) {
	    int		size = 1 ;
	    char	*ep ;
	    size += (kl+1) ;
	    if (vp != NULL) {
	        if (vl < 0) vl = strlen(vp) ;
	        size += vl ;
	    }
	    if ((rs = uc_malloc(size,&ep)) >= 0) {
	        int	el ;
	        char	*bp = ep ;
	        bp = strwcpy(bp,kp,kl) ;
	        *bp++ = '=' ;
		if (vp != NULL) bp = strwcpy(bp,vp,vl) ;
		*bp = '\0' ;
	        el = (bp-ep) ;
	        rs = vecpstr_add(op,ep,el) ;
	        uc_free(ep) ;
	    } /* end if (memory-allocation) */
	} /* end if (was not found) */

	return rs ;
}
/* end subroutine (vecpstr_envadd) */


