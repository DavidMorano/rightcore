/* sysvar */

/* SYSVAR management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_LOOKSELF	0		/* try searching "SELF" for SO */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module implements an interface (a trivial one) that provides access
        to the SYSVAR object (which is dynamically loaded).


*******************************************************************************/


#define	SYSVAR_MASTER	1


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
#include	<ids.h>
#include	<vecstr.h>
#include	<nulstr.h>
#include	<storebuf.h>
#include	<dirseen.h>
#include	<localmisc.h>

#include	"sysvar.h"


/* local defines */

#define	SYSVAR_DEFCUR	struct sysvar_defcur
#define	SYSVAR_OBJNAME	"varmks"
#define	SYSVAR_MODBNAME	"varmks"

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#ifndef	DEFINITFNAME
#define	DEFINITFNAME	"/etc/default/init"
#endif

#ifndef	DEFLOGFNAME
#define	DEFLOGFNAME	"/etc/default/login"
#endif

#define	LIBCNAME	"lib"

#define	VARLIBPATH	"LD_LIBRARY_PATH"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#define	NDEFAULTS	20


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,const char *,
			const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	vecstr_envfile(vecstr *,const char *) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(const char *,int,const char *,const char *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct sysvar_defcur {
	int	i ;
} ;


/* forward references */

static int	sysvar_objloadbegin(SYSVAR *,const char *,const char *) ;
static int	sysvar_objloadend(SYSVAR *) ;
static int	sysvar_loadcalls(SYSVAR *,const char *) ;
static int	sysvar_socurbegin(SYSVAR *,SYSVAR_CUR *) ;
static int	sysvar_socurend(SYSVAR *,SYSVAR_CUR *) ;
static int	sysvar_defaults(SYSVAR *) ;
static int	sysvar_procsysdef(SYSVAR *,const char *) ;
static int	sysvar_defcurbegin(SYSVAR *,SYSVAR_CUR *) ;
static int	sysvar_defcurend(SYSVAR *,SYSVAR_CUR *) ;
static int	sysvar_deffetch(SYSVAR *,cchar *,int,
			struct sysvar_defcur *,char *,int) ;
static int	sysvar_defenum(SYSVAR *,SYSVAR_DEFCUR *,
			char *,int,char *,int) ;

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
	sub_audit,
	sub_close,
	sub_overlast
} ;

static const char	*sysfnames[] = {
	DEFINITFNAME,	
	DEFLOGFNAME,
	NULL
} ;

static const char	*wstrs[] = {
	"TZ",
	"LANG",
	"UMASK",
	"PATH",
	"WSTATION",
	NULL
} ;

static const char	*pstrs[] = {
	"LC_",
	NULL
} ;


/* exported subroutines */


int sysvar_open(SYSVAR *op,cchar *pr,cchar *dbname)
{
	int		rs ;
	const char	*objname = SYSVAR_OBJNAME ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("sysvar_open: pr=%s dbname=%s\n",pr,dbname) ;
#endif

	memset(op,0,sizeof(SYSVAR)) ;

	if ((rs = sysvar_objloadbegin(op,pr,objname)) >= 0) {
	    if ((rs = (*op->call.open)(op->obj,pr,dbname)) >= 0) {
	    	op->magic = SYSVAR_MAGIC ;
	    }
	    if (rs < 0) {
		sysvar_objloadend(op) ;
	    }
	} /* end if */

		if (isNotPresent(rs)) {
	    	    if ((rs = sysvar_defaults(op)) >= 0) {
	    		op->magic = SYSVAR_MAGIC ;
		    }
		}

#if	CF_DEBUGS
	debugprintf("sysvar_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysvar_open) */


/* free up the entire vector string data structure object */
int sysvar_close(SYSVAR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SYSVAR_MAGIC) return SR_NOTOPEN ;

	if (op->f.defaults) {
	    rs1 = vecstr_finish(&op->defaults) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    rs = (*op->call.close)(op->obj) ;
	    rs1 = sysvar_objloadend(op) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (sysvar_close) */


int sysvar_audit(SYSVAR *op)
{
	int	rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SYSVAR_MAGIC) return SR_NOTOPEN ;

	if (op->f.defaults) {
	    rs = vecstr_audit(&op->defaults) ;
	} else if (op->call.audit != NULL)
	    rs = (*op->call.audit)(op->obj) ;

	return rs ;
}
/* end subroutine (sysvar_audit) */


int sysvar_count(SYSVAR *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SYSVAR_MAGIC) return SR_NOTOPEN ;

	if (op->f.defaults) {
	    rs = vecstr_count(&op->defaults) ;
	} else if (op->call.count != NULL)
	    rs = (*op->call.count)(op->obj) ;

	return rs ;
}
/* end subroutine (sysvar_count) */


int sysvar_curbegin(SYSVAR *op,SYSVAR_CUR *curp)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SYSVAR_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(SYSVAR_CUR)) ;

	if (op->f.defaults) {
	    rs = sysvar_defcurbegin(op,curp) ;
	} else {
	    rs = sysvar_socurbegin(op,curp) ;
	}

	if (rs >= 0) {
	    curp->magic = SYSVAR_MAGIC ;
	}

	return rs ;
}
/* end subroutine (sysvar_curbegin) */


