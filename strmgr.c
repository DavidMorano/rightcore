/* strmgr */

/* string management */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-10 David A.D. Morano
	This was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We perform some simple string management.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<string.h>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"strmgr.h"


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpylc(char *,int,const char *) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy2(char *,int,const char *,const char *) ;
extern char	*strdcpy3(char *,int,const char *,const char *,const char *) ;
extern char	*strdcpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnrpbrk(const char *,int,const char *) ;
extern char	*strwcpyblanks(char *,int) ;	/* NUL-terminaed */
extern char	*strncpyblanks(char *,int) ;	/* not NUL-terminated */
extern char	*strwset(char *,int,int) ;
extern char	*strnset(char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int strmgr_start(STRMGR *op,char *dbuf,int dlen)
{
	if (dbuf == NULL) return SR_FAULT ;
	op->dp = dbuf ;
	op->dlen = dlen ;
	op->dl = 0 ;
	return SR_OK ;
}
/* end subroutine (strmgr_start) */


/* space available */
int strmgr_avail(STRMGR *op)
{
	return (op->dlen - op->dl) ;
}
/* end subroutine (strmgr_avail) */


/* space remaining */
int strmgr_rem(STRMGR *op)
{
	return (op->dlen - op->dl) ;
}
/* end subroutine (strmgr_rem) */


int strmgr_str(STRMGR *op,cchar *sp,int sl)
{
	const int	rlen = (op->dlen-op->dl) ;
	int		rs = SR_OK ;
	int		tl = 0 ;
	while ((rs >= 0) && sl-- && *sp) {
	    if (tl < rlen) {
	        op->dp[tl++] = *sp++ ;
	    } else {
	        rs = SR_OVERFLOW ;
	    }
	} /* end for */
	if (rs >= 0) {
	    op->dp += tl ;
	    op->dl += tl ;
	}
	return (rs >= 0) ? tl : rs ;
}
/* end subroutine (strmgr_str) */


int strmgr_char(STRMGR *op,int ch)
{
	int		rs = SR_OK ;
	if (op->dl < op->dlen) {
	    *op->dp++ = ch ;
	    op->dl += 1 ;
	} else {
	    rs = SR_OVERFLOW ;
	}
	return (rs >= 0) ? 1 : rs ;
}
/* end subroutine (strmgr_char) */


int strmgr_finish(STRMGR *op)
{
	op->dp[0] = '\0' ;
	return op->dl ;
}
/* end subroutine (strmgr_finish) */


