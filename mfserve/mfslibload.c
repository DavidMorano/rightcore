/* mfs-libload */
/* lang=C++11 */

/* try to load s shared-lib into the MFSERVE daemon */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed or inspired from |progserve()| and
	|progshlib()| from the TCPMUXD family of server programs.

*/

/* Copyright © 2011,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code (part of the MFSERVE deamon program) tryies to load a shared
        library object into the program in order to call a Acommand" from within
        the shared library.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<char.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"mfsmain.h"
#include	"mfsconfig.h"
#include	"mfslocinfo.h"
#include	"mfslog.h"
#include	"sreqdb.h"
#include	"sreq.h"
#include	"mfswatch.h"


/* local typedefs */


/* local defines */

#ifndef	PBUFLEN
#define	PBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(4 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	NDF		"/tmp/mfs.deb"


/* external subroutines */

extern "C" int	matstr(const char **,const char *,int) ;
extern "C" int	matostr(const char **,int,const char *,int) ;
extern "C" int	mkshlibname(char *,cchar *,int) ;
extern "C" int	xfile(IDS *,cchar *) ;
extern "C" int	getprogpath(IDS *,vecpstr *,char *,cchar *,int) ;
extern "C" int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	strllen(const char *,int,int) ;
#endif

extern "C" char	*strwcpy(char *,const char *,int) ;
extern "C" char	*strnchr(cchar *,int,int) ;
extern "C" char	*strnpbrk(cchar *,int,cchar *) ;


/* external variables */


/* local structures */

struct libinfo {
	int		enl = 0 ;
	int		lnl = 0 ;
	cchar		*enp = NULL ;
	cchar		*lnp = NULL ;
	char		shlibname[MAXNAMELEN+1] ;
	int		soparse(const SREQ *) ;
	int		libent(const SREQ *) ;
	int		libentsvc(const SREQ *) ;
	int		findlib(PROGINFO *,char *) ;
} ;


/* forward references */

static int	mfslibload_findlib(PROGINFO *,char *,cchar *,int) ;


/* local variables */

static cchar	*prlibs[] = {
	"lib",
	NULL
} ;


/* exported subroutines */


