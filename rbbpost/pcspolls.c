/* pcspolls */

/* management interface to the PCSPOLL loadable-object poll facility */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* special debugging */
#define	CF_EARLY	1		/* early exit */


/* revision history:

	- 2008-10-07, David A­D­ Morano
	This module was originally written to allow for the main part of the
	PCS-poll facility to be a loadable module.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object module serves as the manager for the PCS loadable-object
	poll facility.

	Synopsis:

	int pcspolls_start(op,pcp,searchname)
	PCSPOLLS	*op ;
	PCSCONF		*pcp ;
	const char	searchname[] ;

	Arguments:

	- op		object pointer
	- pcp		PCS configuration pointer
	- searchname	search-name to use

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vechand.h>
#include	<pcsconf.h>
#include	<fsdir.h>
#include	<storebuf.h>
#include	<upt.h>
#include	<endianstr.h>
#include	<localmisc.h>

#include	"pcspolls.h"


/* local defines */

#define	LIBCNAME	"lib"
#define	POLLCNAME	PCSPOLLS_POLLCNAME

#define	THREAD		struct pcspolls_thread
#define	THREAD_ARGS	struct thread_args

#define	WORK		struct work_head

#define	POLLINFO	struct pollinfo

#define	POLLOBJ		struct pollobj
#define	POLLOBJ_FL	struct pollobj_flags

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#define	TO_CHECK	4

