/* mkcdpath */
/* land=C++11 */

/* try to make a prefix-variable path */


#define	CF_DEBUGS	 0		/* compile-time debug print-outs */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine possibly expands the given path string if it is prefixed
        with the CDPATH indicator character.

	Synopsis:

	int mkcdpath(ebuf,pp,pl)
	char		ebuf[] ;
	cchar		*pp ;
	int		pl ;

	Arguments:

	ebuf		result buffer (should be MAXPATHLEN+1 long)
	pp		source path pointer
	pl		source path length

	Returns:

	<0		error
	==0		no expansion
	>0		expansion


	Implementation note:

        Yes, we do not add extra slash characters between components of file
        paths (reasonably). And we are not ashamed of this practice. We do not
        though, remove extra slashes that are already preent (although we could
        if we wanted to be extra smart about it).

	Form:

	¬[<varname>]/<path>

	Example:

	¬/stage/daytime
	¬cdpath/stage/daytime


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<new>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */

#undef	CH_EXPAND
#define	CH_EXPAND	MKCHAR('¬')

#ifndef	VARCDPATH
#define	VARCDPATH	"CDPATH"
#endif


/* external subroutines */

extern "C" int	snwcpy(char *,int,const char *,int) ;
extern "C" int	sncpy2w(char *,int,const char *,const char *,int) ;
extern "C" int	mkpath1(char *,const char *) ;
extern "C" int	mkpath2(char *,const char *,const char *) ;
extern "C" int	mkpath1w(char *,const char *,int) ;
extern "C" int	matstr(const char **,const char *,int) ;
extern "C" int	pathadd(char *,int,cchar *) ;
extern "C" int	pathaddw(char *,int,cchar *,int) ;
extern "C" int	sfbasename(cchar *,int,cchar **) ;
extern "C" int	haslc(const char *,int) ;
extern "C" int	isOneOf(const int *,int) ;
extern "C" int	isNotPresent(int) ;

#if	CF_DEBUGS
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif

extern "C" cchar	*getenver(cchar *,int) ;

extern "C" char	*strwcpy(char *,const char *,int) ;
extern "C" char	*strwcpylc(char *,const char *,int) ;
extern "C" char	*strwcpyuc(char *,const char *,int) ;
extern "C" char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

class mkcdpathsub {
	int		vl = 0 ;
	int		sl ;
	int		bl = 0 ;
	cchar		*plist = NULL ;
	cchar		*vp = NULL ;
	cchar		*sp ;
	cchar		*bp = NULL ;
	char		*ebuf ;
public:
	mkcdpathsub(char *abuf,cchar *asp,int asl) : sl(asl), sp(asp) {
	    ebuf = abuf ;
	    vp = sp ;
	} ;
	int getvarname() {
	    cchar	*tp ;
	    if ((tp = strnchr(sp,sl,'/')) != NULL) {
		vl = (tp-sp) ;
		sl -= ((tp+1)-sp) ;
		sp = (tp+1) ;
	    }
	    return vl ;
	} ;
	int getplist() {
	     int	rs = SR_OK ;
	     if (vl > 0) {
		char	*vn ;
		if ((vn = new char [vl+1]) != NULL) {
		    strwcpyuc(vn,vp,vl) ;
		    plist = getenver(vn,vl) ;
		    delete [] vn ;
		} else {
		    rs = SR_NOMEM ;
	 	} /* end if (m-a-f) */
	     } else {
		plist = getenv(VARCDPATH) ;
	     }
	     return rs ;
	} ;
	int getbasename() {
	    if ((bl = sfbasename(sp,sl,&bp)) > 0) {
		sl = (bp-sp-1) ;
	    }
	    return sl ;
	} ;
	int testpaths() ;
	int testpath(cchar *,int) ;
	int mkjoin(cchar *,int) ;
	int mkresult(int) ;
} ;


/* forward references */

extern "C" int	mkcdpath(char *,cchar *,int) ;


/* local variables */


/* exported subroutines */


