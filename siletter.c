/* siletter */

/* is the ... something ... a "leader" (whatever that is)? */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-24, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We check for some condition.


*******************************************************************************/


#define	SILEADER_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"siletter.h"


/* local defines */


/* external subroutines */

extern int	sfshrink(cchar *,int,cchar **) ;
extern int	hasalluc(cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* exported subroutines */


int siletter(SILETTER *lsp,cchar *sp,int sl)
{
	int		si = 0 ;

#if	CF_DEBUGS
	debugprintf("siletter: end 1=>%t<\n",
		sp,strlinelen(sp,sl,50)) ;
#endif

	memset(lsp,0,sizeof(SILETTER)) ;

	if (sl > 0) {
	    int		cl ;
	    const char	*tp, *cp ;
	    if ((tp = strnchr(sp,sl,'.')) != NULL) {

#if	CF_DEBUGS
	        debugprintf("siletter: 1>%t<\n",
	            sp,strnlen(sp,MIN(40,(tp - sp)))) ;
#endif

	        if ((cl = sfshrink(sp,(tp - sp),&cp)) > 0) {
	            if (hasalluc(cp,cl)) {
	                lsp->lp = (char *) cp ;
	                lsp->ll = cl ;
	                si = ((tp + 1) - sp) ;
		    }
	        } /* end if (shrink) */

#if	CF_DEBUGS
	        debugprintf("siletter: si=%u\n",si) ;
#endif

	    } /* end if (had a period) */
	} /* end if (positive) */

#if	CF_DEBUGS
	debugprintf("siletter: ret si=%u\n",si) ;
#endif

	return si ;
}
/* end subroutine (siletter) */


