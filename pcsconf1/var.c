/* var */

/* VAR management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_LOOKSELF	0		/* try searching "SELF" for SO */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module implements an interface (a trivial one) that provides access
        to the VAR object (which is dynamically loaded).


*******************************************************************************/


#define	VAR_MASTER	1


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
#include	<nulstr.h>
#include	<endianstr.h>
#include	<localmisc.h>

#include	"var.h"
#include	"vars.h"


/* local defines */

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#define	VAR_MODBNAME	"vars"
#define	VAR_OBJNAME	"vars"

#define	LIBCNAME	"lib"
#define	SONAME		"vars"
#define	INDSUF		"vi"

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
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getdomainname(char *,int,const char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

int		var_opena(VAR *,const char **) ;

static int	var_objloadbegin(VAR *,const char *) ;
static int	var_objloadend(VAR *) ;
static int	var_loadcalls(VAR *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static const char	*subs[] = {
	"open",
	"count",
	"curbegin",
	"fetch",
	"enum",
	"curend",
	"info",
	"audit",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_count,
	sub_curbegin,
	sub_fetch,
	sub_enum,
	sub_curend,
	sub_info,
	sub_audit,
	sub_close,
	sub_overlast
} ;


/* exported subroutines */


int var_open(VAR *op,cchar *dbname)
{
	int		rs ;
	const char	*objname = VAR_OBJNAME ;

	if (op == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (dbname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("var_open: ent dbname=%s\n",dbname) ;
#endif

	memset(op,0,sizeof(VAR)) ;

	if ((rs = var_objloadbegin(op,objname)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,dbname)) >= 0) {
		op->magic = VAR_MAGIC ;
	    }
	    if (rs < 0)
		var_objloadend(op) ;
	} /* end if (objloadbegin) */

#if	CF_DEBUGS
	debugprintf("var_open: call->open() rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugprintf("var_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (var_open) */


int var_opena(VAR *op,cchar *narr[])
{
	int		rs ;
	const char	*objname = VAR_OBJNAME ;

	if (op == NULL) return SR_FAULT ;
	if (narr == NULL) return SR_FAULT ;

	if (narr[0] == NULL) return SR_INVALID ;
	if (narr[0][0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("var_opena: ent\n") ;
#endif

	memset(op,0,sizeof(VAR)) ;

	if ((rs = var_objloadbegin(op,objname)) >= 0) {
	    int	i ;
	    for (i = 0 ; narr[i] != NULL ; i += 1) {
	        rs = (*op->call.open)(op->obj,narr[i]) ;
	        if ((rs >= 0) || (! isNotPresent(rs))) break ;
	    } /* end for */
	    if (rs >= 0) {
		op->magic = VAR_MAGIC ;
	    }
	    if (rs < 0)
		var_objloadend(op) ;
	} /* end if (objloadbegin) */

#if	CF_DEBUGS
	debugprintf("var_opena: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (var_opena) */


/* free up the entire vector string data structure object */
int var_close(VAR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != VAR_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("var_close: ent\n") ;
#endif

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = var_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (var_close) */


int var_info(VAR *op,VAR_INFO *vip)
{
	VARS_INFO	vsi ;
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (vip == NULL) return SR_FAULT ;

	if (op->magic != VAR_MAGIC) return SR_NOTOPEN ;

	memset(vip,0,sizeof(VAR_INFO)) ;

	if (op->call.info != NULL) {
	    if ((rs = (*op->call.info)(op->obj,&vsi)) >= 0) {
		vip->wtime = vsi.wtime ;
		vip->mtime = vsi.mtime ;
		vip->nvars = vsi.nvars ;
		vip->nskip = vsi.nskip ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (var_info) */


int var_audit(VAR *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != VAR_MAGIC) return SR_NOTOPEN ;

	if (op->call.audit != NULL) {
	    rs = (*op->call.audit)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (var_audit) */


int var_count(VAR *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != VAR_MAGIC) return SR_NOTOPEN ;

	if (op->call.count != NULL) {
	    rs = (*op->call.count)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (var_count) */


int var_curbegin(VAR *op,VAR_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != VAR_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("var_curbegin: ent\n") ;
#endif

	memset(curp,0,sizeof(VAR_CUR)) ;

	if (op->call.curbegin != NULL) {
	    void	*p ;
	    if ((rs = uc_malloc(op->cursize,&p)) >= 0) {
		curp->scp = p ;
	        if ((rs = (*op->call.curbegin)(op->obj,curp->scp)) >= 0) {
	            curp->magic = VAR_MAGIC ;
		}
	        if (rs < 0) {
	            uc_free(curp->scp) ;
	            curp->scp = NULL ;
	        }
	    } /* end if (memory-allocation) */
	} else
	    rs = SR_NOSYS ;

#if	CF_DEBUGS
	debugprintf("var_curbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (var_curbegin) */


int var_curend(VAR *op,VAR_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != VAR_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != VAR_MAGIC) return SR_NOTOPEN ;

	if (curp->scp != NULL) {
	    if (op->call.curend != NULL) {
	        rs1 = (*op->call.curend)(op->obj,curp->scp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        rs = SR_NOSYS ;
	    }
	    rs1 = uc_free(curp->scp) ;
	    if (rs >= 0) rs = rs1 ;
	    curp->scp = NULL ;
	} else
	    rs = SR_NOANODE ;

	curp->magic = 0 ;
	return rs ;
}
/* end subroutine (var_curend) */


/* fetch a variable by name */
int var_fetch(VAR *op,cchar *kp,int kl,VAR_CUR *curp,char vbuf[],int vlen)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (op->magic != VAR_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != VAR_MAGIC) return SR_NOTOPEN ;

	if (op->call.fetch != NULL) {
	    rs = (*op->call.fetch)(op->obj,kp,kl,curp->scp,vbuf,vlen) ;
	}

	return rs ;
}
/* end subroutine (var_fetch) */


/* enumerate entries */
int var_enum(VAR *op,VAR_CUR *curp,char kbuf[],int klen,char vbuf[],int vlen)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (kbuf == NULL) return SR_FAULT ;

	if (op->magic != VAR_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != VAR_MAGIC) return SR_NOTOPEN ;

	if (op->call.enumerate != NULL) {
	    rs = (*op->call.enumerate)(op->obj,curp->scp,
		kbuf,klen,vbuf,vlen) ;
	}

	return rs ;
}
/* end subroutine (var_enum) */


int varinfo(VARINFO *vip,cchar dbnp[],int dbnl)
{
	NULSTR		ns ;
	int		rs ;
	int		rs1 ;
	const char	*np ;

	if (vip == NULL) return SR_FAULT ;
	if (dbnp == NULL) return SR_FAULT ;

	if (dbnp[0] == '\0') return SR_INVALID ;

	if ((rs = nulstr_start(&ns,dbnp,dbnl,&np)) >= 0) {
	    const char	*end = ENDIANSTR ;
	    char	tmpfname[MAXPATHLEN + 1] ;

	    memset(vip,0,sizeof(VARINFO)) ;

	    if ((rs = mkfnamesuf2(tmpfname,np,INDSUF,end)) >= 0) {
	        struct ustat	sb ;
		if ((rs = u_stat(tmpfname,&sb)) >= 0) {
		    vip->size = sb.st_size ;
		    vip->mtime = sb.st_mtime ;
		}
	    }

	    rs1 = nulstr_finish(&ns) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr) */

	return rs ;
}
/* end subroutine (varinfo) */


int varunlink(cchar dbnp[],int dbnl)
{
	NULSTR		ns ;
	int		rs ;
	const char	*np ;

	if (dbnp == NULL) return SR_FAULT ;
	if (dbnp[0] == '\0') return SR_INVALID ;

	if ((rs = nulstr_start(&ns,dbnp,dbnl,&np)) >= 0) {
	    const char	*end = ENDIANSTR ;
	    char	tmpfname[MAXPATHLEN + 1] ;

	    if ((rs = mkfnamesuf2(tmpfname,np,INDSUF,end)) >= 0) {
		rs = uc_unlink(tmpfname) ;
	    }

	    nulstr_finish(&ns) ;
	} /* end if (nulstr) */

	return rs ;
}
/* end subroutine (varunlink) */


/* private subroutines */


/* find and load the DB-access object */
static int var_objloadbegin(VAR *op,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	int		rs ;
	char		dn[MAXHOSTNAMELEN+1] ;

	if ((rs = getnodedomain(NULL,dn)) >= 0) {
	    const char	*prname = VARPRLOCAL ;
	    char	pr[MAXPATHLEN+1] ;
	    if ((rs = mkpr(pr,MAXPATHLEN,prname,dn)) >= 0) {
		VECSTR		syms ;
	        const int	n = nelem(subs) ;
		const int	vo = VECSTR_OCOMPACT ;

	        if ((rs = vecstr_start(&syms,n,vo)) >= 0) {
		    const int	snl = SYMNAMELEN ;
	            int		i ;
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
	                    const char	*modbname = VAR_MODBNAME ;
			    int		mo = 0 ;
	                    mo |= MODLOAD_OLIBVAR ;
			    mo |= MODLOAD_OPRS ;
			    mo |= MODLOAD_OSDIRS ;
	                    rs = modload_open(lp,pr,modbname,objname,mo,sv) ;
			} /* end if (getvec) */
	            } /* end if (ok) */

	            vecstr_finish(&syms) ;
	        } /* end if (allocation) */

		if (rs >= 0) {
		    int		mv[2] ;
		    if ((rs = modload_getmva(lp,mv,2)) >= 0) {
			void	*p ;
			op->objsize = mv[0] ;
			op->cursize = mv[1] ;
			if ((rs = uc_malloc(op->objsize,&p)) >= 0) {
			    op->obj = p ;
			    rs = var_loadcalls(op,objname) ;
			    if (rs < 0) {
	    			uc_free(op->obj) ;
	    			op->obj = NULL ;
			    }
			} /* end if (memory-allocation) */
		    } /* end if (modload_getmva) */
		    if (rs < 0)
			modload_close(lp) ;
		} /* end if (modload_open) */

	    } /* end if (mkpr) */
	} /* end if (getnodedomain) */

	return rs ;
}
/* end subroutine (var_objloadbegin) */


static int var_objloadend(VAR *op)
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
/* end subroutine (var_objloadend) */


static int var_loadcalls(VAR *op,cchar objname[])
{
	MODLOAD		*lp = &op->loader ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	char		symname[SYMNAMELEN + 1] ;
	const void	*snp ;

	for (i = 0 ; subs[i] != NULL ; i += 1) {

	    if ((rs = sncpy3(symname,SYMNAMELEN,objname,"_",subs[i])) >= 0) {
	         if ((rs = modload_getsym(lp,symname,&snp)) == SR_NOTFOUND) {
		     snp = NULL ;
		     if (! isrequired(i)) rs = SR_OK ;
		}
	    }

	    if (rs < 0) break ;

#if	CF_DEBUGS
	    debugprintf("var_loadcalls: call=%s %c\n",
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

		case sub_curbegin:
		    op->call.curbegin = 
			(int (*)(void *,void *)) snp ;
		    break ;

		case sub_fetch:
		    op->call.fetch = 
			(int (*)(void *,const char *,int,void *,char *,int)) 
				snp ;
		    break ;

		case sub_enum:
		    op->call.enumerate = 
			(int (*)(void *,void *,char *,int,char *,int)) snp ;
		    break ;

		case sub_curend:
		    op->call.curend = (int (*)(void *,void *)) snp ;
		    break ;

		case sub_info:
		    op->call.info = (int (*)(void *,VARS_INFO *)) snp ;
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
	debugprintf("var_loadcalls: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (var_loadcalls) */


static int isrequired(int i)
{
	int		f = FALSE ;
	switch (i) {
	case sub_open:
	case sub_curbegin:
	case sub_enum:
	case sub_curend:
	case sub_close:
	    f = TRUE ;
	    break ;
	case sub_fetch:
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isrequired) */


