/* termtype */

/* match on a terminal-type */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2000-07-19, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

	= 2017-12-16, David A­D­ Morano
	Updated.

*/

/* Copyright © 2000,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We try to match a set of paramters to a terminal-type.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"termtype.h"
#include	"termcmd.h"


/* local defines */

#define	U	SHORT_MIN


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	hasParam(const short *,int,int) ;

#if	CF_DEBUGS
extern int	debugprint(cchar *,int) ;
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,char *,int) ;
extern char	*strnwcpy(char *,int,cchar *,int) ;


/* local structures */


/* forward references */

static int	isMatch(const short *,const short *) ;

#if	CF_DEBUGS
static int	debugprintlist(cchar *,const short *) ;
#endif


/* local variables */


/* exported subroutines */


int termtype(const TERMTYPE *types, const short *pvp,const short *svp)
{
	int		i ;
	int		f = FALSE ;
#if	CF_DEBUGS
	debugprintf("termtype: ent\n") ;
	debugprintlist("termtype: pv",pvp) ;
	debugprintlist("termtype: sv",svp) ;
#endif
	for (i = 0 ; types[i].name != NULL ; i += 1) {
#if	CF_DEBUGS
	debugprintf("termtype: type=%s\n",types[i].name) ;
#endif
	    if ((f = isMatch(types[i].pv,pvp)) >= 0) {
	        f = isMatch(types[i].sv,svp) ;
	    }
	    if (f >= 0) break ;
	} /* end for */
#if	CF_DEBUGS
	debugprintf("termtype: ret f=%u i=%u\n",f,i) ;
#endif
	return (f >= 0) ? i : -1 ;
}
/* end subroutine (termtype) */


/* local subroutines */


static int isMatch(const short *vp,const short *pp)
{
	const int	n = TERMCMD_NP ;
	int		i ;
	int		f = TRUE ;
#if	CF_DEBUGS
	debugprintf("isMatch: ent\n") ;
	debugprintlist("isMatch: vp",vp) ;
	debugprintlist("isMatch: pp",pp) ;
#endif
	for (i = 0 ; (i < 4) && (vp[i] != SHORT_MIN) ; i += 1) {
	    const int	v = vp[i] ;
#if	CF_DEBUGS
	debugprintf("isMatch: vp[%2u]=%u\n",i,vp[i]) ;
#endif
	    if (v >= 0) {
		f = hasParam(pp,n,v) ;
	    } else {
		f = (! hasParam(pp,n,-v)) ;
	    }
	    if (! f) break ;
	} /* end for */
#if	CF_DEBUGS
	debugprintf("isMatch: ret f=%u i=%u\n",f,i) ;
#endif
	return (f) ? i : -1 ;
}
/* end subroutine (isMatch) */


#if	CF_DEBUGS
static int debugprintlist(cchar *ids,const short *pp)
{
	SBUF		b ;
	const int	dlen = MAXNAMELEN ;
	int		rs ;
	int		len = 0 ;
	char		dbuf[MAXNAMELEN+1] ;
	if ((rs = sbuf_start(&b,dbuf,dlen)) >= 0) {
	    int		i ;
	    sbuf_strw(&b,ids,-1) ;
	    for (i = 0 ; (i < 16) && (pp[i] != SHORT_MIN) ; i += 1) {
	        sbuf_char(&b,' ') ;
		sbuf_deci(&b,pp[i]) ;
	    }
	    sbuf_char(&b,'\n') ;
	    len = sbuf_finish(&b) ;
	    if (rs >= 0) rs = len ;
	} /* end if (sbuf) */
	if (rs >= 0) {
	    debugprint(dbuf,len) ;
	}
	return (rs >= 0) ? len : rs ;
}
#endif /* CF_DEBUGS */