int sysvar_curend(SYSVAR *op,SYSVAR_CUR *curp)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SYSVAR_MAGIC) return SR_NOTOPEN ;

	if (curp->magic != SYSVAR_MAGIC) return SR_NOTOPEN ;

	if (op->f.defaults) {
	    rs = sysvar_defcurend(op,curp) ;
	} else {
	    rs = sysvar_socurend(op,curp) ;
	}

	curp->magic = 0 ;
	return rs ;
}
/* end subroutine (sysvar_curend) */


/* lookup tags by strings */
int sysvar_fetch(SYSVAR *op,cchar *kp,int kl,SYSVAR_CUR *curp,
		char *vbuf,int vlen)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (op->magic != SYSVAR_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != SYSVAR_MAGIC) return SR_NOTOPEN ;

	if (op->f.defaults) {
	    rs = sysvar_deffetch(op,kp,kl,curp->scp,vbuf,vlen) ;
	} else {
	    rs = (*op->call.fetch)(op->obj,kp,kl,curp->scp,vbuf,vlen) ;
	}

	return rs ;
}
/* end subroutine (sysvar_fetch) */


/* enumerate entries */
int sysvar_enum(op,curp,kbuf,klen,vbuf,vlen)
SYSVAR		*op ;
SYSVAR_CUR	*curp ;
char		kbuf[] ;
int		klen ;
char		vbuf[] ;
int		vlen ;
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (kbuf == NULL) return SR_FAULT ;

	if (op->magic != SYSVAR_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != SYSVAR_MAGIC) return SR_NOTOPEN ;

	if (op->f.defaults) {
	    rs = sysvar_defenum(op,curp->scp,kbuf,klen,vbuf,vlen) ;
	} else if (op->call.enumerate != NULL) {
	    rs = (*op->call.enumerate)(op->obj,curp->scp,kbuf,klen,vbuf,vlen) ;
	}

	return rs ;
}
/* end subroutine (sysvar_enum) */


/* private subroutines */


