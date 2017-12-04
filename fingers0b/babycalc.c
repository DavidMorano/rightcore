/* babycalc */

/* object load management for the BABYCALCS object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module implements an interface (a trivial one) that provides access
        to the BABYCALC object (which is dynamically loaded).


*******************************************************************************/


#define	BABYCALC_MASTER		0


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

#include	"babycalc.h"
#include	"babycalcs.h"


/* local defines */

#define	BABYCALC_MODBNAME	"babycalcs"
#define	BABYCALC_OBJNAME	"babycalcs"

#undef	SMM_DATA
#define	SMM_DATA	BABYCALCS_DATA

#define	LIBCNAME	"lib"

#define	VARLIBPATH	"LD_LIBRARY_PATH"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	babycalc_objloadbegin(BABYCALC *,const char *,const char *) ;
static int	babycalc_objloadend(BABYCALC *) ;
static int	babycalc_loadcalls(BABYCALC *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static cchar	*subs[] = {
	"open",
	"check",
	"lookup",
	"info",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_check,
	sub_lookup,
	sub_info,
	sub_close,
	sub_overlast
} ;


/* exported subroutines */


int babycalc_open(BABYCALC *op,cchar *pr,cchar *dbname)
{
	int		rs ;
	const char	*objname = BABYCALC_OBJNAME ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(BABYCALC)) ;

	if ((rs = babycalc_objloadbegin(op,pr,objname)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,pr,dbname)) >= 0) {
	        op->magic = BABYCALC_MAGIC ;
	    }
	    if (rs < 0)
		babycalc_objloadend(op) ;
	} /* end if (obj-load-begin) */

#if	CF_DEBUGS
	debugprintf("babycalc_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (babycalc_open) */


/* free up the entire vector string data structure object */
int babycalc_close(BABYCALC *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BABYCALC_MAGIC) return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = babycalc_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (babycalc_close) */


int babycalc_check(BABYCALC *op,time_t daytime)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BABYCALC_MAGIC) return SR_NOTOPEN ;

	if (op->call.check != NULL) {
	    rs = (*op->call.check)(op->obj,daytime) ;
	}

	return rs ;
}
/* end subroutine (babycalc_check) */


int babycalc_lookup(BABYCALC *op,time_t datereq,uint *rp)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BABYCALC_MAGIC) return SR_NOTOPEN ;

	if (op->call.lookup != NULL) {
	    rs = (*op->call.lookup)(op->obj,datereq,rp) ;
	}

	return rs ;
}
/* end subroutine (babycalc_lookup) */


int babycalc_info(BABYCALC *op,BABYCALC_INFO *ip)
{
	BABYCALCS_INFO	bi ;
	int		rs = SR_NOSYS ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BABYCALC_MAGIC) return SR_NOTOPEN ;

	if (op->call.info != NULL) {
	    rs = (*op->call.info)(op->obj,&bi) ;
	    n = rs ;
	}

	if (ip != NULL) {
	    memset(ip,0,sizeof(BABYCALC_INFO)) ;
	    if (rs >= 0) {
		ip->wtime = bi.wtime ;
		ip->atime = bi.atime ;
		ip->acount = bi.acount ;
	    }
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (babycalc_info) */


/* private subroutines */


/* find and load the DB-access object */
static int babycalc_objloadbegin(BABYCALC *op,cchar *pr,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	int		rs ;
	int		rs1 ;
	int		opts ;

#if	CF_DEBUGS
	debugprintf("babycalc_objloadbegin: pr=%s\n",pr) ;
	debugprintf("babycalc_objloadbegin: objname=%s\n",objname) ;
#endif

	opts = VECSTR_OCOMPACT ;
	if ((rs = vecstr_start(&syms,n,opts)) >= 0) {
	    const int	nlen = SYMNAMELEN ;
	    int		i ;
	    int		f_modload = FALSE ;
	    char	nbuf[SYMNAMELEN+1] ;

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
	            const char	*modbname = BABYCALC_MODBNAME ;
	            opts = (MODLOAD_OLIBVAR | MODLOAD_OPRS | MODLOAD_OSDIRS) ;
	            rs = modload_open(lp,pr,modbname,objname,opts,sv) ;
		    f_modload = (rs >= 0) ;
		}
	    }

	    rs1 = vecstr_finish(&syms) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && f_modload) {
		modload_close(lp) ;
	    }
	} /* end if (allocation) */

#if	CF_DEBUGS
	debugprintf("babycalc_objloadbegin: modload_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) { /* end if (modload-open) */
	    if ((rs = modload_getmv(lp,0)) >= 0) {
		int	objsize = rs ;
		void	*p ;
		if ((rs = uc_malloc(objsize,&p)) >= 0) {
		    op->obj = p ;
		    rs = babycalc_loadcalls(op,objname) ;
		} /* end if (memory-allocation) */
		if (rs < 0) {
	    	    uc_free(op->obj) ;
	    	    op->obj = NULL ;
		}
	    }
	    if (rs < 0)
		modload_close(lp) ;
	} /* end if (modload-open) */

	return rs ;
}
/* end subroutine (babycalc_objloadbegin) */


static int babycalc_objloadend(BABYCALC *op)
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
/* end subroutine (babycalc_objloadend) */


static int babycalc_loadcalls(BABYCALC *op,cchar *objname)
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
	    debugprintf("babycalc_loadcalls: call=%s %c\n",
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

		case sub_check:
		    op->call.check = (int (*)(void *,time_t)) snp ;
		    break ;

		case sub_lookup:
		    op->call.lookup = (int (*)(void *,time_t,uint *)) snp ;
		    break ;

		case sub_info:
		    op->call.info = (int (*)(void *,BABYCALCS_INFO *)) snp ;
		    break ;

		case sub_close:
		    op->call.close = (int (*)(void *)) snp ;
		    break ;

		} /* end switch */

	    } /* end if (it had the call) */

	} /* end for (subs) */

#if	CF_DEBUGS
	debugprintf("babycalc_loadcalls: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (babycalc_loadcalls) */


static int isrequired(int i)
{
	int	f = FALSE ;
	switch (i) {
	case sub_open:
	case sub_check:
	case sub_lookup:
	case sub_info:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isrequired) */


