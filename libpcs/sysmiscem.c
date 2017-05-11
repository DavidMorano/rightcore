/* sysmiscem */

/* object load management for the SYSMISCEMS object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that provides
	access to the SYSMISCEM object (which is dynamically loaded).


*******************************************************************************/


#define	SYSMISCEM_MASTER	0


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

#include	"sysmiscem.h"
#include	"sysmiscems.h"


/* local defines */

#define	SYSMISCEM_MAGIC		0x97147246
#define	SYSMISCEM_MODBNAME	"sysmiscems"
#define	SYSMISCEM_OBJNAME	"sysmiscems"

#undef	SMM_DATA
#define	SMM_DATA	SYSMISCEMS_DATA

#define	LIBCNAME	"lib"

#define	VARLIBPATH	"LD_LIBRARY_PATH"

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

static int	sysmiscem_objloadinit(SYSMISCEM *,const char *,const char *) ;
static int	sysmiscem_objloadfree(SYSMISCEM *) ;
static int	sysmiscem_loadcalls(SYSMISCEM *,const char *) ;

static int	isrequired(int) ;

#ifdef	COMMENT
static int	istermrs(int) ;
#endif /* COMMENT */


/* global variables */


/* local variables */

static const char	*subs[] = {
	"open",
	"get",
	"audit",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_get,
	sub_audit,
	sub_close,
	sub_overlast
} ;

#ifdef	COMMENT

static const int	termrs[] = {
	SR_FAULT,
	SR_INVALID,
	SR_NOMEM,
	SR_NOANODE,
	SR_BADFMT,
	SR_NOSPC,
	SR_NOSR,
	SR_NOBUFS,
	SR_BADF,
	SR_OVERFLOW,
	SR_RANGE,
	0
} ;

#endif /* COMMENT */


/* exported subroutines */


