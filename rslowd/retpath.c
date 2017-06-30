/* retpath */

/* process the "retpath" NEWS object */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	= 1995-05-01, David A­D­ Morano
        This code module was completely rewritten to replace any original
        garbage that was here before.

*/

/* Copyright © 1995 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        The so called "retpath" object is the thing that provides a route back
        to the sender of a message.

        We use a VECSTR object to hold the retpath components. Note carefully
        that we store the retpath components in the order that they appear (left
        to right) in the retpath header value. This represents the order that
        would be needed to get back to the originator and NOT the order from the
        originator to us!

	This makes this similar to a return-retpath in email circles.


**************************************************************************/


#define	RETPATH_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"retpath.h"


/* local defines */


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	strwcmp(const char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* forward references */

static int retpath_iadd(RETPATH *,const char *,int) ;


/* local variables */


/* exported subroutines */


int retpath_start(RETPATH *plp)
{
	const int	vo = VECSTR_OORDERED ;

	return vecstr_start(plp,10,vo) ;
}
/* end subroutine (retpath_start) */


int retpath_finish(RETPATH *plp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecstr_finish(plp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (retpath_finish) */


int retpath_add(RETPATH *plp,cchar *np,int nl)
{
	int		rs = SR_OK ;
	int		cl ;
	const char	*cp ;

	if (plp == NULL) return SR_FAULT ;
	if (np == NULL) return SR_FAULT ;

	if ((cl = sfshrink(np,nl,&cp)) > 0) {
	    rs = retpath_iadd(plp,cp,cl) ;
	}

	return rs ;
}
/* end subroutine (retpath_add) */


/* add ret-path entries by parsing a string of them */
int retpath_parse(RETPATH *plp,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		cl ;
	int		n = 0 ;
	const char	*tp, *cp ;

	if (plp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	while ((tp = strnpbrk(sp,sl,"!@,%:")) != NULL) {

	    if ((cl = sfshrink(sp,(tp-sp),&cp)) > 0) {
	        n += 1 ;
	        rs = retpath_iadd(plp,cp,cl) ;
	    }

	    sl -= ((tp+1)-sp) ;
	    sp = (tp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (sl > 0)) {
	    if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	        n += 1 ;
	        rs = retpath_iadd(plp,cp,cl) ;
	    }
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (retpath_parse) */


int retpath_count(RETPATH *plp)
{

	return vecstr_count(plp) ;
}
/* end subroutine (retpath_count) */


int retpath_get(RETPATH *plp,int i,cchar **rpp)
{
	return vecstr_get(plp,i,rpp) ;
}
/* end subroutine (retpath_get) */


int retpath_search(RETPATH *plp,cchar *name,cchar **rpp)
{
	int		rs ;
	int		i ;

	if (plp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vecstr_get(plp,i,rpp)) >= 0 ; i += 1) {
	    if ((*rpp) != NULL) {
	        if (strcmp(name,(*rpp)) == 0) break ;
	    }
	} /* end for */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (retpath_search) */


int retpath_mk(RETPATH *plp,char *rbuf,int rlen)
{
	SBUF		b ;
	int		rs ;
	int		rl = 0 ;

	if (plp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (rlen < 0) rlen = INT_MAX ;

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    int		i ;
	    const char	*sp ;

	    for (i = 0 ; vecstr_get(plp,i,&sp) >= 0 ; i += 1) {
		if (i > 0) sbuf_char(&b,'!') ;
		rs = sbuf_strw(&b,sp,-1) ;
		if (rs < 0) break ;
	    } /* end for */

	    rl = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rl ;
	} /* end if (sbuf) */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (retpath_mk) */


/* private subroutines */


/* do not add if already added (as the previous entry) */
static int retpath_iadd(RETPATH *plp,const char *np,int nl)
{
	int		rs ;
	const char	*lp ;

	if ((rs = vecstr_getlast(plp,&lp)) >= 0) {
	    if (strwcmp(lp,np,nl) != 0) rs = SR_NOTFOUND ;
	}
	if (rs == SR_NOTFOUND) {
	    rs = vecstr_add(plp,np,nl) ;
	} else {
	    rs = INT_MAX ;
	}

	return rs ;
}
/* end subroutine (retpath_iadd) */


