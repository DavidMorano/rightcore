/* biblecite */

/* process a possible bible citation */


#define	CF_DEBUGS	0		/* used for little object below */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines if a string is a bible citation.

	Synopsis:

	int isbiblecite(sp,sl,bcp,sip)
	const char	*sp ;
	int		sl ;
	BIBLECITE	*bcp ;
	int		*sip ;

	Arguments:


	Returns:

	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"biblecite.h"


/* local defines */


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	siskipwhite(const char *,int) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int isbiblecite(BIBLECITE *qp,cchar *lp,int ll,int *sip)
{
	uint		v ;
	int		rs1 ;
	int		sl, cl ;
	int		i ;
	int		ch ;
	int		si ;
	int		f = FALSE ;
	const char	*tp, *sp, *cp ;

	memset(qp,0,sizeof(BIBLECITE)) ;

	sp = lp ;
	sl = ll ;
	if ((si = siskipwhite(lp,ll)) > 0) {
	    sp += si ;
	    sl -= si ;
	}

	ch = MKCHAR(sp[0]) ;
	if ((sl >= 5) && isdigitlatin(ch)) {

	for (i = 0 ; i < 3 ; i += 1) {

	    cp = sp ;
	    cl = sl ;
	    if ((tp = strnpbrk(sp,sl,": \t\n")) != NULL) {
	        cl = (tp - sp) ;
	        sl -= ((tp + 1) - sp) ;
	        sp = (tp + 1) ;
	    } else {
	        cl = sl ;
	        sp += sl ;
	        sl = 0 ;
	    }

	    if (cl == 0)
	        break ;

	    si = ((cp + cl) - lp) ;
	    rs1 = cfdecui(cp,cl,&v) ;

#if	CF_DEBUGS && 0
	debugprintf("isbiblecite: cfdecui() rs=%d\n",rs1) ;
#endif

	    if (rs1 < 0)
	        break ;

	    switch (i) {
	    case 0:
	        qp->b = v ;
	        break ;
	    case 1:
	        qp->c = v ;
	        break ;
	    case 2:
	        qp->v = v ;
	        break ;
	    } /* end switch */

	} /* end for */

	f = (i == 3) ;
	if (f) {
	    si += siskipwhite(sp,sl) ;
	}

	} /* end if (ok) */

	if (sip != NULL) {
	    *sip = (f) ? si : 0 ;
	}

#if	CF_DEBUGS && 0
	debugprintf("isbiblecite: f=%u si=%u\n",f,si) ;
#endif

	return (f) ? si : 0  ;
}
/* end subroutine (isbiblecite) */