int sysmiscem_open(op,pr)
SYSMISCEM	*op ;
const char	pr[] ;
{
	int	rs ;

	const char	*objname = SYSMISCEM_OBJNAME ;


	if (op == NULL)
	    return SR_FAULT ;

	if (pr == NULL)
	    return SR_FAULT ;

	if (pr[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("sysmiscem_open: pr=%s\n",pr) ;
#endif

	memset(op,0,sizeof(SYSMISCEM)) ;

	rs = sysmiscem_objloadinit(op,pr,objname) ;

#if	CF_DEBUGS
	debugprintf("sysmiscem_open: _objloadinit() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	rs = (*op->call.open)(op->obj,pr) ;

#if	CF_DEBUGS
	debugprintf("sysmiscem_open: call->open() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

	op->magic = SYSMISCEM_MAGIC ;

ret0:

#if	CF_DEBUGS
	debugprintf("sysmiscem_open: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad1:
	sysmiscem_objloadfree(op) ;

bad0:
	goto ret0 ;
}
/* end subroutine (sysmiscem_open) */


/* free up the entire vector string data structure object */
int sysmiscem_close(op)
SYSMISCEM	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SYSMISCEM_MAGIC)
	    return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = sysmiscem_objloadfree(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (sysmiscem_close) */


int sysmiscem_get(op,daytime,to,dp)
SYSMISCEM	*op ;
time_t		daytime ;
int		to ;
SYSMISCEM_DATA	*dp ;
{
	SYSMISCEMS_DATA	sd ;

	int	rs = SR_NOSYS ;
	int	n = 0 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SYSMISCEM_MAGIC)
	    return SR_NOTOPEN ;

	if (op->call.get != NULL) {
	    rs = (*op->call.get)(op->obj,daytime,to,&sd) ;
	    n = rs ;
	}

	if (dp != NULL) {
	    memset(dp,0,sizeof(SYSMISCEM_DATA)) ;
	    if (rs >= 0) {
		int	i ;
		dp->intstale = sd.intstale ;
		dp->utime = sd.utime ;
		dp->btime = sd.btime ;
		dp->ncpu = sd.ncpu ;
		dp->nproc = sd.nproc ;
		for (i = 0 ; i < 3 ; i += 1)
		    dp->la[i] = sd.la[i] ;
	    }
	} /* end if */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (sysmiscem_get) */


int sysmiscem_audit(op)
SYSMISCEM	*op ;
{
	int	rs = SR_NOSYS ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SYSMISCEM_MAGIC)
	    return SR_NOTOPEN ;

	if (op->call.audit != NULL)
	    rs = (*op->call.audit)(op->obj) ;

	return rs ;
}
/* end subroutine (sysmiscem_audit) */


/* private subroutines */


/* find and load the DB-access object */
static int sysmiscem_objloadinit(op,pr,objname)
SYSMISCEM	*op ;
const char	*pr ;
const char	*objname ;
{
	MODLOAD	*lp = &op->loader ;

	VECSTR	syms ;

	int	rs ;
	int	opts ;
	int	snl ;
	int	objsize ;
	int	n, i ;

	const char	*modbname ;
	const char	**sv ;

	char	symname[SYMNAMELEN + 1] ;


#if	CF_DEBUGS
	debugprintf("sysmiscem_objloadinit: pr=%s\n",pr) ;
	debugprintf("sysmiscem_objloadinit: objname=%s\n",objname) ;
#endif

	n = nelem(subs) ;
	opts = VECSTR_OCOMPACT ;
	if ((rs = vecstr_start(&syms,n,opts)) >= 0) {

	    for (i = 0 ; (i < n) && (subs[i] != NULL) ; i += 1) {
	        if (isrequired(i)) {
	            rs = sncpy3(symname,SYMNAMELEN,objname,"_",subs[i]) ;
		    snl = rs ;
		    if (rs >= 0) 
			rs = vecstr_add(&syms,symname,snl) ;
		}
		if (rs < 0)
		    break ;
	    } /* end for */

	    if (rs >= 0)
	        rs = vecstr_getvec(&syms,&sv) ;

	    if (rs >= 0) {
	        modbname = SYSMISCEM_MODBNAME ;
	        opts = (MODLOAD_OLIBVAR | MODLOAD_OPRS | MODLOAD_OSDIRS) ;
	        rs = modload_open(lp,pr,modbname,objname,opts,sv) ;
	    }

	    vecstr_finish(&syms) ;
	} /* end if (allocation) */

#if	CF_DEBUGS
	debugprintf("sysmiscem_objloadinit: modload_open() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	rs = modload_getmv(lp,0) ;
	objsize = rs ;
	if (rs < 0)
	    goto bad1 ;

/* allocate the object */

	rs = uc_malloc(objsize,&op->obj) ;
	if (rs < 0)
	    goto bad1 ;
		
/* load the SO-interface calls */

	rs = sysmiscem_loadcalls(op,objname) ;

#if	CF_DEBUGS
	debugprintf("sysmiscem_objloadinit: sysmiscem_loadcalls() rs=%d\n",
		rs) ;
#endif

	if (rs < 0)
	    goto bad2 ;

ret0:
	return rs ;

/* bad stuff */
bad2:
	{
	    uc_free(op->obj) ;
	    op->obj = NULL ;
	}

bad1:
	modload_close(lp) ;

bad0:
	goto ret0 ;
}
/* end subroutine (sysmiscem_objloadinit) */


static int sysmiscem_objloadfree(op)
SYSMISCEM	*op ;
{
	int	rs ;


	if (op->obj != NULL) {
	    uc_free(op->obj) ;
	    op->obj = NULL ;
	}

	rs = modload_close(&op->loader) ;

	return rs ;
}
/* end subroutine (sysmiscem_objloadfree) */


static int sysmiscem_loadcalls(op,objname)
SYSMISCEM	*op ;
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
	    if (rs < 0)
		break ;

	    rs1 = modload_getsym(lp,symname,&snp) ;
	    if (rs1 == SR_NOTFOUND) {
		snp = NULL ;
		if (isrequired(i))
		    break ;
	    } else
		rs = rs1 ;

	    if (rs < 0)
		break ;

#if	CF_DEBUGS
	    debugprintf("sysmiscem_loadcalls: call=%s %c\n",
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

		case sub_get:
		    op->call.get = (int (*)(void *,time_t,int,SMM_DATA *)) snp ;
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
	debugprintf("sysmiscem_loadcalls: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (sysmiscem_loadcalls) */


static int isrequired(i)
int	i ;
{
	int	f = FALSE ;


	switch (i) {

	case sub_open:
	case sub_get:
	case sub_close:
	    f = TRUE ;
	    break ;

	} /* end switch */

	return f ;
}
/* end subroutine (isrequired) */


#ifdef	COMMENT

static int istermrs(rs)
int	rs ;
{
	int	i ;
	int	f = FALSE ;


	for (i = 0 ; termrs[i] != 0 ; i += 1) {
	    f = (rs == termrs[i]) ;
	    if (f)
		break ;
	} /* end if */

	return f ;
}
/* end subroutine (istermrs) */

#endif /* COMMENT */


