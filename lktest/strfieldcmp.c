/* strfieldcmp */

/* string field comparison */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-12-12, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines are used to compare fields of a string (like an
        environment variables type of string 'HOME=/here'). Fields that can be
        compared are:

		key
		value

	Subroutines available:

		int strnkeycmp(s,kp,kl)
		const char	s[], k[] ;
		const char	kp[] ;
		int		kl ;

		int strnvaluecmp(sp,vp,vl)
		const char	*sp ;
		const char	*vp ;
		int		vl ;


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<localmisc.h>


/* external subroutines */


/* external variables */


/* exported subroutines */


/* our own key comparison routine (to handle the '=' character) */
int strnkeycmp(cchar *s,cchar *kp,int kl)
{
	int		rc = -1 ;

	if (kl < 0) kl = strlen(kp) ;

#if	CF_DEBUGS
	    debugprintf("keycmp: s=\"%s\" k=\"%s\" klen=%d\n",
	        s,kp,kl) ;
#endif

	if ((strncmp(s,kp,kl) == 0) && (s[kl] == '=')) {
	    rc = 0 ;
	}

	return rc ;
}
/* end subroutine (strnkeycmp) */


/* compare a new value with the exiting values of a variable */
int strnvaluecmp(cchar *sp,cchar *vp,int vl)
{
	int		rc = -1 ;
	const char	*tp ;

	if (vl < 0)
	    vl = strlen(vp) ;

#if	CF_DEBUGS
	    debugprintf("var/valuecmp: ent\n") ;
	    debugprintf("var/valuecmp: sp=%s\n",sp) ;
	    debugprintf("var/valuecmp: v=%t\n",vp,vl) ;
#endif

	if ((tp = strchr(sp,'=')) != NULL) {
	    sp = (tp+1) ;
	    while (*sp) {
	        if ((strncmp(sp,vp,vl) == 0) &&
	            ((sp[vl] == '\0') || (sp[vl] == ':'))) {
		    rc = 0 ;
		}
	        if ((tp = strchr(sp,':')) == NULL) break ;
	        sp = (tp+1) ;
		if (rc == 0) break ;
	    } /* end while */
	} /* end if */

	return rc ;
}
/* end subroutine (strnvaluecmp) */