int mkcdpath(char *ebuf,cchar *fp,int fl)
{
	const int	ec = CH_EXPAND ;
	int		rs = SR_OK ;
	int		el = 0 ;

	if (ebuf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mkcdpath: ent p=%t\n",
	    fp,strlinelen(fp,fl,60)) ;
#endif

	ebuf[0] = '\0' ;
	if (fp == NULL) return SR_FAULT ;

	if (fl < 0) fl = strlen(fp) ;

	if ((fl > 0) && (MKCHAR(fp[0]) == ec)) {
	    mkcdpathsub *sip = new mkcdpathsub(ebuf,(fp+1),(fl+1)) ;
	    if ((rs = sip->getvarname()) >= 0) {
		if ((rs = sip->getplist()) >= 0) {
		    if ((rs = sip->getbasename()) >= 0) {
			if ((rs = sip->testpaths()) > 0) {
			    rs = sip->mkresult(rs) ;
			    el = rs ;
		        }
		    }
		} /* end if (mkcdpathsub::getplist) */
	    } /* end if (mkcdpathsub::getvarname) */
	    delete sip ;
	} /* end if (have one) */

#if	CF_DEBUGS
	debugprintf("mkcdpath: ret rs=%d el=%u\n",rs,el) ;
	debugprintf("mkcdpath: ret ebuf=%s\n",ebuf) ;
#endif

	return (rs >= 0) ? el : rs ;
}
/* end subroutine (mkcdpath) */


/* local srubroutines */


int mkcdpathsub::testpaths()
{
	int		rs = SR_OK ;
	int		el = 0 ;
#if	CF_DEBUGS
	debugprintf("mkcdpathsub::testpaths: ent\n") ;
#endif
	if (plist != NULL) {
	    int		pl = strlen(plist) ;
	    cchar	*pp = plist ;
	    cchar	*tp ;

	    while ((tp = strnchr(pp,pl,':')) != NULL) {
		if ((tp-pp) > 0) {
		    rs = testpath(pp,(tp-pp)) ;
		    el = rs ;
	        }
		pl -= ((tp+1)-pp) ;
	        pp = (tp+1) ;
		if (rs != 0) break ;
	    } /* end while */

	    if ((rs == 0) && (pl > 0)) {
	        rs = testpath(pp,pl) ;
		el = rs ;
	    }

	} /* end if (plist) */

#if	CF_DEBUGS
	debugprintf("mkcdpathsub::testpaths: ret rs=%d el=%u\n",rs,el) ;
#endif

	return (rs >= 0) ? el : rs ;
}
/* end subroutine (mkcdpathsub::testpaths) */


int mkcdpathsub::testpath(cchar *cp,int cl)
{
	int		rs ;
	int		el = 0 ;
#if	CF_DEBUGS
	debugprintf("mkcdpathsub::testpath: ent c=%t\n",
		cp,strlinelen(cp,cl,50)) ;
#endif
	if ((rs = mkjoin(cp,cl)) >= 0) {
	    USTAT	sb ;
	    el = rs ;
	    if ((rs = u_stat(ebuf,&sb)) >= 0) {
		rs = 0 ;
	    } else if (isNotPresent(rs)) {
		el = 0 ;
		rs = SR_OK ;
	    }
	} /* end if (mkjoin) */
#if	CF_DEBUGS
	debugprintf("mkcdpathsub::testpath: ret rs=%d el=%u\n",rs,el) ;
#endif
	return (rs >= 0) ? el : rs ;
}
/* end subroutine (mkcdpathsub::testpath) */


int mkcdpathsub::mkjoin(cchar *cp,int cl)
{
	const int	elen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		el = 0 ;

#if	CF_DEBUGS
	debugprintf("mkcdpathsub::mkjoin: ent c=%t\n",
		cp,strlinelen(cp,cl,50)) ;
#endif

	if (rs >= 0) {
	    rs = storebuf_strw(ebuf,elen,el,cp,cl) ;
	    el += rs ;
	}

	if ((rs >= 0) && el && (ebuf[el-1] != '/')) {
	    rs = storebuf_char(ebuf,elen,el,'/') ;
	    el += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(ebuf,elen,el,sp,sl) ;
	    el += rs ;
	}

#if	CF_DEBUGS
	debugprintf("mkcdpathsub::mkjoin: ret rs=%d el=%u\n",rs,el) ;
	debugprintf("mkcdpathsub::mkjoin: ret ebuf=%s\n",ebuf) ;
#endif

	return (rs >= 0) ? el : rs ;
}
/* end subroutine (mkcdpathsub::mkjoin) */


int mkcdpathsub::mkresult(int el)
{
	return pathaddw(ebuf,el,bp,bl) ;
}
/* end subroutine (mkcdpathsub::mkresult) */


