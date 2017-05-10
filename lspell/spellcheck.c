/* spellcheck */

/* load management and interface for the SPELLCHECKS object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that provides
	access to the SPELLCHECKS object (which is dynamically loaded).


*******************************************************************************/


#define	SPELLCHECK_MASTER	0


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
#include	<localmisc.h>

#include	"spellcheck.h"
#include	"spellchecks.h"
#include	"modload.h"


/* local defines */

#define	SPELLCHECK_MODBNAME	"spellchecks"
#define	SPELLCHECK_OBJNAME	"spellchecks"

#define	SCS_C		SPELLCHECKS_CUR

#define	LIBCNAME	"lib"

#ifndef	VARLIBPATH
#define	VARLIBPATH	"LD_LIBRARY_PATH"
#endif

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
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	spellcheck_objloadbegin(SPELLCHECK *,const char *,
			const char *) ;
static int	spellcheck_objloadend(SPELLCHECK *) ;
static int	spellcheck_loadcalls(SPELLCHECK *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static const char	*subs[] = {
	"start",
	"count",
	"look",
	"looks",
	"curbegin",
	"enum",
	"curend",
	"audit",
	"finish",
	NULL
} ;

enum subs {
	sub_start,
	sub_count,
	sub_look,
	sub_looks,
	sub_curbegin,
	sub_enum,
	sub_curend,
	sub_audit,
	sub_finish,
	sub_overlast
} ;


/* exported subroutines */


int spellcheck_start(op,pr,dbname)
SPELLCHECK	*op ;
const char	pr[] ;
const char	dbname[] ;
{
	int	rs ;

	const char	*objname = SPELLCHECK_OBJNAME ;


	if (op == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("spellcheck_open: pr=%s\n",pr) ;
	debugprintf("spellcheck_open: dbname=%s\n",dbname) ;
#endif

	if (pr == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;
	if (dbname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(SPELLCHECK)) ;

	if ((rs = spellcheck_objloadbegin(op,pr,objname)) >= 0) {
	    if ((rs = (*op->call.start)(op->obj,pr,dbname)) >= 0) {
		op->magic = SPELLCHECK_MAGIC ;
	    }
	    if (rs < 0)
		spellcheck_objloadend(op) ;
	} /* end if (objload-begin) */

#if	CF_DEBUGS
	debugprintf("spellcheck_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (spellcheck_start) */


/* free up the entire vector string data structure object */
int spellcheck_finish(op)
SPELLCHECK	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SPELLCHECK_MAGIC)
	    return SR_NOTOPEN ;

	rs1 = (*op->call.finish)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = spellcheck_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (spellcheck_finish) */


int spellcheck_count(op)
SPELLCHECK	*op ;
{
	int	rs = SR_NOSYS ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SPELLCHECK_MAGIC)
	    return SR_NOTOPEN ;

	if (op->call.count != NULL)
	    rs = (*op->call.count)(op->obj) ;

	return rs ;
}
/* end subroutine (spellcheck_count) */


int spellcheck_look(op,wp,wl)
SPELLCHECK	*op ;
const char	*wp ;
int		wl ;
{
	int	rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (wp == NULL) return SR_FAULT ;

	if (op->magic != SPELLCHECK_MAGIC) return SR_NOTOPEN ;

	if (op->call.look != NULL) {
	    rs = (*op->call.look)(op->obj,wp,wl) ;
	}

	return rs ;
}
/* end subroutine (spellcheck_look) */


int spellcheck_looks(op,bop,wa,wn)
SPELLCHECK	*op ;
BITS		*bop ;
const char	**wa ;
int		wn ;
{
	int	rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (bop == NULL) return SR_FAULT ;
	if (wa == NULL) return SR_FAULT ;

	if (op->magic != SPELLCHECK_MAGIC) return SR_NOTOPEN ;

	if (op->call.looks != NULL) {
	    rs = (*op->call.looks)(op->obj,bop,wa,wn) ;
	}

	return rs ;
}
/* end subroutine (spellcheck_looks) */


int spellcheck_curbegin(op,curp)
SPELLCHECK	*op ;
SPELLCHECK_CUR	*curp ;
{
	int	rs = SR_OK ;


	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SPELLCHECK_MAGIC) return SR_NOTOPEN ;

	if (op->call.curbegin != NULL) {
	    void	*p ;
	    if ((rs = uc_malloc(op->cursize,&p)) >= 0) {
	        curp->scp = p ;
		if ((rs = (*op->call.curbegin)(op->obj,curp->scp)) >= 0) {
		    curp->magic = SPELLCHECK_MAGIC ;
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
/* end subroutine (spellcheck_curbegin) */


int spellcheck_curend(op,curp)
SPELLCHECK	*op ;
SPELLCHECK_CUR	*curp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SPELLCHECK_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != SPELLCHECK_MAGIC) return SR_NOTOPEN ;

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
/* end subroutine (spellcheck_curend) */


/* enumerate entries */
int spellcheck_enum(op,curp,vbuf,vlen)
SPELLCHECK	*op ;
SPELLCHECK_CUR	*curp ;
char		vbuf[] ;
int		vlen ;
{
	int	rs = SR_OK ;


	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;

	if (op->magic != SPELLCHECK_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != SPELLCHECK_MAGIC) return SR_NOTOPEN ;

	if (op->call.enumerate != NULL) {
	    rs = (*op->call.enumerate)(op->obj,curp->scp,vbuf,vlen) ;
	} else
	    rs = SR_NOTSUP ;

#if	CF_DEBUGS
	debugprintf("spellcheck_enum: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (spellcheck_enum) */


int spellcheck_audit(op)
SPELLCHECK	*op ;
{
	int	rs = SR_NOSYS ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SPELLCHECK_MAGIC)
	    return SR_NOTOPEN ;

	if (op->call.audit != NULL)
	    rs = (*op->call.audit)(op->obj) ;

	return rs ;
}
/* end subroutine (spellcheck_audit) */


/* private subroutines */


/* find and load the DB-access object */
static int spellcheck_objloadbegin(op,pr,objname)
SPELLCHECK	*op ;
const char	*pr ;
const char	*objname ;
{
	MODLOAD	*lp = &op->loader ;

	VECSTR	syms ;

	int	rs ;
	int	opts ;
	int	snl ;
	int	n ;


#if	CF_DEBUGS
	debugprintf("spellcheck_objloadbegin: pr=%s\n",pr) ;
	debugprintf("spellcheck_objloadbegin: objname=%s\n",objname) ;
#endif

	n = nelem(subs) ;
	opts = VECSTR_OCOMPACT ;
	if ((rs = vecstr_start(&syms,n,opts)) >= 0) {
	    int		i ;
	    char	symname[SYMNAMELEN + 1] ;

	    for (i = 0 ; (i < n) && (subs[i] != NULL) ; i += 1) {
	        if (isrequired(i)) {
	            rs = sncpy3(symname,SYMNAMELEN,objname,"_",subs[i]) ;
		    snl = rs ;
		    if (rs >= 0) 
			rs = vecstr_add(&syms,symname,snl) ;
		}
		if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
		const char	**sv ;
	        if ((rs = vecstr_getvec(&syms,&sv)) >= 0) {
	            const char	*modbname = SPELLCHECK_MODBNAME ;
	            opts = (MODLOAD_OLIBVAR | MODLOAD_OPRS | MODLOAD_OSDIRS) ;
	            rs = modload_open(lp,pr,modbname,objname,opts,sv) ;
		}
	    }

	    vecstr_finish(&syms) ;
	} /* end if (allocation) */

#if	CF_DEBUGS
	debugprintf("spellcheck_objloadbegin: modload_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    int	mv[2] ;
	    if ((rs = modload_getmva(lp,mv,2)) >= 0) {
		int	objsize = rs ;
		char	*p ;
		if ((rs = uc_malloc(objsize,&p)) >= 0) {
		    op->obj = p ;
		    if ((rs = spellcheck_loadcalls(op,objname)) >= 0) {
			op->objsize = mv[0] ;
			op->cursize = mv[1] ;
		    }
		    if (rs < 0) {
	    		uc_free(op->obj) ;
	    		op->obj = NULL ;
		    }
		} /* end if (memory-allocation) */
	    } /* end if */
	    if (rs < 0) {
		modload_close(lp) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (spellcheck_objloadbegin) */


static int spellcheck_objloadend(op)
SPELLCHECK	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op->obj != NULL) {
	    rs1 = uc_free(op->obj) ;
	    if (rs >= 0) rs = rs1 ;
	    op->obj = NULL ;
	}

	rs1 = modload_close(&op->loader) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (spellcheck_objloadend) */


static int spellcheck_loadcalls(op,objname)
SPELLCHECK	*op ;
const char	objname[] ;
{
	MODLOAD	*lp = &op->loader ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	c = 0 ;

	char	symname[SYMNAMELEN + 1] ;

	const void	*snp ;


	for (i = 0 ; subs[i] != NULL ; i += 1) {

	    rs = sncpy3(symname,SYMNAMELEN,objname,"_",subs[i]) ;

#if	CF_DEBUGS
	    debugprintf("spellcheck_loadcalls: sncpy3() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
		break ;

#if	CF_DEBUGS
	    debugprintf("spellcheck_loadcalls: symname=%s\n",symname) ;
#endif

	    rs1 = modload_getsym(lp,symname,&snp) ;

#if	CF_DEBUGS
	    debugprintf("spellcheck_loadcalls: modload_getsym() rs=%d\n",rs1) ;
#endif

	    if (rs1 == SR_NOTFOUND) {
		snp = NULL ;
		if (isrequired(i))
		    break ;
	    } else
		rs = rs1 ;

	    if (rs < 0)
		break ;

#if	CF_DEBUGS
	    debugprintf("spellcheck_loadcalls: call=%s %c\n",
		subs[i],
		((snp != NULL) ? 'Y' : 'N')) ;
#endif

	    if (snp != NULL) {

	        c += 1 ;
		switch (i) {

		case sub_start:
		    op->call.start = 
			(int (*)(void *,const char *,const char *)) snp ;
		    break ;

		case sub_count:
		    op->call.count = (int (*)(void *)) snp ;
		    break ;

		case sub_look:
		    op->call.look = 
			(int (*)(void *,const char *,int)) snp ;
		    break ;

		case sub_looks:
		    op->call.looks = 
			(int (*)(void *,BITS *,const char **,int)) snp ;
		    break ;

		case sub_curbegin:
		    op->call.curbegin = 
			(int (*)(void *,SCS_C *)) snp ;
		    break ;

		case sub_enum:
		    op->call.enumerate = 
			(int (*)(void *,SCS_C *,char *,int)) snp ;
		    break ;

		case sub_curend:
		    op->call.curend= 
			(int (*)(void *,SCS_C *)) snp ;
		    break ;


		case sub_audit:
		    op->call.audit = (int (*)(void *)) snp ;
		    break ;

		case sub_finish:
		    op->call.finish = (int (*)(void *)) snp ;
		    break ;

		} /* end switch */

	    } /* end if (it had the call) */

	} /* end for (subs) */

#if	CF_DEBUGS
	debugprintf("spellcheck_loadcalls: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (spellcheck_loadcalls) */


static int isrequired(i)
int	i ;
{
	int	f = FALSE ;

	switch (i) {
	case sub_start:
	case sub_count:
	case sub_look:
	case sub_curbegin:
	case sub_enum:
	case sub_curend:
	case sub_audit:
	case sub_finish:
	    f = TRUE ;
	    break ;
	} /* end switch */

	return f ;
}
/* end subroutine (isrequired) */



