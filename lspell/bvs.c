/* bvs */

/* Bible Verse Structure */
/* load management and interface for the BVSES object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module implements an interface (a trivial one) that provides access
        to the BVSES object (which is dynamically loaded).


*******************************************************************************/


#define	BVS_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"bvs.h"
#include	"bvses.h"


/* local defines */

#define	BVS_MODBNAME	"bvses"
#define	BVS_OBJNAME	"bvses"

#undef	SMM_INFO
#define	SMM_INFO	BVSES_INFO
#undef	SMM_VERSE
#define	SMM_VERSE	BVSES_VERSE

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,const char *,
			const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	bvs_objloadbegin(BVS *,const char *,const char *) ;
static int	bvs_objloadend(BVS *) ;
static int	bvs_loadcalls(BVS *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static const char	*subs[] = {
	"open",
	"count",
	"info",
	"mkmodquery",
	"audit",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_count,
	sub_info,
	sub_mkmodquery,
	sub_audit,
	sub_close,
	sub_overlast
} ;


/* exported subroutines */


int bvs_open(BVS *op,cchar *pr,cchar *dbname)
{
	int		rs ;
	const char	*objname = BVS_OBJNAME ;

#if	CF_DEBUGS
	debugprintf("bvs_open: pr=%s\n",pr) ;
	debugprintf("bvs_open: dbname=%s\n",dbname) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;
	if (dbname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(BVS)) ;

	if ((rs = bvs_objloadbegin(op,pr,objname)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,pr,dbname)) >= 0) {
		op->magic = BVS_MAGIC ;
	    }
	    if (rs < 0)
		bvs_objloadend(op) ;
	} /* end if (bvs-objloadbegin) */

