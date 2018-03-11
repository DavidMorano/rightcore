/* bibleq */

/* BIBLEQ object-load management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_LOOKOTHER	0		/* look elsewhere */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that provides
	access to the BIBLEQ object (which is dynamically loaded).


*******************************************************************************/


#define	BIBLEQ_MASTER		1


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

#include	"bibleq.h"
#include	"bibleqs.h"


/* local defines */

#define	BIBLEQ_MODBNAME	"bibleqs"
#define	BIBLEQ_OBJNAME	"bibleqs"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#undef	TIS_CUR
#define	TIS_CUR		BIBLEQS_CUR
#undef	TIS_CITE
#define	TIS_CITE	BIBLEQS_CITE


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,const char *,
			const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	bibleq_objloadbegin(BIBLEQ *,const char *,const char *) ;
static int	bibleq_objloadend(BIBLEQ *) ;
static int	bibleq_loadcalls(BIBLEQ *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static const char	*subs[] = {
	"open",
	"count",
	"curbegin",
	"lookup",
	"read",
	"curend",
	"audit",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_count,
	sub_curbegin,
	sub_lookup,
	sub_read,
	sub_curend,
	sub_audit,
	sub_close,
	sub_overlast
} ;


/* exported subroutines */


int bibleq_open(BIBLEQ *op,cchar *pr,cchar *dbname)
{
	int		rs ;
	const char	*objname = BIBLEQ_OBJNAME ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

#ifdef	COMMENT /* we let both NULL and NIL go down */
	if (dbname[0] == '\0')
	    return SR_INVALID ;
#endif

	memset(op,0,sizeof(BIBLEQ)) ;

	if ((rs = bibleq_objloadbegin(op,pr,objname)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,pr,dbname)) >= 0) {
		op->magic = BIBLEQ_MAGIC ;
	    }
	    if (rs < 0)
		bibleq_objloadend(op) ;
	} /* end if (objload-begin) */

#if	CF_DEBUGS
	debugprintf("bibleq_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bibleq_open) */


int bibleq_close(BIBLEQ *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEQ_MAGIC) return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = bibleq_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (bibleq_close) */


int bibleq_audit(BIBLEQ *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEQ_MAGIC) return SR_NOTOPEN ;

	if (op->call.audit != NULL) {
	    rs = (*op->call.audit)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (bibleq_audit) */


int bibleq_count(BIBLEQ *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEQ_MAGIC) return SR_NOTOPEN ;

	if (op->call.count != NULL) {
	    rs = (*op->call.count)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (bibleq_count) */


int bibleq_curbegin(BIBLEQ *op,BIBLEQ_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != BIBLEQ_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(BIBLEQ_CUR)) ;

	if (op->call.curbegin != NULL) {
	    void	*p ;
	    if ((rs = uc_malloc(op->cursize,&p)) >= 0) {
		curp->scp = p ;
		if ((rs = (*op->call.curbegin)(op->obj,curp->scp)) >= 0) {
	    	    curp->magic = BIBLEQ_MAGIC ;
		}
		if (rs < 0) {
	    	    uc_free(curp->scp) ;
	    	    curp->scp = NULL ;
		}
	    } /* end if (memory-allocation) */
	} else
	    rs = SR_NOTSUP ;

	return rs ;
}
/* end subroutine (bibleq_curbegin) */


int bibleq_curend(BIBLEQ *op,BIBLEQ_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != BIBLEQ_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != BIBLEQ_MAGIC) return SR_NOTOPEN ;

	if (curp->scp != NULL) {
	    if (op->call.curend != NULL) {
	        rs1 = (*op->call.curend)(op->obj,curp->scp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = uc_free(curp->scp) ;
	    if (rs >= 0) rs = rs1 ;
	    curp->scp = NULL ;
	} else
	    rs = SR_NOTSUP ;

	curp->magic = 0 ;
	return rs ;
}
/* end subroutine (bibleq_curend) */


/* lookup tags by strings */
int bibleq_lookup(BIBLEQ *op,BIBLEQ_CUR *curp,int qo,cchar **klp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (klp == NULL) return SR_FAULT ;

	if (op->magic != BIBLEQ_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != BIBLEQ_MAGIC) return SR_NOTOPEN ;

	rs = (*op->call.lookup)(op->obj,curp->scp,qo,klp) ;

	return rs ;
}
/* end subroutine (bibleq_lookup) */


/* enumerate entries */
int bibleq_read(BIBLEQ *op,BIBLEQ_CUR *curp,BIBLEQ_CITE *bcp,
		char *vbuf,int vlen)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (bcp == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;

	if (op->magic != BIBLEQ_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != BIBLEQ_MAGIC) return SR_NOTOPEN ;

	if (op->call.enumerate != NULL) {
	    rs = (*op->call.enumerate)(op->obj,curp->scp,bcp,vbuf,vlen) ;
	}

	return rs ;
}
/* end subroutine (bibleq_read) */


/* private subroutines */


/* find and load the DB-access object */
static int bibleq_objloadbegin(BIBLEQ *op,cchar *pr,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	const int	vo = VECSTR_OCOMPACT ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("bibleq_objloadbegin: pr=%s\n",pr) ;
	debugprintf("bibleq_objloadbegin: objname=%s\n",objname) ;
#endif

	if ((rs = vecstr_start(&syms,n,vo)) >= 0) {
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
		cchar	**sv ;
	        if ((rs = vecstr_getvec(&syms,&sv)) >= 0) {
#if	CF_LOOKOTHER
	            const int	mo = (MODLOAD_OLIBVAR | MODLOAD_OSDIRS) ;
#else
	            const int	mo = 0 ;
#endif
	            cchar	*modbname = BIBLEQ_MODBNAME ;
	            rs = modload_open(lp,pr,modbname,objname,mo,sv) ;
		    f_modload = (rs >= 0)  ;
		}
	    }

	    rs1 = vecstr_finish(&syms) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && f_modload) {
		modload_close(lp) ;
	    }
	} /* end if (allocation) */

#if	CF_DEBUGS
	debugprintf("bibleq_objloadbegin: modload_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    int		mv[2] ;
	    if ((rs = modload_getmva(lp,mv,2)) >= 0) {
		void	*p ;
		op->objsize = mv[0] ;
		op->cursize = mv[1] ;
		if ((rs = uc_malloc(op->objsize,&p)) >= 0) {
		    op->obj = p ;
		    rs = bibleq_loadcalls(op,objname) ;
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
/* end subroutine (bibleq_objloadbegin) */


static int bibleq_objloadend(BIBLEQ *op)
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
/* end subroutine (bibleq_objloadend) */


static int bibleq_loadcalls(BIBLEQ *op,cchar *objname)
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
	    debugprintf("bibleq_loadcalls: call=%s %c\n",
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
		case sub_curbegin:
		    op->call.curbegin = 
			(int (*)(void *,void *)) snp ;
		    break ;
		case sub_lookup:
		    op->call.lookup = 
			(int (*)(void *,void *,int,const char **)) snp ;
		    break ;
		case sub_read:
		    op->call.enumerate = 
			(int (*)(void *,void *,TIS_CITE *,char *,int)) snp ;
		    break ;
		case sub_curend:
		    op->call.curend = 
			(int (*)(void *,void *)) snp ;
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

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bibleq_loadcalls) */


static int isrequired(int i)
{
	int		f = FALSE ;
	switch (i) {
	case sub_open:
	case sub_curbegin:
	case sub_lookup:
	case sub_read:
	case sub_curend:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isrequired) */


