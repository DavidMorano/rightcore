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


/* local defines */

#define	POLLPROG	struct pollprog_head
#define	POLLPROG_FL	struct pollprog_flags
#define	POLLPROG_MAGIC	0x88773422


/* typedefs */

typedef int	(*thrsub_t)(void *) ;


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

extern int	pollprogcheck(cchar *,cchar *,cchar **,PCSCONF *) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern cchar	**environ ;


/* local structures */

struct pollprog_flags {
	uint		working:1 ;
} ;

struct pollprog_head {
	uint		magic ;
	POLLPROG_FL	f ;
	pid_t		pid ;
	pthread_t	tid ;
	const char	*a ;		/* memory allocation */
	const char	*pr ;
	const char	*sn ;
	const char	**envv ;
	PCSCONF		*pcp ;
	volatile int	f_exiting ;
} ;

enum cmds {
	cmd_noop,
	cmd_exit,
	cmd_overlast
} ;


/* forward references */

static int	pollprog_argsbegin(POLLPROG *,cchar *,cchar *) ;
static int	pollprog_argsend(POLLPROG *) ;
static int	pollprog_worker(POLLPROG *) ;


/* local variables */


/* exported variables */

PCSPOLLS_NAME	pollprog = {
	"pollprog",
	sizeof(POLLPROG),
	0
} ;


/* exported subroutines */


int pollprog_start(POLLPROG *op,cchar *pr,cchar *sn,cchar **envv,PCSCONF *pcp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("pollprog_start: ent\n") ;
	debugprintf("pollprog_start: pr=%s\n",pr) ;
	debugprintf("pollprog_start: sn=%s\n",sn) ;
	debugprintf("pollprog_start: pcp={%p}\n",pcp) ;
#endif

	if (envv == NULL) envv = environ ;

	memset(op,0,sizeof(POLLPROG)) ;
	op->envv = envv ;
	op->pcp = pcp ;
	op->pid = getpid() ;

	if ((rs = pollprog_argsbegin(op,pr,sn)) >= 0) {
	    if ((pr != NULL) && (sn != NULL)) {
	        pthread_t	tid ;
	        thrsub_t	thr = (thrsub_t) pollprog_worker ;
	        if ((rs = uptcreate(&tid,NULL,thr,op)) >= 0) {
	            op->f.working = TRUE ;
		    op->tid = tid ;
	        }
	    } /* end if (non-null) */
	    if (rs >= 0) {
	        op->magic = POLLPROG_MAGIC ;
	    }
	    if (rs < 0)
		pollprog_argsend(op) ;
	} /* end if (pollprog_argsbegin) */

#if	CF_DEBUGS
	debugprintf("pollprog_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pollprog_start) */


int pollprog_finish(POLLPROG *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != POLLPROG_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("pollprog_finish: f_working=%d\n",op->f.working) ;
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

	rs1 = pollprog_argsend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("pollprog_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (pollprog_finish) */


int pollprog_check(POLLPROG *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != POLLPROG_MAGIC) return SR_NOTOPEN ;

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
/* end subroutine (pollprog_check) */


/* private subroutines */


static int pollprog_argsbegin(POLLPROG *op,cchar *pr,cchar *sn)
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
/* end subroutine (pollprog_argsbegin) */


static int pollprog_argsend(POLLPROG *op)
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
/* end subroutine (pollprog_argsend) */


static int pollprog_worker(POLLPROG *op)
{
	PCSCONF		*pcp = op->pcp ;
	int		rs ;
	const char	**envv = op->envv ;
	const char	*pr = op->pr ;
	const char	*sn = op->sn ;

#if	CF_DEBUGS
	debugprintf("pollprog_worker: ent\n") ;
#endif

	    rs = pollprogcheck(pr,sn,envv,pcp) ;

#if	CF_DEBUGS
	debugprintf("pollprog/work_start: ret rs=%d\n",rs) ;
#endif

	op->f_exiting = TRUE ;
	return rs ;
}
/* end subroutine (pollprog_worker) */


