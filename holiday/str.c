/* strop */

/* special string manipulations */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	This subroutine was originally written.  


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object allows for some specialized manipulations on a
	counted-string object.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>

#include	"strop.h"


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	siskipwhite(const char *,int) ;
extern int	sibreak(const char *,int,const char *) ;
extern int	sispan(const char *,int,const char *) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	isalphalatin(int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strncasestr(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int strop_start(STROP *sop,cchar *sp,int sl)
{

	sop->sp = sp ;
	sop->sl = sl ;
	return SR_OK ;
}
/* end subroutine (strop_start) */


int strop_finish(STROP *sop)
{

	sop->sp = NULL ;
	sop->sl = 0 ;
	return SR_OK ;
}
/* end subroutine (strop_finish) */


int strop_breakfield(STROP *sop,cchar ss[],cchar **rpp)
{
	int		si = -1 ;
	int		rl = -1 ;

#if	CF_DEBUGS
	debugprintf("strop_breakfield: s=>%t<\n",sop->sp,sop->sl) ;
#endif

	*rpp = NULL ;
	if (sop->sl > 0) {
	    if ((si = siskipwhite(sop->sp,sop->sl)) > 0) {
	        sop->sp += si ;
	        sop->sl -= si ;
	    }
	    if (sop->sl > 0) {
	        *rpp = sop->sp ;
	        if ((si = sibreak(sop->sp,sop->sl,ss)) >= 0) {
	            rl = si ;
	            sop->sp += (si + 1) ;
	            sop->sl -= (si + 1) ;
	        } else {
	            rl = sop->sl ;
	            sop->sl = 0 ;
	        }
	    } /* end if (non-zero) */
	} /* end if (non-zero) */

	return rl ;
}
/* end subroutine (strop_breakfield) */


int strop_whitecolon(STROP *sop)
{

	while ((sop->sl > 0) && 
	    (CHAR_ISWHITE(sop->sp[0]) || (sop->sp[0] == ':'))) {
	    sop->sp += 1 ;
	    sop->sl -= 1 ;
	}

	return sop->sl ;
}
/* end subroutine (strop_whitecolon) */


int strop_whitedash(STROP *sop)
{

	while ((sop->sl > 0) && 
	    (CHAR_ISWHITE(sop->sp[0]) || (sop->sp[0] == '-'))) {
	    sop->sp += 1 ;
	    sop->sl -= 1 ;
	}

	return sop->sl ;
}
/* end subroutine (strop_whitedash) */


int strop_span(STROP *sop,cchar stuff[])
{
	int		si ;

	if ((si = sispan(sop->sp,sop->sl,stuff)) > 0) {
	    sop->sp += si ;
	    sop->sl -= si ;
	}

	return sop->sl ;
}
/* end subroutine (strop_span) */


