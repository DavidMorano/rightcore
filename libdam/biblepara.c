/* biblepara */

/* BIBLEPARA object-load management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_LOOKSELF	0		/* try searching "SELF" for SO */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that allows access
	to the BIBLEPARA datbase.


*******************************************************************************/


#define	BIBLEPARA_MASTER	1


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

#include	"biblepara.h"
#include	"bibleparas.h"


/* local defines */

#define	BIBLEPARA_DEFENTS	(44 * 1000)

#define	BIBLEPARA_MODBNAME	"bibleparas"
#define	BIBLEPARA_OBJNAME	"bibleparas"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#define	BVS_Q			BIBLEPARAS_CITE
#define	BVS_C			BIBLEPARAS_CUR
#define	BVS_I			BIBLEPARAS_INFO


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,const char *,
			const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	biblepara_objloadbegin(BIBLEPARA *,const char *,const char *) ;
static int	biblepara_objloadend(BIBLEPARA *) ;
static int	biblepara_loadcalls(BIBLEPARA *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static const char	*subs[] = {
	"open",
	"count",
	"ispara",
	"curbegin",
	"enum",
	"curend",
	"audit",
	"info",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_count,
	sub_ispara,
	sub_curbegin,
	sub_enum,
	sub_curend,
	sub_audit,
	sub_info,
	sub_close,
	sub_overlast
} ;


/* exported subroutines */


int biblepara_open(BIBLEPARA *op,cchar pr[],cchar dbname[])
{
	int		rs ;
	const char	*objname = BIBLEPARA_OBJNAME ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(BIBLEPARA)) ;

	if ((rs = biblepara_objloadbegin(op,pr,objname)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,pr,dbname)) >= 0) {
		op->magic = BIBLEPARA_MAGIC ;
	    }
	    if (rs < 0)
		biblepara_objloadend(op) ;
	} /* end if (biblepara_objloadbegin) */

#if	CF_DEBUGS
	debugprintf("biblepara_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (biblepara_open) */


/* free up the entire vector string data structure object */
int biblepara_close(BIBLEPARA *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARA_MAGIC) return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("biblepara_close: OBJ_close() rs=%d\n",rs) ;
#endif

	rs1 = biblepara_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("biblepara_close: _objloadend() rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (biblepara_close) */


int biblepara_count(BIBLEPARA *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARA_MAGIC) return SR_NOTOPEN ;

	if (op->call.count != NULL)
	    rs = (*op->call.count)(op->obj) ;

	return rs ;
}
/* end subroutine (biblepara_count) */


int biblepara_audit(BIBLEPARA *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARA_MAGIC) return SR_NOTOPEN ;

	if (op->call.audit != NULL)
	    rs = (*op->call.audit)(op->obj) ;

	return rs ;
}
/* end subroutine (biblepara_audit) */


