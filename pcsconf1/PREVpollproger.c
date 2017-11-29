/* pollproger (POLLPROG) */

/* this is a PCSPOLLS module for running the PCSPOLL program */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.

	= 2008-10-97, David A­D­ Morano
	Changed somewhat to fit into the new polling structure.

*/

/* Copyright © 1998,2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	This object is a PCSPOLLS module for running the PCSPOLL program.

	Synopsis:

	int pollprog_start(op,pr,sn,envv,pcp)
	PCSPOLLS	*op ;
	const char	*pr ;
	const char	*sn ;
	const char	**envv ;
	PCSCONF		*pcp ;

	Arguments:

	op		object pointer
	pr		program-root
	sn		search-name (of program calling us)
	envv		calling environment
	pcp		PCSCONF pointer

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
#include	<pcsconf.h>
#include	<storebuf.h>
#include	<upt.h>
#include	<localmisc.h>

#include	"pcspolls.h"
#include	"thrbase.h"


/* local defines */

#define	POLLPROG	struct pollprog_head
#define	POLLPROG_FL	struct pollprog_flags
#define	POLLPROG_MAGIC	0x88773422

#define	WORK		struct work_head
#define	WORK_FL		struct work_flags
#define	WORKARGS	struct work_args


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,cchar *,cchar *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	pathadd(char *,int,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;

extern int pollprogcheck(const char *,const char *,const char **,PCSCONF *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */

struct pollprog_flags {
	uint		working:1 ;
} ;

struct pollprog_head {
	uint		magic ;
	THRBASE		t ;
	POLLPROG_FL	f ;
	WORKARGS	*wap ;
	int		dummy ;
} ;

struct work_args {
	POLLPROG	*op ;
	const char	*pr ;
	const char	*sn ;
	const char	**envv ;
	PCSCONF		*pcp ;
} ;

struct work_flags {
	uint		dummy:1 ;
} ;

struct work_head {
	uint		magic ;
	THRBASE		*tip ;
	WORKARGS	*wap ;
	WORK_FL		f ;
	volatile int	f_term ;
} ;

enum cmds {
	cmd_noop,
	cmd_exit,
	cmd_overlast
} ;


/* forward references */

static int workargs_load(WORKARGS *,POLLPROG *,cchar *,cchar *,
		cchar **,PCSCONF *) ;

static int	worker(THRBASE *,WORKARGS *) ;

static int work_start(WORK *,THRBASE *,WORKARGS *) ;
static int work_term(WORK *) ;
static int work_finish(WORK *) ;


/* local variables */


/* exported variables */

PCSPOLLS_NAME	pollprog = {
	"pollprog",
	sizeof(POLLPROG),
	0
} ;


/* exported subroutines */


int pollprog_start(op,pr,sn,envv,pcp)
POLLPROG	*op ;
const char	*pr ;
const char	*sn ;
const char	**envv ;
PCSCONF		*pcp ;
{
	WORKARGS	*wap ;
	const int	wsize = sizeof(WORKARGS) ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("pollprog_start: ent\n") ;
	debugprintf("pollprog_start: pr=%s\n",pr) ;
	debugprintf("pollprog_start: sn=%s\n",sn) ;
	debugprintf("pollprog_start: pcp={%p}\n",pcp) ;
#endif

	memset(op,0,sizeof(POLLPROG)) ;

	if ((rs = uc_malloc(wsize,&wap)) >= 0) {
	    int	(*thr)(THRBASE *,void *) = (int (*)(THRBASE *,void *)) worker ;
	    workargs_load(wap,op,pr,sn,envv,pcp) ;
	    if ((rs = thrbase_start(&op->t,thr,wap)) >= 0) {
		op->f.working = TRUE ;
	        op->wap = wap ;
		op->magic = POLLPROG_MAGIC ;
	    }
	    if (rs < 0) {
		uc_free(wap) ;
		op->wap = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("pollprog_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pollprog_start) */


int pollprog_finish(op)
POLLPROG	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != POLLPROG_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("pollprog_finish: f_working=%d\n",op->f.working) ;
#endif

	if (op->f.working) {
	    op->f.working = FALSE ;
	    rs1 = thrbase_finish(&op->t) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->wap != NULL) {
	    rs1 = uc_free(op->wap) ;
	    if (rs >= 0) rs = rs1 ;
	    op->wap = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("pollprog_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (pollprog_finish) */


#ifdef	COMMENT

int pollprog_info(op,ip)
POLLPROG	*op ;
POLLPROG_INFO	*ip ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != POLLPROG_MAGIC) return SR_NOTOPEN ;

	if (ip != NULL) {
	    memset(ip,0,sizeof(POLLPROG_INFO)) ;
	    ip->dummy = 1 ;
	}

	return rs ;
}
/* end subroutine (pollprog_info) */


int pollprog_cmd(op,cmd)
POLLPROG	*op ;
int		cmd ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != POLLPROG_MAGIC) return SR_NOTOPEN ;

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (pollprog_cmd) */

#endif /* COMMENT */


/* private subroutines */


static int workargs_load(wap,op,pr,sn,envv,pcp)
WORKARGS	*wap ;
POLLPROG	*op ;
const char	*pr ;
const char	*sn ;
const char	**envv ;
PCSCONF		*pcp ;
{
	memset(wap,0,sizeof(WORKARGS)) ;
	wap->op = op ;
	wap->pr = pr ;
	wap->sn = sn ;
	wap->envv = envv ;
	wap->pcp = pcp ;
	return SR_OK ;
}
/* end subroutine (workargs_load) */


static int worker(THRBASE *tip,WORKARGS *wap)
{
	WORK		w ;
	const int	to = 1 ;
	int		rs ;
	int		rs1 ;
	int		ctime = 0 ;

#if	CF_DEBUGS
	debugprintf("pollprog/worker: started\n") ;
#endif

	if ((rs = work_start(&w,tip,wap)) >= 0) {
	    int		f_exit = FALSE ;
	    while ((rs = thrbase_cmdrecv(tip,to)) >= 0) {
		const int	cmd = rs ;
	        switch (cmd) {
		case cmd_noop:
		    ctime += 1 ;
		    break ;
	        case cmd_exit:
		    f_exit = TRUE ;
		    rs = work_term(&w) ;
		    break ;
	        } /* end switch */
		if (f_exit) break ;
		if (rs < 0) break ;
	    } /* end while */
	    rs1 = work_finish(&w) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (work) */

#if	CF_DEBUGS
	debugprintf("pollprog/worker: ret rs=%d ctime=%u\n",rs,ctime) ;
#endif

	return (rs >= 0) ? ctime : rs ;
}
/* end subroutine (worker) */


static int work_start(WORK *wp,THRBASE *tip,WORKARGS *wap)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*pr, *sn ;

	if (wp == NULL) return SR_FAULT ;

	memset(wp,0,sizeof(WORK)) ;
	wp->tip = tip ;
	wp->wap = wap ;

	pr = wap->pr ;
	sn = wap->sn ;

#if	CF_DEBUGS
	debugprintf("pollprog/work_start: pr=%s\n",pr) ;
	debugprintf("pollprog/work_start: sn=%s\n",sn) ;
#endif

	if (pr != NULL) {
	    PCSCONF	*pcp = wap->pcp ;
	    const char	**envv = wap->envv ;
#if	CF_DEBUGS
	debugprintf("pollprog/work_start: pcp={%p}\n",pcp) ;
#endif
	    rs = pollprogcheck(pr,sn,envv,pcp) ;
	    c = rs ;
	}

#if	CF_DEBUGS
	debugprintf("pollprog/work_start: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (work_start) */


static int work_finish(WORK *wp)
{
	int		rs = SR_OK ;

	if (wp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("pollprog/work_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (work_finish) */


static int work_term(WORK *wp)
{
	if (wp == NULL) return SR_FAULT ;
#if	CF_DEBUGS
	debugprintf("pollprog/work_term: ent\n") ;
#endif
	return SR_OK ;
}
/* end subroutine (work_term) */


