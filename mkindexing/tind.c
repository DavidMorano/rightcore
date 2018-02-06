/* txtindex */

/* interface to the TXTINDEXES loadable object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that provides
	access to the TXTINDEX object (which is dynamically loaded).


*******************************************************************************/


#define	TXTINDEX_MASTER		1


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

#include	"txtindex.h"
#include	"txtindexes.h"


/* local defines */

#define	TXTINDEX_DEFENTS	(44 * 1000)

#define	TXTINDEX_MODBNAME	"txtindexes"
#define	TXTINDEX_OBJNAME	"txtindexes"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#undef	TIS_CUR
#define	TIS_CUR			TXTINDEXES_CUR
#undef	TIS_TAG
#define	TIS_TAG			TXTINDEXES_TAG


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	pathclean(char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* local structures */


/* forward references */

static int	txtindex_objloadbegin(TXTINDEX *,const char *,const char *) ;
static int	txtindex_objloadend(TXTINDEX *) ;
static int	txtindex_loadcalls(TXTINDEX *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static const char	*subs[] = {
	"open",
	"count",
	"neigen",
	"info",
	"iseigen",
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
	sub_neigen,
	sub_info,
	sub_iseigen,
	sub_curbegin,
	sub_lookup,
	sub_enum,
	sub_curend,
	sub_audit,
	sub_close,
	sub_overlast
} ;


/* exported subroutines */


int txtindex_open(TXTINDEX *op,cchar pr[],cchar dbname[])
{
	int		rs ;
	const char	*objname = TXTINDEX_OBJNAME ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (dbname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("txtindex_open: ent dbname=%s\n",dbname) ;
#endif

	memset(op,0,sizeof(TXTINDEX)) ;

	if ((rs = txtindex_objloadbegin(op,pr,objname)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,dbname)) >= 0) {
		op->magic = TXTINDEX_MAGIC ;
	    }
	    if (rs < 0)
		txtindex_objloadend(op) ;
	} /* end if (txtindex_objloadbegin) */

#if	CF_DEBUGS
	debugprintf("txtindex_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (txtindex_open) */


/* free up the entire vector string data structure object */
int txtindex_close(TXTINDEX *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEX_MAGIC) return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = txtindex_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (txtindex_close) */


int txtindex_audit(TXTINDEX *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEX_MAGIC) return SR_NOTOPEN ;

	if (op->call.audit != NULL) {
	    rs = (*op->call.audit)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (txtindex_audit) */


int txtindex_count(TXTINDEX *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEX_MAGIC) return SR_NOTOPEN ;

	if (op->call.count != NULL) {
	    rs = (*op->call.count)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (txtindex_count) */


int txtindex_neigen(TXTINDEX *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEX_MAGIC) return SR_NOTOPEN ;

	if (op->call.neigen != NULL) {
	    rs = (*op->call.neigen)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (txtindex_neigen) */


int txtindex_info(TXTINDEX *op,TXTINDEX_INFO *ip)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEX_MAGIC) return SR_NOTOPEN ;

	if (op->call.info != NULL) {
	    rs = (*op->call.info)(op->obj,ip) ;
	}

	return rs ;
}
/* end subroutine (txtindex_info) */


int txtindex_curbegin(TXTINDEX *op,TXTINDEX_CUR *curp)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEX_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(TXTINDEX_CUR)) ;

	if (op->call.curbegin != NULL) {
	    void	*p ;
	    if ((rs = uc_malloc(op->cursize,&p)) >= 0) {
		curp->scp = p ;
		if ((rs = (*op->call.curbegin)(op->obj,curp->scp)) >= 0) {
	    	    curp->magic = TXTINDEX_MAGIC ;
		}
	        if (rs < 0) {
	    	    uc_free(curp->scp) ;
	    	    curp->scp = NULL ;
		}
	    } /* end if (m-a) */
	} else
	    rs = SR_NOSYS ;

	return rs ;
}
/* end subroutine (txtindex_curbegin) */


int txtindex_curend(TXTINDEX *op,TXTINDEX_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEX_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != TXTINDEX_MAGIC) return SR_NOTOPEN ;

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
/* end subroutine (txtindex_curend) */


/* lookup tags by strings */
int txtindex_lookup(TXTINDEX *op,TXTINDEX_CUR *curp,cchar **klp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (klp == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEX_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != TXTINDEX_MAGIC) return SR_NOTOPEN ;

	rs = (*op->call.lookup)(op->obj,curp->scp,klp) ;

	return rs ;
}
/* end subroutine (txtindex_lookup) */


/* read entries */
int txtindex_read(TXTINDEX *op,TXTINDEX_CUR *curp,TXTINDEX_TAG *tagp)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEX_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != TXTINDEX_MAGIC) return SR_NOTOPEN ;

	if (op->call.read != NULL) {
	    rs = (*op->call.read)(op->obj,curp->scp,tagp) ;
	}

	return rs ;
}
/* end subroutine (txtindex_read) */


