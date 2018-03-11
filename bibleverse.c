/* bibleverse */

/* BIBLEVERSE object-load management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that allows access
	to the BIBLEVERSE datbase.


*******************************************************************************/


#define	BIBLEVERSE_MASTER	1


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

#include	"bibleverse.h"
#include	"bibleverses.h"


/* local defines */

#define	BIBLEVERSE_DEFENTS	(44 * 1000)

#define	BIBLEVERSE_MODBNAME	"bibleverses"
#define	BIBLEVERSE_OBJNAME	"bibleverses"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#define	BVS_Q		BIBLEVERSES_CITE
#define	BVS_C		BIBLEVERSES_CUR
#define	BVS_I		BIBLEVERSES_INFO


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	bibleverse_objloadbegin(BIBLEVERSE *,cchar *,cchar *) ;
static int	bibleverse_objloadbeginer(BIBLEVERSE *op,cchar *,cchar *) ;
static int	bibleverse_objloadend(BIBLEVERSE *) ;
static int	bibleverse_loadcalls(BIBLEVERSE *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static const char	*subs[] = {
	"open",
	"count",
	"read",
	"get",
	"curbegin",
	"enum",
	"curend",
	"audit",
	"info",
	"chapters",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_count,
	sub_read,
	sub_get,
	sub_curbegin,
	sub_enum,
	sub_curend,
	sub_audit,
	sub_info,
	sub_chapters,
	sub_close,
	sub_overlast
} ;


/* exported subroutines */


int bibleverse_open(BIBLEVERSE *op,cchar pr[],cchar dbname[])
{
	int		rs ;
	const char	*objname = BIBLEVERSE_OBJNAME ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(BIBLEVERSE)) ;

	if ((rs = bibleverse_objloadbegin(op,pr,objname)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,pr,dbname)) >= 0) {
		op->magic = BIBLEVERSE_MAGIC ;
	    }
	    if (rs < 0)
		bibleverse_objloadend(op) ;
	} /* end if (objload-begin) */