/* get a string by its index */
int biblepara_ispara(BIBLEPARA *op,BIBLEPARA_CITE *qp)
{
	BIBLEPARAS_CITE	sq ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARA_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("biblepara_ispara: q=%u:%u:%u\n",
		qp->b,qp->c,qp->v) ;
#endif

	sq.b = qp->b ;
	sq.c = qp->c ;
	sq.v = qp->v ;
	rs = (*op->call.ispara)(op->obj,&sq) ;

#if	CF_DEBUGS
	debugprintf("biblepara_ispara: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (biblepara_ispara) */


int biblepara_curbegin(BIBLEPARA *op,BIBLEPARA_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARA_MAGIC) return SR_NOTOPEN ;

	if (op->call.curbegin != NULL) {
	    void	*p ;
	    if ((rs = uc_malloc(op->cursize,&p)) >= 0) {
		curp->scp = p ;
	        if ((rs = (*op->call.curbegin)(op->obj,curp->scp)) >= 0) {
		    curp->magic = BIBLEPARA_MAGIC ;
		}
		if (rs < 0) {
		    uc_free(curp->scp) ;
		    curp->scp = NULL ;
		}
	    } /* end if (m-a) */
	    if (rs < 0)
		memset(curp,0,sizeof(BIBLEPARA_CUR)) ;
	} /* end if (curbegin) */

	return rs ;
}
/* end subroutine (biblepara_curbegin) */


int biblepara_curend(BIBLEPARA *op,BIBLEPARA_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARA_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != BIBLEPARA_MAGIC) return SR_NOTOPEN ;

	if (curp->scp == NULL) return SR_NOTSOCK ;

	if (op->call.curend != NULL) {
	    rs1 = (*op->call.curend)(op->obj,curp->scp) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = uc_free(curp->scp) ;
	if (rs >= 0) rs = rs1 ;
	curp->scp = NULL ;

	curp->magic = 0 ;
	return rs ;
}
/* end subroutine (biblepara_curend) */


/* enumerate entries */
int biblepara_enum(BIBLEPARA *op,BIBLEPARA_CUR *curp,BIBLEPARA_CITE *qp)
{
	BIBLEPARAS_CITE	sq ;
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARA_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != BIBLEPARA_MAGIC) return SR_NOTOPEN ;

	if (op->call.enumerate != NULL) {
	    if ((rs = (*op->call.enumerate)(op->obj,curp->scp,&sq)) >= 0) {
	        qp->b = sq.b ;
	        qp->c = sq.c ;
	        qp->v = sq.v ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("biblepara_enum: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (biblepara_enum) */


int biblepara_info(BIBLEPARA *op,BIBLEPARA_INFO *ip)
{
	BIBLEPARAS_INFO	bi ;
	int		rs = SR_NOSYS ;
	int		nverses = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEPARA_MAGIC) return SR_NOTOPEN ;

	if (ip != NULL)
	    memset(ip,0,sizeof(BIBLEPARAS_INFO)) ;

	if (op->call.info != NULL)
	    rs = (*op->call.info)(op->obj,&bi) ;

	nverses = bi.nverses ;
	if (ip != NULL) {
	    memset(ip,0,sizeof(BIBLEPARA_INFO)) ;
	    ip->dbtime = bi.dbtime ;
	    ip->vitime = bi.vitime ;
	    ip->maxbook = bi.maxbook ;
	    ip->maxchapter = bi.maxchapter ;
	    ip->nverses = bi.nverses ;
	    ip->nzverses = bi.nzverses ;
	}

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (biblepara_info) */


/* private subroutines */


/* find and load the DB-access object */
static int biblepara_objloadbegin(BIBLEPARA *op,cchar *pr,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	int		rs ;
	int		rs1 ;
	int		opts ;

#if	CF_DEBUGS
	debugprintf("biblepara_objloadbegin: pr=%s\n",pr) ;
	debugprintf("biblepara_objloadbegin: objname=%s\n",objname) ;
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
	            const char	*modbname = BIBLEPARA_MODBNAME ;
	            opts = (MODLOAD_OLIBVAR | MODLOAD_OSDIRS) ;
	            rs = modload_open(lp,pr,modbname,objname,opts,sv) ;
		    f_modload = (rs >= 0) ;
		}
	    }

	    rs1 = vecstr_finish(&syms) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && f_modload)
		modload_close(lp) ;
	} /* end if (allocation) */

#if	CF_DEBUGS
	debugprintf("biblepara_objloadbegin: modload_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    int		mv[2] ;
	    if ((rs = modload_getmva(lp,mv,2)) >= 0) {
		void	*p ;
		op->objsize = mv[0] ;
		op->cursize = mv[1] ;
		if ((rs = uc_malloc(op->objsize,&p)) >= 0) {
		    op->obj = p ;
		    rs = biblepara_loadcalls(op,objname) ;
		    if (rs < 0) {
			uc_free(op->obj) ;
			op->obj = NULL ;
		    }
		} /* end if (memory-allocation) */
	    } /* end if (getmva) */
	    if (rs < 0)
		modload_close(lp) ;
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (biblepara_objloadbegin) */


static int biblepara_objloadend(BIBLEPARA *op)
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
/* end subroutine (biblepara_objloadend) */


static int biblepara_loadcalls(BIBLEPARA *op,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	const int	nlen = SYMNAMELEN ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	char		nbuf[SYMNAMELEN + 1] ;
	const void	*snp ;

	for (i = 0 ; subs[i] != NULL ; i += 1) {

	    if ((rs = sncpy3(nbuf,nlen,objname,"_",subs[i])) >= 0) {
	         if ((rs = modload_getsym(lp,nbuf,&snp)) == SR_NOTFOUND) {
		     snp = NULL ;
		     if (! isrequired(i)) rs = SR_OK ;
		}
	    }

	    if (rs < 0) break ;

#if	CF_DEBUGS
	    debugprintf("biblepara_loadcalls: call=%s %c\n",
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

		case sub_ispara:
		    op->call.ispara = (int (*)(void *,BVS_Q *)) snp ;
		    break ;

		case sub_curbegin:
		    op->call.curbegin = 
			(int (*)(void *,BVS_C *)) snp ;
		    break ;

		case sub_enum:
		    op->call.enumerate = 
			(int (*)(void *,BVS_C *,BVS_Q *)) snp ;
		    break ;

		case sub_curend:
		    op->call.curend= 
			(int (*)(void *,BVS_C *)) snp ;
		    break ;

		case sub_audit:
		    op->call.audit = (int (*)(void *)) snp ;
		    break ;

		case sub_info:
		    op->call.info = (int (*)(void *,BVS_I *)) snp ;
		    break ;

		case sub_close:
		    op->call.close = (int (*)(void *)) snp ;
		    break ;

		} /* end switch */

	    } /* end if (it had the call) */

	} /* end for (subs) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (biblepara_loadcalls) */


static int isrequired(int i)
{
	int	f = FALSE ;

	switch (i) {
	case sub_open:
	case sub_ispara:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */

	return f ;
}
/* end subroutine (isrequired) */


