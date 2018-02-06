/* varmk */

/* VARMK management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_LOOKSELF	0		/* try searching "SELF" for SO */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module implements an interface (a trivial one) that provides access
        to the VARMK object (which is dynamically loaded).


*******************************************************************************/


#define	VARMK_MASTER	1


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

#include	"varmk.h"
#include	"varmks.h"


/* local defines */

#define	VARMK_OBJNAME	"varmks"
#define	VARMK_MODBNAME	"varmks"

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
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

static int	varmk_objloadbegin(VARMK *,const char *,const char *) ;
static int	varmk_objloadend(VARMK *) ;
static int	varmk_loadcalls(VARMK *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static const char	*subs[] = {
	"open",
	"addvar",
	"abort",
	"chgrp",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_addvar,
	sub_abort,
	sub_chgrp,
	sub_close,
	sub_overlast
} ;


/* exported subroutines */


int varmk_open(VARMK *op,cchar dbname[],int of,mode_t om,int n)
{
	int		rs ;
	const char	*objname = VARMK_OBJNAME ;
	char		dn[MAXHOSTNAMELEN+1] ;

	if (op == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (dbname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("varmk_open: ent dbname=%s\n",dbname) ;
#endif

	memset(op,0,sizeof(VARMK)) ;

	if ((rs = getnodedomain(NULL,dn)) >= 0) {
	    const int	prlen = MAXPATHLEN ;
	    const char	*pn = VARPRLOCAL ;
	    char	prbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpr(prbuf,prlen,pn,dn)) >= 0) {
	        if ((rs = varmk_objloadbegin(op,prbuf,objname)) >= 0) {
	            if ((rs = (*op->call.open)(op->obj,dbname,of,om,n)) >= 0) {
	                op->magic = VARMK_MAGIC ;
	            }
	            if (rs < 0)
	                varmk_objloadend(op) ;
	        } /* end if (objloadbegin) */
	    } /* end if (mkpr) */
	} /* end if (getnodedomain) */

#if	CF_DEBUGS
	debugprintf("varmk_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (varmk_open) */


/* free up the entire vector string data structure object */
int varmk_close(VARMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != VARMK_MAGIC) return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("varmk_close: varmks_close() rs=%d\n",rs) ;
#endif

	rs1 = varmk_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("varmk_close: varmk_objloadend() rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (varmk_close) */


int varmk_addvar(VARMK *op,cchar k[],cchar vp[],int vl)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != VARMK_MAGIC) return SR_NOTOPEN ;

	rs = (*op->call.addvar)(op->obj,k,vp,vl) ;

	return rs ;
}
/* end subroutine (varmk_addvar) */


int varmk_abort(VARMK *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != VARMK_MAGIC) return SR_NOTOPEN ;

	if (op->call.abort != NULL) {
	    rs = (*op->call.abort)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (varmk_abort) */


int varmk_chgrp(VARMK *op,gid_t gid)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != VARMK_MAGIC) return SR_NOTOPEN ;

	if (op->call.chgrp != NULL) {
	    rs = (*op->call.chgrp)(op->obj,gid) ;
	}

	return rs ;
}
/* end subroutine (varmk_chgrp) */


/* private subroutines */


/* find and load the DB-access object */
static int varmk_objloadbegin(VARMK *op,cchar *pr,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	const int	vo = VECSTR_OCOMPACT ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("varmk_objloadbegin: ent pr=%s on=%s\n",pr,objname) ;
#endif

	if ((rs = vecstr_start(&syms,n,vo)) >= 0) {
	    const int	snl = SYMNAMELEN ;
	    int		i ;
	    int		f_modload = FALSE ;
	    const char	**sv ;
	    const char	*on = objname ;
	    char	snb[SYMNAMELEN + 1] ;

	    for (i = 0 ; (i < n) && (subs[i] != NULL) ; i += 1) {
	        if (isrequired(i)) {
	            if ((rs = sncpy3(snb,snl,on,"_",subs[i])) >= 0) {
	                rs = vecstr_add(&syms,snb,rs) ;
		    }
	        }
	        if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
	        if ((rs = vecstr_getvec(&syms,&sv)) >= 0) {
	            const char	*mn = VARMK_MODBNAME ;
	            const char	*on = objname ;
	            int		mo = 0 ;
	            mo |= MODLOAD_OLIBVAR ;
	            mo |= MODLOAD_OPRS ;
	            mo |= MODLOAD_OSDIRS ;
	            mo |= MODLOAD_OAVAIL ;
	            rs = modload_open(lp,pr,mn,on,mo,sv) ;
		    f_modload = (rs >= 0) ;
	        } /* end if (getvec) */
	    } /* end if (ok) */

	    rs1 = vecstr_finish(&syms) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && f_modload)
		modload_close(lp) ;
	} /* end if (vecstr_start) */

	if (rs >= 0) {
	    int		mv[2] ;
	    if ((rs = modload_getmva(lp,mv,1)) >= 0) {
	        void	*p ;
	        op->objsize = mv[0] ;
	        if ((rs = uc_malloc(op->objsize,&p)) >= 0) {
	            op->obj = p ;
	            rs = varmk_loadcalls(op,objname) ;
#if	CF_DEBUGS
	            debugprintf("varmk_objloadbegin: _loadcalls() rs=%d\n",rs) ;
#endif
	            if (rs < 0) {
	                uc_free(op->obj) ;
	                op->obj = NULL ;
	            }
	        } /* end if (memory-allocation) */
	    } /* end if (modload_getmva) */
	    if (rs < 0)
	        modload_close(lp) ;
	} /* end if (modload_open) */

#if	CF_DEBUGS
	debugprintf("varmk_objloadbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (varmk_objloadbegin) */


static int varmk_objloadend(VARMK *op)
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
/* end subroutine (varmk_objloadend) */


static int varmk_loadcalls(VARMK *op,cchar soname[])
{
	MODLOAD		*lp = &op->loader ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	char		symname[SYMNAMELEN + 1] ;
	const void	*snp ;

	for (i = 0 ; subs[i] != NULL ; i += 1) {

	    if ((rs = sncpy3(symname,SYMNAMELEN,soname,"_",subs[i])) >= 0) {
	        if ((rs = modload_getsym(lp,symname,&snp)) == SR_NOTFOUND) {
	            snp = NULL ;
	            if (! isrequired(i)) rs = SR_OK ;
	        }
	    }

	    if (rs < 0) break ;

#if	CF_DEBUGS
	    debugprintf("varmk_loadcalls: call=%s %c\n",
	        subs[i],
	        ((snp != NULL) ? 'Y' : 'N')) ;
#endif

	    if (snp != NULL) {

	        c += 1 ;
	        switch (i) {

	        case sub_open:
	            op->call.open = (int (*)(void *,
	                const char *,int,mode_t,int)) snp ;
	            break ;

	        case sub_addvar:
	            op->call.addvar = (int (*)(void *,const char *,
	                const char *,int)) snp ;
	            break ;

	        case sub_abort:
	            op->call.abort = (int (*)(void *)) snp ;
	            break ;

	        case sub_chgrp:
	            op->call.chgrp = (int (*)(void *,gid_t)) snp ;
	            break ;

	        case sub_close:
	            op->call.close = (int (*)(void *)) snp ;
	            break ;

	        } /* end switch */

	    } /* end if (it had the call) */

	} /* end for (subs) */

#if	CF_DEBUGS
	debugprintf("varmk_loadcalls: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (varmk_loadcalls) */


static int isrequired(int i)
{
	int		f = FALSE ;
	switch (i) {
	case sub_open:
	case sub_addvar:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isrequired) */