#if	CF_DEBUGS
	debugprintf("bvs_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvs_open) */


/* free up the entire vector string data structure object */
int bvs_close(BVS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVS_MAGIC) return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = bvs_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (bvs_close) */


int bvs_count(BVS *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVS_MAGIC) return SR_NOTOPEN ;

	if (op->call.count != NULL) {
	    rs = (*op->call.count)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (bvs_count) */


int bvs_info(BVS *op,BVS_INFO *ip)
{
	BVSES_INFO	bvsi ;
	int		rs = SR_NOSYS ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVS_MAGIC) return SR_NOTOPEN ;

	if (op->call.info != NULL) {
	    rs = (*op->call.info)(op->obj,&bvsi) ;
	    n = rs ;
	}

	if (ip != NULL) {
	    memset(ip,0,sizeof(BVS_INFO)) ;
	    if (rs >= 0) {
		ip->ctime = bvsi.ctime ;
		ip->mtime = bvsi.mtime ;
		ip->nzbooks = bvsi.nzbooks ;
		ip->nbooks = bvsi.nbooks ;
		ip->nchapters = bvsi.nchapters ;
		ip->nverses = bvsi.nverses ;
		ip->nzverses = bvsi.nzverses ;
	    }
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (bvs_info) */


int bvs_mkmodquery(BVS *op,BVS_VERSE *bvep,int mjd)
{
	BVSES_VERSE	bv ;
	int		rs = SR_NOSYS ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVS_MAGIC) return SR_NOTOPEN ;

	if (op->call.mkmodquery != NULL) {
	    rs = (*op->call.mkmodquery)(op->obj,&bv,mjd) ;
	    n = rs ;
	}

	if (bvep != NULL) {
	    memset(bvep,0,sizeof(BVS_VERSE)) ;
	    if (rs >= 0) {
		bvep->b = bv.b ;
		bvep->c = bv.c ;
		bvep->v = bv.v ;
	    }
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (bvs_mkmodquery) */


int bvs_audit(BVS *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVS_MAGIC) return SR_NOTOPEN ;

	if (op->call.audit != NULL) {
	    rs = (*op->call.audit)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (bvs_audit) */


/* private subroutines */


/* find and load the DB-access object */
static int bvs_objloadbegin(BVS *op,cchar *pr,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	int		rs ;
	int		rs1 ;
	int		opts ;

#if	CF_DEBUGS
	debugprintf("bvs_objloadbegin: pr=%s\n",pr) ;
	debugprintf("bvs_objloadbegin: objname=%s\n",objname) ;
#endif

	opts = VECSTR_OCOMPACT ;
	if ((rs = vecstr_start(&syms,n,opts)) >= 0) {
	    const int	nlen = SYMNAMELEN ;
	    int		i ;
	    int		f_modload = FALSE ;
	    char	nbuf[SYMNAMELEN + 1] ;

	    for (i = 0 ; (i < n) && (subs[i] != NULL) ; i += 1) {
	        if (isrequired(i)) {
	            if ((rs = sncpy3(nbuf,nlen,objname,"_",subs[i])) >= 0) {
			rs = vecstr_add(&syms,nbuf,rs) ;
		    }
		}
		if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
		const char	**sv ;
	        if ((rs = vecstr_getvec(&syms,&sv)) >= 0) {
	            const char	*modbname = BVS_MODBNAME ;
	            opts = (MODLOAD_OLIBVAR | MODLOAD_OSDIRS) ;
	            rs = modload_open(lp,pr,modbname,objname,opts,sv) ;
		    f_modload = (rs >= 0) ;
		}
	    }

	    rs1 = vecstr_finish(&syms) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && f_modload) {
		modload_close(lp) ;
	    }
	} /* end if (allocation) */

#if	CF_DEBUGS
	debugprintf("bvs_objloadbegin: modload_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    if ((rs = modload_getmv(lp,0)) >= 0) {
		int	objsize = rs ;
		void	*p ;
		if ((rs = uc_malloc(objsize,&p)) >= 0) {
		    op->obj = p ;
		    rs = bvs_loadcalls(op,objname) ;
		    if (rs < 0) {
			uc_free(op->obj) ;
			op->obj = NULL ;
		    }
		} /* end if (memory-allocation) */
	    } /* end if (getmv) */
	    if (rs < 0)
		modload_close(lp) ;
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (bvs_objloadbegin) */


static int bvs_objloadend(BVS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->obj != NULL) {
	    rs1 = uc_free(op->obj) ;
	    if (rs >= 0) rs = rs1 ;
	    op->obj = NULL ;
	}

	rs1 = modload_close(&op->loader) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (bvs_objloadend) */


static int bvs_loadcalls(BVS *op,cchar objname[])
{
	MODLOAD		*lp = &op->loader ;
	const int	nlen = SYMNAMELEN ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	char		nbuf[SYMNAMELEN + 1] ;
	const void	*snp = NULL ;

	for (i = 0 ; subs[i] != NULL ; i += 1) {

	    if ((rs = sncpy3(nbuf,nlen,objname,"_",subs[i])) >= 0) {
	         if ((rs = modload_getsym(lp,nbuf,&snp)) == SR_NOTFOUND) {
		     snp = NULL ;
		     if (! isrequired(i)) rs = SR_OK ;
		}
	    }

	    if (rs < 0) break ;

#if	CF_DEBUGS
	    debugprintf("bvs_loadcalls: call=%s %c\n",
		subs[i],
		((snp != NULL) ? 'Y' : 'N')) ;
#endif

	    if (snp != NULL) {
	        c += 1 ;
		switch (i) {
		case sub_open:
		    op->call.open = 
			(int (*)(void *,const char *,const char *)) snp ;
		    break ;
		case sub_count:
		    op->call.count = (int (*)(void *)) snp ;
		    break ;
		case sub_info:
		    op->call.info = (int (*)(void *,BVSES_INFO *)) snp ;
		    break ;
		case sub_mkmodquery:
		    op->call.mkmodquery = 
			(int (*)(void *,BVSES_VERSE *,int)) snp ;
		    break ;
		case sub_audit:
		    op->call.audit = (int (*)(void *)) snp ;
		    break ;
		case sub_close:
		    op->call.close = (int (*)(void *)) snp ;
		    break ;
		} /* end switch */
	    } /* end if (it had the call) */

	} /* end for (subs) */

#if	CF_DEBUGS
	debugprintf("bvs_loadcalls: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bvs_loadcalls) */


static int isrequired(int i)
{
	int		f = FALSE ;

	switch (i) {
	case sub_open:
	case sub_count:
	case sub_info:
	case sub_mkmodquery:
	case sub_audit:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */

	return f ;
}
/* end subroutine (isrequired) */


