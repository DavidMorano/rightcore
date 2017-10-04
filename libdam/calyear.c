/* calyear */

/* CALYEAR object loader */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_LOOKSELF	0		/* try searching "SELF" for SO */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that allows access
	to the CALYEAR datbase.


*******************************************************************************/


#define	CALYEAR_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<dlfcn.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<modload.h>
#include	<localmisc.h>

#include	"calyear.h"
#include	"calyears.h"


/* local defines */

#define	CALYEAR_DEFENTS		(44 * 1000)
#define	CALYEAR_MODBNAME	"calyears"
#define	CALYEAR_OBJNAME 	"calyears"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN		60
#endif

#define	CLS_Q			CALYEARS_CITE
#define	CLS_C			CALYEARS_CUR


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;


/* local structures */


/* forward references */

static int	calyear_objloadbegin(CALYEAR *,const char *,const char *) ;
static int	calyear_objloadend(CALYEAR *) ;
static int	calyear_loadcalls(CALYEAR *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static const char	*subs[] = {
	"open",
	"count",
	"curbegin",
	"lookcite",
	"read",
	"curend",
	"check",
	"audit",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_count,
	sub_curbegin,
	sub_lookcite,
	sub_read,
	sub_curend,
	sub_check,
	sub_audit,
	sub_close,
	sub_overlast
} ;


/* exported subroutines */


int calyear_open(CALYEAR *op,cchar pr[],cchar *dirnames[],cchar *calnames[])
{
	int		rs ;
	const char	*objname = CALYEAR_OBJNAME ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(CALYEAR)) ;

	if ((rs = calyear_objloadbegin(op,pr,objname)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,pr,dirnames,calnames)) >= 0) {
		op->magic = CALYEAR_MAGIC ;
	    }
	    if (rs < 0)
		calyear_objloadend(op) ;
	} /* end if (objload-begin) */

#if	CF_DEBUGS
	debugprintf("calyear_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (calyear_open) */


/* free up the entire vector string data structure object */
int calyear_close(CALYEAR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CALYEAR_MAGIC) return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = calyear_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (calyear_close) */


int calyear_count(CALYEAR *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CALYEAR_MAGIC) return SR_NOTOPEN ;

	if (op->call.count != NULL) {
	    rs = (*op->call.count)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (calyear_count) */


int calyear_audit(CALYEAR *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CALYEAR_MAGIC) return SR_NOTOPEN ;

	if (op->call.audit != NULL) {
	    rs = (*op->call.audit)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (calyear_audit) */


int calyear_check(CALYEAR *op,time_t dt)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CALYEAR_MAGIC) return SR_NOTOPEN ;

	if (op->call.check != NULL) {
	    rs = (*op->call.check)(op->obj,dt) ;
	}

	return rs ;
}
/* end subroutine (calyear_check) */


int calyear_curbegin(CALYEAR *op,CALYEAR_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != CALYEAR_MAGIC) return SR_NOTOPEN ;

	if (op->call.curbegin != NULL)  {
	    void	*p ;
	    if ((rs = uc_malloc(op->cursize,&p)) >= 0) {
		curp->scp = p ;
	        rs = (*op->call.curbegin)(op->obj,curp->scp) ;
		if (rs < 0) {
		    uc_free(curp->scp) ;
		    curp->scp = NULL ;
		    memset(curp,0,sizeof(CALYEAR_CUR)) ;
		}
	    }
	} else
	    rs = SR_NOSYS ;

	return rs ;
}
/* end subroutine (calyear_curbegin) */


int calyear_curend(CALYEAR *op,CALYEAR_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != CALYEAR_MAGIC) return SR_NOTOPEN ;

	if (curp->scp != NULL)  {
	    if (op->call.curend != NULL) {
	        rs = (*op->call.curend)(op->obj,curp->scp) ;
	    } else
	        rs = SR_NOSYS ;
	    rs1 = uc_free(curp->scp) ;
	    if (rs >= 0) rs = rs1 ;
	    curp->scp = NULL ;
	} else
	    rs = SR_NOTSOCK ;

	return rs ;
}
/* end subroutine (calyear_curend) */


