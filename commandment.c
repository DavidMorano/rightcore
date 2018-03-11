/* commandment */

/* COMMANDMENT object-load management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2008-03-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that allows access
	to the COMMANDMENT datbase.


*******************************************************************************/


#define	COMMANDMENT_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<modload.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"commandment.h"
#include	"commandments.h"


/* local defines */

#define	COMMANDMENT_DEFENTS	67

#define	COMMANDMENT_MODBNAME	"commandments"
#define	COMMANDMENT_OBJNAME	"commandments"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	commandment_objloadbegin(COMMANDMENT *,cchar *,cchar *) ;
static int	commandment_objloadbeginer(COMMANDMENT *,cchar *,cchar *) ;
static int	commandment_objloadend(COMMANDMENT *) ;
static int	commandment_loadcalls(COMMANDMENT *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static cchar	*subs[] = {
	"open",
	"audit",
	"count",
	"max",
	"read",
	"get",
	"curbegin",
	"curend",
	"enum",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_audit,
	sub_count,
	sub_max,
	sub_read,
	sub_get,
	sub_curbegin,
	sub_curend,
	sub_enum,
	sub_close,
	sub_overlast
} ;


/* exported subroutines */


int commandment_open(COMMANDMENT *op,cchar *pr,cchar *dbname)
{
	int		rs ;
	const char	*objname = COMMANDMENT_OBJNAME ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(COMMANDMENT)) ;

	if ((rs = commandment_objloadbegin(op,pr,objname)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,pr,dbname)) >= 0) {
		op->magic = COMMANDMENT_MAGIC ;
	    }
	    if (rs < 0)
		commandment_objloadend(op) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("commandment_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (commandment_open) */


/* free up the entire vector string data structure object */
int commandment_close(COMMANDMENT *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENT_MAGIC) return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = commandment_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (commandment_close) */


int commandment_audit(COMMANDMENT *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENT_MAGIC) return SR_NOTOPEN ;

	if (op->call.audit != NULL) {
	    rs = (*op->call.audit)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (commandment_audit) */


int commandment_count(COMMANDMENT *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENT_MAGIC) return SR_NOTOPEN ;

	if (op->call.count != NULL) {
	    rs = (*op->call.count)(op->obj) ;
	}

#if	CF_DEBUGS
	debugprintf("commandment_count: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (commandment_count) */


int commandment_max(COMMANDMENT *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENT_MAGIC) return SR_NOTOPEN ;

	if (op->call.max != NULL) {
	    rs = (*op->call.max)(op->obj) ;
	}

#if	CF_DEBUGS
	debugprintf("commandment_max: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (commandment_max) */


int commandment_read(COMMANDMENT *op,char *rbuf,int rlen,uint cn)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENT_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("commandment_read: ent cn=%d\n",cn) ;
#endif

	rs = (*op->call.read)(op->obj,rbuf,rlen,cn) ;

#if	CF_DEBUGS
	debugprintf("commandment_read: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (commandment_read) */


int commandment_get(COMMANDMENT *op,int i,char *rbuf,int rlen)
{
	uint		cn = (uint) i ;
	if (i < 0) return SR_INVALID ;
	return commandment_read(op,rbuf,rlen,cn) ;
}
/* end subroutine (commandment_get) */


int commandment_curbegin(COMMANDMENT *op,COMMANDMENT_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENT_MAGIC) return SR_NOTOPEN ;

	if (op->call.curbegin != NULL) {
	    void	*p ;
	    if ((rs = uc_malloc(op->cursize,&p)) >= 0) {
		curp->scp = p ;
		if ((rs = (*op->call.curbegin)(op->obj,curp->scp)) >= 0) {
		     curp->magic = COMMANDMENT_MAGIC ;
		}
	        if (rs < 0) {
		    uc_free(curp->scp) ;
		    curp->scp = NULL ;
	        }
	    }
	} else
	    rs = SR_NOTSOCK ;

	if (rs < 0)
	    memset(curp,0,sizeof(COMMANDMENT_CUR)) ;

	return rs ;
}
/* end subroutine (commandment_curbegin) */


int commandment_curend(COMMANDMENT *op,COMMANDMENT_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENT_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != COMMANDMENT_MAGIC) return SR_NOTOPEN ;

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
/* end subroutine (commandment_curend) */


/* enumerate entries */
int commandment_enum(COMMANDMENT *op,COMMANDMENT_CUR *curp,uint *cnp,
	char *rbuf,int rlen)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != COMMANDMENT_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("commandment_enum: ent\n") ;
#endif

	if (op->call.enumerate != NULL) {
	    COMMANDMENTS_ENT	cse ;
	    rs = (*op->call.enumerate)(op->obj,curp->scp,&cse,rbuf,rlen) ;
	    if (cnp != NULL) *cnp = cse.cn ;
	}

#if	CF_DEBUGS
	debugprintf("commandment_enum: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (commandment_enum) */


#ifdef	COMMENT

/* search for a string */
int commandment_search(op,s,cmpfunc,rpp)
commandment	*op ;
const char	s[] ;
int		(*cmpfunc)() ;
char		**rpp ;
{
	int		rs ;
	int		i ;
	char		**rpp2 ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (cmpfunc == NULL)
	    cmpfunc = defaultcmp ;

	if (op->f.osorted && (! op->f.issorted)) {

	    op->f.issorted = TRUE ;
	    if (op->c > 1)
	        qsort(op->va,(size_t) op->i,
	            sizeof(char *),cmpfunc) ;

	} /* end if (sorting) */

	if (op->f.issorted) {

	    rpp2 = (char **) bsearch(&s,op->va,op->i,
	        sizeof(char *),cmpfunc) ;

	    rs = SR_NOTFOUND ;
	    if (rpp2 != NULL) {

	        i = rpp2 - op->va ;
	        rs = SR_OK ;

	    }

	} else {

	    for (i = 0 ; i < op->i ; i += 1) {

	        rpp2 = op->va + i ;
	        if (*rpp2 == NULL) continue ;

	        if ((*cmpfunc)(&s,rpp2) == 0)
	            break ;

	    } /* end for */

	    rs = (i < op->i) ? SR_OK : SR_NOTFOUND ;

	} /* end if (sorted or not) */

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? op->va[i] : NULL ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (commandment_search) */

#endif /* COMMENT */


/* private subroutines */


static int commandment_objloadbegin(COMMANDMENT *op,cchar *pr,cchar *objname)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("commandment_objloadbegin: ent\n") ;
	debugprintf("commandment_objloadbegin: pr=%s\n",pr) ;
	debugprintf("commandment_objloadbegin: objname=%s\n",objname) ;
#endif

	if ((rs = commandment_objloadbeginer(op,pr,objname)) >= 0) {
	    MODLOAD	*lp = &op->loader ;
	    int		mv[2] ;
	    if ((rs = modload_getmva(lp,mv,2)) >= 0) {
		const int	objsize = rs ;
		void		*p ;
#if	CF_DEBUGS
		debugprintf("commandment_objloadbegin: "
			"modload_getmva() rs=%d\n",rs) ;
#endif

		op->objsize = mv[0] ;
		op->cursize = mv[1] ;

#if	CF_DEBUGS
		debugprintf("commandment_objloadbegin: os=%u cs=%u\n",
			mv[0],mv[1]) ;
#endif

		if ((rs = uc_malloc(objsize,&p)) >= 0) {
		    op->obj = p ;
		    rs = commandment_loadcalls(op,objname) ;
		    if (rs < 0) {
			uc_free(op->obj) ;
			op->obj = NULL ;
		    }
		} /* end if (memory-allocation) */
	    } /* end if (getmva) */
	    if (rs < 0)
		modload_close(lp) ;
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("commandment_objloadbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (commandment_objloadbegin) */


static int commandment_objloadbeginer(COMMANDMENT *op,cchar *pr,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	const int	vo = VECSTR_OCOMPACT ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("commandment_objloadbeginer: ent\n") ;
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
	            const int	mo = (MODLOAD_OLIBVAR | MODLOAD_OSDIRS) ;
	            cchar	*modbname = COMMANDMENT_MODBNAME ;
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
	debugprintf("commandment_objloadbeginer: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (commandment_objloadbeginer) */


static int commandment_objloadend(COMMANDMENT *op)
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
/* end subroutine (commandment_objloadend) */


static int commandment_loadcalls(COMMANDMENT *op,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	const int	nlen = SYMNAMELEN ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	char		nbuf[SYMNAMELEN + 1] ;
	const void	*snp = NULL ;

#if	CF_DEBUGS
	debugprintf("commandment_loadcalls: ent objname=%s\n",objname) ;
#endif

	for (i = 0 ; subs[i] != NULL ; i += 1) {

	    if ((rs = sncpy3(nbuf,nlen,objname,"_",subs[i])) >= 0) {
	         if ((rs = modload_getsym(lp,nbuf,&snp)) == SR_NOTFOUND) {
		     snp = NULL ;
		     if (! isrequired(i)) rs = SR_OK ;
		}
	    }

	    if (rs < 0) break ;

#if	CF_DEBUGS
	    debugprintf("commandment_loadcalls: call=%s %c\n",
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
		case sub_audit:
		    op->call.audit = (int (*)(void *)) snp ;
		    break ;
		case sub_count:
		    op->call.count = (int (*)(void *)) snp ;
		    break ;
		case sub_max:
		    op->call.max = (int (*)(void *)) snp ;
		    break ;
		case sub_read:
		    op->call.read = (int (*)(void *,char *,int,uint)) snp ;
		    break ;
		case sub_get:
		    op->call.get = (int (*)(void *,int,char *,int)) snp ;
		    break ;
		case sub_curbegin:
		    op->call.curbegin = (int (*)(void *,void *)) snp ;
		    break ;
		case sub_curend:
		    op->call.curend = (int (*)(void *,void *)) snp ;
		    break ;
		case sub_enum:
		    op->call.enumerate = 
			(int (*)(void *,void *,void *,char *,int)) snp ;
		    break ;
		case sub_close:
		    op->call.close = (int (*)(void *)) snp ;
		    break ;
		} /* end switch */
	    } /* end if (it had the call) */

	} /* end for (subs) */

#if	CF_DEBUGS
	debugprintf("commandment_loadcalls: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (commandment_loadcalls) */


static int isrequired(int i)
{
	int		f = FALSE ;
	switch (i) {
	case sub_open:
	case sub_audit:
	case sub_count:
	case sub_read:
	case sub_curbegin:
	case sub_curend:
	case sub_enum:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isrequired) */


