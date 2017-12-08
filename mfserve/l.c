/* mfs-listen */

/* MFSERVE Listen */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 2008-10-10, David A­D­ Morano
	This was adapted from the BACKGROUND program.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 2008,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module contains the subroutines that manage program listening.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<vecstr.h>
#include	<poller.h>
#include	<localmisc.h>

#include	"listenspec.h"
#include	"prsetfname.h"
#include	"defs.h"
#include	"mfsmain.h"
#include	"mfslocinfo.h"
#include	"mfslisten.h"
#include	"mfswatch.h"


/* local defines */


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mksublogid(char *,int,cchar *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;
extern int	cfdecmfu(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* forward references */

static int	mfslisten_acqmerge(PROGINFO *,MFSLISTEN_ACQ *) ;
static int	mfslisten_acqpresent(PROGINFO *,MFSLISTEN_ACQ *,LISTENSPEC *) ;
static int	mfslisten_acqtmpdel(PROGINFO *,MFSLISTEN_ACQ *,int) ;
static int	mfslisten_acqfins(PROGINFO *,MFSLISTEN_ACQ *) ;
static int	mfslisten_delmarked(PROGINFO *,POLLER *) ;
static int	mfslisten_hit(PROGINFO *,LISTENSPEC **,int,int) ;
static int	mfslisten_new(PROGINFO *,int,int) ;
static int	mfslisten_fins(PROGINFO *) ;


/* local variables */


/* exported subroutines */


int mfslisten_begin(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	if (! lip->open.listens) {
	    VECOBJ	*llp = &lip->listens ;
	    const int	esize = sizeof(LISTENSPEC) ;
	    const int	n = 4 ;
	    const int	vo = 0 ;
	    if ((rs = vecobj_start(llp,esize,n,vo)) >= 0) {
	        lip->open.listens = TRUE ;
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfslisten_begin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfslisten_begin) */


int mfslisten_end(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	if (lip->open.listens) {
	    VECOBJ	*llp = &lip->listens ;
	    rs1 = mfslisten_fins(pip) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->open.listens = FALSE ;
	    rs1 = vecobj_finish(llp) ;
	    if (rs >= 0) rs = rs1 ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfslisten_end: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfslisten_end) */


int mfslisten_acqbegin(PROGINFO *pip,MFSLISTEN_ACQ *acp)
{
	int		rs = SR_OK ;
	if (pip->f.daemon) {
	    VECOBJ	*tlp = &acp->tmps ;
	    const int	esize = sizeof(LISTENSPEC) ;
	    const int	n = 4 ;
	    const int	vo = 0 ;
	    rs = vecobj_start(tlp,esize,n,vo) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfslisten_acqbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfslisten_acqbegin) */


int mfslisten_acqend(PROGINFO *pip,MFSLISTEN_ACQ *acp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->f.daemon) {
	    VECOBJ	*tlp = &acp->tmps ;
	    rs1 = mfslisten_acqmerge(pip,acp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = mfslisten_acqfins(pip,acp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = vecobj_finish(tlp) ;
	    if (rs >= 0) rs = rs1 ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfslisten_acqend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfslisten_acqend) */


int mfslisten_acqadd(PROGINFO *pip,MFSLISTEN_ACQ *acp,cchar *ebuf,int elen)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		n = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfslisten_acqadd: ent ebuf=>%t<\n",ebuf,elen) ;
#endif

	if (pip->f.daemon) {
	    VECSTR	al, *alp = &al ;
	    if ((rs = vecstr_start(alp,5,0)) >= 0) {
	        cchar	*sp = ebuf ;
	        cchar	*tp ;
	        int	sl = elen ;
	        int	c = 0 ;

	        while ((tp = strnchr(sp,sl,CH_FS)) != NULL) {
	            c += 1 ;
	            rs = vecstr_add(alp,sp,(tp-sp)) ;
	            if (rs < 0) break ;
	            sl -= (tp+1)-sp ;
	            sp = (tp+1) ;
	        } /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfslisten_acqadd: mid1 rs=%d\n",rs) ;
#endif

	        if ((rs >= 0) && (sl > 0)) {
	            c += 1 ;
	            rs = vecstr_add(alp,sp,sl) ;
	        }

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfslisten_acqadd: mid2 rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            cchar	**av ;
	            if ((rs = vecstr_getvec(alp,&av)) >= 0) {
	    		LISTENSPEC	ls ;
	                if ((rs = listenspec_start(&ls,c,av)) >= 0) {
	                    n = rs ;

	                    if (n > 0) {
	    			VECOBJ	*tlp = &acp->tmps ;
	                        rs = vecobj_add(tlp,&ls) ;
				if (rs < 0)
				    listenspec_finish(&ls) ;
	                    } else {
	                        listenspec_finish(&ls) ;
			    }

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfslisten_acqadd: mid2c rs=%d\n",rs) ;
#endif

	                } /* end if (listenspec_start) */
	            } /* end if (vecstr_getvec) */
	        } /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfslisten_acqadd: mid3 rs=%d\n",rs) ;
#endif

	        rs1 = vecstr_finish(alp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (vecstr) */
	} /* end if (daemon mode) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfslisten_acqadd: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (mfslisten_acqadd) */


int mfslisten_maint(PROGINFO *pip,POLLER *pmp)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mfslisten_maint: ent\n") ;
#endif

	if ((rs = mfslisten_delmarked(pip,pmp)) >= 0) {
	    LISTENSPEC		*lsp ;
	    LISTENSPEC_INFO	li ;
	    POLLER_SPEC		ps ;
	    vecobj		*llp = &lip->listens ;
	    const int		events = (POLLIN | POLLPRI) ;
	    int			i ;
	    int			f_active ;
	    int			f_broken ;

	    for (i = 0 ; vecobj_get(llp,i,&lsp) >= 0 ; i += 1) {
	        if (lsp != NULL) {
	            if ((rs = listenspec_info(lsp,&li)) >= 0) {
	                f_active = (li.state & LISTENSPEC_MACTIVE) ;
	                f_broken = (li.state & LISTENSPEC_MBROKEN) ;
	                if ((! f_active) && (! f_broken)) {
			    const int	f = TRUE ;
		            int		lo = 0 ;
		            if (pip->f.reuseaddr) {
				lo |= LISTENSPEC_MREUSE ;
			    }
	                    if ((rs = listenspec_active(lsp,lo,f)) >= 0) {
				int	fd ;
	            		if ((fd = listenspec_getfd(lsp)) >= 0) {
	            		    ps.fd = fd ;
	            		    ps.events = events ;
	            		    rs = poller_reg(pmp,&ps) ;
				}
	        	    } /* end if (successful activation) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mfslisten_maint: listenspec_active out rs=%d\n",rs) ;
#endif
	                } /* end if (need activation) */
	            } /* end if (listenspec_info) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mfslisten_maint: listenspec_info out rs=%d\n",rs) ;
#endif
	        } /* end if (non-null) */
		if (rs < 0) break ;
	    } /* end for */
	} /* end if (mfslisten_delmarked) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mfslisten_maint: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mfslisten_maint) */


/* ATHSUSED */
int mfslisten_poll(PROGINFO *pip,POLLER *pmp,int fd,int re)
{
	LOCINFO		*lip = pip->lip ;
	LISTENSPEC	*lsp ;
	int		rs ;
	int		f = FALSE ;
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mfslisten_poll: ent fd=%u\n",fd) ;
#endif
	if ((rs = mfslisten_hit(pip,&lsp,fd,re)) > 0) {
	    SOCKADDRESS		sa ;
	    int			salen = sizeof(SOCKADDRESS) ;
	    f = TRUE ;
	    if ((rs = locinfo_getaccto(lip)) >= 0) { /* "accept" timeout */
	        const int	to = rs ;
	        if ((rs = listenspec_accept(lsp,&sa,&salen,to)) >= 0) {
	            const int	cfd = rs ;
		    if ((rs = listenspec_gettype(lsp)) >= 0) {
			const int	stype = rs ; /* sub-type */
		        if ((rs = mfslisten_new(pip,stype,cfd)) >= 0) {
			    POLLER_SPEC	ps ;
			    ps.fd = fd ;
			    ps.events = (POLLIN | POLLPRI) ;
		            rs = poller_reg(pmp,&ps) ;
			} /* end if (mfslisten_new) */
		    } /* end if (listenspec_gettype) */
		    if (rs < 0)
			u_close(cfd) ;
		} /* end if (listenspec_accept) */
	    } /* end if (locinfo_getaccto) */
	} /* end if (mfslisten_hit) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mfslisten_poll: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mfslisten_poll) */


int mfslisten_getinst(PROGINFO *pip,MFSLISTEN_INST *ip,int idx)
{
	LOCINFO		*lip = pip->lip ;
	LISTENSPEC	*lsp ;
	VECOBJ		*tlp ;
	int		rs ;
	tlp = &lip->listens ;
	if ((rs = vecobj_get(tlp,idx,&lsp)) >= 0) {
	    rs = listenspec_info(lsp,ip) ;
	}
	return rs ;
}
/* end subroutine (mfslisten_inst) */


/* local subroutines */


static int mfslisten_acqmerge(PROGINFO *pip,MFSLISTEN_ACQ *acp)
{
	LOCINFO		*lip = pip->lip ;
	VECOBJ		*tlp = &acp->tmps ;
	VECOBJ		*llp ;
	LISTENSPEC	*lsp ;
	LISTENSPEC	*tlsp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfslisten_acqmerge: ent\n") ;
#endif

/* phase-1 */

	llp = &lip->listens ;
	for (i = 0 ; vecobj_get(llp,i,&lsp) >= 0 ; i += 1) {
	    if (lsp != NULL) {
	        if ((rs1 = mfslisten_acqpresent(pip,acp,lsp)) >= 0) {
	            mfslisten_acqtmpdel(pip,acp,rs1) ;
	        } else {
	            listenspec_delset(lsp,TRUE) ;
	        }
	    }
	} /* end for */

/* phase-2 */

	if (rs >= 0) {
	    for (i = 0 ; vecobj_get(tlp,i,&tlsp) >= 0 ; i += 1) {
	        if (tlsp != NULL) {

	            rs1 = listenspec_delmarked(tlsp) ; /* cannot happen! */

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("progconfig/mfslisten_acqmerge: "
	                    "listenspec_delmarked() rs1=%d\n",rs1) ;
#endif

	            if (rs1 <= 0) {
	                rs = vecobj_add(llp,tlsp) ;
	                vecobj_del(tlp,i--) ;
	            }

	        }
	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfslisten_acqmerge: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mfslisten_acqmerge) */


static int mfslisten_acqpresent(PROGINFO *pip,MFSLISTEN_ACQ *acp,
		LISTENSPEC *lsp)
{
	VECOBJ		*tlp = &acp->tmps ;
	LISTENSPEC	*tlsp ;
	int		rs ;
	int		rs1 ;
	int		i = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfslisten_acqpresent: ent\n") ;
#endif

	if (pip == NULL) return SR_FAULT ; /* lint */
	if (lsp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vecobj_get(tlp,i,&tlsp)) >= 0 ; i += 1) {
	    if (tlsp != NULL) {
	        rs1 = listenspec_issame(lsp,tlsp) ;
	        if (rs1 > 0) break ;
	    }
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfslisten_acqpresent: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mfslisten_acqpresent) */


static int mfslisten_acqtmpdel(PROGINFO *pip,MFSLISTEN_ACQ *acp,int ei)
{
	VECOBJ		*tlp = &acp->tmps ;
	LISTENSPEC	*tlsp = NULL ;
	int		rs ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ; /* lint */

	if ((rs = vecobj_get(tlp,ei,&tlsp)) >= 0) {
	    if (tlsp != NULL) {
	        rs1 = listenspec_finish(tlsp) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = vecobj_del(tlp,ei) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (mfslisten_acqtmpdel) */


static int mfslisten_acqfins(PROGINFO *pip,MFSLISTEN_ACQ *acp)
{
	VECOBJ		*tlp = &acp->tmps ;
	LISTENSPEC	*lsp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (pip == NULL) return SR_FAULT ; /* lint */

	for (i = 0 ; vecobj_get(tlp,i,&lsp) >= 0 ; i += 1) {
	    if (lsp != NULL) {
	        rs1 = listenspec_finish(lsp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (mfslisten_acqfins) */


static int mfslisten_delmarked(PROGINFO *pip,POLLER *pmp)
{
	LOCINFO		*lip = pip->lip ;
	LISTENSPEC	*lsp ;
	vecobj		*llp ;
	int		rs = SR_OK ;
	int		i ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mfslisten_delmarked: ent\n") ;
#endif

	llp = &lip->listens ;
	for (i = 0 ; vecobj_get(llp,i,&lsp) >= 0 ; i += 1) {
	    if (lsp != NULL) {
	        if ((rs = listenspec_delmarked(lsp)) > 0) {
	            if ((rs = listenspec_getfd(lsp)) >= 0) {
	                poller_cancelfd(pmp,rs) ;
		    }
		    listenspec_finish(lsp) ;
		    vecobj_del(llp,i--) ;
	        } /* end if (marked for deletion) */
	    } /* end if */
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mfslisten_delmarked: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mfslisten_delmarked) */


/* ARGSUSED */
static int mfslisten_hit(PROGINFO *pip,LISTENSPEC **rpp,int fd,int re)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	*rpp = NULL ;
	if (re != 0) {
	    vecobj	*llp = &lip->listens ;
	    LISTENSPEC	*lsp ;
	    int		lfd ;
	    int		i ;
	    for (i = 0 ; vecobj_get(llp,i,&lsp) >= 0 ; i += 1) {
	        if (lsp != NULL) {
	            if ((lfd = listenspec_getfd(lsp)) >= 0) {
	                if (lfd == fd) {
		            f = TRUE ;
		            *rpp = lsp ;
		        }
		    }
	        }
	        if (f) break ;
	    } /* end for */
	} /* end if (had events) */
#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mfslisten_hit: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mfslisten_hit) */


static int mfslisten_new(PROGINFO *pip,int stype,int fd)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mfslisten_new: ent stype=%u fd=%u\n",stype,fd) ;
#endif

	if (pip->watch != NULL) {
	    const int	jtype = jobtype_listen ;
	    rs = mfswatch_newjob(pip,jtype,stype,fd,-1) ;
	} else {
	    u_close(fd) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("mfslisten_new: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfslisten_new) */


static int mfslisten_fins(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	{
	    VECOBJ	*llp = &lip->listens ;
	    LISTENSPEC	*lsp ;
	    int		i ;
	    for (i = 0 ; (rs1 = vecobj_get(llp,i,&lsp)) >= 0 ; i += 1) {
	        if (lsp != NULL) {
		    rs1 = listenspec_finish(lsp) ;
		    if (rs >= 0) rs = rs1 ;
	        }
	    } /* end for */
	    if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (mfslisten_fins) */


