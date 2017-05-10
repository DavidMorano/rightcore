/* opendialer_prog */

/* open-dialer (prog) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 2003-11-04, David A­D­ Morano
	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather I should
	have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an open-dialer.

	Synopsis:

	int opendialer_prog(pr,prn,svc,of,om,argv,envv,to)
	const char	*pr ;
	const char	*prn ;
	const char	*svc ;
	int		of ;
	mode_t		om ;
	const char	**argv ;
	const char	**envv ;
	int		to ;

	Arguments:

	pr		program-root
	prn		facility name
	svc		service name
	of		open-flags
	om		open-mode
	argv		argument array
	envv		environment array
	to		time-out

	Returns:

	>=0		file-descriptor
	<0		error

	= Notes

	Q. Why use |vecstr_addpath()| rather than |vecstr_addpathclean()|?
	A. We only need to traverse the path list once, so the overhead
	   of using |vecstr_addpathclean()| is probably not justified.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"opendialer_prog.h"
#include	"defs.h"


/* local defines */

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#define	EXTRABIN	"/usr/extra/bin"

#define	NDF		"opendialer_prog.deb"


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	findxfile(IDS *,char *,const char *) ;
extern int	getpwd(char *,int) ;
extern int	getprogpath(IDS *,VECSTR *,char *,const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	vecstr_addpath(vecstr *,const char *,int) ;
extern int	vecstr_addpathclean(vecstr *,const char *,int) ;
extern int	vecstr_addcspath(vecstr *) ;

#if	CF_DEBUGS || CF_DEBUGN
extern int	debugprintf(const char *,...) ;
extern int	nprintf(const char *,const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* forward references */

static int findprog(const char *,char *,const char *) ;
static int vecstr_addpr(VECSTR *,char *,const char *) ;


/* local variables */

static const char	*bins[] = {
	"bin",
	"sbin",
	NULL
} ;


/* exported subroutines */


/* ARGSUSED */
int opendialer_prog(pr,prn,svc,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
const char	*svc ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	int		rs = SR_OK ;
	char		progfname[MAXPATHLEN+1] = { 0 } ;

#if	CF_DEBUGS
	debugprintf("opendialer_prog: svc=%s\n",svc) ;
#endif

	if (svc[0] == '\0') return SR_INVALID ;

	if (strchr(svc,'/') == NULL) {
	    rs = findprog(pr,progfname,svc) ;
	    svc = progfname ;
	} else if (svc[0] != '/') {
	    char	pwd[MAXPATHLEN+1] ;
	    if ((rs = getpwd(pwd,MAXPATHLEN)) >= 0) {
		rs = mkpath2(progfname,pwd,svc) ;
		svc = progfname ;
	    }
	} /* end if (simple name) */

	if (rs >= 0) {
	    rs = uc_openprog(svc,of,argv,envv) ;
	}

	return rs ;
}
/* end subroutine (opendialer_prog) */


/* local subroutines */


static int findprog(const char *pr,char *progfname,const char *svc)
{
	IDS		id ;
	int		rs ;
	int		rs1 ;
	int		pl = 0 ;
	if ((rs = ids_load(&id)) >= 0) {
	    VECSTR	p ;
	    if ((rs = vecstr_start(&p,20,0)) >= 0) {
		const char	*pp = getenv(VARPATH) ;
		if ((pp != NULL) && (pp[0] != '\0')) {
		    rs = vecstr_addpath(&p,pp,-1) ;
		} else {
		    if ((rs = vecstr_addcspath(&p)) >= 0) {
			rs = vecstr_add(&p,EXTRABIN,-1) ;
		    }
		}
		if (rs >= 0) {
		    const int	rsn = SR_NOENT ;
		    if ((rs = getprogpath(&id,&p,progfname,svc,-1)) == rsn) {
			if ((rs = vecstr_addpr(&p,progfname,pr)) > 0) {
		    	    rs = getprogpath(&id,&p,progfname,svc,-1) ;
		            pl = rs ;
			} else if (rs == 0) {
			    rs = rsn ;
			}
		    } else {
			pl = rs ;
		    }
		} /* end if (ok) */
		rs1 = vecstr_finish(&p) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (vecstr) */
	    rs1 = ids_release(&id) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ids) */
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (findprog) */


static int vecstr_addpr(VECSTR *plp,char *rbuf,const char *pr)
{
	VECSTR		mores ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;
	if ((rs = vecstr_start(&mores,2,0)) >= 0) {
	    const int	rsn = SR_NOTFOUND ;
	    int		rl ;
	    int		i ;
	    for (i = 0 ; bins[i] != NULL ; i += 1) {
		if ((rs = mkpath2(rbuf,pr,bins[i])) > 0) {
		    rl = rs ;
		    if ((rs = vecstr_find(plp,rbuf)) == rsn) {
			n += 1 ;
			rs = vecstr_add(&mores,rbuf,rl) ;
		    }
		}
		if (rs < 0) break ;
	    } /* end for */
	    if (rs >= 0) {
		if (n > 0) {
		    if ((rs = vecstr_delall(plp)) >= 0) {
		        const char	*pp ;
		        for (i = 0 ; vecstr_get(&mores,i,&pp) >= 0 ; i += 1) {
			    if (pp != NULL) {
				rs = vecstr_add(plp,pp,-1) ;
			    }
			    if (rs < 0) break ;
		        } /* end for */
		    } /* end if (vecstr_delall) */
		} /* end if (positive) */
	    } /* end if (ok) */
	    rs1 = vecstr_finish(&mores) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (vecstr_addpr) */


