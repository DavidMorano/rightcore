/* sreq */

/* perform various functions on a job */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module is responsible for providing means to store a job and to
        retrieve it later by its PID.  This is the lower half of a pair.
	The SREQDB calls on this object for support.


*******************************************************************************/


#define	SREQ_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<toxc.h>
#include	<upt.h>
#include	<localmisc.h>

#include	"sreq.h"
#include	"mfslocinfo.h"


/* local defines */

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfnext(cchar *,int,cchar **) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	chmods(const char *,mode_t) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	sreq_fdfins(SREQ *) ;

static int	mkfile(cchar *,cchar **) ;


/* global variables */


/* exported subroutines */


int sreq_start(SREQ *jep,cchar *template,cchar *jobid,int ifd,int ofd)
{
	const time_t	dt = time(NULL) ;
	int		rs ;
	cchar		*cp ;

	if (jep == NULL) return SR_FAULT ;
	if (template == NULL) return SR_FAULT ;
	if (jobid == NULL) return SR_FAULT ;

	memset(jep,0,sizeof(SREQ)) ;
	jep->ifd = ifd ;
	jep->ofd = ofd ;
	jep->efd = -1 ;
	jep->pid = -1 ;
	jep->stime = dt ;

	strwcpy(jep->logid,jobid,SREQ_JOBIDLEN) ;

	if ((rs = mkfile(template,&cp)) >= 0) {
	    jep->efname = (char *) cp ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("sreq_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sreq_start) */


int sreq_finish(SREQ *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("sreq_finish: ent ji=%d {%p}\n",jep->ji,jep) ;
#endif

	if (jep->f.ss) {
	    rs1 = sreq_svcentend(jep) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = sreq_thrdone(jep) ;
	if (rs >= 0) rs = rs1 ;

	if (jep->svcbuf != NULL) {
	    rs1 = uc_free(jep->svcbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->svcbuf = NULL ;
	}

	rs1 = sreq_fdfins(jep) ;
	if (rs >= 0) rs = rs1 ;

	if (jep->efname != NULL) {
	    if (jep->efname[0] != '\0') {
		u_unlink(jep->efname) ;
		jep->efname[0] = '\0' ;
	    }
	    rs1 = uc_free(jep->efname) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->efname = NULL ;
	}

	if (jep->svc != NULL) {
	    rs1 = uc_free(jep->svc) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->svc = NULL ;
	}

	jep->pid = -1 ;
	jep->logid[0] = '\0' ;

#if	CF_DEBUGS
	debugprintf("sreq_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sreq_finish) */


int sreq_typeset(SREQ *op,int jt,int st)
{
	op->jtype = jt ;
	op->stype = st ;
	return SR_OK ;
}
/* end subroutine (sreq_typeset) */


int sreq_getfd(SREQ *op)
{
	return op->ifd ;
}
/* end subroutine (sreq_getfd) */


int sreq_havefd(SREQ *op,int fd)
{
	int		f = FALSE ;
	f = f || ((op->ifd >= 0) && (op->ifd == fd)) ;
	f = f || ((op->ofd >= 0) && (op->ofd == fd)) ;
	return f ;
}
/* end subroutine (sreq_havefd) */


int sreq_svcaccum(SREQ *op,cchar *sp,int sl)
{
	int		rs ;
	char		*bp ;
	if (sl < 0) sl = strlen(sp) ;
	if (op->svclen == 0) {
	    if ((rs = uc_malloc((sl+1),&bp)) >= 0) {
		strwcpy(bp,sp,sl) ;
		op->svcbuf = bp ;
		op->svclen = sl ;
	    }
	} else {
	    const int	nlen = (op->svclen+sl) ;
	    if ((rs = uc_realloc(op->svcbuf,(nlen+1),&bp)) >= 0) {
		strwcpy((bp+op->svclen),sp,sl) ;
		op->svcbuf = bp ;
		op->svclen = nlen ;
	    }
	}
	return rs ;
}
/* end subroutine (sreq_svcaccun) */


/* extract the service from the "service-buffer" and alloc-store in 'svc' */
int sreq_svcparse(SREQ *op,int f_long)
{
	int		rs = SR_OK ;
#if	CF_DEBUGS
		debugprintf("sreq_svcparse: svcbuf=>%s<\n",op->svcbuf) ;
#endif
	if (op->svc == NULL) {
	    int		sl ;
	    cchar	*sp ;
	    op->f.longopt = f_long ;
	    if ((sl = sfnext(op->svcbuf,op->svclen,&sp)) >= 0) {
		cchar	*tp ;
	        char	*bp ;
		if ((tp = strnchr(sp,sl,'/')) != NULL) {
		    if (! f_long) {
			if (((sp+sl)-tp) > 1) {
			    op->f.longopt = (tolc(tp[1]) == 'w') ;
			}
		    }
		    sl = (tp-sp) ;
		}
#if	CF_DEBUGS
		debugprintf("sreq_svcparse: s=>%t<\n",sp,sl) ;
#endif
	        if ((rs = uc_malloc((sl+1),&bp)) >= 0) {
	            op->svc = bp ;
	            op->subsvc = strwcpy(bp,sp,sl) ;
	            if ((tp = strnchr(sp,sl,'+')) != NULL) {
		        bp[tp-sp] = '\0' ;
		        op->subsvc = bp+((tp+1)-sp) ;
	            }
	        } /* end if (m-a) */
#if	CF_DEBUGS
		debugprintf("sreq_svcparse: svc=%s\n",op->svc) ;
		debugprintf("sreq_svcparse: subsvc=>%s<\n",op->subsvc) ;
#endif
	    } /* end if (sfnext) */
	} else {
	    rs = SR_BUGCHECK ;
	}
	return rs ;
}
/* end subroutine (sreq_svcparse) */


int sreq_setlong(SREQ *op,int f)
{
	op->f.longopt = MKBOOL(f) ;
	return SR_OK ;
}
/* end subroutine (sreq_setlong) */


int sreq_setstate(SREQ *op,int state)
{
	op->state = state ;
	if (state == sreqstate_done) {
	    op->etime = time(NULL) ;
	    op->f_exiting = TRUE ;
	}
	return SR_OK ;
}
/* end subroutine (sreq_setstate) */


int sreq_getsvc(SREQ *op,cchar **rpp)
{
	int		sl = strlen(op->svc) ;
	if (rpp != NULL) *rpp = op->svc ;
	return sl ;
}
/* end subroutine (sreq_getsvc) */


int sreq_getsubsvc(SREQ *op,cchar **rpp)
{
	int		sl ;
	if (rpp != NULL) *rpp = op->subsvc ;
	sl = strlen(op->subsvc) ;
	return sl ;
}
/* end subroutine (sreq_getsubsvc) */


int sreq_getstate(SREQ *op)
{
	return op->state ;
}
/* end subroutine (sreq_getstate) */


int sreq_ofd(SREQ *op)
{
	if (op->ofd < 0) op->ofd = op->ifd ;
	return op->ofd ;
}
/* end subroutine (sreq_ofd) */


int sreq_svcentbegin(SREQ *jep,LOCINFO *lip,SVCENT *sep)
{
	int		rs ;
	if ((rs = svcentsub_start(&jep->ss,lip,sep)) >= 0) {
	    jep->f.ss = TRUE ;
	}
	return rs ;
}
/* end subroutine (sreq_evcentbegin) */


int sreq_svcentend(SREQ *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;
#if	CF_DEBUGS
	debugprintf("sreq_svcentend: ent ji=%d {%p}\n",jep->ji,jep) ;
#endif
	if (jep->f.ss) {
	    jep->f.ss = FALSE ;
	    rs1 = svcentsub_finish(&jep->ss) ;
	    if (rs >= 0) rs = rs1 ;
	}
#if	CF_DEBUGS
	debugprintf("sreq_svcentend: ret rs=%d \n",rs) ;
#endif
	return rs ;
}
/* end subroutine (sreq_svcentend) */


int sreq_exiting(SREQ *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = sreq_fdfins(jep) ;
	if (rs >= 0) rs = rs1 ;

	jep->f_exiting = TRUE ;

	return rs ;
}
/* end subroutine (sreq_exiting) */


int sreq_thrdone(SREQ *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;
#if	CF_DEBUGS
	debugprintf("sreq_thrdone: ent ji=%d {%p}\n",jep->ji,jep) ;
	debugprintf("sreq_thrdone: f_thread=%u\n",jep->f.thread) ;
#endif
	if (jep->f.thread) {
	    pthread_t	tid = jep->tid ;
	    int		trs ;
	    jep->f.thread = FALSE ;
	    rs1 = uptjoin(tid,&trs) ;
	    if (rs >= 0) rs = rs1 ;
	    if (rs >= 0) rs = trs ;
	}
#if	CF_DEBUGS
	debugprintf("sreq_thrdone: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (sreq_thrdone) */


/* private subroutines */


static int sreq_fdfins(SREQ *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;
#if	CF_DEBUGS
	debugprintf("sreqdb_fdfins: ent ofd=%d ifd=%d\n",jep->ofd,jep->ifd) ;
#endif
	if (jep->ofd >= 0) {
	    if (jep->ofd != jep->ifd) {
	        rs1 = u_close(jep->ofd) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    jep->ofd = -1 ;
	}
#if	CF_DEBUGS
	debugprintf("sreqdb_fdfins: mid1 rs=%d\n",rs) ;
#endif
	if (jep->ifd >= 0) {
	    rs1 = u_close(jep->ifd) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->ifd = -1 ;
	}
#if	CF_DEBUGS
	debugprintf("sreqdb_fdfins: mid2 rs=%d\n",rs) ;
#endif
	if (jep->efd >= 0) {
	    rs1 = u_close(jep->efd) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->efd = -1 ;
	}
#if	CF_DEBUGS
	debugprintf("sreqdb_fdfins: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (sreq_fdfins) */


static int mkfile(cchar *template,cchar **rpp)
{
	int		rs ;
	int		tl = 0 ;
	char		tbuf[MAXPATHLEN + 1] ;

	tbuf[0] = '\0' ;
	if ((rs = mktmpfile(tbuf,0666,template)) >= 0) {
	    tl = rs ;
	    rs = uc_mallocstrw(tbuf,tl,rpp) ;
	    if (rs < 0) {
	        u_unlink(tbuf) ;
		*rpp = NULL ;
	    } /* end if (error-recovery) */
	} /* end if (mktmpfile) */

	return (rs >= 0) ? tl : rs ;
}
/* end subroutines (mkfile) */


