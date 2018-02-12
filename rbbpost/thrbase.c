/* thrbase */

/* this object implements a sort of base thread manager (THRBASE) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2008-10-07, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages a basic thread.

	Synopsis:

	int thrbase_start(op,worker,ap)
	THRBASE		*op ;
	int		(*worker)(THRBASE *,void *) ;
	void		*ap ;

	Arguments:

	op		object pointer
	worker		subroutine to start as a thread
	ap		argument to the worker thread

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
#include	<upt.h>
#include	<localmisc.h>

#include	"thrbase.h"


/* local defines */


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

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	startworker(THRBASE_SI *) ;


/* local variables */


/* exported variables */


/* exported subroutines */


int thrbase_start(THRBASE *tip,int (*worker)(THRBASE *,void *),void *ap)
{
	int		rs ;

	if (tip == NULL) return SR_FAULT ;
	if (worker == NULL) return SR_FAULT ;

	memset(tip,0,sizeof(THRBASE)) ;
	tip->ap = ap ;
	if ((rs = thrcomm_start(&tip->tc,0)) >= 0) {
	    sigset_t	osm, nsm ;
	    if ((rs = uc_sigsetfill(&nsm)) >= 0) {
		if ((rs = pt_sigmask(SIG_BLOCK,&nsm,&osm)) >= 0) {
	            THRBASE_SI	*sip ;
	            const int	size = sizeof(THRBASE_SI) ;
	            if ((rs = uc_malloc(size,&sip)) >= 0) {
	                pthread_t	tid ;
	                int (*thrsub)(void *) = (int (*)(void *)) startworker ;
		        {
		            memset(sip,0,size) ;
		            sip->worker = worker ;
		            sip->tip = tip ;
		        }
	                rs = uptcreate(&tid,NULL,thrsub,sip) ;
	                if (rs >= 0) tip->tid = tid ;
		        if (rs < 0)
		            uc_free(sip) ;
	            } /* end if (memory-allocation) */
		    pt_sigmask(SIG_SETMASK,&osm,NULL) ;
		} /* end if (sigmask) */
	    } /* end if (signal handling) */
	} /* end if (thrcomm-start) */

#if	CF_DEBUGS
	debugprintf("thrbase_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (thrbase_start) */


int thrbase_finish(THRBASE *tip)
{
	int		rs ;
	int		rs1 ;

	if ((rs = thrbase_cmdexit(tip)) >= 0) {

#if	CF_DEBUGS
	debugprintf("thrbase_finish: _cmdexit() rs=%d\n",rs) ;
#endif
	    rs = thrbase_waitexit(tip) ;

#if	CF_DEBUGS
	debugprintf("thrbase_finish: _waitexit() rs=%d\n",rs) ;
#endif
	    if (rs >= 0) rs = tip->trs ;
#if	CF_DEBUGS
	debugprintf("thrbase_finish: trs=%d\n",rs) ;
#endif

	}

#if	CF_DEBUGS
	debugprintf("thrbase_finish: mid rs=%d\n",rs) ;
#endif
	rs1 = thrcomm_finish(&tip->tc) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("thrbase_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (thrbase_finish) */


int thrbase_cmdsend(THRBASE *tip,int cmd,int to)
{
	int		rs = SR_OK ;
	if (! tip->f_exiting) {
	    rs = thrcomm_cmdsend(&tip->tc,cmd,to) ;
	}
	return rs ;
}
/* end subroutine (thrbase_cmdsend) */


int thrbase_cmdexit(THRBASE *tip)
{
	const int	cmd = thrbasecmd_exit ;
	int		rs = SR_OK ;
	if (! tip->f_exiting) {
	    rs = thrbase_cmdsend(tip,cmd,-1) ;
	}
	return rs ;
}
/* end subroutine (thrbase_cmdexit) */


/* called by child-thread */
int thrbase_cmdrecv(THRBASE *tip,int to)
{
	int		rs ;
	int		cmd = 0 ;

	if ((rs = thrcomm_cmdrecv(&tip->tc,to)) >= 0) {
	    cmd = rs ;
	} else if (rs == SR_TIMEDOUT)
	    rs = SR_OK ;

	return (rs >= 0) ? cmd : rs ;
}
/* end subroutine (thrbase_cmdrecv) */


/* called by child-thread */
int thrbase_exiting(THRBASE *tip)
{
	int		rs ;
	tip->f_exiting = TRUE ;
	rs = thrcomm_exiting(&tip->tc) ;
	return rs ;
}
/* end subroutine (thrbase_exiting) */


int thrbase_waitexit(THRBASE *tip)
{
	int		rs = SR_OK ;
	int		f_exited = FALSE ;
#if	CF_DEBUGS
	debugprintf("thrbase_waitexit: tid=%u\n",tip->tid) ;
#endif
	if (! tip->f_exited) {
	    int	trs = SR_INPROGRESS ;
	    if ((rs = uptjoin(tip->tid,&trs)) >= 0) {
	        tip->trs = trs ;
	        tip->f_exited = TRUE ;
		f_exited = TRUE ;
	    }
	}
#if	CF_DEBUGS
	debugprintf("thrbase_waitexit: ret rs=%d trs=%d\n",rs,trs) ;
#endif
	return (rs >= 0) ? f_exited : rs ;
}
/* end subroutine (thrbase_waitexit) */


#ifdef	COMMENT
static int thrbase_cmddone(THRBASE *tip)
{
	const int	rrs = 1 ;
	return thrcomm_rspsend(&tip->tc,rrs,-1) ;
}
/* end subroutine (thrbase_setdone) */
#endif /* COMMENT */


/* private subroutines */


static int startworker(THRBASE_SI *sip)
{
	THRBASE		*tip = sip->tip ;
	int		(*worker)(THRBASE *,void *) = sip->worker ;
	int		rs ;
	int		rs1 ;

	if ((rs = uc_free(sip)) >= 0) {

	    rs = (*worker)(tip,tip->ap) ;

	    rs1 = thrbase_exiting(tip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	return rs ;
}
/* end subroutine (startworker) */


