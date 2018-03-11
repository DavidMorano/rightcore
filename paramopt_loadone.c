/* paramopt_loadone */

/* load one parameter (similar to 'paramopt_loadu(3dam)' but special) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This code module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is very similar to 'paramopt_loadu(3dam)' but it allows for a
        special separator character between the key and the data values; the
        special separator character is ':'.


*******************************************************************************/


#define	PARAMOPT_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<nulstr.h>
#include	<paramopt.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sibreak(const char *,int,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* forward references */


/* local variables */


/* exported subroutines */


int paramopt_loadone(PARAMOPT *php,cchar *sp,int sl)
{
	int		rs ;
	int		rs1 ;
	int		i ;

	if (php == NULL) return SR_FAULT ;

	if (php->magic != PARAMOPT_MAGIC) return SR_NOTOPEN ;

	if (sl <= 0) sl = strlen(sp) ;

	while ((sl > 0) && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	}

	if ((i = sibreak(sp,sl," \t=,")) >= 0) {
	    NULSTR	kn ;
	    cchar	*keyname ;
	    if ((rs = nulstr_start(&kn,sp,i,&keyname)) >= 0) {

	        sp += i ;
	        sl -= i ;
	        while ((sl > 0) && CHAR_ISWHITE(*sp)) {
	            sp += 1 ;
	            sl -= 1 ;
	        }

	        if ((sl > 0) && ((*sp == '=') || (*sp == ':'))) {
	            sp += 1 ;
	            sl -= 1 ;
	        }

	        rs = paramopt_load(php,keyname,sp,sl) ;
		i = rs ;

	        rs1 = nulstr_finish(&kn) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (nulstr) */
	} else {
	    rs = SR_INVALID ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (paramopt_loadone) */


