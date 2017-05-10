/* progfindprog */

/* find the program to execute */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs (non-switchable) */
#define	CF_DEBUG	0		/* debug print-outs switchable */
#define	CF_DEBUGN	0		/* special debugging */


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

extern int	progdefprog(struct proginfo *,const char **) ;

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	procsearch(struct proginfo *,VECSTR *,char *,const char *) ;

static int	loadpathlist(struct proginfo *,VECSTR *,VECSTR *) ;
static int	loadpathcomp(struct proginfo *,VECSTR *,const char *) ;


/* local variables */


/* exported subroutines */


int progfindprog(PROGINFO *pip,char *rbuf,cchar *pn)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progfindprog: pn=>%s<\n",pn) ;
#endif

#if	CF_DEBUGN
	nprintf(DEBUGFNAME,"progfindprog: pn=%s\n",pn) ;
#endif

	if ((pn == NULL) || (pn[0] != '/')) {
	    vecstr	*elp ;
	    int		f ;
	    const char	*pnp = pn ;

	    rbuf[0] = '\0' ;
	    elp = &pip->exports ;
	    if (pn == NULL) {
	        rs = progdefprog(pip,&pnp) ;
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
	            rs = procsearch(pip,elp,rbuf,pnp) ;
	            if ((rs == SR_NOTFOUND) && (pn == NULL)) {
	                f = FALSE ;
	                f = f || (rs == SR_NOENT) ;
	                if (f) {
	                    pnp = DEFPROGFNAME ;
	                    rs = mkpath1(rbuf,pnp) ;
	                }
	            }
	        } else {
	            rs = mkpath1(rbuf,pnp) ;
	        }
	    } else
	        rs = SR_NOENT ;

	} /* end if (requested) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progfindprog: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progfindprog) */


/* local subroutines */


static int procsearch(PROGINFO *pip,vecstr *elp,char *rbuf,cchar *pn)
{
	VECSTR		pathlist ;
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfindprog/procsearch: pn=>%s<\n",pn) ;
#endif

	if ((rs = vecstr_start(&pathlist,10,0)) >= 0) {
	    if ((rs = loadpathlist(pip,&pathlist,elp)) >= 0) {
	        const int	nrs = SR_NOENT ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            int	i ;
	            char	*cp ;
	            for (i = 0 ; vecstr_get(&pathlist,i,&cp) >= 0 ; i += 1)
	                debugprintf("progfindprog/procsearch: pc=%s\n",cp) ;
	        }
#endif

	        if ((rs = getprogpath(&pip->id,&pathlist,rbuf,pn,-1)) == nrs) {
	            if ((rs = xfile(&pip->id,pn)) >= 0) {
	                rs = mkpath1(rbuf,pn) ;
	            }
	        }

	    } /* end if (loadpathlist) */
	    vecstr_finish(&pathlist) ;
	} /* end if (vecstr) */

	return rs ;
}
/* end subroutine (procsearch) */


static int loadpathlist(pip,plp,elp)
struct proginfo	*pip ;
VECSTR		*plp ;
VECSTR		*elp ;
{
	int		rs ;
	const char	*varpath = VARPATH ;
	const char	*pp ;

	if ((rs = vecstr_search(elp,varpath,vstrkeycmp,&pp)) >= 0) {
	    const char	*tp ;
	    rs = SR_NOENT ;
	    if ((tp = strchr(pp,'=')) != NULL) {
	        rs = loadpathcomp(pip,plp,(tp + 1)) ;
	    }
	}

	return rs ;
}
/* end subroutine (loadpathlist) */


static int loadpathcomp(pip,lp,pp)
struct proginfo	*pip ;
vecstr		*lp ;
const char	*pp ;
{
	int		rs = SR_OK, rs1 ;
	int		cl ;
	int		c = 0 ;
	const char	*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (pip == NULL) return SR_FAULT ; /* LINT */

	while ((cp = strpbrk(pp,":;")) != NULL) {
	    if ((cl = pathclean(tmpfname,pp,(cp - pp))) >= 0) {
	    rs1 = vecstr_findn(lp,tmpfname,cl) ;
	    if (rs1 == SR_NOTFOUND) {
	        c += 1 ;
	        rs = vecstr_add(lp,tmpfname,cl) ;
	    }
	    }
	    pp = (cp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (pp[0] != '\0')) {
	    if ((cl = pathclean(tmpfname,pp,-1)) >= 0) {
	    rs1 = vecstr_findn(lp,tmpfname,cl) ;
	    if (rs1 == SR_NOTFOUND) {
	        c += 1 ;
	        rs = vecstr_add(lp,tmpfname,cl) ;
	    }
	    }
	} /* end if (trailing one) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpathcomp) */


