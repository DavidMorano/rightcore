/* mkvarpath */

/* try to make a prefix-variable path */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine possibly expands the given path string if it contains a
	multi-path environment variable.

	Synopsis:

	int mkvarpath(rbuf,pp,pl)
	char		rbuf[] ;
	const char	*pp ;
	int		pl ;

	Arguments:

	rbuf		result buffer (should be MAXPATHLEN+1 long)
	pp		source path pointer
	pl		source path length

	Returns:

	<0		error
	==0		no expansion
	>0		expansion


	Example:

	/%cpath/bin/daytime


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */

#undef	CH_EXPAND
#define	CH_EXPAND	'%'


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	haslc(cchar *,int) ;
extern int	isNotPresent(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getenver(cchar *,int) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strwcpyuc(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	mkvarpath_list(char *,cchar *,cchar *) ;
static int	mkvarpath_one(char *,vecstr *,cchar *,int,cchar *) ;
static int	mkvarpath_join(char *,cchar *,int,cchar *) ;


/* local variables */


/* exported subroutines */


int mkvarpath(char *rbuf,cchar *fp,int fl)
{
	const int	ec = CH_EXPAND ;
	int		rs = SR_OK ;
	int		pl = 0 ;

	if (rbuf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mkvarpath: ent p=%t\n",
	    fp,strlinelen(fp,fl,40)) ;
#endif

	rbuf[0] = '\0' ;
	if (fp == NULL) return SR_FAULT ;

	if (fl < 0) fl = strlen(fp) ;

#if	CF_DEBUGS
	debugprintf("mkvarpath: f=%t\n",fp,fl) ;
#endif

	if ((fp[0] == ec) || ((fp[0] == '/') && fl && (fp[1] == ec))) {
	    int		vl = (fl - 1) ;
	    cchar	*vp = (fp + 1) ;
	    cchar	*rp = NULL ;
	    cchar	*tp ;
	    cchar	*cp ;

/* test for extra special case to skip over the prefix character */

	    if (vl && (vp[0] == ec)) {
	        vp += 1 ;
	        vl -= 1 ;
	    }

#if	CF_DEBUGS
	    debugprintf("mkvarpath: 1 v=%t\n",vp,vl) ;
#endif

	    if ((tp = strnchr(vp,vl,'/')) != NULL) {
	        vl = (tp-vp) ;
	        rp = tp ;
	    }

#if	CF_DEBUGS
	    debugprintf("mkvarpath: 1 vl=%u v=%t\n",vl,vp,vl) ;
#endif

	    if (vl > 0) {
	        char	*vbuf = NULL ; /* writable (will be) */

	        if ((cp = getenver(vp,vl)) == NULL) {
	            if (haslc(vp,vl)) {
	                char	*p ;
#if	CF_DEBUGS
	                debugprintf("mkvarpath: has some LC\n") ;
#endif
	                if ((rs = uc_malloc((vl+1),&p)) >= 0) {
	                    vbuf = p ;
	                    strwcpyuc(vbuf,vp,vl) ;
	                    cp = getenver(vbuf,vl) ;
	                }
	            }
	        }

#if	CF_DEBUGS
	        debugprintf("mkvarpath: mid2 rs=%d\n",rs) ;
	        debugprintf("mkvarpath: cp=%t\n",cp,strlinelen(cp,-1,40)) ;
#endif

	        if (rs >= 0) {
	            if (cp != NULL) {

	                if (strchr(cp,':') != NULL) {
	                    rs = mkvarpath_list(rbuf,cp,rp) ;
	                    pl = rs ;
	                } else {
	                    rs = mkvarpath_join(rbuf,cp,-1,rp) ;
	                    pl = rs ;
	                }

	                if (rs == SR_OVERFLOW) rs = SR_NAMETOOLONG ;

	            } else {
	                rs = (rp != NULL) ? SR_NOTDIR : SR_NOENT ;
	            }
	        } /* end if */

	        if (vbuf != NULL) uc_free(vbuf) ;
	    } else {
	        rs = SR_NOTDIR ;
	    }

	} /* end if (go) */

#if	CF_DEBUGS
	debugprintf("mkvarpath: ret rs=%d pl=%u\n",rs,pl) ;
	debugprintf("mkvarpath: ret rbuf=%s\n",rbuf) ;
#endif

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (mkvarpath) */


/* local subroutines */


static int mkvarpath_list(char *rbuf,cchar *pathlist,cchar *rp)
{
	vecstr		paths ;
	int		rs ;
	int		rs1 ;
	int		pl = 0 ;

#if	CF_DEBUGS
	debugprintf("mkvarpath_list: ent rp=%s\n",rp) ;
#endif

	if ((rs = vecstr_start(&paths,2,0)) >= 0) {
	    int		sl ;
	    int		f_zero = FALSE ;
	    const char	*sp = pathlist ;
	    const char	*tp ;

	    while ((tp = strchr(sp,':')) != NULL) {
	        sl = (tp-sp) ;
	        if (sl || (! f_zero)) {
	            if ((! f_zero) && (sl == 0)) f_zero = TRUE ;
	            rs = mkvarpath_one(rbuf,&paths,sp,sl,rp) ;
	            pl = rs ;
	        }
	        sp = (tp+1) ;
	        if (((rs >= 0) && (pl > 0)) || (! isNotPresent(rs))) break ;
	    } /* end while */

	    if ((rs >= 0) && (pl == 0) && ((sp[0] != '\0') || (! f_zero))) {
	        rs = mkvarpath_one(rbuf,&paths,sp,-1,rp) ;
	        pl = rs ;
	    }

	    rs1 = vecstr_finish(&paths) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (paths) */

#if	CF_DEBUGS
	debugprintf("mkvarpath_list: ret rs=%d pl=%u\n",rs,pl) ;
#endif

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (mkvarpath_list) */


static int mkvarpath_one(char *rbuf,vecstr *plp,cchar *sp,int sl,cchar *rp)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	int		rs1 ;
	int		pl = 0 ;

	if (sl < 0) sl = strlen(sp) ;

#if	CF_DEBUGS
	debugprintf("mkvarpath_one: s=>%t<\n",sp,sl) ;
#endif

	if ((rs = vecstr_findn(plp,sp,sl)) == rsn) {
	    if ((rs = vecstr_add(plp,sp,sl)) >= 0) {
	        USTAT	sb ;

	        rs1 = mkvarpath_join(rbuf,sp,sl,rp) ;
	        pl = rs1 ;

	        if ((rs1 == SR_OVERFLOW) || (rs1 == SR_NAMETOOLONG)) {
	            rs1 = SR_OK ;
	            pl = 0 ;
	        }

	        if ((rs1 >= 0) && (pl > 0) && (rbuf[0] != '\0')) {
	            rs = u_lstat(rbuf,&sb) ;
	            if (rs < 0) pl = 0 ;
	        }

	    } /* end if (vecstr_add) */
	} /* end if (not-found) */

#if	CF_DEBUGS
	debugprintf("mkvarpath_one: ret rs=%d pl=%u\n",rs,pl) ;
#endif

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (mkvarpath_one) */


static int mkvarpath_join(char *rbuf,cchar *sp,int sl,cchar *rp)
{
	const int	maxpl = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		pl = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,maxpl,pl,sp,sl) ;
	    pl += rs ;
	}
	if ((rs >= 0) && (rp != NULL)) {
	    rs = storebuf_strw(rbuf,maxpl,pl,rp,-1) ;
	    pl += rs ;
	}

	if (rs == SR_OVERFLOW) rs = SR_NAMETOOLONG ;

#if	CF_DEBUGS
	debugprintf("mkvarpath_join: ret rs=%d pl=%u\n",rs,pl) ;
#endif

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (mkvarpath_join) */


