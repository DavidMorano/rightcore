/* ema_haveaddr */

/* EMA have a given address */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines if a given address is inside an EMA.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ema.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;

extern int	sfsubstance(const char *,int,const char **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strwset(char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int ema_haveaddr(EMA *emap,const char *ap,int al)
{
	EMA_ENT	*ep ;
	int	rs = SR_OK ;
	int	i ;
	int	f = FALSE ;

	if (al < 0) al = strlen(ap) ;

	for (i = 0 ; (rs = ema_get(emap,i,&ep)) >= 0 ; i += 1) {
	    int		cl = 0 ;
	    const char	*cp = NULL ;
	    if ((ep->rp != NULL) && (ep->rl > 0)) {
		cp = ep->rp ;
		cl = ep->rl ;
	    } else if ((ep->ap != NULL) && (ep->al > 0)) {
		cp = ep->ap ;
		cl = ep->al ;
	    }
	    if (cp != NULL) {
		f = (cl == al) ;
		f = f && (strncmp(cp,ap,al) == 0) ;
		if (f) break ;
	    }
	} /* end for */
	if (rs == SR_NOTFOUND) rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("ema_haveaddr: ret rs=%d f=%d\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
} 
/* end subroutine (ema_haveaddr) */