/* find and load the DB-access object */
static int sysvar_objloadbegin(SYSVAR *op,cchar *pr,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	const int	vo = VECSTR_OCOMPACT ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("sysvar_objloadbegin: ent pr=%s on=%s\n",pr,objname) ;
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
	            const char	*mn = SYSVAR_MODBNAME ;
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
	            rs = sysvar_loadcalls(op,objname) ;
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
	debugprintf("sysvar_objloadbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysvar_objloadbegin) */


static int sysvar_objloadend(SYSVAR *op)
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
/* end subroutine (sysvar_objloadend) */


static int sysvar_loadcalls(SYSVAR *op,cchar *soname)
{
	MODLOAD		*lp = &op->loader ;
	const int	symlen = SYMNAMELEN ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	char		symbuf[SYMNAMELEN + 1] ;
	const void	*snp ;

	for (i = 0 ; subs[i] != NULL ; i += 1) {

	    if ((rs = sncpy3(symbuf,symlen,soname,"_",subs[i])) >= 0) {
	        if ((rs = modload_getsym(lp,symbuf,&snp)) == SR_NOTFOUND) {
	            snp = NULL ;
	            if (! isrequired(i)) rs = SR_OK ;
	        }
	    }

	    if (rs < 0) break ;

#if	CF_DEBUGS
	    debugprintf("sysvar_loadcalls: call=%s %c\n",
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
/* end subroutine (sysvar_loadcalls) */


static int sysvar_socurbegin(SYSVAR *op,SYSVAR_CUR *curp)
{
	int		rs = SR_OK ;

	if (op->call.curbegin != NULL) {
	    void	*p ;
	    if ((rs = uc_malloc(op->cursize,&p)) >= 0) {
		curp->scp = p ;
		rs = (*op->call.curbegin)(op->obj,curp->scp) ;
		if (rs < 0) {
	    	    uc_free(curp->scp) ;
	    	    curp->scp = NULL ;
		}
	    } /* end if (m-a) */
	} else
	    rs = SR_NOSYS ;

	return rs ;
}
/* end subroutine (sysvar_socurbegin) */


static int sysvar_socurend(SYSVAR *op,SYSVAR_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (curp->scp == NULL) return SR_NOTSOCK ;

	if (op->call.curend != NULL) {
	    rs1 = (*op->call.curend)(op->obj,curp->scp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(curp->scp) ;
	    if (rs >= 0) rs = rs1 ;
	    curp->scp = NULL ;
	} else
	    rs = SR_NOSYS ;

	return rs ;
}
/* end subroutine (sysvar_socurend) */


static int sysvar_defaults(SYSVAR *op)
{
	int		rs ;
	int		i ;
	int		f ;

#if	CF_DEBUGS
	debugprintf("sysvar_defaults: ent\n") ;
#endif
	if ((rs = vecstr_start(&op->defaults,NDEFAULTS,0)) >= 0) {
	op->f.defaults = (rs >= 0) ;
	for (i = 0 ; sysfnames[i] != NULL ; i += 1) {
	    rs = sysvar_procsysdef(op,sysfnames[i]) ;
	    f = FALSE ;
	    f = f || (rs == SR_NOENT) ;
	    f = f || (rs == SR_ACCESS) ;
	    if ((rs < 0) && (! f)) break ;
	} /* end for */
	if (rs >= 0) {
	vecstr_sort(&op->defaults,vstrkeycmp) ;
	}
	    if (rs < 0) {
		op->f.defaults = FALSE ;
		vecstr_finish(&op->defaults) ;
	    }
	} /* end if (vecstr_start) */

#if	CF_DEBUGS
	debugprintf("sysvar_defaults: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (sysvar_defaults) */


static int sysvar_procsysdef(SYSVAR *op,cchar *fname)
{
	vecstr		lvars ;
	int		rs ;
	int		rs1 ;

	if ((rs = vecstr_start(&lvars,10,0)) >= 0) {
	    int		i ;
	    int		f ;
	    cchar	*tp, *cp ;
	    if ((rs = vecstr_envfile(&lvars,fname)) >= 0) {
	        for (i = 0 ; vecstr_get(&lvars,i,&cp) >= 0 ; i += 1) {
	            if (cp != NULL) {
	                if ((tp = strchr(cp,'=')) != NULL) {
	                    f = (matstr(wstrs,cp,(tp - cp)) >= 0) ;
	                    f = f || (matpstr(pstrs,10,cp,(tp - cp)) >= 0) ;
	                    if (f) {
	                        rs = vecstr_adduniq(&op->defaults,cp,-1) ;
	                     } /* end if */
		         }
		    } /* end if (non-null) */
		    if (rs < 0) break ;
	        } /* end for */
	    } /* end if (vecstr_envfile) */
	    rs1 = vecstr_finish(&lvars) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (lvars) */

	return rs ;
}
/* end subroutine (sysvar_procsysdef) */


static int sysvar_defcurbegin(SYSVAR *op,SYSVAR_CUR *curp)
{
	struct sysvar_defcur	*dcp ;
	const int	size = sizeof(struct sysvar_defcur) ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if ((rs = uc_malloc(size,&curp->scp)) >= 0) {
	    dcp = (struct sysvar_defcur *) curp->scp ;
	    dcp->i = -1 ;
	}

	return rs ;
}
/* end subroutine (sysvar_defcurbegin) */


static int sysvar_defcurend(SYSVAR *op,SYSVAR_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (curp->scp == NULL) return SR_NOTSOCK ;

	rs1 = uc_free(curp->scp) ;
	if (rs >= 0) rs = rs1 ;
	curp->scp = NULL ;

	return rs ;
}
/* end subroutine (sysvar_defcurend) */


static int sysvar_deffetch(op,kp,kl,dcp,vbuf,vlen)
SYSVAR		*op ;
const char	*kp ;
int		kl ;
struct sysvar_defcur	*dcp ;
char		vbuf[] ;
int		vlen ;
{
	NULSTR		ns ;
	int		rs ;
	int		rs1 ;
	int		vl = 0 ;
	const char	*key ;

	if (vbuf != NULL)
	    vbuf[0] = '\0' ;

	if ((rs = nulstr_start(&ns,kp,kl,&key)) >= 0) {
	    int		i ;
	    const char	*cp ;

	    i = (dcp->i >= 0) ? (dcp->i + 1) : 0 ;

	    while ((rs = vecstr_get(&op->defaults,i,&cp)) >= 0) {
	        if (strkeycmp(key,cp) == 0) break ;
	        i += 1 ;
	    } /* end while */

	    if (rs >= 0) {
	        const char	*tp, *vp ;

	        vp = NULL ;
	        if ((tp = strchr(cp,'=')) != NULL) {
		    vp = (tp + 1) ;
		}

	        if (vp != NULL) {
	            if (vbuf != NULL) {
		        rs = sncpy1(vbuf,vlen,vp) ;
		        vl = rs ;
	            } else
		        vl = strlen(vp) ;
	        }

	        if (rs >= 0)
	            dcp->i = i ;

	    } /* end if */

	    rs1 = nulstr_finish(&ns) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nul-string) */

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (sysvar_deffetch) */


static int sysvar_defenum(op,dcp,kbuf,klen,vbuf,vlen)
SYSVAR		*op ;
SYSVAR_DEFCUR	*dcp ;
char		kbuf[] ;
int		klen ;
char		vbuf[] ;
int		vlen ;
{
	int		rs = SR_OK ;
	int		i ;
	int		kl ;
	int		vl = 0 ;
	const char	*tp, *vp ;
	const char	*cp ;

#ifdef	OPTIONAL
	if (vbuf != NULL)
	    vbuf[0] = '\0' ;
#endif

	i = (dcp->i >= 0) ? (dcp->i + 1) : 0 ;

	while ((rs = vecstr_get(&op->defaults,i,&cp)) >= 0) {
	    if (cp != NULL) break ;
	    i += 1 ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("sysvar_defenum: mid rs=%d\n",rs) ;
	if (rs >= 0)
	debugprintf("sysvar_defenum: cp=>%t<\n",cp,strnlen(cp,40)) ;
#endif

	if (rs >= 0) {

	    kl = -1 ;
	    vp = NULL ;
	    if ((tp = strchr(cp,'=')) != NULL) {
		vp = (tp + 1) ;
		kl = (tp - cp) ;
	    }

	    if (kbuf != NULL) {
		rs = snwcpy(kbuf,klen,cp,kl) ;
	    }

	    if ((rs >= 0) && (vp != NULL)) {
	        if (vbuf != NULL) {
		    rs = sncpy1(vbuf,vlen,vp) ;
		    vl = rs ;
	        } else {
		    vl = strlen(vp) ;
		}
	    }

	    if (rs >= 0)
	        dcp->i = i ;

	} else if (vbuf != NULL) {
	    vbuf[0] = '\0' ;
	}

#if	CF_DEBUGS
	debugprintf("sysvar_defenum: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (sysvar_defenum) */


static int isrequired(int i)
{
	int		f = FALSE ;

	switch (i) {
	case sub_open:
	case sub_curbegin:
	case sub_fetch:
	case sub_enum:
	case sub_curend:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */

	return f ;
}
/* end subroutine (isrequired) */


