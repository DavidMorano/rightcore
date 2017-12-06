/* daytime (DAYTIME) */

/* this is a MFSERVE loadable service-module */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2008-10-07, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	This object is a MFSERVE loadable service-module.

	Synopsis:

	int daytime_start(op,pr,sn,argv,envv)
	DAYTIME		*op ;
	const char	*pr ;
	const char	*sn ;
	const char	**argv ;
	const char	**envv ;

	Arguments:

	op		object pointer
	pr		program-root
	sn		search-name (of program calling us)
	argv		array-arguments
	envv		array-environment array

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"daytime.h"


/* local defines */


/* typedefs */

typedef int	(*thrsub_t)(void *) ;


/* external subroutines */

extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	hasNotDots(const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int daytime_argsbegin(DAYTIME *,cchar *,cchar *) ;
static int daytime_argsend(DAYTIME *) ;
static int daytime_worker(DAYTIME *) ;

static int mklogentry(cchar *,cchar *,cchar **,PCSCONF *) ;


/* local variables */


/* exported variables */

DAYTIME_OBJ	daytime = {
	"daytime",
	sizeof(DAYTIME),
	0
} ;


/* exported subroutines */


int daytime_start(DAYTIME *op,cchar *pr,cchar *sn,cchar **argv,cchar **envv)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("daytime_start: ent\n") ;
	debugprintf("daytime_start: pr=%s\n",pr) ;
	debugprintf("daytime_start: sn=%s\n",sn) ;
#endif

	if (pr == NULL) envv = environ ;
	if (sn == NULL) envv = environ ;
	if (argv == NULL) envv = environ ;
	if (envv == NULL) envv = environ ;

	memset(op,0,sizeof(DAYTIME)) ;
	op->envv = envv ;
	op->pcp = pcp ;
	op->pid = getpid() ;

	if ((rs = daytime_argsbegin(op,pr,sn)) >= 0) {
	    if ((pr != NULL) && (sn != NULL)) {
	        pthread_t	tid ;
	        thrsub_t	thr = (thrsub_t) daytime_worker ;
	        if ((rs = uptcreate(&tid,NULL,thr,op)) >= 0) {
	            op->f.working = TRUE ;
		    op->tid = tid ;
	        }
	    } /* end if (non-null) */
	    if (rs >= 0) {
	        op->magic = DAYTIME_MAGIC ;
	    }
	    if (rs < 0)
		daytime_argsend(op) ;
	} /* end if (daytime_argsbegin) */

#if	CF_DEBUGS
	debugprintf("daytime_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (daytime_start) */


int daytime_check(DAYTIME *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DAYTIME_MAGIC) return SR_NOTOPEN ;

	if (op->f.working) {
	    const pid_t	pid = getpid() ;
	    if (pid == op->pid) {
	        if (op->f_exiting) {
	            int		trs = 0 ;
	            op->f.working = FALSE ;
	            rs1 = uptjoin(op->tid,&trs) ;
	            if (rs >= 0) rs = rs1 ;
	            if (rs >= 0) rs = trs ;
	            f = TRUE ;
		}
	    } else {
		op->f.working = FALSE ;
	    }
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (daytime_check) */


int daytime_finish(DAYTIME *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DAYTIME_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("daytime_finish: f_working=%d\n",op->f.working) ;
#endif

	if (op->f.working) {
	    const pid_t	pid = getpid() ;
	    if (pid == op->pid) {
	        int	trs = 0 ;
	        op->f.working = FALSE ;
	        rs1 = uptjoin(op->tid,&trs) ;
	        if (rs >= 0) rs = rs1 ;
	        if (rs >= 0) rs = trs ;
	    } else {
		op->f.working = FALSE ;
		op->tid = 0 ;
	    }
	}

	rs1 = daytime_argsend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("daytime_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (daytime_finish) */


/* provate subroutines */


static int daytime_argsbegin(DAYTIME *op,cchar *pr,cchar *sn)
{
	int		rs ;
	int		size = 0 ;
	char		*bp ;
	size += (((pr !=NULL)?strlen(pr):0)+1) ;
	size += (((sn !=NULL)?strlen(sn):0)+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    op->a = bp ;
	    if (pr != NULL) {
	        op->pr = bp ;
	        bp = (strwcpy(bp,pr,-1)+1) ;
	    }
	    if (sn != NULL) {
	        op->sn = bp ;
	        bp = (strwcpy(bp,sn,-1)+1) ;
	    }
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (daytime_argsbegin) */


static int daytime_argsend(DAYTIME *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->a != NULL) {
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	}
	return rs ;
}
/* end subroutine (daytime_argsend) */


static int daytime_worker(DAYTIME *op)
{
	PCSCONF		*pcp = op->pcp ;
	int		rs ;
	cchar		*pr = op->pr ;
	cchar		*sn = op->sn ;
	cchar		**envv = op->envv ;

#if	CF_DEBUGS
	debugprintf("daytime_worker: ent\n") ;
#endif

	rs = mklogentry(pr,sn,envv,pcp) ;

#if	CF_DEBUGS
	debugprintf("daytime/work_start: ret rs=%d\n",rs) ;
#endif

	op->f_exiting = TRUE ;
	return rs ;
}
/* end subroutine (work_start) */


/* ARGSUSED */
static int mklogentry(cchar *pr,cchar *sn,cchar **envv,PCSCONF *pcp)
{
	int		rs ;
	int		rs1 ;
	const char	*lcname = DAYTIME_LCNAME ;
	const char	*lbname = DAYTIME_LBNAME ;
	char		lfname[MAXPATHLEN+1] ;

	if ((rs = mkpath3(lfname,pr,lcname,lbname)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = u_stat(lfname,&sb)) >= 0) {
		USERINFO	u ;
		if ((rs = userinfo_start(&u,NULL)) >= 0) {
		    LOGFILE	lh, *lhp = &lh ;
		    const char	*logid = u.logid ;
		    if ((rs1 = logfile_open(lhp,lfname,0,0666,logid)) >= 0) {
		        time_t	daytime = time(NULL) ;
			const char	*pv = "¥" ;
	                logfile_userinfo(lhp,&u,daytime,sn,pv) ;
		        logfile_close(lhp) ;
		    } else if (! isNotPresent(rs1)) {
		        rs = rs1 ;
		    }
		    rs1 = userinfo_finish(&u) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (userinfo) */
	    } else if (isNotPresent(rs)) {
		rs = SR_OK ;
	    } /* end if (stat) */
	} /* end if (mkpath) */

	return rs ;
}
/* end subroutine (mklogentry) */


