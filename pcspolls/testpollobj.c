/* testpollobj */

/* this is a test POLLOBJ object for PCSPOLLS */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	= 2008-10-07, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object is a test POLLOBJ for PCSPOLLS.

	Synopsis:

	int testpollobj_start(op,pr,sn,envv,pcp)
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
#include	<sys/stat.h>
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

#define	TESTPOLLOBJ		struct testpollobj_head
#define	TESTPOLLOBJ_FL		struct testpollobj_flags
#define	TESTPOLLOBJ_MAGIC	0x88773422

#define	WORK			struct work_head
#define	WORK_FL			struct work_flags
#define	WORKARGS		struct work_args

#define	TO_CHECK	4


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	pathadd(char *,int,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	hasNotDots(const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern char	*timestr_log(time_t,char *) ;
#endif

extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */

struct testpollobj_flags {
	uint		working:1 ;
} ;

struct testpollobj_head {
	uint		magic ;
	THRBASE		t ;
	TESTPOLLOBJ_FL	f ;
	WORKARGS	*wap ;
	int		dummy ;
} ;

struct work_args {
	TESTPOLLOBJ	*op ;
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
	volatile int	f_term ;
	WORK_FL		f ;
} ;

enum cmds {
	cmd_noop,
	cmd_exit,
	cmd_overlast
} ;


/* forward references */

static int workargs_load(WORKARGS *,TESTPOLLOBJ *,
		cchar *,cchar *,cchar **,PCSCONF *) ;

static int	worker(THRBASE *) ;

static int work_start(WORK *,THRBASE *,WORKARGS *) ;
static int work_term(WORK *) ;
static int work_finish(WORK *) ;


/* local variables */


/* exported variables */

PCSPOLLS_NAME	testpollobj = {
	"testpollobj",
	sizeof(TESTPOLLOBJ),
	0
} ;


/* exported subroutines */


int testpollobj_start(op,pr,sn,envv,pcp)
TESTPOLLOBJ	*op ;
const char	*pr ;
const char	*sn ;
const char	**envv ;
PCSCONF		*pcp ;
{
	WORKARGS	*wap ;
	const int	wsize = sizeof(WORKARGS) ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("testpollobj_start: entered\n") ;
	debugprintf("testpollobj_start: pr=%s\n",pr) ;
	debugprintf("testpollobj_start: sn=%s\n",sn) ;
#endif

	memset(op,0,sizeof(TESTPOLLOBJ)) ;

	if ((rs = uc_malloc(wsize,&wap)) >= 0) {
	    workargs_load(wap,op,pr,sn,envv,pcp) ;
	    if ((rs = thrbase_start(&op->t,worker,wap)) >= 0) {
		op->f.working = TRUE ;
		op->wap = wap ;
		op->magic = TESTPOLLOBJ_MAGIC ;
	    }
	    if (rs < 0) {
		uc_free(wap) ;
		op->wap = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("testpollobj_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (testpollobj_start) */


int testpollobj_finish(op)
TESTPOLLOBJ	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TESTPOLLOBJ_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("testpollobj_finish: f_working=%d\n",op->f.working) ;
#endif

	if (op->f.working) {
	    op->f.working = FALSE ;
	    rs1 = thrbase_finish(&op->t) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->wap != NULL) {
	    rs1 = uyc_free(op->wap) ;
	    if (rs >= 0) rs = rs1 ;
	    op->wap = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("testpollobj_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (testpollobj_finish) */


#ifdef	COMMENT

int testpollobj_info(op,ip)
TESTPOLLOBJ	*op ;
TESTPOLLOBJ_INFO	*ip ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TESTPOLLOBJ_MAGIC) return SR_NOTOPEN ;

	if (ip != NULL) {
	    memset(ip,0,sizeof(TESTPOLLOBJ_INFO)) ;
	    ip->dummy = 1 ;
	}

	return rs ;
}
/* end subroutine (testpollobj_info) */


int testpollobj_cmd(op,cmd)
TESTPOLLOBJ	*op ;
int		cmd ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TESTPOLLOBJ_MAGIC) return SR_NOTOPEN ;

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (testpollobj_cmd) */

#endif /* COMMENT */


/* private subroutines */


static int workargs_load(wap,op,pr,sn,envv,pcp)
WORKARGS	*wap ;
TESTPOLLOBJ	*op ;
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


static int worker(THRBASE *tip)
{
	WORK		w ;
	WORKARGS	*wap = (WORKARGS *) tip->ap ;
	const int	to = 1 ;
	int		rs ;
	int		ctime = 0 ;

#if	CF_DEBUGS
	debugprintf("testpollobj/worker: started\n") ;
#endif

	if ((rs = work_start(&w,tip,wap)) >= 0) {
	    int		f_exit = FALSE ;

	    while ((rs = thrbase_cmdrecv(tip,to)) >= 0) {
		const int	cmd = rs ;

	        switch (cmd) {
		case cmd_noop:
		    ctime += 1 ;
#if	CF_DEBUGS
	debugprintf("testpollobj/worker: timed-poll\n") ;
#endif
		    break ;
	        case cmd_exit:
		    f_exit = TRUE ;
#if	CF_DEBUGS
	debugprintf("testpollobj/worker: exit\n") ;
#endif
		    rs = work_term(&w) ;
		    break ;
	        } /* end switch */

		if (f_exit) break ;
		if (rs < 0) break ;
	    } /* end while */

	    work_finish(&w) ;
	} /* end if (work) */

#if	CF_DEBUGS
	debugprintf("testpollobj/worker: ret rs=%d ctime=%u\n",rs,ctime) ;
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
	debugprintf("testpollobj/work_start: pr=%s\n",pr) ;
	debugprintf("testpollobj/work_start: sn=%s\n",sn) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (work_start) */


static int work_finish(WORK *wp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (wp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("testpollobj/work_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (work_finish) */


static int work_term(WORK *wp)
{
	if (wp == NULL) return SR_FAULT ;
#if	CF_DEBUGS
	debugprintf("testpollobj/work_term: entered\n") ;
#endif
	return SR_OK ;
}
/* end subroutine (work_term) */


