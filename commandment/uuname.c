/* uuname */

/* vector string operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module implements an interface (a trivial one) that provides access
        to the UUNAME object (which is dynamically loaded).


*******************************************************************************/


#define	UUNAME_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<modload.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"uuname.h"
#include	"uunames.h"


/* local defines */

#define	UUNAME_MODBNAME	"uunames"
#define	UUNAME_OBJNAME	"uunames"

#define	LIBCNAME	"lib"

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
extern int	mksofname(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	uuname_objloadbegin(UUNAME *,cchar *,cchar *) ;
static int	uuname_objloadbeginer(UUNAME *,cchar *,cchar *) ;
static int	uuname_objloadend(UUNAME *) ;
static int	uuname_loadcalls(UUNAME *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static cchar	*subs[] = {
	"open",
	"count",
	"exists",
	"curbegin",
	"enum",
	"curend",
	"audit",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_count,
	sub_exists,
	sub_curbegin,
	sub_enum,
	sub_curend,
	sub_audit,
	sub_close,
	sub_overlast
} ;


/* exported subroutines */


int uuname_open(UUNAME *op,cchar *pr,cchar *dbname)
{
	const char	*objname = UUNAME_OBJNAME ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

#ifdef	COMMENT
	if (dbname[0] == '\0') return SR_INVALID ;
#endif

	memset(op,0,sizeof(UUNAME)) ;

	if ((rs = uuname_objloadbegin(op,pr,objname)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,pr,dbname)) >= 0) {
	        op->magic = UUNAME_MAGIC ;
	    }
	    if (rs < 0)
		uuname_objloadend(op) ;
	} /* end if )uuname_objloadbegin) */

#if	CF_DEBUGS
	debugprintf("uuname_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uuname_open) */


/* free up the entire vector string data structure object */
int uuname_close(UUNAME *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UUNAME_MAGIC) return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = uuname_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (uuname_close) */


int uuname_audit(UUNAME *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UUNAME_MAGIC) return SR_NOTOPEN ;

	if (op->call.audit != NULL) {
	    rs = (*op->call.audit)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (uuname_audit) */


int uuname_count(UUNAME *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UUNAME_MAGIC) return SR_NOTOPEN ;

	if (op->call.count != NULL) {
	    rs = (*op->call.count)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (uuname_count) */


int uuname_curbegin(UUNAME *op,UUNAME_CUR *curp)
{
	int		rs = SR_NOSYS ;
	void		*p ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != UUNAME_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(UUNAME_CUR)) ;

	if (op->call.curbegin != NULL) {
	    if ((rs = uc_malloc(op->cursize,&p)) >= 0) {
	        curp->scp = p ;
	        if ((rs = (*op->call.curbegin)(op->obj,curp->scp)) >= 0) {
	            curp->magic = UUNAME_MAGIC ;
	        }
	        if (rs < 0) {
		    uc_free(curp->scp) ;
		    curp->scp = NULL ;
	        }
	    } /* end if (m-a) */
	} /* end if (have SO method) */

	return rs ;
}
/* end subroutine (uuname_curbegin) */


int uuname_curend(UUNAME *op,UUNAME_CUR *curp)
{
	int		rs = SR_NOSYS ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != UUNAME_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != UUNAME_MAGIC) return SR_NOTOPEN ;

	if (curp->scp == NULL) return SR_BADFMT ;

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
/* end subroutine (uuname_curend) */


/* check for existence */
int uuname_exists(UUNAME *op,cchar *sp,int sl)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != UUNAME_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("uuname_exists: call.exists()\n") ;
#endif

	rs = (*op->call.exists)(op->obj,sp,sl) ;

#if	CF_DEBUGS
	debugprintf("uuname_exists: call.exists() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uuname_exists) */


/* enumerate entries */
int uuname_enum(UUNAME *op,UUNAME_CUR *curp,char *rbuf,int rlen)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != UUNAME_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != UUNAME_MAGIC) return SR_NOTOPEN ;

	if (op->call.enumerate != NULL) {
	    rs = (*op->call.enumerate)(op->obj,curp->scp,rbuf,rlen) ;
	}

	return rs ;
}
/* end subroutine (uuname_enum) */


/* private subroutines */


static int uuname_objloadbegin(UUNAME *op,cchar *pr,cchar *objname)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("uuname_objloadbegin: pr=%s\n",pr) ;
	debugprintf("uuname_objloadbegin: objname=%s\n",objname) ;
#endif

	if ((rs = uuname_objloadbeginer(op,pr,objname)) >= 0) {
	    MODLOAD	*lp = &op->loader ;
	    if ((rs = modload_getmv(lp,0)) >= 0) {
		int	objsize = rs ;
		void	*p ;
		if ((rs = uc_malloc(objsize,&p)) >= 0) {
		    op->obj = p ;
		    rs = uuname_loadcalls(op,objname) ;
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
/* end subroutine (uuname_objloadbegin) */


static int uuname_objloadbeginer(UUNAME *op,cchar *pr,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	const int	vo = VECSTR_OCOMPACT ;
	int		rs ;
	int		rs1 ;

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
		const char	**sv ;
	        if ((rs = vecstr_getvec(&syms,&sv)) >= 0) {
	            const char	*modbname = UUNAME_MODBNAME ;
	            const int	mo = (MODLOAD_OLIBVAR | MODLOAD_OSDIRS) ;
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

	return rs ;
}
/* end subroutine (uuname_objloadbeginer) */


static int uuname_objloadend(UUNAME *op)
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
/* end subroutine (uuname_objloadend) */


static int uuname_loadcalls(UUNAME *op,cchar *objname)
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
	    debugprintf("uuname_loadcalls: call=%s %c\n",
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
		case sub_exists:
		    op->call.exists = 
			(int (*)(void *,const char *,int)) snp ;
		    break ;
		case sub_curbegin:
		    op->call.curbegin = 
			(int (*)(void *,void *)) snp ;
		    break ;
		case sub_enum:
		    op->call.enumerate = 
			(int (*)(void *,void *,char *,int)) snp ;
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
/* end subroutine (uuname_loadcalls) */


static int isrequired(int i)
{
	int		f = FALSE ;
	switch (i) {
	case sub_open:
	case sub_exists:
	case sub_curbegin:
	case sub_enum:
	case sub_curend:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isrequired) */