#if	CF_DEBUGS
	debugprintf("bibleverse_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleverse_open) */


/* free up the entire vector string data structure object */
int bibleverse_close(BIBLEVERSE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("bibleverse_close: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSE_MAGIC) return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("bibleverse_close: OBJ_close() rs=%d\n",rs) ;
#endif

	rs1 = bibleverse_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("bibleverse_close: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (bibleverse_close) */


int bibleverse_count(BIBLEVERSE *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSE_MAGIC) return SR_NOTOPEN ;

	if (op->call.count != NULL) {
	    rs = (*op->call.count)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (bibleverse_count) */


int bibleverse_audit(BIBLEVERSE *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSE_MAGIC) return SR_NOTOPEN ;

	if (op->call.audit != NULL) {
	    rs = (*op->call.audit)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (bibleverse_audit) */


int bibleverse_read(BIBLEVERSE *op,char *vbuf,int vlen,BIBLEVERSE_Q *qp)
{
	BIBLEVERSES_Q	sq ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("bibleverse_read: ent q=%u:%u:%u\n",
		qp->b,qp->c,qp->v) ;
#endif

	sq.b = qp->b ;
	sq.c = qp->c ;
	sq.v = qp->v ;
	rs = (*op->call.read)(op->obj,vbuf,vlen,&sq) ;

#if	CF_DEBUGS
	debugprintf("bibleverse_read: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleverse_read) */


int bibleverse_get(BIBLEVERSE *op,BIBLEVERSE_Q *qp,char *vbuf,int vlen)
{
	return bibleverse_read(op,vbuf,vlen,qp) ;
}
/* end subroutine (bibleverse_get) */


int bibleverse_curbegin(BIBLEVERSE *op,BIBLEVERSE_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSE_MAGIC) return SR_NOTOPEN ;

	if (op->call.curbegin != NULL) {
	    void	*p ;
	    if ((rs = uc_malloc(op->cursize,&p)) >= 0) {
		curp->scp = p ;
		if ((rs = (*op->call.curbegin)(op->obj,curp->scp)) >= 0) {
		     curp->magic = BIBLEVERSE_MAGIC ;
		}
	        if (rs < 0) {
		    uc_free(curp->scp) ;
		    curp->scp = NULL ;
	        }
	    }
	} else
	    rs = SR_NOTSOCK ;

	if (rs < 0)
	    memset(curp,0,sizeof(BIBLEVERSE_CUR)) ;

	return rs ;
}
/* end subroutine (bibleverse_curbegin) */


int bibleverse_curend(BIBLEVERSE *op,BIBLEVERSE_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSE_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != BIBLEVERSE_MAGIC) return SR_NOTOPEN ;

	if (curp->scp != NULL) {
	    if (op->call.curend != NULL) {
	        rs1 = (*op->call.curend)(op->obj,curp->scp) ;
		if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = uc_free(curp->scp) ;
	    if (rs >= 0) rs = rs1 ;
	    curp->scp = NULL ;
	} else
	    return SR_NOTSUP ;

	curp->magic = 0 ;
	return rs ;
}
/* end subroutine (bibleverse_curend) */


/* enumerate entries */
int bibleverse_enum(op,curp,qp,vbuf,vlen)
BIBLEVERSE	*op ;
BIBLEVERSE_CUR	*curp ;
BIBLEVERSE_CITE	*qp ;
char		vbuf[] ;
int		vlen ;
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSE_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != BIBLEVERSE_MAGIC) return SR_NOTOPEN ;

	if (op->call.enumerate != NULL) {
	    BIBLEVERSES_Q	sq ;
	    rs = (*op->call.enumerate)(op->obj,curp->scp,&sq,vbuf,vlen) ;
	    if (rs >= 0) {
	        qp->b = sq.b ;
	        qp->c = sq.c ;
	        qp->v = sq.v ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("bibleverse_enum: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleverse_enum) */


int bibleverse_info(BIBLEVERSE *op,BIBLEVERSE_INFO *ip)
{
	BIBLEVERSES_I	bi ;
	int		rs = SR_NOSYS ;
	int		nverses = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("bibleverse_info: ent\n") ;
#endif

	if (ip != NULL) {
	    memset(ip,0,sizeof(BIBLEVERSES_I)) ;
	}

	if (op->call.info != NULL) {
	    if ((rs = (*op->call.info)(op->obj,&bi)) >= 0) {
	        nverses = bi.nverses ;
	        if (ip != NULL) {
	            memset(ip,0,sizeof(BIBLEVERSE_INFO)) ;
	            ip->dbtime = bi.dbtime ;
	            ip->vitime = bi.vitime ;
	            ip->maxbook = bi.maxbook ;
	            ip->maxchapter = bi.maxchapter ;
	            ip->nverses = bi.nverses ;
	            ip->nzverses = bi.nzverses ;
		}
	    }
	}

#if	CF_DEBUGS
	debugprintf("bibleverse_info: ret rs=%d nv=%u\n",rs,nverses) ;
#endif

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bibleverse_info) */


int bibleverse_chapters(BIBLEVERSE *op,int book,uchar *ap,int al)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEVERSE_MAGIC) return SR_NOTOPEN ;

	if (op->call.chapters != NULL) {
	    rs = (*op->call.chapters)(op->obj,book,ap,al) ;
	}

	return rs ;
}
/* end subroutine (bibleverse_chapters) */


/* private subroutines */


/* find and load the DB-access object */
static int bibleverse_objloadbegin(BIBLEVERSE *op,cchar *pr,cchar *objname)
{
	int		rs ;

	if ((rs = bibleverse_objloadbeginer(op,pr,objname)) >= 0) {
	    MODLOAD	*lp = &op->loader ;
	    int		mv[2] ;
	    if ((rs = modload_getmva(lp,mv,2)) >= 0) {
		void	*p ;
		op->objsize = mv[0] ;
		op->cursize = mv[1] ;
		if ((rs = uc_malloc(op->objsize,&p)) >= 0) {
		    op->obj = p ;
		    rs = bibleverse_loadcalls(op,objname) ;
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
/* end subroutine (bibleverse_objloadbegin) */


static int bibleverse_objloadbeginer(BIBLEVERSE *op,cchar *pr,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	int		rs ;
	int		rs1 ;
	int		opts ;

#if	CF_DEBUGS
	debugprintf("bibleverse_objloadbegin: pr=%s\n",pr) ;
	debugprintf("bibleverse_objloadbegin: objname=%s\n",objname) ;
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
	            cchar	*modbname = BIBLEVERSE_MODBNAME ;
	            opts = (MODLOAD_OLIBVAR | MODLOAD_OSDIRS) ;
	            rs = modload_open(lp,pr,modbname,objname,opts,sv) ;
		    f_modload = (rs >= 0)  ;
		}
	    }

	    rs1 = vecstr_finish(&syms) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && f_modload) {
		modload_close(lp) ;
	    }
	} /* end if (allocation) */

	return rs ;
}
/* end subroutine (bibleverse_objloadbeginer) */


static int bibleverse_objloadend(BIBLEVERSE *op)
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
/* end subroutine (bibleverse_objloadend) */


static int bibleverse_loadcalls(BIBLEVERSE *op,cchar *objname)
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
	    debugprintf("bibleverse_loadcalls: call=%s %c\n",
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

		case sub_read:
		    op->call.read = (int (*)(void *,char *,int,BVS_Q *)) snp ;
		    break ;

		case sub_get:
		    op->call.get = (int (*)(void *,BVS_Q *,char *,int)) snp ;
		    break ;

		case sub_curbegin:
		    op->call.curbegin = 
			(int (*)(void *,BVS_C *)) snp ;
		    break ;

		case sub_enum:
		    op->call.enumerate = 
			(int (*)(void *,BVS_C *,BVS_Q *,char *,int)) snp ;
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

		case sub_chapters:
		    op->call.chapters = (int (*)(void *,int,uchar *,int)) snp ;
		    break ;

		case sub_close:
		    op->call.close = (int (*)(void *)) snp ;
		    break ;

		} /* end switch */

	    } /* end if (it had the call) */

	} /* end for (subs) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bibleverse_loadcalls) */


static int isrequired(int i)
{
	int		f = FALSE ;
	switch (i) {
	case sub_open:
	case sub_read:
	case sub_get:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isrequired) */


