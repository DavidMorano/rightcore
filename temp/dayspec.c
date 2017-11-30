/* dayspec */

/* load a day-specification */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2013-02-07, David A­D­ Morano

	This subroutine was originally written.  


*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object parses and loads a given day-specification string into
	itself.  The given day-specification string looks like:

		[[<year>]<mon>[-]]<day>
	or
		[[<year>-]<mon>[-]]<day>
	or
		[<year>/]<mon>[/]<day>

	Examples are:

		20130214
		2013-0214
		2013-02-14
		2-14
		feb14
		feb-14
		2013/2/14
		2013feb14
		2013feb-14
		2013-feb14
		2/14
		14

	Results (same as |localtime(3c)|):

	y		0-2037
	m		0-11
	d		1-31


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"dayspec.h"


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sialnum(const char *,int) ;
extern int	sialpha(const char *,int) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	matpcasestr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	getyrd(int,int,int) ;
extern int	hasalldig(const char *,int) ;
extern int	isalphalatin(int) ;
extern int	isalnumlatin(int) ;
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

static int	dayspec_parse(DAYSPEC *,const char *,int) ;

static int	siourbrk(const char *,int,int) ;
static int	parsemonth(const char *,int) ;


/* local variables */

static const char	*months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL
} ;


/* exported subroutines */


int dayspec_default(DAYSPEC *op)
{

	if (op == NULL) return SR_FAULT ;

	op->y = -1 ;
	op->m = -1 ;
	op->d = -1 ;
	return SR_OK ;
}
/* end subroutine (dayspec_default) */


int dayspec_load(DAYSPEC *op,cchar *sp,int sl)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	op->y = -1 ;
	op->m = -1 ;
	op->d = -1 ;
	if (sp != NULL) {
	    rs = dayspec_parse(op,sp,sl) ;
	}

	return rs ;
}
/* end subroutine (dayspec_load) */


int dayspec_yday(DAYSPEC *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->y < 0) return SR_DOM ;
	if (op->m < 0) return SR_DOM ;
	if (op->d < 0) return SR_DOM ;

	rs = getyrd(op->y,op->m,op->d) ;

	return rs ;
}
/* end subroutine (dayspec_yday) */


/* private subroutines */


static int dayspec_parse(DAYSPEC *op,const char *sp,int sl)
{
	int		rs = SR_OK ;
	int		si ;
	int		ti ;
	int		yl = 0 ;
	int		ml = 0 ;
	int		dl = 0 ;
	int		v ;
	const char	*yp = NULL ;
	const char	*mp = NULL ;
	const char	*dp = NULL ;

	if (sl < 0) sl = strlen(sp) ;

	if (hasalldig(sp,sl)) {

	    switch (sl) {
	    case 8:
	    case 7:
	    case 6:
	    case 5:
	        yp = sp ;
	        yl = MIN(4,(sl-4)) ;
	        sp += 4 ;
	        sl -= 4 ;
/* FALLTHROUGH */
	    case 4:
	    case 3:
	        mp = sp ;
	        ml = MIN(2,(sl-2)) ;
	        sp += 2 ;
	        sl -= 2 ;
/* FALLTHROUGH */
	    case 2:
	    case 1:
	        dp = sp ;
	        dl = MIN(2,(sl-0)) ;
/* FALLTHROUGH */
	    } /* end switch */

	} else {
	    int	f_dig ;

	    if ((ti = siourbrk(sp,sl,0)) >= 0) {

	        yp = sp ;
	        yl = ti ;

	        mp = sp ;
	        ml = ti ;

	        sl -= ti ;
	        sp += ti ;
		if (sl > 0) {
		    if ((si = sialnum(sp,sl)) > 0) {
			sp += si ;
			sl -= si ;
		    }
		}

		f_dig = FALSE ;
		if (sl > 0) {
		    int	ch = MKCHAR(sp[0]) ;
		    f_dig = isalphalatin(ch) ;
	        }
	        if ((ti = siourbrk(sp,sl,f_dig)) >= 0) {

	            mp = sp ;
	            ml = ti ;

	            dp = (sp+ti) ;
	            dl = (sl-ti) ;
		if (sl > 0) {
		    if ((si = sialnum(dp,dl)) > 0) {
			dp += si ;
			dl -= si ;
		    }
		}

	        } else {

		    switch (sl) {
		    case 4:
		    case 3:
	        	mp = sp ;
	        	ml = MIN(2,(sl-2)) ;
	        	sp += 2 ;
	        	sl -= 2 ;
/* FALLTHROUGH */
		    case 2:
		    case 1:
	                dp = sp ;
	        	dl = MIN(2,(sl-0)) ;
			break ;
		    default:
			rs = SR_INVALID ;
			break ;
		    } /* end switch */

	        } /* end if */

	    } else {

	        dp = sp ;
	        dl = sl ;

	    } /* end if */

	} /* end if (all-digital) */

#if	CF_DEBUGS
	debugprintf("dayspec_parse: y=%t\n",yp,yl) ;
	debugprintf("dayspec_parse: m=%t\n",mp,ml) ;
	debugprintf("dayspec_parse: d=%t\n",dp,dl) ;
#endif

	if ((rs >= 0) && (yp != NULL) && (yl > 0)) {
	    rs = cfdeci(yp,yl,&v) ;
	    op->y = v ;
	}

	if ((rs >= 0) && (mp != NULL) && (ml > 0)) {
	    rs = parsemonth(mp,ml) ;
	    op->m = rs ;
	}

	if ((rs >= 0) && (dp != NULL) && (dl > 0)) {
	    rs = cfdeci(dp,dl,&v) ;
	    op->d = v ;
	}

#if	CF_DEBUGS
	debugprintf("dayspec_parse: ret rs=%d\n",rs) ;
	debugprintf("dayspec_parse: ret y=%d m=%d d=%d\n",op->y,op->m,op->d) ;
#endif

	return rs ;
}
/* end subroutine (dayspec_parse) */


static int parsemonth(cchar *mp,int ml)
{
	int		rs = SR_OK ;
	int		cl ;
	int		mi = -1 ;
	cchar		*cp ;

	if ((cl = sfshrink(mp,ml,&cp)) > 0) {
	    const int	ch = MKCHAR(cp[0]) ;
	    if (isalphalatin(ch)) {
	        mi = matpcasestr(months,2,cp,cl) ;
	        rs = (mi >= 0) ? mi : SR_INVALID ;
	    } else {
	        rs = cfdeci(cp,cl,&mi) ;
	        mi -= 1 ;
	    }
	} else {
	    rs = SR_INVALID ;
	}

	return (rs >= 0) ? mi : rs ;
}
/* end subroutine (parsemonth) */


static int siourbrk(cchar *sp,int sl,int f_dig)
{
	register int	i = -1 ;
	register int	ch ;
	register int	f = FALSE ;

	for (i = 0 ; i < sl ; i += 1) {
	    ch = MKCHAR(sp[i]) ;
	    if (f_dig) {
		f = isdigitlatin(ch) ;
	    } else {
		f = isalphalatin(ch) ;
	    }
	    f = f || ((ch == '-') || (ch == '/') || (ch == ':')) ;
	    f = f || CHAR_ISWHITE(ch) ;
	    if (f) break ;
	} /* end for */

	return (f) ? i : -1 ;
}
/* end subroutine (siourbrk) */