int mfslibload_jobstart(PROGINFO *pip,SREQ *jep)
{
	libinfo		li ;
	int		rs ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = li.soparse(jep)) >= 0) {
	    int	el ;
	    if ((el = li.libent(jep)) >= 0) {
		if ((rs = li.libentsvc(jep)) >= 0) {
		    char	rbuf[MAXPATHLEN+1] ;
		    if ((rs = li.findlib(pip,rbuf)) >= 0) {
			rs = 1 ;

		    }
		}
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (mfslibload_jobstart) */


int mfslibload_jobfinish(PROGINFO *pip,SREQ *jep)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;


	return rs ;
}
/* end subroutine (mfslibload_jobfinish) */


/* object methods */


int libinfo::soparse(const SREQ *jep)
{
	int		rs = SR_NOTFOUND ;
	cchar		*so = jep->ss.var[svckey_so] ;

	if ((lnl = sfshrink(so,-1,&lnp)) > 0) {
	    cchar	*tp ;
	    rs = lnl ;
	    if ((tp = strnchr(lnp,lnl,':')) == NULL) {
	        tp = strnpbrk(lnp,lnl," \t") ;
	    }
	    if (tp != NULL) {
	        enl = sfshrink((tp+1),((lnp+lnl)-(tp+1)),&enp) ;
	        lnl = (tp-lnp) ;
	        while ((lnl > 0) && CHAR_ISWHITE(lnp[lnl-1])) lnl -= 1 ;
	    }
	}

	return rs ;
}
/* end method (libinfo::soparse) */


int libinfo::libent(const SREQ *jep)
{
	int		rs = SR_OK ;

	if ((enp == NULL) || (enl == 0)) {
	    cchar	*a = jep->ss.var[svckey_a] ;
	    cchar	*p = jep->ss.var[svckey_p] ;
	    if ((a != NULL) && (a[0] != '\0')) {
	        int	cl ;
		cchar	*cp ;
		if ((cl = sfnext(a,-1,&cp)) > 0) {
	            enp = cp ;
	            enl = cl ;
		    rs = cl ;
		} else if ((p != NULL) && (p[0] != '\0')) {
		    if ((cl = sfbasename(p,-1,&cp)) > 0) {
	                enp = cp ;
	                enl = cl ;
		        rs = cl ;
		    }
		}
	    }
	}

	return rs ;
}
/* end method (libinfo::libent) */


int libinfo::libentsvc(const SREQ *jep)
{
	if (enl == 0) {
	    enp = jep->svc ;
	    enl = strlen(enp) ;
	}
	return enl ;
}
/* end method (lib::libentsvc) */


int libinfo::findlib(PROGINFO *pip,char *rbuf)
{
	MFSWATCH	*wip = (MFSWATCH *) pip->watch ;
	IDS		*idp = &pip->id ;
	int		rs = SR_OK ;
	int		rl = 0 ;

	if (lnl < 0) lnl = strlen(lnp) ;

	while (lnl && (lnp[lnl-1] == '/')) lnl -= 1 ;

	if (strnchr(lnp,lnl,'.') == NULL) {
	    rs = mkshlibname(shlibname,lnp,lnl) ;
	    lnl = rs ;
	    lnp = shlibname ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progserve/procfindprog: lfn=%t\n",lnp,lnl) ;
#endif

	if (lnp[0] == '/') {
	    if ((rs = mkpath1w(rbuf,lnp,lnl)) >= 0) {
	        rl = rs ;
	        rs = xfile(idp,rbuf) ;
	    }
	} else {
	    cchar	*pr = pip->pr ;
	    int		i ;
	    for (i = 0 ; prlibs[i] != NULL ; i += 1) {
		cchar	*dir = prlibs[i] ;
	        if ((rs = mkpath3w(rbuf,pr,dir,lnp,lnl)) >= 0) {
	            rl = rs ;
	            if ((rs = xfile(idp,rbuf)),isNotPresent(rs)) {
			rl = 0 ;
			rs = SR_OK ;
		    }
		}
		if (rl > 0) break ;
	        if (rs < 0) break ;
	    } /* end for */
	    if ((rs >= 0) && (rl == 0) && wip->open.libdirs) {
	        rs = mfslibload_findlib(pip,rbuf,lnp,lnl) ;
	        rl = rs ;
	    }
	} /* end if */

	return (rs >= 0) ? rl : rs ;
}
/* end method (libinfo::findlib) */


/* local subroutines */


static int mfslibload_findlib(PROGINFO *pip,char *rbuf,cchar *lnp,int lnl)
{
	MFSWATCH	*wip = (MFSWATCH *) pip->watch ;
	IDS		*idp = &pip->id ;
	vecpstr		*plp ;
	int		rs = SR_OK ;
	int		pl = 0 ;
	int		i ;
	cchar		*pp ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_pathfindbinlist: ent\n") ;
#endif
	plp = &wip->libdirs ;
	for (i = 0 ; vecpstr_get(plp,i,&pp) >= 0 ; i += 1) {
	    if (pp != NULL) {
	        if ((rs = mkpath2w(rbuf,pp,lnp,lnl)) >= 0) {
	            pl = rs ;
	            if (((rs = xfile(idp,rbuf)) < 0) && isNotPresent(rs)) {
	                rs = SR_OK ;
	                pl = 0 ;
	            }
	        }
	    }
	    if (pl > 0) break ;
	    if (rs < 0) break ;
	} /* end for */
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfswatch_pathfindbinlist: ret rs=%d pl=%u\n",rs,pl) ;
#endif
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (mfslibload_findlib) */


