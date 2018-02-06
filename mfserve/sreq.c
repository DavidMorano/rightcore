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
#include	<estrings.h>
#include	<ctdec.h>
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


/* type-defs */

typedef int	(*objstart_t)(void *,cchar *,SREQ *,cchar **,cchar **) ;
typedef int	(*objcheck_t)(void *) ;
typedef int	(*objabort_t)(void *) ;
typedef int	(*objfinish_t)(void *) ;


/* external subroutines */

extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	sfnext(cchar *,int,cchar **) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	mktmpfile(char *,mode_t,cchar *) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	chmods(cchar *,mode_t) ;
extern int	vecstr_svcargs(vecstr *,int *,cchar *) ;
extern int	vecstr_avmkstr(vecstr *,cchar **,int,char *,int) ;

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
static int	sreq_builtdone(SREQ *) ;

static int	mkfile(cchar *,cchar **) ;


/* local variables */


/* exported subroutines */


int sreq_start(SREQ *jep,cchar *template,cchar *jobid,int ifd,int ofd)
{
	const time_t	dt = time(NULL) ;
	int		rs ;
	cchar		*cp ;

#if	CF_DEBUGS
	debugprintf("sreq_start: ent {%p}\n",jep) ;
#endif

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
	    jep->magic = SREQ_MAGIC ;
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

	if (jep == NULL) return SR_FAULT ;
	if (jep->magic != SREQ_MAGIC) return SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("sreq_finish: ent ji=%d {%p}\n",jep->ji,jep) ;
#endif

	rs1 = sreq_builtdone(jep) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("sreq_finish: mid2 rs=%d\n",rs) ;
#endif

	rs1 = sreq_thrdone(jep) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("sreq_finish: mid3 rs=%d\n",rs) ;
#endif

	rs1 = sreq_fdfins(jep) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = sreq_stderrend(jep) ;
	if (rs >= 0) rs = rs1 ;

	if (jep->av != NULL) {
	    rs1 = uc_free(jep->av) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->av = NULL ;
	}

	if (jep->argstab != NULL) {
	    rs1 = uc_free(jep->argstab) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->argstab = NULL ;
	}

	if (jep->open.namesvcs) {
	    rs1 = sreq_sndestroy(jep) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (jep->f.ss) {
	    rs1 = sreq_svcentend(jep) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (jep->svcbuf != NULL) {
	    rs1 = uc_free(jep->svcbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->svcbuf = NULL ;
	}

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

	jep->magic = 0 ;
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


/* accummulate the service bufffer string */
int sreq_svcaccum(SREQ *op,cchar *sp,int sl)
{
	int		rs ;
	char		*bp ;
	if (sl < 0) sl = strlen(sp) ;
	if ((op->svclen + sl) < SREQ_SVCBUFLEN) {
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
	} else {
	    rs = SR_PROTO ;
	}
	return rs ;
}
/* end subroutine (sreq_svcaccun) */


int sreq_svcmunge(SREQ *jep)
{
	vecstr		sa ; /* server arguments */
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
#if	CF_DEBUGS
	debugprintf("sreq_svcmunge: ent\n") ;
#endif
	if ((rs = vecstr_start(&sa,1,0)) >= 0) {
	    int	f_long = FALSE ;
	    if ((rs = vecstr_svcargs(&sa,&f_long,jep->svcbuf)) >= 0) {
	        c = rs ;
#if	CF_DEBUGS
	        debugprintf("sreq_svcmunge: vecstr_svcargs() rs=%d\n",rs) ;
#endif
	        if ((rs = sreq_svcparse(jep,f_long)) >= 0) {
	            const int	ts = vecstr_strsize(&sa) ;
	            int		as ;
	            cchar	**av = NULL ;

#if	CF_DEBUGS
	            debugprintf("sreq_svcmunge: mid2 c=%u\n",c) ;
	            debugprintf("sreq_svcmunge: mid2 ts=%d\n",ts) ;
#endif

	            as = ((c + 2) * sizeof(cchar *)) ;
	            if ((rs = uc_malloc(as,&av)) >= 0) {
	                char	*at = NULL ;
	                if ((rs = uc_malloc(ts,&at)) >= 0) {

	                    if ((rs = vecstr_avmkstr(&sa,av,as,at,ts)) >= 0) {
	                        jep->nav = (c+1) ;
	                        jep->av = av ;
	                        jep->argstab = at ;
#if	CF_DEBUGS
	                        {
	                            int	i ;
	                            debugprintf("sreq_svcmunge: args¬\n") ;
	                            for (i = 0 ; 
	                                (i < jep->nav) && (av[i] != NULL) ; i += 1) {
	                                debugprintf("sreq_svcmunge: a[%u]=%s\n",
	                                    i,av[i]) ;
	                            }
	                        }
#endif
	                    }

#if	CF_DEBUGS
	                    debugprintf("sreq_svcmunge: vecstr_avmkstr-out rs=%d\n",
	                        rs) ;
#endif

	                    if (rs < 0) uc_free(at) ;
	                } /* end if (memory-allocation) */

#if	CF_DEBUGS
	                debugprintf("sreq_svcmunge: m2-out rs=%d\n",rs) ;
#endif

	                if (rs < 0)
	                    uc_free(av) ;
	            } /* end if (memory-allocation) */

#if	CF_DEBUGS
	            debugprintf("sreq_svcmunge: m1-out rs=%d\n",rs) ;
#endif

	        } /* end if (sreq_argparse) */
	    } /* end if (vecstr_svcargs) */
	    rs1 = vecstr_finish(&sa) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */
#if	CF_DEBUGS
	debugprintf("sreq_svcmunge: ret rs=%d c=%d\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (sreq_svcmunge) */


/* extract the service from the "service-buffer" and alloc-store in 'svc' */
int sreq_svcparse(SREQ *op,int f_long)
{
	int		rs = SR_OK ;
	int		len = 0 ;
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
	            len = sl ;
	            op->svc = bp ;
	            op->subsvc = strwcpy(bp,sp,sl) ;
	            if ((tp = strnchr(sp,sl,'+')) != NULL) {
	                len = (tp-sp) ;
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
	return (rs >= 0) ? len : rs ;
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


int sreq_getjsn(SREQ *jep)
{
	return jep->jsn ;
}
/* end subroutine (sreq_getjsn) */


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


int sreq_getav(SREQ *jep,cchar ***avp)
{
	const int	nav = jep->nav ;
	if (avp != NULL) *avp = jep->av ;
	return  nav ;
}
/* end subroutine (sreq_getav) */


int sreq_ofd(SREQ *op)
{
	if (op->ofd < 0) op->ofd = op->ifd ;
	return op->ofd ;
}
/* end subroutine (sreq_ofd) */


int sreq_getstdin(SREQ *jep)
{
	return jep->ifd ;
}
/* end subroutine (sreq_getstdin) */


int sreq_getstdout(SREQ *op)
{
	if (op->ofd < 0) op->ofd = op->ifd ;
	return op->ofd ;
}
/* end subroutine (sreq_getstdout) */


int sreq_closefds(SREQ *jep)
{
	return sreq_fdfins(jep) ;
}
/* end subroutine (sreq_closefds) */


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


/* spawned thread calls this from its exit-handler */
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


/* services-name - |help| service helper - create new instance */
int sreq_sncreate(SREQ *jep)
{
	int		rs = SR_OK ;
	if (! jep->open.namesvcs) {
	    OSETSTR	*ssp = &jep->namesvcs ;
	    const int	ne = 50 ;
	    if ((rs = osetstr_start(ssp,ne)) >= 0) {
	        jep->open.namesvcs = TRUE ;
	    }
	}
	return rs ;
}
/* end subroutine (sreq_sncreate) */


/* services-name - |help| service helper - destroy existing instance */
int sreq_sndestroy(SREQ *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (jep->open.namesvcs) {
	    OSETSTR	*ssp = &jep->namesvcs ;
	    jep->open.namesvcs = FALSE ;
	    rs1 = osetstr_finish(ssp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (sreq_sndestroy) */


/* services-name - |help| service helper - add a service name */
int sreq_snadd(SREQ *jep,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	if (jep->open.namesvcs) {
	    OSETSTR	*ssp = &jep->namesvcs ;
	    rs = osetstr_add(ssp,sp,sl) ;
	}
	return rs ;
}
/* end subroutine (sreq_snadd) */


/* services-name - |help| service helper - begin enumerate */
int sreq_snbegin(SREQ *jep,SREQ_SNCUR *scp)
{
	int		rs = SR_OK ;
	if (jep->open.namesvcs) {
	    OSETSTR	*ssp = &jep->namesvcs ;
	    OSETSTR_CUR	*curp = &scp->cur ;
	    rs = osetstr_curbegin(ssp,curp) ;
	}
	return rs ;
}
/* end subroutine (sreq_snbegin) */


/* services-name - |help| service helper - end enumerate */
int sreq_snend(SREQ *jep,SREQ_SNCUR *scp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (jep->open.namesvcs) {
	    OSETSTR	*ssp = &jep->namesvcs ;
	    OSETSTR_CUR	*curp = &scp->cur ;
	    rs1 = osetstr_curend(ssp,curp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (sreq_snend) */


/* services-name - |help| service helper - execute enumerate */
int sreq_snenum(SREQ *jep,SREQ_SNCUR *scp,cchar **rpp)
{
	int		rs = SR_OK ;
	if (jep->open.namesvcs) {
	    OSETSTR	*ssp = &jep->namesvcs ;
	    OSETSTR_CUR	*curp = &scp->cur ;
	    rs = osetstr_enum(ssp,curp,rpp) ;
	}
	return rs ;
}
/* end subroutine (sreq_snenum) */


/* biltin operation */
int sreq_builtload(SREQ *jep,MFSERVE_INFO *ip)
{
	const int	osize = ip->objsize ;
	int		rs ;
	void		*p ;
	if (jep == NULL) return SR_FAULT ;
	if (jep->magic != SREQ_MAGIC) return SR_NOTFOUND ;
	if ((rs = uc_malloc(osize,&p)) >= 0) {
	    jep->binfo = *ip ;
	    jep->objp = p ;
	}
	return rs ;
}
/* end subroutine (sreq_builtload) */


/* biltin operation */
int sreq_builtrelease(SREQ *jep)
{
	int		rs = SR_OK ;
	if (jep == NULL) return SR_FAULT ;
	if (jep->magic != SREQ_MAGIC) return SR_NOTFOUND ;
	if (jep->objp != NULL) {
	    if ((rs = uc_free(jep->objp)) >= 0) {
	        jep->objp = NULL ;
	        jep->binfo.objsize = 0 ;
	        jep->f.builtout = FALSE ;
	        jep->f.builtdone = FALSE ;
	        rs = 1 ;
	    }
	}
	return rs ;
}
/* end subroutine (sreq_builtrelease) */


/* biltin operation */
int sreq_objstart(SREQ *jep,cchar *pr,cchar **envv)
{
	int		rs = SR_OK ;
	if (jep == NULL) return SR_FAULT ;
	if (jep->magic != SREQ_MAGIC) return SR_NOTFOUND ;
	if (jep->objp != NULL) {
	    if (! jep->f.builtout) {
	        objstart_t	m = (objstart_t) jep->binfo.start ;
	        cchar	**av = jep->av ;
#if	CF_DEBUGS
	        debugprintf("sreq_objstart: obj{&p} m{%p}\n",
	            jep->objp,m) ;
#endif
	        if ((rs = (*m)(jep->objp,pr,jep,av,envv)) >= 0) {
	            jep->f.builtout = TRUE ;
	        }
	    } else {
	        rs = SR_NOANODE ;
	    }
	} else {
	    rs = SR_NOANODE ;
	}
#if	CF_DEBUGS
	debugprintf("sreq_objstart: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (sreq_objstart) */


/* biltin operation, returns 0=not_done, (!0)=done */
int sreq_objcheck(SREQ *jep)
{
	int		rs = SR_OK ;
	if (jep == NULL) return SR_FAULT ;
	if (jep->magic != SREQ_MAGIC) return SR_NOTFOUND ;
	if (jep->objp != NULL) {
	    if (jep->f.builtout) {
	        if (! jep->f.builtdone) {
	            objcheck_t	m = (objcheck_t) jep->binfo.check ;
	            if ((rs = (*m)(jep->objp)) > 0) {
	                jep->f.builtdone = TRUE ;
	            }
	        } else {
	            rs = TRUE ;
	        }
	    }
	} else {
	    rs = SR_NOANODE ;
	}
#if	CF_DEBUGS
	debugprintf("sreq_objcheck: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (sreq_objcheck) */


/* biltin operation */
int sreq_objabort(SREQ *jep)
{
	int		rs = SR_OK ;
#if	CF_DEBUGS
	debugprintf("sreq_objabort: ent {%p}\n",jep) ;
#endif
	if (jep == NULL) return SR_FAULT ;
	if (jep->magic != SREQ_MAGIC) return SR_NOTFOUND ;
	if (jep->objp != NULL) {
	    if (jep->f.builtout) {
	        if (! jep->f.builtdone) {
	            objabort_t	m = (objabort_t) jep->binfo.abort ;
#if	CF_DEBUGS
	            debugprintf("sreq_objabort: objp{%p} m{%p}\n",jep->objp,m) ;
#endif
	            rs = (*m)(jep->objp) ;
	        } else {
	            rs = TRUE ;
	        }
	    }
	} else {
	    rs = SR_NOANODE ;
	}
#if	CF_DEBUGS
	debugprintf("sreq_objabort: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (sreq_objstart) */


/* biltin operation */
int sreq_objfinish(SREQ *jep)
{
	int		rs = SR_OK ;
	if (jep == NULL) return SR_FAULT ;
	if (jep->magic != SREQ_MAGIC) return SR_NOTFOUND ;
	if (jep->objp != NULL) {
	    if (jep->f.builtout) {
	        objfinish_t	m = (objfinish_t) jep->binfo.finish ;
	        if ((rs = (*m)(jep->objp)) >= 0) {
	            jep->f.builtdone = FALSE ;
	            jep->f.builtout = FALSE ;
	        }
	    }
	} else {
	    rs = SR_NOANODE ;
	}
#if	CF_DEBUGS
	debugprintf("sreq_objfinish: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (sreq_objfinish) */


int sreq_stderrbegin(SREQ *jep,cchar *edname)
{
	const int	elen = MAXPATHLEN ;
	int		rs ;
	int		fd = -1 ;
	cchar		*lead = "err" ;
	char		*ebuf ;
	if ((rs = uc_malloc((elen+1),&ebuf)) >= 0) {
	    const int	dlen = DIGBUFLEN ;
	    char	dbuf[DIGBUFLEN+1] ;
	    jep->efname = ebuf ; /* load address */
	    if ((rs = ctdeci(dbuf,dlen,jep->jsn)) >= 0) {
	        const int	clen = MAXNAMELEN ;
	        cchar		*x = "XXXXXX" ;
	        char		cbuf[MAXNAMELEN+1] ;
	        if ((rs = sncpy4(cbuf,clen,lead,dbuf,".",x)) >= 0) {
	            char	tbuf[MAXPATHLEN+1] ;
	            if ((rs = mkpath2(tbuf,edname,cbuf)) >= 0) {
	                const mode_t	om = 0664 ;
	                const int	of = (O_CREAT|O_WRONLY|O_TRUNC) ;
	                char		*ebuf = jep->efname ;
	                if ((rs = opentmpfile(tbuf,of,om,ebuf)) >= 0) {
	                    jep->efd = rs ;
	                    fd = rs ;
	                } /* end if (uc_open) */
	            } /* end if (mkpath) */
	        } /* end if (sncpy) */
	    } /* end if (cfdeci) */
	    if (rs < 0) {
	        uc_free(jep->efname) ;
	        jep->efname = NULL ;
	    }
	} /* end if (m-a) */
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (sreq_stderrbegin) */


int sreq_stderrclose(SREQ *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (jep->efd >= 0) {
	    rs1 = u_close(jep->efd) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->efd = -1 ;
	}
	return rs ;
}
/* end subroutine (sreq_stderrclose) */


int sreq_stderrend(SREQ *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (jep->efd >= 0) {
	    rs1 = u_close(jep->efd) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->efd = -1 ;
	}
	if (jep->efname != NULL) {
	    if (jep->efname[0] != '\0') {
	        const int	rsn = SR_NOTFOUND ;
	        rs1 = uc_unlink(jep->efname) ;
	        if (rs1 == rsn) rs1 = SR_OK ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = uc_free(jep->efname) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->efname = NULL ;
	}
	return rs ;
}
/* end subroutine (sreq_stderrend) */


/* private subroutines */


static int sreq_builtdone(SREQ *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;
#if	C_FDEBUGS
	debugprintf("sreq_builtdone: ent {%p}\n",jep) ;
#endif
	if (jep->objp != NULL) {
	    if ((rs = sreq_objabort(jep)) >= 0) {
	        if ((rs = sreq_objfinish(jep)) >= 0) {
	            rs1 = uc_free(jep->objp) ;
	            if (rs >= 0) rs = rs1 ;
	            jep->objp = NULL ;
	        }
	    }
	}
#if	C_FDEBUGS
	debugprintf("sreq_builtdone: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (sreq_builtdone) */


static int sreq_fdfins(SREQ *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;
#if	CF_DEBUGS
	debugprintf("sreq_fdfins: ent ofd=%d ifd=%d\n",jep->ofd,jep->ifd) ;
#endif

	rs1 = sreq_stderrclose(jep) ;
	if (rs >= 0) rs = rs1 ;

	if (jep->ofd >= 0) {
	    if (jep->ofd != jep->ifd) {
	        rs1 = u_close(jep->ofd) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    jep->ofd = -1 ;
	}

	if (jep->ifd >= 0) {
	    rs1 = u_close(jep->ifd) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->ifd = -1 ;
	}

#if	CF_DEBUGS
	debugprintf("sreq_fdfins: ret rs=%d\n",rs) ;
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


