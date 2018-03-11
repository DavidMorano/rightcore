/* sysrealname */

/* load management and interface for the System REALNAME data-base */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module interfaces to the system REALNAME database.


*******************************************************************************/


#define	SYSREALNAME_MASTER	0


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
#include	<realname.h>
#include	<modload.h>
#include	<localmisc.h>

#include	"sysrealname.h"


/* local defines */

#define	SYSREALNAME_MODBNAME	"ipasswd"
#define	SYSREALNAME_OBJNAME	"ipasswd"

#undef	SMM_INFO
#define	SMM_INFO	IPASSWD_INFO

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
extern int	nleadstr(const char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	sysrealname_objloadbegin(SYSREALNAME *,const char *) ;
static int	sysrealname_objloadend(SYSREALNAME *) ;
static int	sysrealname_loadcalls(SYSREALNAME *,const char *) ;

static int	sysrealname_curload(SYSREALNAME *,SYSREALNAME_CUR *,
			int,cchar **,int) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static cchar	*subs[] = {
	"open",
	"info",
	"curbegin",
	"curend",
	"fetcher",
	"enum",
	"audit",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_info,
	sub_curbegin,
	sub_curend,
	sub_fetcher,
	sub_enum,
	sub_audit,
	sub_close,
	sub_overlast
} ;


/* exported subroutines */


int sysrealname_open(SYSREALNAME *op,cchar *dbname)
{
	int		rs ;
	const char	*objname = SYSREALNAME_OBJNAME ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("sysrealname_open: dbname=%s\n",dbname) ;
#endif

	if (dbname == NULL) dbname = SYSREALNAME_DBNAME ;

	memset(op,0,sizeof(SYSREALNAME)) ;

	if ((rs = sysrealname_objloadbegin(op,objname)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,dbname)) >= 0) {
	        op->magic = SYSREALNAME_MAGIC ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("sysrealname_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysrealname_open) */


/* free up the entire vector string data structure object */
int sysrealname_close(SYSREALNAME *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SYSREALNAME_MAGIC) return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = sysrealname_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (sysrealname_close) */


int sysrealname_info(SYSREALNAME *op,SYSREALNAME_INFO *ip)
{
	IPASSWD_INFO	iinfo ;
	int		rs = SR_NOSYS ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SYSREALNAME_MAGIC) return SR_NOTOPEN ;

	if (op->call.info != NULL) {
	    rs = (*op->call.info)(op->obj,&iinfo) ;
	    n = rs ;
	}

	if (ip != NULL) {
	    memset(ip,0,sizeof(SYSREALNAME_INFO)) ;
	    if (rs >= 0) {
		ip->writetime = iinfo.writetime ;
		ip->writecount = iinfo.writecount ;
		ip->entries = iinfo.entries ;
		ip->version = iinfo.version ;
		ip->encoding = iinfo.encoding ;
		ip->type = iinfo.type ;
		ip->collisions = iinfo.collisions ;
	    }
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (sysrealname_info) */


int sysrealname_curbegin(SYSREALNAME *op,SYSREALNAME_CUR *curp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("sysrealname_curbegin: ent\n") ;
#endif

	memset(curp,0,sizeof(SYSREALNAME_CUR)) ;

	if (op->call.curbegin != NULL) {
	    char	*p ;
	    if ((rs = uc_malloc(op->cursize,&p)) >= 0) {
		curp->scp = p ;
	        if ((rs = (*op->call.curbegin)(op->obj,curp->scp)) >= 0) {
		    curp->magic = SYSREALNAME_CURMAGIC ;
		}

#if	CF_DEBUGS
	debugprintf("sysrealname_curbegin: call->curbegin() rs=%d\n",rs) ;
#endif

		if (rs < 0) {
		    uc_free(curp->scp) ;
		    curp->scp = NULL ;
		}
	    } /* end if (memory-allocation) */
	} else {
	    rs = SR_NOSYS ;
	}

#if	CF_DEBUGS
	debugprintf("sysrealname_curbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysrealname_curbegin) */


int sysrealname_curend(SYSREALNAME *op,SYSREALNAME_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SYSREALNAME_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != SYSREALNAME_CURMAGIC) return SR_NOTOPEN ;

	if (op->call.curend != NULL) {
	    rs1 = (*op->call.curend)(op->obj,curp->scp) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = uc_free(curp->scp) ;
	if (rs >= 0) rs = rs1 ;
	curp->scp = NULL ;

	if (curp->sa != NULL) {
	    rs1 = uc_free(curp->sa) ;
	    if (rs >= 0) rs = rs1 ;
	    curp->sa = NULL ;
	}

	curp->magic = 0 ;
	return rs ;
}
/* end subroutine (sysrealname_curend) */


int sysrealname_look(SYSREALNAME *op,SYSREALNAME_CUR *curp,int fo,
		cchar *sbuf,int slen)
{
	REALNAME	rn ;
	int		rs ;

	if (sbuf == NULL) return SR_FAULT ;

	if ((rs = realname_start(&rn,sbuf,slen)) >= 0) {
	    const char	*sa[6] ;
	    if ((rs = realname_getpieces(&rn,sa)) > 0) {
		rs = sysrealname_lookparts(op,curp,fo,sa,rs) ;
	    }
	    realname_finish(&rn) ;
	} /* end if (realname) */

	return rs ;
}
/* end subroutine (sysrealname_look) */


int sysrealname_lookparts(SYSREALNAME *op,SYSREALNAME_CUR *curp,int fo,
		cchar **sa,int sn)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (sa == NULL) return SR_FAULT ;

	if (op->magic != SYSREALNAME_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != SYSREALNAME_CURMAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	{
	    IPASSWD_CUR	*icp = curp->scp ;
	    debugprintf("sysrealname_lookparts: icp->magic{%p}\n",
		icp->magic) ;
	}
#endif

	if (op->call.fetcher != NULL) {
	    rs = sysrealname_curload(op,curp,fo,sa,sn) ;
	} else {
	    rs = SR_NOSYS ;
	}

	return rs ;
}
/* end subroutine (sysrealname_lookparts) */


int sysrealname_lookread(SYSREALNAME *op,SYSREALNAME_CUR *curp,char *rbuf)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("sysrealname_lookread: ent\n") ;
#endif

	if (op->magic != SYSREALNAME_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != SYSREALNAME_CURMAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("sysrealname_lookread: con\n") ;
#endif

	if (op->call.fetcher != NULL) {
	    IPASSWD_CUR	*icp = curp->scp ;
	    const int	rsn = SR_NOTFOUND ;
	    int		sn = curp->sn ;
	    int		fo = curp->fo;
	    cchar	**sa = curp->sa ;
#if	CF_DEBUGS
	    debugprintf("sysrealname_lookparts: icp->magic{%p}\n",
		icp->magic) ;
#endif
	    if ((rs = (*op->call.fetcher)(op->obj,icp,fo,rbuf,sa,sn)) == rsn) {
		rs = SR_OK ;
	    }
	} else {
	    rs = SR_NOSYS ;
	}

#if	CF_DEBUGS
	debugprintf("sysrealname_lookread: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysrealname_lookread) */


int sysrealname_enum(SYSREALNAME *op,SYSREALNAME_CUR *curp,char *ubuf,
		cchar **sa,char *rbuf,int rlen)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (sa == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != SYSREALNAME_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != SYSREALNAME_CURMAGIC) return SR_NOTOPEN ;

	if (op->call.enumerate != NULL) {
	    IPASSWD_CUR	*icp = curp->scp ;
	    rs = (*op->call.enumerate)(op->obj,icp,ubuf,sa,rbuf,rlen) ;
	} else {
	    rs = SR_NOTAVAIL ;
	}

	return rs ;
}
/* end subroutine (sysrealname_enum) */


int sysrealname_audit(SYSREALNAME *op)
{
	int		rs = SR_NOTAVAIL ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SYSREALNAME_MAGIC) return SR_NOTOPEN ;

	if (op->call.audit != NULL) {
	    rs = (*op->call.audit)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (sysrealname_audit) */


/* private subroutines */


/* find and load the DB-access object */
static int sysrealname_objloadbegin(SYSREALNAME *op,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	int		rs ;
	int		rs1 ;
	int		vo ;
	const char	*pr = SYSREALNAME_PR ;

#if	CF_DEBUGS
	debugprintf("sysrealname_objloadbegin: pr=%s\n",pr) ;
	debugprintf("sysrealname_objloadbegin: objname=%s\n",objname) ;
#endif

	vo = VECSTR_OCOMPACT ;
	if ((rs = vecstr_start(&syms,n,vo)) >= 0) {
	    int		i ;
	    int		snl ;
	    int		f_modload = FALSE ;
	    char	symname[SYMNAMELEN + 1] ;

	    for (i = 0 ; (i < n) && (subs[i] != NULL) ; i += 1) {
	        if (isrequired(i)) {
	            rs = sncpy3(symname,SYMNAMELEN,objname,"_",subs[i]) ;
		    snl = rs ;
		    if (rs >= 0) {
			rs = vecstr_add(&syms,symname,snl) ;
		    }
		}
		if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
		const char	**sv ;
	        if ((rs = vecstr_getvec(&syms,&sv)) >= 0) {
		    int		mo ;
		    const char	*mn = SYSREALNAME_MODBNAME ;
	            mo = (MODLOAD_OLIBVAR | MODLOAD_OPRS | MODLOAD_OSDIRS) ;
	            if ((rs = modload_open(lp,pr,mn,objname,mo,sv)) >= 0) {
			int	mva[2] ;
			f_modload = TRUE ;
	                if ((rs = modload_getmva(lp,mva,2)) >= 0) {
	                    const int	objsize = mva[0] ;
	                    const int	cursize = mva[1] ;
	                    if ((rs = uc_malloc(objsize,&op->obj)) >= 0) {
			        const char	*on = objname ;
	                        if ((rs = sysrealname_loadcalls(op,on)) >= 0) {
				    op->objsize = objsize ;
				    op->cursize = cursize ;
				}
	                        if (rs < 0) {
	                            uc_free(op->obj) ;
	                            op->obj = NULL ;
	                        }
			    }
	                } /* end if (modload_getmv) */
		        if (rs < 0) {
			    f_modload = FALSE ;
		            modload_close(lp) ;
			}
	            } /* end if (modload_open) */
	        } /* end if (vecstr_getvec) */
	    } /* end if (ok) */

	    rs1 = vecstr_finish(&syms) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && f_modload) {
		modload_close(lp) ;
	    }
	} /* end if (allocation) */

#if	CF_DEBUGS
	debugprintf("sysrealname_objloadbegin: sysrealname_loadcalls() rs=%d\n",
		rs) ;
#endif

	return rs ;
}
/* end subroutine (sysrealname_objloadbegin) */


static int sysrealname_objloadend(SYSREALNAME *op)
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
/* end subroutine (sysrealname_objloadend) */


static int sysrealname_loadcalls(SYSREALNAME *op,cchar objname[])
{
	MODLOAD		*lp = &op->loader ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;
	char		symname[SYMNAMELEN + 1] ;
	const void	*snp ;

	for (i = 0 ; subs[i] != NULL ; i += 1) {

	    rs = sncpy3(symname,SYMNAMELEN,objname,"_",subs[i]) ;
	    if (rs < 0) break ;

	    rs1 = modload_getsym(lp,symname,&snp) ;

	    if (rs1 == SR_NOTFOUND) {
		snp = NULL ;
		if (isrequired(i)) break ;
	    } else {
		rs = rs1 ;
	    }

	    if (rs < 0) break ;

#if	CF_DEBUGS
	    debugprintf("sysrealname_loadcalls: call=%s %c\n",
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

		case sub_info:
		    op->call.info = (int (*)(void *,IPASSWD_INFO *)) snp ;
		    break ;

		case sub_curbegin:
		    op->call.curbegin = (int (*)(void *,IPASSWD_CUR *)) snp ;
		    break ;

		case sub_curend:
		    op->call.curend = (int (*)(void *,IPASSWD_CUR *)) snp ;
		    break ;

		case sub_fetcher:
		    op->call.fetcher = 
			(int (*)(void *,IPASSWD_CUR *,int,
			char *,const char **,int)) snp ;
		    break ;

		case sub_enum:
		    op->call.enumerate = 
			(int (*)(void *,IPASSWD_CUR *,
			char *,const char **,char *,int)) snp ;
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
	debugprintf("sysrealname_loadcalls: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (sysrealname_loadcalls) */


static int sysrealname_curload(SYSREALNAME *op,SYSREALNAME_CUR *curp,
		int fo,cchar **sa,int sn)
{
	int		rs ;
	int		i ;
	int		sasize ;
	int		ssize = 0 ;
	char		*bp ;

	if (op == NULL) return SR_FAULT ;
	if (sa == NULL) return SR_FAULT ;

	if (sn < 0) {
	    for (sn = 0 ; sa[sn] != NULL ; sn += 1) ;
	}

	sasize = ((sn+1) * sizeof(const char *)) ;
	ssize += sasize ;
	for (i = 0 ; i < sn ; i += 1) {
	    ssize += (strlen(sa[i]) + 1) ;
	}

	if ((rs = uc_malloc(ssize,&bp)) >= 0) {
	    const char	**sap = (const char **) bp ;
	    char	*sbp = (bp + sasize) ;
	    curp->fo = fo ;
	    curp->sa = (const char **) bp ;
	    curp->sn = sn ;
	    for (i = 0 ; i < sn ; i += 1) {
		sap[i] = sbp ;
		sbp = (strwcpy(sbp,sa[i],-1) + 1) ;
	    }
	    sap[i] = NULL ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (sysrealname_curload) */


static int isrequired(int i)
{
	int		f = FALSE ;

	switch (i) {
	case sub_open:
	case sub_curbegin:
	case sub_curend:
	case sub_fetcher:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */

	return f ;
}
/* end subroutine (isrequired) */


