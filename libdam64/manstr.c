/* manstr */

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

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>

#include	"manstr.h"


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


int manstr_start(sop,sp,sl)
MANSTR		*sop ;
const char	*sp ;
int		sl ;
{

	sop->sp = sp ;
	sop->sl = sl ;
	return SR_OK ;
}
/* end subroutine (manstr_start) */


int manstr_finish(sop)
MANSTR		*sop ;
{

	sop->sp = NULL ;
	sop->sl = 0 ;
	return SR_OK ;
}
/* end subroutine (manstr_finish) */


int manstr_breakfield(sop,ss,rpp)
MANSTR		*sop ;
const char	ss[] ;
const char	**rpp ;
{
	int		si = -1 ;
	int		rl = -1 ;

#if	CF_DEBUGS
	debugprintf("manstr_breakfield: s=>%t<\n",sop->sp,sop->sl) ;
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
	    } /* end if (greater-than-zero) */

	} /* end if (greater-than-zero) */

	return rl ;
}
/* end subroutine (manstr_breakfield) */


int manstr_whitecolon(sop)
MANSTR		*sop ;
{

	while ((sop->sl > 0) && 
	    (CHAR_ISWHITE(sop->sp[0]) || (sop->sp[0] == ':'))) {
	    sop->sp += 1 ;
	    sop->sl -= 1 ;
	}

	return sop->sl ;
}
/* end subroutine (manstr_whitecolon) */


int manstr_whitedash(sop)
MANSTR		*sop ;
{

	while ((sop->sl > 0) && 
	    (CHAR_ISWHITE(sop->sp[0]) || (sop->sp[0] == '-'))) {
	    sop->sp += 1 ;
	    sop->sl -= 1 ;
	}

	return sop->sl ;
}
/* end subroutine (manstr_whitedash) */


int manstr_span(sop,stuff)
MANSTR		*sop ;
const char	stuff[] ;
{
	int		si ;

	if ((si = sispan(sop->sp,sop->sl,stuff)) > 0) {
	    sop->sp += si ;
	    sop->sl -= si ;
	}

	return sop->sl ;
}
/* end subroutine (manstr_span) */