/* private subroutines */


/* find and load the DB-access object */
static int txtindex_objloadbegin(TXTINDEX *op,cchar *pr,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	const int	vo = VECSTR_OCOMPACT ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("txtindex_objloadbegin: pr=%s\n",pr) ;
	debugprintf("txtindex_objloadbegin: objname=%s\n",objname) ;
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
	            const char	*modbname = TXTINDEX_MODBNAME ;
	            int	mo = (MODLOAD_OLIBVAR | MODLOAD_OPRS | MODLOAD_OSDIRS) ;
	            rs = modload_open(lp,pr,modbname,objname,mo,sv) ;
		    f_modload = (rs >= 0) ;
		}
	    } /* end if (ok) */

	    rs1 = vecstr_finish(&syms) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && f_modload)
		modload_close(lp) ;
	} /* end if (vecstr) */

#if	CF_DEBUGS
	debugprintf("txtindex_objloadbegin: modload_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    int	mv[2] ;
	    if ((rs = modload_getmva(lp,mv,2)) >= 0) {
		void	*p ;

#if	CF_DEBUGS
		debugprintf("txtindex_objloadbegin: getmv rs=%d\n",rs) ;
#endif

		op->objsize = mv[0] ;
		op->cursize = mv[1] ;
		if ((rs = uc_malloc(op->objsize,&p)) >= 0) {
		    op->obj = p ;
		    rs = txtindex_loadcalls(op,objname) ;
#if	CF_DEBUGS
		debugprintf("txtindex_objloadbegin: calls rs=%d\n",rs) ;
#endif
		    if (rs < 0) {
			uc_free(op->obj) ;
			op->obj = NULL ;
		    }
		} /* end if (memory-allocation) */
	    } /* end if (modload_getmva) */
	    if (rs < 0)
		modload_close(lp) ;
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("txtindex_objloadbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (txtindex_objloadbegin) */


static int txtindex_objloadend(TXTINDEX *op)
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
/* end subroutine (txtindex_objloadend) */


static int txtindex_loadcalls(TXTINDEX *op,cchar objname[])
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
	    debugprintf("txtindex_loadcalls: call=%s %c\n",
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

		case sub_count:
		    op->call.count = (int (*)(void *)) snp ;
		    break ;

		case sub_neigen:
		    op->call.neigen = (int (*)(void *)) snp ;
		    break ;

		case sub_info:
		    op->call.info = (int (*)(void *,TXTINDEXES_INFO *)) snp ;
		    break ;

		case sub_iseigen:
		    op->call.iseigen = (int (*)(void *,const char *,int)) snp ;
		    break ;

		case sub_curbegin:
		    op->call.curbegin = 
			(int (*)(void *,void *)) snp ;
		    break ;

		case sub_lookup:
		    op->call.lookup = 
			(int (*)(void *,void *,const char **)) snp ;
		    break ;

		case sub_enum:
		    op->call.read = 
			(int (*)(void *,void *,TIS_TAG *)) snp ;
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
/* end subroutine (txtindex_loadcalls) */


static int isrequired(int i)
{
	int		f = FALSE ;

	switch (i) {
	case sub_open:
	case sub_neigen:
	case sub_iseigen:
	case sub_curbegin:
	case sub_lookup:
	case sub_enum:
	case sub_curend:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */

	return f ;
}
/* end subroutine (isrequired) */


