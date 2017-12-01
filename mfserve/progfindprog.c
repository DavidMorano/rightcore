/* progfindprog */

/* find the program to execute */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */
#define	CF_DEBUG	0		/* debug print-outs switchable */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int progfindprog(PROGINFO *pip,char *rbuf,cchar *pn)

	Arguments:

	pip		pointer to PROGINFO
	rbuf		result buffer
	pn		program name to find

	Returns:

	<0		error
	>=0		found


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"envs.h"


/* local defines */

#ifndef	NOFILE
#define	NOFILE		20
#endif

#ifndef	BUFLEN
#define	BUFLEN		((2 * MAXPATHLEN) + 20)
#endif

#ifndef	ARCHBUFLEN
#define	ARCHBUFLEN	80
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#define	VARBUFLEN	(20 * MAXPATHLEN)

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	VARAST
#define	VARAST		"AST"
#endif

#undef	DEBUGFNAME
#define	DEBUGFNAME	"/tmp/lsh.nd"


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_loadfile(vecstr *,int,const char *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	fperm(int,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	getprogpath(IDS *,VECSTR *,char *,const char *,int) ;
extern int	xfile(IDS *,cchar *) ;

extern int	progdefprog(PROGINFO *,const char **) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strnnlen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	procsearch(PROGINFO *,VECSTR *,char *,const char *) ;

static int	loadpathlist(PROGINFO *,VECSTR *,VECSTR *) ;
static int	loadpathcomp(PROGINFO *,VECSTR *,const char *) ;


/* local variables */


/* exported subroutines */


int progfindprog(PROGINFO *pip,char *rbuf,cchar *pn)
{
	int		rs = SR_OK ;
	int		pl = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progfindprog: pn=>%s<\n",pn) ;
#endif

	if ((pn == NULL) || (pn[0] != '/')) {
	    vecstr	*elp ;
	    cchar	*pnp = pn ;

	    rbuf[0] = '\0' ;
	    elp = &pip->exports ;
	    if (pn == NULL) {
	        rs = progdefprog(pip,&pnp) ;
		pl = rs ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progfindprog: progdefprog() rs=%d\n",rs) ;
#endif
	    } /* end if */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3) && (rs >= 0))
	        debugprintf("progfindprog: pnp=%s\n",pnp) ;
#endif

	    if ((rs >= 0) && (pnp != NULL) && (pnp[0] != '\0')) {
	        if (strchr(pnp,'/') == NULL) {
		    const int	rsn = SR_NOTFOUND ;
	            if ((rs = procsearch(pip,elp,rbuf,pnp)) == rsn) {
	                if (pn == NULL) {
	                        pnp = DEFPROGFNAME ;
	                        rs = mkpath1(rbuf,pnp) ;
			        pl = rs ;
			}
		    } else {
		        pl = rs ;
	            }
	        } else {
	            rs = mkpath1(rbuf,pnp) ;
		    pl = rs ;
	        }
	    } else {
	        rs = SR_NOENT ;
	    }

	} /* end if (requested) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progfindprog: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (progfindprog) */


/* local subroutines */


static int procsearch(PROGINFO *pip,vecstr *elp,char *rbuf,cchar *pn)
{
	VECSTR		pathlist ;
	int		rs ;
	int		rs1 ;
	int		pl = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progfindprog/procsearch: pn=>%s<\n",pn) ;
#endif

	if ((rs = vecstr_start(&pathlist,10,0)) >= 0) {
	    if ((rs = loadpathlist(pip,&pathlist,elp)) >= 0) {
	        const int	nrs = SR_NOENT ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            int		i ;
	            cchar	*cp ;
	            for (i = 0 ; vecstr_get(&pathlist,i,&cp) >= 0 ; i += 1) {
	                debugprintf("progfindprog/procsearch: pc=%s\n",cp) ;
		    }
	        }
#endif

	        if ((rs = getprogpath(&pip->id,&pathlist,rbuf,pn,-1)) == nrs) {
#if	CF_DEBUG
		    if (DEBUGLEVEL(4))
	    	   	     debugprintf("progfindprog/procsearch: NF\n") ;
#endif
	            if ((rs = xfile(&pip->id,pn)) >= 0) {
	                rs = mkpath1(rbuf,pn) ;
			pl = rs ;
	            }
	        } else {
		    pl = rs ;
	        }

	    } /* end if (loadpathlist) */
	    rs1 = vecstr_finish(&pathlist) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("progfindprog/procsearch: ret rs=%d pl=%u\n",rs,pl) ;
	    debugprintf("progfindprog/procsearch: rbuf=%s\n",rbuf) ;
	}
#endif

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (procsearch) */


static int loadpathlist(PROGINFO *pip,VECSTR *plp,VECSTR *elp)
{
	int		rs ;
	int		c = 0 ;
	cchar		*varpath = VARPATH ;
	cchar		*pp ;

	if ((rs = vecstr_search(elp,varpath,vstrkeycmp,&pp)) >= 0) {
	    cchar	*tp ;
	    rs = SR_NOENT ;
	    if ((tp = strchr(pp,'=')) != NULL) {
	        rs = loadpathcomp(pip,plp,(tp + 1)) ;
		c = rs ;
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpathlist) */


static int loadpathcomp(PROGINFO *pip,vecstr *lp,cchar *pp)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		pl ;
	int		c = 0 ;
	cchar		*tp ;
	char		pbuf[MAXPATHLEN + 1] ;

	if (pip == NULL) return SR_FAULT ; /* LINT */

	while ((tp = strpbrk(pp,":;")) != NULL) {
	    if ((rs = pathclean(pbuf,pp,(tp - pp))) >= 0) {
		pl = rs ;
	        if ((rs = vecstr_findn(lp,pbuf,pl)) == rsn) {
	            c += 1 ;
	            rs = vecstr_add(lp,pbuf,pl) ;
	        }
	    }
	    pp = (tp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (pp[0] != '\0')) {
	    if ((rs = pathclean(pbuf,pp,-1)) >= 0) {
		pl = rs ;
	        if ((rs = vecstr_findn(lp,pbuf,pl)) == rsn) {
	            c += 1 ;
	            rs = vecstr_add(lp,pbuf,pl) ;
	        }
	    }
	} /* end if (trailing one) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpathcomp) */


