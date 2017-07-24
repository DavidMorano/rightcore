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

	This subroutine possibly expands the given path string if it is
	prefixed with the CDPATH indicator character.

	Synopsis:

	int mkcdpath(ebuf,pp,pl) ;
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
	paths.  And we are not ashamed of this practice.  We do not though,
	remove extra slashes that are already preent (although we could if we
	wanted to be extra smart about it).

	Example:

	¬/stage/daytime


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

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

class subinfo {
	int		vl ;
	int		sl ;
	int		bl = 0 ;
	cchar		*plist = NULL ;
	cchar		*vp = NULL ;
	cchar		*sp ;
	cchar		*bp = NULL ;
	char		*ebuf ;
public:
	subinfo(char *abuf,cchar *asp,int asl) 
		: vl(0), sl(asl), vp(NULL), sp(asp) {
	    ebuf = abuf ;
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
	     if (vl == 0) {
		plist = getenv(VARCDPATH) ;
	     } else {
		plist = getenver(vp,vl) ;
	     }
	     return SR_OK ;
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


/* local variables */


/* exported subroutines */


extern "C" int	mkcdpath(char *,cchar *,int) ;

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
	    subinfo	*sip = new subinfo(ebuf,(fp+1),(fl+1)) ;
#if	CF_DEBUGS
	debugprintf("mkcdpath: got\n") ;
#endif
	    if ((rs = sip->getvarname()) >= 0) {
#if	CF_DEBUGS
	debugprintf("mkcdpath: subinfo::getvarname() rs=%d\n",rs) ;
#endif
		if ((rs = sip->getplist()) >= 0) {
#if	CF_DEBUGS
	debugprintf("mkcdpath: subinfo::getplist() rs=%d\n",rs) ;
#endif
		    if ((rs = sip->getbasename()) >= 0) {
#if	CF_DEBUGS
	debugprintf("mkcdpath: subinfo::getbasename() rs=%d\n",rs) ;
#endif
			if ((rs = sip->testpaths()) > 0) {
#if	CF_DEBUGS
	debugprintf("mkcdpath: subinfo::testpaths() rs=%d\n",rs) ;
#endif
			    rs = sip->mkresult(rs) ;
			    el = rs ;
#if	CF_DEBUGS
	debugprintf("mkcdpath: subinfo::mkresult() rs=%d\n",rs) ;
#endif
		        }
#if	CF_DEBUGS
	debugprintf("mkcdpath: subinfo::testpaths out rs=%d\n",rs) ;
#endif
		    }
#if	CF_DEBUGS
	debugprintf("mkcdpath: subinfo::getbasename out rs=%d\n",rs) ;
#endif
		} /* end if (subinfo::getplist) */
	    } /* end if (subinfo::getvarname) */
	    delete sip ;
	} /* end if (have one) */

#if	CF_DEBUGS
	debugprintf("mkcdpath: ret rs=%d el=%u\n",rs,el) ;
	debugprintf("mkcdpath: ret ebuf=%s\n",ebuf) ;
#endif

	return (rs >= 0) ? el : rs ;
}
/* end subroutine (mkcdpath) */


/* local srubtoueines */


int subinfo::testpaths()
{
	int		rs = SR_OK ;
	int		el = 0 ;
#if	CF_DEBUGS
	debugprintf("subinfo::testpaths: ent\n") ;
#endif
	if (plist != NULL) {
	    int		pl = strlen(plist) ;
	    cchar	*pp = plist ;
	    cchar	*tp ;

#if	CF_DEBUGS
	debugprintf("subinfo::testpaths: plist=%t\n",
		plist,strlinelen(plist,-1,50)) ;
#endif

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
	debugprintf("subinfo::testpaths: ret rs=%d el=%u\n",rs,el) ;
#endif

	return (rs >= 0) ? el : rs ;
}
/* end subroutine (subinfo::testpaths) */


int subinfo::testpath(cchar *cp,int cl)
{
	int		rs ;
	int		el = 0 ;
#if	CF_DEBUGS
	debugprintf("subinfo::testpath: ent c=%t\n",
		cp,strlinelen(cp,cl,50)) ;
#endif
	if ((rs = mkjoin(cp,cl)) >= 0) {
	    USTAT	sb ;
	    el = rs ;
#if	CF_DEBUGS
	    debugprintf("subinfo::testpath: mkjoin() rs=%d\n",rs) ;
	    debugprintf("subinfo::testpath: ebuf=%s\n",ebuf) ;
#endif
	    if ((rs = u_stat(ebuf,&sb)) >= 0) {
#if	CF_DEBUGS
	debugprintf("subinfo::testpath: uc_stat() rs=%d\n",rs) ;
#endif
		rs = 0 ;
	    } else if (isNotPresent(rs)) {
#if	CF_DEBUGS
	debugprintf("subinfo::testpath: uc_stat() rs=%d\n",rs) ;
#endif
		el = 0 ;
		rs = SR_OK ;
	    }
	} /* end if (mkjoin) */
#if	CF_DEBUGS
	debugprintf("subinfo::testpath: ret rs=%d el=%u\n",rs,el) ;
#endif
	return (rs >= 0) ? el : rs ;
}
/* end subroutine (subinfo::testpath) */


int subinfo::mkjoin(cchar *cp,int cl)
{
	const int	elen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		el = 0 ;

#if	CF_DEBUGS
	debugprintf("subinfo::mkjoin: ent c=%t\n",
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

#if	CF_DEBUGS
	debugprintf("subinfo::mkjoin: s=%t\n",
		sp,strlinelen(sp,sl,50)) ;
#endif

	if (rs >= 0) {
	    rs = storebuf_strw(ebuf,elen,el,sp,sl) ;
	    el += rs ;
	}

#if	CF_DEBUGS
	debugprintf("subinfo::mkjoin: ret rs=%d el=%u\n",rs,el) ;
	debugprintf("subinfo::mkjoin: ret ebuf=%s\n",ebuf) ;
#endif

	return (rs >= 0) ? el : rs ;
}
/* end subroutine (subinfo::mkjoin) */


int subinfo::mkresult(int el)
{
	return pathaddw(ebuf,el,bp,bl) ;
}
/* end subroutine (subinfo::mkresult) */


