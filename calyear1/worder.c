/* calworder */

/* word management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano

	This object module was originally written.


*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We manage words, for calendars.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"calworder.h"


/* local defines */


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	sibreak(const char *,int,const char *) ;
extern int	siskipwhite(const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	matocasestr(const char **,int,const char *,int) ;
extern int	matpcasestr(const char **,int,const char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* local structures */


/* forward references */


/* exported variables */


/* local variables */


/* exported subroutines */


int calworder_start(CALWORDER *wp,cchar *md,CALENT *ep)
{
	CALENT_LINE	*lines = ep->lines ;
	wp->i = 0 ;
	wp->nlines = ep->i ;
	wp->lines = ep->lines ;
	wp->md = md ;
	if (lines != NULL) {
	    wp->sp = (md + lines[0].loff) ;
	    wp->sl = (lines[0].llen) ;
	}

	return SR_OK ;
}
/* end subroutine (calworder_start) */


/* ARGSUSED */
int calworder_finish(CALWORDER *wp)
{
	if (wp == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (calworder_finish) */


int calworder_get(CALWORDER *wp,cchar **rpp)
{
	int		cl = 0 ;
	const char	*cp = NULL ; /* ¥ GCC is stupid! */

	while (wp->i < wp->nlines) {
	    if ((cl = nextfield(wp->sp,wp->sl,&cp)) > 0) {
	        wp->sl -= ((cp + cl) - wp->sp) ;
	        wp->sp = (cp + cl) ;
	    } else {
	        wp->i += 1 ;
	        if (wp->i < wp->nlines) {
	            wp->sp = (wp->md + wp->lines[wp->i].loff) ;
	            wp->sl = (wp->lines[wp->i].llen) ;
	        }
	    }
	    if (cl > 0) break ;
	} /* end while */

	if (rpp != NULL)
	    *rpp = cp ;

	return cl ;
}
/* end subroutine (calworder_get) */


