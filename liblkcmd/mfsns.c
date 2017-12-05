/* mfs-ns */

/* MFSNS object-load management */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_LOOKOTHER	0		/* look elsewhere */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 2008,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that provides
	access to the MFSNS object (which is dynamically loaded).


*******************************************************************************/


#define	MFSNS_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"mfsns.h"


/* local defines */

#define	MFSNS_MODBNAME	"mfsnso"
#define	MFSNS_OBJNAME	"mfsnso"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif


/* external subroutines */


/* local structures */


/* forward references */

static int	mfsns_objloadbegin(MFSNS *,const char *,const char *) ;
static int	mfsns_objloadend(MFSNS *) ;
static int	mfsns_loadcalls(MFSNS *,const char *) ;

static int	isrequired(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* global variables */


/* local variables */

static const char	*subs[] = {
	"open",
	"setopts",
	"get",
	"curbegin",
	"enum",
	"curend",
	"audit",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_setopts,
	sub_get,
	sub_curbegin,
	sub_enum,
	sub_curend,
	sub_audit,
	sub_close,
	sub_overlast
} ;


/* exported subroutines */


int mfsns_open(MFSNS *op,cchar *pr)
{
	int		rs ;
	const char	*objname = MFSNS_OBJNAME ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(MFSNS)) ;

	if ((rs = mfsns_objloadbegin(op,pr,objname)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,pr)) >= 0) {
		op->magic = MFSNS_MAGIC ;
	    }
	    if (rs < 0)
		mfsns_objloadend(op) ;
	} /* end if (objload-begin) */

#if	CF_DEBUGS
	debugprintf("mfsns_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mfsns_open) */


int mfsns_close( MFSNS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MFSNS_MAGIC) return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = mfsns_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (mfsns_close) */


int mfsns_setopts(MFSNS *op,int opts)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MFSNS_MAGIC) return SR_NOTOPEN ;

	if (op->call.setopts != NULL) {
	    rs = (*op->call.setopts)(op->obj,opts) ;
	}

	return rs ;
}
/* end subroutine (mfsns_setopts) */


int mfsns_audit(MFSNS *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MFSNS_MAGIC) return SR_NOTOPEN ;

	if (op->call.audit != NULL) {
	    rs = (*op->call.audit)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (mfsns_audit) */


int mfsns_get(MFSNS *op,char *rbuf,int rlen,cchar *un,int w)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MFSNS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("mfsns_get ent un=%s w=%u\n",un,w) ;
#endif

	if (op->call.get != NULL) {
	    rs = (*op->call.get)(op->obj,rbuf,rlen,un,w) ;
	}

	return rs ;
}
/* end subroutine (mfsns_count) */


int mfsns_curbegin(MFSNS *op,MFSNS_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != MFSNS_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(MFSNS_CUR)) ;

	if (op->call.curbegin != NULL) {
	    void	*p ;
	    if ((rs = uc_malloc(op->cursize,&p)) >= 0) {
		curp->scp = p ;
		if ((rs = (*op->call.curbegin)(op->obj,curp->scp)) >= 0) {
	    	    curp->magic = MFSNS_MAGIC ;
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
/* end subroutine (mfsns_curbegin) */


int mfsns_curend(MFSNS *op,MFSNS_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != MFSNS_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != MFSNS_MAGIC) return SR_NOTOPEN ;

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
/* end subroutine (mfsns_curend) */


/* enumerate entries */
int mfsns_enum(MFSNS *op,MFSNS_CUR *curp,char *vbuf,int vlen,int w)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;

	if (op->magic != MFSNS_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != MFSNS_MAGIC) return SR_NOTOPEN ;

	if (op->call.enumerate != NULL) {
	    rs = (*op->call.enumerate)(op->obj,curp->scp,vbuf,vlen,w) ;
	}

	return rs ;
}
/* end subroutine (mfsns_enumerate) */


/* private subroutines */


/* find and load the DB-access object */
static int mfsns_objloadbegin(MFSNS *op,cchar *pr,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	const int	vo = VECSTR_OCOMPACT ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("mfsns_objloadbegin: pr=%s\n",pr) ;
	debugprintf("mfsns_objloadbegin: objname=%s\n",objname) ;
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
		const char	**sv ;
	        if ((rs = vecstr_getvec(&syms,&sv)) >= 0) {
	            const char	*modbname = MFSNS_MODBNAME ;
#if	CF_LOOKOTHER
	            const int	mo = (MODLOAD_OLIBVAR | MODLOAD_OSDIRS) ;
#else
	            const int	mo = 0 ;
#endif
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
	debugprintf("mfsns_objloadbegin: modload_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    int		mv[2] ;
	    if ((rs = modload_getmva(lp,mv,2)) >= 0) {
		void	*p ;
		op->objsize = mv[0] ;
		op->cursize = mv[1] ;
		if ((rs = uc_malloc(op->objsize,&p)) >= 0) {
		    op->obj = p ;
		    rs = mfsns_loadcalls(op,objname) ;
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
/* end subroutine (mfsns_objloadbegin) */


static int mfsns_objloadend(MFSNS *op)
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
/* end subroutine (mfsns_objloadend) */


static int mfsns_loadcalls(MFSNS *op,cchar *objname)
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
	    debugprintf("mfsns_loadcalls: call=%s %c\n",
		subs[i],
		((snp != NULL) ? 'Y' : 'N')) ;
#endif

	    if (snp != NULL) {

	        c += 1 ;
		switch (i) {

		case sub_open:
		    op->call.open = 
			(int (*)(void *,const char *)) snp ;
		    break ;

		case sub_setopts:
		    op->call.setopts =
			(int (*)(void *,int)) snp ;
		    break ;

		case sub_get:
		    op->call.get =
			(int (*)(void *,char *,int,cchar *,int)) snp ;
		    break ;

		case sub_curbegin:
		    op->call.curbegin = 
			(int (*)(void *,void *)) snp ;
		    break ;

		case sub_enum:
		    op->call.enumerate = 
			(int (*)(void *,void *,char *,int,int)) snp ;
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
/* end subroutine (mfsns_loadcalls) */


static int isrequired(int i)
{
	int		f = FALSE ;
	switch (i) {
	case sub_open:
	case sub_setopts:
	case sub_get:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isrequired) */