#define	NDF		"pcspolls.deb"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	pathadd(char *,int,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	fileobject(const char *) ;
extern int	hasfext(const char **,const char *,int) ;
extern int	hasNotDots(const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

#if	CF_DEUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */

struct pollinfo {
	void		*sop ;
	const void	*objname ;
	int		(*start)(void *) ;
	int		(*check)(void *) ;
	int		(*info)(void *) ;
	int		(*cmd)(void *) ;
	int		(*finish)(void *) ;
} ;

struct pollobj_flags {
	uint		running:1 ;
	uint		active:1 ;
} ;

struct pollobj {
	const char	*name ;
	void		*sop ;
	void		*obj ;
	int		(*start)(void *) ;
	int		(*check)(void *) ;
	int		(*info)(void *) ;
	int		(*cmd)(void *) ;
	int		(*finish)(void *) ;
	POLLOBJ_FL	f ;
	uint		objsize ;
	uint		infosize ;
} ;

struct thread_args {
	PCSPOLLS	*op ;
	PCSCONF		*pcp ;
} ;

struct work_head {
	THREAD		*tip ;
	VECHAND		polls ;
	volatile int	f_term ;
} ;

enum cmds {
	cmd_noop,
	cmd_exit,
	cmd_overlast
} ;


/* forward references */

static int pcspolls_valsbegin(PCSPOLLS *,PCSCONF *,cchar *) ;
static int pcspolls_valsend(PCSPOLLS *) ;

static int thread_start(THREAD *,PCSPOLLS *) ;
static int thread_finish(THREAD *) ;
static int thread_cmdrecv(THREAD *,int) ;
static int thread_exiting(THREAD *) ;
static int thread_cmdexit(THREAD *) ;
static int thread_waitexit(THREAD *) ;
static int thread_worker(THREAD *) ;

#ifdef	COMMENT
static int thread_setdone(THREAD *) ;
#endif

static int work_start(WORK *,THREAD *) ;
static int work_term(WORK *) ;
static int work_finish(WORK *) ;
static int work_objloads(WORK *,THREAD *,char *,int) ;
static int work_objloadcheck(WORK *,cchar *,cchar *,int) ;
static int work_objload(WORK *,POLLINFO *) ;
static int work_objstarts(WORK *,THREAD *) ;
static int work_objchecks(WORK *) ;
static int work_objfins(WORK *) ;

static int pollinfo_syms(POLLINFO *,void *,cchar *,int) ;

static int pollobj_callstart(POLLOBJ *,THREAD *) ;
static int pollobj_check(POLLOBJ *) ;
static int pollobj_finish(POLLOBJ *) ;

static int mksymname(char *,cchar *,int,cchar *) ;
static int isrequired(int) ;


/* local variables */

static cchar	*exts[] = {
	"so",
	"o",
	"",
	NULL
} ;

static cchar	*subs[] = {
	"start",
	"check",
	"info",
	"cmd",
	"finish",
	NULL
} ;

enum subs {
	sub_start,
	sub_check,
	sub_info,
	sub_cmd,
	sub_finish,
	sub_overlast
} ;


/* exported variables */

PCSPOLLS_OBJ	pcspolls = {
	"pcspolls",
	sizeof(PCSPOLLS),
	sizeof(PCSPOLLS_INFO)
} ;


/* exported subroutines */


int pcspolls_start(PCSPOLLS *op,PCSCONF *pcp,cchar *sn)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (pcp == NULL) return SR_FAULT ;
	if (sn == NULL) return SR_FAULT ;

	if (sn[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("pcspolls_start: sn=%s\n",sn) ;
#endif

	memset(op,0,sizeof(PCSPOLLS)) ;

	if ((rs = pcspolls_valsbegin(op,pcp,sn)) >= 0) {
	    if ((rs = thread_start(&op->t,op)) >= 0) {
		op->f.working = TRUE ;
		op->magic = PCSPOLLS_MAGIC ;
	    }
	    if (rs < 0)
	        pcspolls_valsend(op) ;
	} /* end if (vals) */

#if	CF_DEBUGS
	debugprintf("pcspolls_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pcspolls_start) */


int pcspolls_finish(PCSPOLLS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != PCSPOLLS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGN
	nprintf(NDF,"pcspolls_finish: ent\n") ;
#endif

	if (op->f.working) {
	    op->f.working = FALSE ;
	    rs1 = thread_finish(&op->t) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = pcspolls_valsend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"pcspolls_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (pcspolls_finish) */


int pcspolls_info(PCSPOLLS *op,PCSPOLLS_INFO *ip)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSPOLLS_MAGIC) return SR_NOTOPEN ;

	if (ip != NULL) {
	    memset(ip,0,sizeof(PCSPOLLS_INFO)) ;
	    ip->dummy = 1 ;
	}

	return rs ;
}
/* end subroutine (pcspolls_info) */


int pcspolls_cmd(PCSPOLLS *op,int cmd)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PCSPOLLS_MAGIC) return SR_NOTOPEN ;

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (pcspolls_cmd) */


/* private subroutines */


static int pcspolls_valsbegin(PCSPOLLS *op,PCSCONF *pcp,const char *sn)
{
	int		rs ;
	int		size = 0 ;
	const char	*pr = pcp->pr ;
	char		*bp ;

	op->pcp = pcp ;			/* supposedly stable throughout */
	op->envv = pcp->envv ;		/* supposedly stable throughout */

	size += (strlen(pr)+1) ;
	size += (strlen(sn)+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    op->a = bp ;
	    op->pr = bp ;
	    bp = (strwcpy(bp,pr,-1)+1) ;
	    op->sn = bp ;
	    bp = (strwcpy(bp,sn,-1)+1) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (pcspolls_valsbegin) */


static int pcspolls_valsend(PCSPOLLS *op)
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
/* end subroutine (pcspolls_valsend) */


#ifdef	COMMENT
struct pcspolls_thread {
	PCSPOLLS	*op ;
	PCSCONF		*pcp ;
	const char	*sn ;
	const char 	*pr ;
	const char	*envv ;
	PSEM		s ;
	pthread_t	tid ;
	volatile int	cmd ;
	volatile int	trs ;
	volatile int	f_response ;
	volatile int	f_done ;
	volatile int	f_exiting ;
} ;
#endif /* COMMENT */


static int thread_start(THREAD *tip,PCSPOLLS *op)
{
	int		rs ;

	memset(tip,0,sizeof(THREAD)) ;
	tip->op = op ;
	tip->pr = op->pr ;
	tip->sn = op->sn ;
	tip->envv = op->envv ;
	tip->pcp = op->pcp ;
	tip->trs = SR_INPROGRESS ;
	tip->pid = getpid() ;

	if ((rs = thrcomm_start(&tip->tc,0)) >= 0) {
	    sigset_t	osm, nsm ;
	    if ((rs = uc_sigsetfill(&nsm)) >= 0) {
		if ((rs = pt_sigmask(SIG_BLOCK,&nsm,&osm)) >= 0) {
	    	    pthread_t	tid ;
	    	    uptsub_t	fn = (uptsub_t) thread_worker ;
	    	    if ((rs = uptcreate(&tid,NULL,fn,tip)) >= 0) {
	    	        tip->tid = tid ;
		    }
		    pt_sigmask(SIG_SETMASK,&osm,NULL) ;
		} /* end if (sigmask) */
	    } /* end if (signal handling) */
	} /* end if (thrcomm-start) */

	return rs ;
}
/* end subroutine (thread_start) */


static int thread_finish(THREAD *tip)
{
	const pid_t	pid = getpid() ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"pcspolls/thread__finish: ent\n") ;
#endif

	if (pid == tip->pid) {
	    if ((rs = thread_cmdexit(tip)) >= 0) {
	        rs = thread_waitexit(tip) ;
	        if (rs >= 0) rs = tip->trs ;
	    }
	} else {
	    tip->tid = 0 ;
	} /* end if (ours) */

#if	CF_DEBUGN
	nprintf(NDF,"pcspolls/thread__finish: mid rs=%d\n",rs) ;
#endif

	rs1 = thrcomm_finish(&tip->tc) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"pcspolls/thread__finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (thread_finish) */


static int thread_cmdrecv(THREAD *tip,int to)
{
	int		rs ;
	int		cmd = 0 ;

	if ((rs = thrcomm_cmdrecv(&tip->tc,to)) >= 0) {
	    cmd = rs ;
	} else if (rs == SR_TIMEDOUT) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (thread_cmdrecv) */


static int thread_exiting(THREAD *tip)
{
	tip->f_exiting = TRUE ;
	return thrcomm_exiting(&tip->tc) ;
}
/* end subroutine (thread_exiting) */


static int thread_cmdexit(THREAD *tip)
{
	int		rs = SR_OK ;
	if (! tip->f_exiting) {
	    const int	cmd = cmd_exit ;
	    rs = thrcomm_cmdsend(&tip->tc,cmd,-1) ;
	}
	return rs ;
}
/* end subroutine (thread_cmdexit) */


static int thread_waitexit(THREAD *tip)
{
	int		rs ;
	int		trs ;
	if ((rs = uptjoin(tip->tid,&trs)) >= 0) {
	    tip->trs = trs ;
	}
	return rs ;
}
/* end subroutine (thread_waitexit) */


#ifdef	COMMENT
static int thread_setdone(THREAD *tip)
{
	const int	rrs = 1 ;
	return thrcomm_rspsend(&tip->tc,rrs,-1) ;
}
/* end subroutine (thread_setdone) */
#endif /* COMMENT */


static int thread_worker(THREAD *tip)
{
	WORK		w ;
	const int	to = 4 ;
	int		rs ;
	int		rs1 ;
	int		ctime = 0 ;

#if	CF_DEBUGS
	debugprintf("pcspolls/worker: ent\n") ;
#endif

#if	CF_DEBUGN
	nprintf(NDF,"pcspolls/worker: ent\n") ;
#endif

	if ((rs = work_start(&w,tip)) >= 0) {
	    int		f_exit = FALSE ;
	    while ((rs = thread_cmdrecv(tip,to)) >= 0) {
	        const int	cmd = rs ;
		if ((rs = work_objchecks(&w)) >= 0) {
	            switch (cmd) {
	            case cmd_noop:
	                ctime += 1 ;
	                break ;
	            case cmd_exit:
	                f_exit = TRUE ;
	                rs = work_term(&w) ;
	                break ;
	            } /* end switch */
		} /* end if (work_objchecks) */
	        if (f_exit) break ;
		if (rs < 0) break ;
	    } /* end while */
	    rs1 = work_finish(&w) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (work) */

	rs1 = thread_exiting(tip) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("pcspolls/worker: ret rs=%d ctime=%u\n",rs,ctime) ;
#endif

#if	CF_DEBUGN
	nprintf(NDF,"pcspolls/worker: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (thread_worker) */


static int work_start(WORK *wp,THREAD *tip)
{
	int		rs ;
	int		c = 0 ;
	const char	*pr = tip->pr ;

	if (wp == NULL) return SR_FAULT ;

	memset(wp,0,sizeof(WORK)) ;
	wp->tip = tip ;

#if	CF_DEBUGS
	debugprintf("pcspolls/work_start: ent\n") ;
#endif

	if ((rs = vechand_start(&wp->polls,2,0)) >= 0) {
	    cchar	*ld = LIBCNAME ;
	    cchar	*pd = POLLCNAME ;
	    char	pdname[MAXPATHLEN+1] ;

	    if ((rs = mkpath3(pdname,pr,ld,pd)) >= 0) {
		USTAT	sb ;
	        int	dl = rs ;

#if	CF_DEBUGS
	        debugprintf("pcspolls/work_start: pdname=%s\n",pdname) ;
#endif

	        if ((rs = u_stat(pdname,&sb)) >= 0) {
		    if (S_ISDIR(sb.st_mode)) {
	                if ((rs = work_objloads(wp,tip,pdname,dl)) >= 0) {
	                    if ((c = rs) > 0) {
	                        rs = work_objstarts(wp,tip) ;
	                    }
	                    if (rs < 0)
	                        work_objfins(wp) ;
		        } /* end if (work_objloads) */
		    } /* end if (is-dir) */
		} else if (isNotPresent(rs)) {
		    rs = SR_OK ;
	        } /* end if (stat) */
	    } /* end if (mkpath) */

	    if (rs < 0)
	        vechand_finish(&wp->polls) ;
	} /* end if (vechand-objs) */

#if	CF_DEBUGS
	debugprintf("pcspolls/work_start: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (work_start) */


static int work_finish(WORK *wp)
{
	VECHAND		*plp ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (wp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("pcspolls/work_finish: ent\n") ;
#endif

#if	CF_DEBUGN
	nprintf(NDF,"pcspolls/work_finish: ent\n") ;
#endif

	plp = &wp->polls ;

	rs1 = work_objfins(wp) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(plp) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("pcspolls/work_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (work_finish) */


static int work_term(WORK *wp)
{
	if (wp == NULL) return SR_FAULT ;
	wp->f_term = TRUE ;

#if	CF_DEBUGS
	debugprintf("pcspolls/work_term: ent\n") ;
#endif
	return SR_OK ;
}
/* end subroutine (work_term) */


static int work_objloads(WORK *wp,THREAD *tip,char *dbuf,int dlen)
{
	FSDIR		d ;
	FSDIR_ENT	e ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if (tip == NULL) return SR_FAULT ;
#if	CF_DEBUGS
	debugprintf("pcspolls/work_objloads: ent dl=%d db=%s\n",dlen,dbuf) ;
#endif
	if ((rs = fsdir_open(&d,dbuf)) >= 0) {
	    int		nl ;
	    cchar	*np ;
#if	CF_DEBUGS
	debugprintf("pcspolls/work_objloads: fsdir_open() rs=%d\n",rs) ;
#endif
	    while ((rs = fsdir_read(&d,&e)) > 0) {
	        np = e.name ;
	        nl = rs ;
	        if (hasNotDots(np,nl)) {
	            int	ol ;
	            if ((ol = hasfext(exts,np,nl)) > 0) {
	                if ((rs = pathadd(dbuf,dlen,np)) >= 0) {
#if	CF_DEBUGS
	debugprintf("pcspolls/work_objloads: db=%s\n",dbuf) ;
	debugprintf("pcspolls/work_objloads: _objloadcheck() \n") ;
#endif
	                    rs = work_objloadcheck(wp,dbuf,np,ol) ;
	                    c += rs ;
#if	CF_DEBUGS
	debugprintf("pcspolls/work_objloads: _objloadcheck() rs=%d\n",rs) ;
#endif
	                } /* end if (pathadd) */
	            } /* end if (has proper suffix) */
	        } /* end if (not-dots) */
		if (rs < 0) break ;
	    } /* end while (entries) */
	    dbuf[dlen] = '\0' ;
	    rs1 = fsdir_close(&d) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (fsdir) */
#if	CF_DEBUGS
	debugprintf("pcspolls/work_objloads: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (work_objloads) */


static int work_objloadcheck(WORK *wp,cchar *fname,cchar *sp,int sl)
{
	struct ustat	sb ;
	struct pollinfo	oi ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("pcspolls/work_objloadcheck: ent\n") ;
#endif

	if ((rs = u_stat(fname,&sb)) >= 0) {
	    if (S_ISREG(sb.st_mode) && (sb.st_size > 0)) {
	        if ((rs = fileobject(fname)) > 0) {
	            const int	m = RTLD_LAZY ;
	            void	*sop ;
	            if ((sop = dlopen(fname,m)) != NULL) {
	                memset(&oi,0,sizeof(struct pollinfo)) ;
	                oi.sop = sop ;
	                if ((rs = pollinfo_syms(&oi,sop,sp,sl)) > 0) {
#if	CF_DEBUGS
	debugprintf("pcspolls/work_objloadcheck: work_objload()\n") ;
#endif
	                    c = 1 ;
	                    rs = work_objload(wp,&oi) ;
#if	CF_DEBUGS
	                    debugprintf("pcspolls/work_objloadcheck: "
				"fn=%s rs=%d\n",fname,
	                        rs) ;
#endif
	                }
	                if ((rs < 0) || (c == 0))
	                    dlclose(sop) ;
	            } /* end if (dlopen) */
	        } /* end if (file-object-code) */
	    } /* end if (non-zero file-size) */
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	} /* end if (stat) */

#if	CF_DEBUGS
	debugprintf("pcspolls/work_objloadcheck: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (work_objloadcheck) */


static int work_objload(WORK *wp,POLLINFO *oip)
{
	const int	psize = sizeof(POLLOBJ) ;
	int		rs ;
	void		*p ;

	if (wp == NULL) return SR_FAULT ;
	if (oip == NULL) return SR_FAULT ;

	if ((rs = uc_malloc(psize,&p)) >= 0) {
	    POLLOBJ	*pop = p ;
	    memset(pop,0,psize) ;
	    pop->sop = oip->sop ;
	    pop->start = oip->start ;
	    pop->check = oip->check ;
	    pop->info = oip->info ;
	    pop->cmd = oip->cmd ;
	    pop->finish = oip->finish ;
	    {
	        PCSPOLLS_NAME	*pnp = (PCSPOLLS_NAME *) oip->objname ;
	        pop->name = pnp->name ;
	        pop->objsize = pnp->objsize ;
	        pop->infosize = pnp->infosize ;
	    }
	    {
	        const int	osize = pop->objsize ;
	        if ((rs = uc_malloc(osize,&p)) >= 0) {
	            pop->obj = p ;
	            rs = vechand_add(&wp->polls,pop) ;
#if	CF_DEBUGS
	            debugprintf("pcspolls/work_objload: vechand_add() rs=%d\n",
			rs) ;
#endif
	            if (rs < 0) {
	                uc_free(pop->obj) ;
	                pop->obj = NULL ;
	            }
	        } /* end if (memory-allocation) */
	    }
	    if (rs < 0)
	        uc_free(pop) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (work_objload) */


static int work_objstarts(WORK *wp,THREAD *tip)
{
	VECHAND		*plp = &wp->polls ;
	POLLOBJ		*pop ;
	int		rs = SR_OK ;
	int		i ;
#if	CF_DEBUGS
	debugprintf("pcspolls/work_objstarts: ent\n") ;
#endif
	for (i = 0 ; vechand_get(plp,i,&pop) >= 0 ; i += 1) {
	    if (pop != NULL) {
	        rs = pollobj_callstart(pop,tip) ;
	        if (rs < 0) break ;
	    }
	} /* end for */
#if	CF_DEBUGS
	debugprintf("pcspolls/work_objstarts: ret rs=%d u=%d\n",rs,i) ;
#endif
	return rs ;
}
/* end subroutine (work_objstarts) */


static int work_objchecks(WORK *wp)
{
	VECHAND		*plp = &wp->polls ;
	POLLOBJ		*pop ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	int		i ;
#if	CF_DEBUGN
	nprintf(NDF,"pcspolls/work_objchecks: ent\n") ;
#endif
	for (i = 0 ; vechand_get(plp,i,&pop) >= 0 ; i += 1) {
	    if (pop != NULL) {
	        rs1 = pollobj_check(pop) ;
	        if (rs >= 0) rs = rs1 ;
		c += ((rs1 > 0)?1:0) ;
	    }
	} /* end for */
#if	CF_DEBUGN
	nprintf(NDF,"pcspolls/work_objchecks: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (work_objchecks) */


static int work_objfins(WORK *wp)
{
	VECHAND		*plp = &wp->polls ;
	POLLOBJ		*pop ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;
#if	CF_DEBUGN
	nprintf(NDF,"pcspolls/work_objfins: ent\n") ;
#endif
	for (i = 0 ; vechand_get(plp,i,&pop) >= 0 ; i += 1) {
	    if (pop != NULL) {
	        rs1 = pollobj_finish(pop) ;
	        if (rs >= 0) rs = rs1 ;
		c += ((rs1 > 0)?1:0) ;
	        rs1 = uc_free(pop) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */
#if	CF_DEBUGN
	nprintf(NDF,"pcspolls/work_objfins: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (work_objfins) */


static int pollinfo_syms(POLLINFO *oip,void *sop,cchar *sp,int sl)
{
	int		rs ;
	char		symname[SYMNAMELEN+1] ;

	if ((rs = snwcpy(symname,SYMNAMELEN,sp,sl)) >= 0) {
	    void	*snp ;
	    if ((snp = dlsym(sop,symname)) != NULL) {
	        int	i ;
	        oip->objname = snp ;
	        for (i = 0 ; subs[i] != NULL ; i += 1) {
	            if ((rs = mksymname(symname,sp,sl,subs[i])) >= 0) {
	                if ((snp = dlsym(sop,symname)) != NULL) {
	                    switch (i) {
	                    case sub_start:
	                        oip->start = (int (*)(void *)) snp ;
	                        break ;
	                    case sub_check:
	                        oip->check = (int (*)(void *)) snp ;
	                        break ;
	                    case sub_info:
	                        oip->info = (int (*)(void *)) snp ;
	                        break ;
	                    case sub_cmd:
	                        oip->cmd = (int (*)(void *)) snp ;
	                        break ;
	                    case sub_finish:
	                        oip->finish = (int (*)(void *)) snp ;
	                        break ;
	                    } /* end switch */
	                } /* end if (dlsym) */
	                if ((snp == NULL) && isrequired(i)) {
	                    rs = 0 ; /* mark failure */
	                }
	            } /* end if (mksymname) */
	            if (rs <= 0) break ;
	        } /* end for */
	    } /* end if (dlsym) */
	} /* end if (snwcpy) */

	return rs ;
}
/* end subroutine (pollinfo_syms) */


static int pollobj_callstart(POLLOBJ *pop,THREAD *tip)
{
	int		rs = SR_OK ;

	if (tip == NULL) return SR_FAULT ;

	if (pop->obj != NULL) {
	    void	*pr = (void *) tip->pr ;
	    void	*sn = (void *) tip->sn ;
	    void	*envv = (void *) tip->envv ;
	    PCSCONF	*pcp = tip->pcp ;
	    void	*saddr = (void *) pop->start ;
	    int	(*start)(void *,void *,void *,void *,void *) ;
	    start = (int (*)(void *,void *,void *,void *,void *)) saddr ;
	    if ((rs = (*start)(pop->obj,pr,sn,envv,pcp)) >= 0) {
	        pop->f.running = TRUE ;
	        pop->f.active = TRUE ;
	    }
#if	CF_DEBUGS
	    debugprintf("pcspolls/pollobj_callstart: "
		"pollobj->start() rs=%d\n",rs) ;
#endif
	} else
	    rs = SR_BUGCHECK ;

	return rs ;
}
/* end subroutine (pollobj_callstart) */


static int pollobj_check(POLLOBJ *pop)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

#if	CF_DEBUGN
	nprintf(NDF,"pcspolls/pollobj_check: ent n=%s\n",pop->name) ;
#endif

	if ((pop->check != NULL) && pop->f.running) {
	    int	(*check)(void *) = pop->check ;
	    rs1 = (*check)(pop->obj) ;
	    if (rs >= 0) rs = rs1 ;
	    f = ((rs1 > 0)?1:0) ;
#if	CF_EARLY
	    if (f) {
	    	int	(*finish)(void *) = pop->finish ;
		pop->f.running = FALSE ;
	    	rs1 = (*finish)(pop->obj) ;
	    	if (rs >= 0) rs = rs1 ;
		pop->f.active = FALSE ;
	    }
#endif /* CF_EARLY */
	} /* end if (checking) */

#if	CF_DEBUGN
	nprintf(NDF,"pcspolls/pollobj_check: ret rs=%d\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (pollobj_check) */


static int pollobj_finish(POLLOBJ *pop)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("pcspolls/pollobj_finish: ent n=%s\n",pop->name) ;
#endif

#if	CF_DEBUGN
	nprintf(NDF,"pcspolls/pollobj_finish: ent n=%s\n",pop->name) ;
#endif

	if ((pop->finish != NULL) && pop->f.active) {
	    int	(*finish)(void *) = pop->finish ;
	    rs1 = (*finish)(pop->obj) ;
#if	CF_DEBUGS
	    debugprintf("pcspolls/pollobj_finish: obj->finish() rs=%d\n",rs1) ;
#endif
	    if (rs >= 0) rs = rs1 ;
	    pop->f.running = FALSE ;
	    pop->f.active = FALSE ;
	    f = TRUE ;
	}

	if (pop->obj != NULL) {
	    rs1 = uc_free(pop->obj) ;
	    if (rs >= 0) rs = rs1 ;
	    pop->obj = NULL ;
	}

	if (pop->sop != NULL) {
	   dlclose(pop->sop) ;
	   pop->sop = NULL ;
	}

#if	CF_DEBUGN
	nprintf(NDF,"pcspolls/pollobj_finish: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (pollobj_finish) */


static int mksymname(char *rbuf,const char *sp,int sl,const char *sub)
{
	const int	rlen = SYMNAMELEN ;
	int		rs = SR_OK ;
	int		nl = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,nl,sp,sl) ;
	    nl += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(rbuf,rlen,nl,'_') ;
	    nl += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,nl,sub,-1) ;
	    nl += rs ;
	}

	return (rs >= 0) ? nl : rs ;
}
/* end subroutine (mksymname) */


static int isrequired(int i)
{
	int		f = FALSE ;
	switch (i) {
	case sub_start:
	case sub_finish:
	    f = TRUE ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isrequired) */


