/* vecstr_adduniqs */

/* add string(s) to a vector-string object */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2004-06-02, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We add strings to the list, but we only add a given string once. So all
        of the strings in the list should end up being unique.

	Synopsis:

	int vecstr_adduniqs(vlp,sp,sl)
	vecstr		*vlp ;
	const char	*sp ;
	int		sl ;

	Arguments:

	vlp		pointer to object
	sp		source string pointer
	sl		source string length

	Returns:

	>=0		number of strings entered
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	nextfield(cchar *,int,cchar **) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int vecstr_adduniqs(vecstr *qlp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	int		c = 0 ;
	const char	*tp, *cp ;

	if (qlp == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	while ((tp = strnpbrk(sp,sl," ,")) != NULL) {
	    if ((cl = sfshrink(sp,(tp-sp),&cp)) > 0) {
	        rs = vecstr_adduniq(qlp,cp,cl) ;
	        c += ((rs < INT_MAX) ? 1 : 0) ;
	    }
	    sl -= ((tp+1)-sp) ;
	    sp = (tp+1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	        rs = vecstr_adduniq(qlp,cp,cl) ;
	        c += ((rs < INT_MAX) ? 1 : 0) ;
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_adduniqs) */