int calyear_lookcite(CALYEAR *op,CALYEAR_CUR *curp,CALCITE *qp)
{
	CALYEARS_CUR	*ocurp ;
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;

	if (op->magic != CALYEAR_MAGIC) return SR_NOTOPEN ;

	if (curp->scp != NULL) {
	    if (op->call.lookcite != NULL) {
	        ocurp = (CALYEARS_CUR *) curp->scp ;
	        rs = (*op->call.lookcite)(op->obj,ocurp,qp) ;
#if	CF_DEBUGS
		debugprintf("calyuear_lookcite: calyears_lookcite() rs=%d\n",
			rs) ;
#endif
	    } else
	        rs = SR_NOSYS ;
	} else
	    rs = SR_NOTSOCK ;

#if	CF_DEBUGS
	debugprintf("calyuear_lookcite: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (calyear_lookcite) */


int calyear_read(CALYEAR *op,CALYEAR_CUR *curp,CALCITE *qp,
		char *rbuf,int rlen)
{
	CALYEARS_CUR	*ocurp ;
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;

	if (op->magic != CALYEAR_MAGIC) return SR_NOTOPEN ;

	if (curp->scp != NULL) {
	    if (op->call.read != NULL) {
	        ocurp = (CALYEARS_CUR *) curp->scp ;
	        rs = (*op->call.read)(op->obj,ocurp,qp,rbuf,rlen) ;
	    } else
	        rs = SR_NOSYS ;
	} else
	    rs = SR_NOTSOCK ;

	return rs ;
}
/* end subroutine (calyear_read) */


/* private subroutines */


/* find and load the DB-access object */
static int calyear_objloadbegin(CALYEAR *op,cchar *pr,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	int		rs ;
	int		rs1 ;
	int		opts ;

#if	CF_DEBUGS
	debugprintf("calyear_objloadbegin: pr=%s\n",pr) ;
	debugprintf("calyear_objloadbegin: objname=%s\n",objname) ;
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
	            const char	*modbname = CALYEAR_MODBNAME ;
	            opts = (MODLOAD_OLIBVAR | MODLOAD_OSDIRS) ;
	            rs = modload_open(lp,pr,modbname,objname,opts,sv) ;
		}
	    }

	    rs1 = vecstr_finish(&syms) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && f_modload)
		modload_close(lp) ;
	} /* end if (allocation) */

#if	CF_DEBUGS
	debugprintf("calyear_objloadbegin: modload_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    int		mv[2] ;
	    if ((rs = modload_getmva(lp,mv,2)) >= 0) {
		void	*p ;
		op->objsize = mv[0] ;
		op->cursize = mv[1] ;
		if ((rs = uc_malloc(op->objsize,&p)) >= 0) {
		    op->obj = p ;
		    rs = calyear_loadcalls(op,objname) ;
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
/* end subroutine (calyear_objloadbegin) */


static int calyear_objloadend(CALYEAR *op)
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
/* end subroutine (calyear_objloadend) */


static int calyear_loadcalls(CALYEAR *op,cchar *objname)
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
	    debugprintf("calyear_loadcalls: call=%s %c\n",
		subs[i],
		((snp != NULL) ? 'Y' : 'N')) ;
#endif

	    if (snp != NULL) {

	        c += 1 ;
		switch (i) {

		case sub_open:
		    op->call.open = (int (*)(void *,const char *,
				const char **,const char **)) snp ;
		    break ;

		case sub_count:
		    op->call.count = (int (*)(void *)) snp ;
		    break ;

		case sub_curbegin:
		    op->call.curbegin = 
			(int (*)(void *,CLS_C *)) snp ;
		    break ;

		case sub_lookcite:
		    op->call.lookcite = (int (*)(void *,CLS_C *,CLS_Q *)) snp ;
		    break ;

		case sub_read:
		    op->call.read = 
			(int (*)(void *,CLS_C *,CLS_Q *,char *,int)) snp ;
		    break ;

		case sub_curend:
		    op->call.curend= 
			(int (*)(void *,CLS_C *)) snp ;
		    break ;

		case sub_check:
		    op->call.check = (int (*)(void *,time_t)) snp ;
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
/* end subroutine (calyear_loadcalls) */


static int isrequired(int i)
{
	int		f = FALSE ;

	switch (i) {
	case sub_open:
	case sub_curbegin:
	case sub_lookcite:
	case sub_read:
	case sub_curend:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */

	return f ;
}
/* end subroutine (isrequired) */


