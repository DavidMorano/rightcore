/* mkcdpath */

/* try to make a prefix-variable path */


#define	CF_DEBUGS	 0		/* compile-time debug print-outs */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine possibly expands the given path string if it contains a
	multi-path environment variable.

	Synopsis:

	int mkcdpath(ebuf,pp,pl) ;
	char		ebuf[] ;
	const char	*pp ;
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

	/%cdpath/stage/daytime


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
#define	CH_EXPAND	MKCHAR('¬')

#ifndef	VARCDPATH
#define	VARCDPATH	"CDPATH"
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2w(char *,int,const char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	haslc(const char *,int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	mkcdpath_list(char *,const char *,const char *,int) ;
static int	mkcdpath_one(char *,vecstr *,cchar *,int,cchar *,int) ;
static int	mkcdpath_join(char *,const char *,int,const char *,int) ;


/* local variables */


/* exported subroutines */


int mkcdpath(char *ebuf,cchar *fp,int fl)
{
	const int	ec = CH_EXPAND ;
	int		rs = SR_OK ;
	int		pl = 0 ;

	if (ebuf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mkcdpath: ent p=%t\n",
	    fp,strlinelen(fp,fl,50)) ;
#endif

	ebuf[0] = '\0' ;
	if (fp == NULL) return SR_FAULT ;

	if (fl < 0) fl = strlen(fp) ;

	if ((fl > 0) && (MKCHAR(fp[0]) == ec)) {
	int		elen = MAXPATHLEN ;
	int		el = 0 ;
	int		rl = 0 ;
	int		ul = 0 ;
	const char	*varcdpath = VARCDPATH ;
	const char	*tp ;
	const char	*rp ;
	const char	*up ;
	const char	*cp ;

	rp = (fp + 1) ;
	rl = (fl - 1) ;

	{
	    up = rp ;
	    ul = rl ;
	    if ((tp = strnchr(rp,rl,'/')) != NULL) {
		ul = (tp - rp) ;
		rl -= ((tp+1)-rp) ;
		rp = (tp+1) ;
	    } else {
		rp += rl ;
	   	rl = 0 ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("mkcdpath: u=>%t<\n",up,ul) ;
	debugprintf("mkcdpath: r=>%t<\n",
		rp,strlinelen(rp,rl,60)) ;
#endif

/* search */

	if (ul > 0) {
	    if ((cp = getenv(varcdpath)) != NULL) {

	        rs = mkcdpath_list(ebuf,cp,up,ul) ;
	        el = rs ;

#if	CF_DEBUGS
	debugprintf("mkcdpath: _list() rs=%d\n",rs) ;
#endif

		pl = el ;
	        if (rs == SR_OVERFLOW) rs = SR_NAMETOOLONG ;

		if ((rs == SR_NOENT) && (rl > 0)) rs = SR_NOTDIR ;

	    } else
		rs = (rl > 0) ? SR_NOTDIR : SR_NOENT ;
	} else {
	    rs = (rl > 0) ? SR_NOTDIR : SR_NOENT ;
	}

	if ((rs >= 0) && (el > 0) && (rl > 0)) {
	    char	*bbuf = (ebuf+el) ;
	    int		blen = (elen-el) ;
	    const char	*slash = (ebuf[el-1] != '/') ? "/" : "" ;
	    rs = sncpy2w(bbuf,blen,slash,rp,rl) ;
	    pl = (el+rs) ;
	}

	} /* end if (ready) */

#if	CF_DEBUGS
	debugprintf("mkcdpath: ret rs=%d pl=%u\n",rs,pl) ;
	debugprintf("mkcdpath: ebuf=%s\n",ebuf) ;
#endif

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (mkcdpath) */


/* local subroutines */


static int mkcdpath_list(char *ebuf,cchar *pathlist,cchar *up,int ul)
{
	vecstr		paths ;
	int		rs ;
	int		rs1 ;
	int		pl = 0 ;

	if ((rs = vecstr_start(&paths,2,0)) >= 0) {
	    int		sl ;
	    int		f_zero = FALSE ;
	    const char	*sp = pathlist ;
	    const char	*tp ;

	    while ((tp = strchr(sp,':')) != NULL) {
	        sl = (tp-sp) ;
	        if (sl || (! f_zero)) {
	            if ((! f_zero) && (sl == 0)) f_zero = TRUE ;
	            rs = mkcdpath_one(ebuf,&paths,up,ul,sp,sl) ;
	            pl = rs ;
	        }
	        sp = (tp+1) ;
	        if (((rs >= 0) && (pl > 0)) || (! isNotPresent(rs))) break ;
	    } /* end while */

	    if ((rs >= 0) && (pl == 0) && ((sp[0] != '\0') || (! f_zero))) {
	        rs = mkcdpath_one(ebuf,&paths,up,ul,sp,-1) ;
	        pl = rs ;
	    }

	    rs1 = vecstr_finish(&paths) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (paths) */

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (mkcdpath_list) */


static int mkcdpath_one(ebuf,plp,up,ul,sp,sl)
char		ebuf[] ;
vecstr		*plp ;
const char	*up ;
int		ul ;
const char	*sp ;
int		sl ;
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	int		rs1 ;
	int		pl = 0 ;

#if	CF_DEBUGS
	debugprintf("mkcdpath_one: u=%t\n",up,ul) ;
	debugprintf("mkcdpath_one: s=%t\n",sp,sl) ;
#endif

	if (sl < 0) sl = strlen(sp) ;

	if ((rs = vecstr_findn(plp,sp,sl)) == rsn) {
	    if ((rs = vecstr_add(plp,sp,sl)) >= 0) {

	        rs1 = mkcdpath_join(ebuf,sp,sl,up,ul) ;
	        pl = rs1 ;

#if	CF_DEBUGS
		debugprintf("mkcdpath_one: _join() rs=%d\n",rs1) ;
		debugprintf("mkcdpath_one: ebuf=%s\n",ebuf) ;
#endif

	        if ((rs1 == SR_OVERFLOW) || (rs1 == SR_NAMETOOLONG)) {
	            rs1 = SR_OK ;
	            pl = 0 ;
	        }

	        if ((rs1 >= 0) && (pl > 0) && (ebuf[0] != '\0')) {
	            struct ustat	sb ;
	            rs = u_lstat(ebuf,&sb) ;
	            if (rs < 0) pl = 0 ;
	        }

	    } /* end if (vecstr_add) */
	} /* end if (not-found) */

#if	CF_DEBUGS
	debugprintf("mkcdpath_one: ret rs=%d pl=%u\n",rs,pl) ;
#endif

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (mkcdpath_one) */


static int mkcdpath_join(char *ebuf,cchar *sp,int sl,cchar *up,int ul)
{
	const int	elen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		pl = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(ebuf,elen,pl,sp,sl) ;
	    pl += rs ;
	}

	if ((rs >= 0) && pl && (ebuf[pl-1] != '/')) {
	    rs = storebuf_char(ebuf,elen,pl,'/') ;
	    pl += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(ebuf,elen,pl,up,ul) ;
	    pl += rs ;
	}

	if (rs == SR_OVERFLOW) rs = SR_NAMETOOLONG ;

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (mkcdpath_join) */


