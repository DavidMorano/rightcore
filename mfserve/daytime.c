/* daytime (DAYTIME) */

/* this is a MFSERVE loadable service-module */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 2008-10-07, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	This object is a MFSERVE loadable service-module.

	Synopsis:

	int daytime_start(op,pr,jep,argv,envv)
	DAYTIME		*op ;
	const char	*pr ;
	SREQ		*jep ;
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
#include	<upt.h>
#include	<nistinfo.h>
#include	<localmisc.h>

#include	"mfserve.h"
#include	"daytime.h"


/* local defines */

#ifndef	ORGCODELEN
#define	ORGCODELEN	MAXNAMELEN
#endif

#ifndef	ORGCODELEN
#define	ORGCODELEN	USERNAMELEN
#endif

#define	DAYTIME_CSIZE	100 	/* default arg-chuck size */

#define	NDF		"daytime.nd"


/* typedefs */

typedef int	(*thrsub_t)(void *) ;


/* external subroutines */

extern int	nleadstr(cchar *,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	getusername(char *,int,uid_t) ;
extern int	localgetorg(cchar *,char *,int,cchar *) ;
extern int	localgetorgcode(cchar *,char *,int,cchar *) ;
extern int	hasNotDots(cchar *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_nist(time_t,NISTINFO *,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int daytime_argsbegin(DAYTIME *,cchar **) ;
static int daytime_argsend(DAYTIME *) ;
static int daytime_worker(DAYTIME *) ;


/* local variables */


/* exported variables */

MFSERVE_MOD	daytime = {
	"daytime",
	sizeof(DAYTIME),
	0
} ;


/* exported subroutines */


int daytime_start(DAYTIME *op,cchar *pr,SREQ *jep,cchar **argv,cchar **envv)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("daytime_start: ent {%p}\n",op) ;
	debugprintf("daytime_start: pr=%s\n",pr) ;
#endif

	if (pr == NULL) return SR_FAULT ;
	if (argv == NULL) return SR_FAULT ;
	if (envv == NULL) return SR_FAULT ;

	memset(op,0,sizeof(DAYTIME)) ;
	op->pr = pr ;
	op->jep = jep ;
	op->envv = envv ;

	if ((rs = sreq_getstdout(jep)) >= 0) {
	    op->ofd = rs ;
	    if ((rs = daytime_argsbegin(op,argv)) >= 0) {
	        pthread_t	tid ;
	        thrsub_t	thr = (thrsub_t) daytime_worker ;
	        if ((rs = uptcreate(&tid,NULL,thr,op)) >= 0) {
	            op->f.working = TRUE ;
	            op->tid = tid ;
	            op->magic = DAYTIME_MAGIC ;
#if	CF_DEBUGS
	            debugprintf("daytime_start: magic=%08x\n",op->magic) ;
#endif
	        }
	        if (rs < 0)
	            daytime_argsend(op) ;
	    } /* end if (daytime_argsbegin) */
	} /* end if (sreq_getstdout) */

#if	CF_DEBUGS
	debugprintf("daytime_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (daytime_start) */


int daytime_finish(DAYTIME *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("daytime_finish: ent {%p}\n",op) ;
#endif

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


int daytime_check(DAYTIME *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("daytime_check: ent {%p}\n",op) ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DAYTIME_MAGIC) return SR_NOTOPEN ;

	if (op->f.working) {
	    const pid_t		pid = getpid() ;
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
	} else {
	    f = TRUE ;
	} /* end if (working) */

#if	CF_DEBUGS
	debugprintf("daytime_check: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (daytime_check) */


int daytime_abort(DAYTIME *op)
{
	const int	f = op->f_exiting ;
#if	CF_DEBUGS
	debugprintf("daytime_abort: ent {%p}\n",op) ;
#endif
	if (op == NULL) return SR_FAULT ;
	if (op->magic != DAYTIME_MAGIC) return SR_NOTOPEN ;
#if	CF_DEBUGS
	debugprintf("daytime_abort: cont f=%u\n",f) ;
#endif
	op->f_abort = TRUE ;
	return f ;
}
/* end subroutine (daytime_abort) */


/* provate subroutines */


static int daytime_argsbegin(DAYTIME *op,cchar **argv)
{
	VECPSTR		*alp = &op->args ;
	const int	ss = DAYTIME_CSIZE ;
	int		rs ;
	if ((rs = vecpstr_start(alp,5,0,ss)) >= 0) {
	    int		i ;
	    op->f.args = TRUE ;
	    for (i = 0 ; (rs >= 0) && (argv[i] != NULL) ; i += 1) {
	        rs = vecpstr_add(alp,argv[i],-1) ;
	    }
	    if (rs < 0) {
	        op->f.args = FALSE ;
	        vecpstr_finish(alp) ;
	    }
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (daytime_argsbegin) */


static int daytime_argsend(DAYTIME *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->f.args) {
	    VECPSTR	*alp = &op->args ;
	    rs1 = vecpstr_finish(alp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (daytime_argsend) */


/* independent thread */
static int daytime_worker(DAYTIME *op)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	debugprintf("daytime_worker: ent\n") ;
#endif

	if (! op->f_abort) {
	    USTAT	sb ;
	    cchar	*pr = op->pr ;
	    if ((rs = uc_stat(pr,&sb)) >= 0) {
	        const uid_t	uid = sb.st_uid ;
	        const int	ulen = USERNAMELEN ;
	        char		ubuf[USERNAMELEN+1] ;
	        if ((rs = getusername(ubuf,ulen,uid)) >= 0) {
	            const int	olen = ORGCODELEN ;
	            char	obuf[ORGCODELEN+1] ;
	            if ((rs = localgetorgcode(pr,obuf,olen,ubuf)) >= 0) {
	                NISTINFO	ni ;
	                const time_t	dt = time(NULL) ;
	                char		ntbuf[NISTINFO_BUFLEN+1+1] ;
#if	CF_DEBUGS
			debugprintf("daytime: obuf=%s\n",obuf) ;
#endif
#if	CF_DEBUGN
			nprintf(NDF,"daytime: obuf=%s\n",obuf) ;
#endif
	                memset(&ni,0,sizeof(NISTINFO)) ;
	                strdcpy1(ni.org,NISTINFO_ORGLEN,obuf) ;
#if	CF_DEBUGN
			nprintf(NDF,"daytime: timestr_nist()\n") ;
#endif
	                timestr_nist(dt,&ni,ntbuf) ;
#if	CF_DEBUGN
			nprintf(NDF,"daytime: t=>%s<\n",ntbuf) ;
#endif
	                {
	                    int	tl = strlen(ntbuf) ;
			    ntbuf[tl++] = '\n' ;
			    ntbuf[tl] = '\0' ;
	                    if ((rs = uc_writen(op->ofd,ntbuf,tl)) >= 0) {
	                        wlen += rs ;
				rs = sreq_closefds(op->jep) ;
			    }
	                }
	            } /* end if (localgetorg) */
	        } /* end if (getusername) */
	    } /* end if (uc_stat) */
	} /* end if (not aborting) */

#if	CF_DEBUGS
	debugprintf("daytime/work_start: ret rs=%d\n",rs) ;
#endif

	op->f_exiting = TRUE ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (daytime_worker) */


