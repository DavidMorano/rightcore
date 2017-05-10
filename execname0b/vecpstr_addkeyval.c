/* vecpstr_addkeyval */

/* vector-packed-string object (add environment variable) */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-24, David A­D­ Morano

	This object module was morphed from some previous one.
	I do not remember what the previous one was.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a little add-on to the VECPSTR object to add
	an environment variable in the form of a key-value pair.

	int vecpstr_addkeyval(op,kp,kl,vl,vl)
	VECPSTR		*op ;
	const char	*kp ;
	int		kl ;
	const char	*vp ;
	int		vl ;

	Arguments:

	op		object pointer
	kp		key pointer
	kl		key length
	vp		value pointer
	kl		value length

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

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int vecpstr_addkeyval(op,kp,kl,vp,vl)
VECPSTR		*op ;
const char	*kp ;
int		kl ;
const char	*vp ;
int		vl ;
{
	int	rs = SR_OK ;
	int	size = 1 ;		/* terminating NUL */
	char	*ep ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (kl < 0) kl = strlen(kp) ;

	size += (kl+1) ;
	if (vp != NULL) {
	    if (vl < 0) vl = strlen(vp) ;
	    size += vl ;
	}

	if ((rs = uc_malloc(size,&ep)) >= 0) {
	    int		el ;
	    char	*bp = ep ;
	    bp = strwcpy(bp,kp,kl) ;
	    *bp++ = '=' ;
	    bp = strwcpy(bp,vp,vl) ;
	    el = (bp-ep) ;
	    rs = vecpstr_add(op,ep,el) ;
	    uc_free(ep) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (vecpstr_addkeyval) */


