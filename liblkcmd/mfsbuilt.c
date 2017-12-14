/* mfs-built */

/* built-in services */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine was originally written.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSBUILT.

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module is responsible for providing some built-in services for the
        MFSBUILT server.


*******************************************************************************/


#define	MFSBUILT_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<dlfcn.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<fsdir.h>
#include	<localmisc.h>

#include	"mfsbuilt.h"
#include	"mfserve.h"


/* local defines */

#define	ENT		struct mfsbuilt_ent

#ifndef	SYMBUFLEN
#define	SYMBUFLEN	MAXNAMELEN
#endif


/* type-defs */

typedef int	(*objstart_t)(void *,cchar *,void *,cchar **,cchar **) ;
typedef int	(*objcheck_t)(void *) ;
typedef int	(*objabort_t)(void *) ;
typedef int	(*objfinish_t)(void *) ;


/* external subroutines */

extern int	pathadd(char *,int,cchar *) ;
extern int	mktmpfile(char *,mode_t,cchar *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	chmods(const char *,mode_t) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	hasNotDots(cchar *,int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* external variables */


/* global variables */


/* local structures */

struct mfsbuilt_ent {
	cchar		*a ;		/* allocation */
	cchar		*svc ;		/* service name */
	cchar		*fname ;	/* file (component) name */
	void		*sop ;		/* shared object pointer (handle) */
	MFSERVE_INFO	info ;
	int		rcount ;	/* reference count */
	int		objsize ;	/* size of object */
} ;


/* forward references */

static int	mfsbuilt_entprune(MFSBUILT *) ;
static int	mfsbuilt_entload(MFSBUILT *) ;
static int	mfsbuilt_ent(MFSBUILT *,cchar *,int,cchar *,int) ;
static int	mfsbuilt_fins(MFSBUILT *) ;

static int	ent_start(ENT *,cchar *,int,cchar *,int) ;
static int	ent_getsvc(ENT *,cchar **) ;
static int	ent_loadbegin(ENT *,MFSERVE_INFO *) ;
static int	ent_loadend(ENT *) ;
static int	ent_loadinfo(ENT *) ;
static int	ent_getinfo(ENT *,MFSERVE_INFO *) ;
static int	ent_isnotloaded(ENT *) ;
static int	ent_finish(ENT *) ;

#ifdef	COMMENT
static int	mkfile(cchar *,cchar **) ;
#endif /* COMMENT */

static int	hasFileType(cchar *,int) ;


/* local variables */

static cchar	*exts[] = {
	"so",
	"o",
	NULL
} ;

static cchar	*methods[] = {
	"start",
	"check",
	"abort",
	"finish",
	NULL
} ;


/* exported subroutines */


int mfsbuilt_start(MFSBUILT *op,cchar *dname)
{
	int		rs ;
	int		c = 0 ;
	cchar		*cp ;

#if	CF_DEBUGS
	debugprintf("mfsbuilt_start: ent dn=%s\n",dname) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (dname == NULL) return SR_FAULT ;

	memset(op,0,sizeof(MFSBUILT)) ;

	if ((rs = uc_mallocstrw(dname,-1,&cp)) >= 0) {
	    HDB		*dbp = &op->db ;
	    const int	n = MFSBUILT_NENTS ;
	    const int	at = FALSE ;
	    op->dname = cp ;
	    if ((rs = hdb_start(dbp,n,at,NULL,NULL)) >= 0) {
		if ((rs = mfsbuilt_entload(op)) >= 0) {
		    c = rs ;
		    op->ti_check = time(NULL) ;
		    op->magic = MFSBUILT_MAGIC ;
		}
		if (rs < 0) {
		    mfsbuilt_fins(op) ;
		    hdb_finish(dbp) ;
		}
	    } /* end if (hdb_start) */
	    if (rs < 0) {
		uc_free(op->dname) ;
		op->dname = NULL ;
	    }
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("mfsbuilt_start: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mfsbuilt_start) */


int mfsbuilt_finish(MFSBUILT *op)
{
	HDB		*dbp ;
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("mfsbuilt_finish: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;

	rs1 = mfsbuilt_fins(op) ;
	if (rs >= 0) rs = rs1 ;

	dbp = &op->db ;
	rs1 = hdb_finish(dbp) ;
	if (rs >= 0) rs = rs1 ;

	if (op->dname != NULL) {
	    rs1 = uc_free(op->dname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("mfsbuilt_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (mfsbuilt_finish) */


/* 0=no, 1=yes */
int mfsbuilt_have(MFSBUILT *op,cchar *sp,int sl)
{
	HDB		*dbp ;
	HDB_DATUM	k, v ;
	int		rs ;
	int		f = 0 ;
#if	CF_DEBUGS
	debugprintf("mfsbuilt_have: ent s=%t\n",sp,sl) ;
#endif
	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	dbp = &op->db ;
	if (sl < 0) sl = strlen(sp) ;
	k.buf = sp ;
	k.len = sl ;
	if ((rs = hdb_fetch(dbp,k,NULL,&v)) >= 0) {
	    f = TRUE ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
#if	CF_DEBUGS
	debugprintf("mfsbuilt_have: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mfsbuilt_have) */


/* 0=no, 1=yes */
int mfsbuilt_loadbegin(MFSBUILT *op,MFSERVE_INFO *ip,cchar *sp,int sl)
{
	HDB		*dbp ;
	HDB_DATUM	k, v ;
	int		rs ;
	int		f = FALSE ;
#if	CF_DEBUGS
	    debugprintf("mfsbuilt_loadbegin: ent s=%t\n",sp,sl) ;
#endif
	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	dbp = &op->db ;
	if (sl < 0) sl = strlen(sp) ;
	k.buf = sp ;
	k.len = sl ;
	if ((rs = hdb_fetch(dbp,k,NULL,&v)) >= 0) {
	    ENT		*ep = (ENT *) v.buf ;
	    f = TRUE ;
	    rs = ent_loadbegin(ep,ip) ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
#if	CF_DEBUGS
	    debugprintf("mfsbuilt_loadbegin: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mfsbuilt_loadbegin) */


int mfsbuilt_loadend(MFSBUILT *op,cchar *sp,int sl)
{
	HDB		*dbp ;
	HDB_DATUM	k, v ;
	int		rs ;
	int		f = FALSE ;
	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	dbp = &op->db ;
	if (sl < 0) sl = strlen(sp) ;
	k.buf = sp ;
	k.len = sl ;
	if ((rs = hdb_fetch(dbp,k,NULL,&v)) >= 0) {
	    ENT	*ep = (ENT *) v.buf ;
	    f = TRUE ;
	    rs = ent_loadend(ep) ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mfsbuilt_loadend) */


int mfsbuilt_count(MFSBUILT *op)
{
	HDB		*dbp ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	dbp = &op->db ;
	return hdb_count(dbp) ;
}
/* end subroutine (mfsbuilt_count) */


int mfsbuilt_curbegin(MFSBUILT *op,MFSBUILT_CUR *curp)
{
	HDB		*dbp ;
	HDB_CUR		*hcp ;
	int		rs ;
#if	CF_DEBUGS
	debugprintf("mfsbuilt_curbegin: ent\n") ;
#endif
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	dbp = &op->db ;
	hcp = &curp->hcur ;
	if ((rs = hdb_curbegin(dbp,hcp)) >= 0) {
	    curp->magic = MFSBUILT_MAGIC ;
	}
#if	CF_DEBUGS
	debugprintf("mfsbuilt_curbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfsbuilt_curbegin) */


int mfsbuilt_curend(MFSBUILT *op,MFSBUILT_CUR *curp)
{
	HDB		*dbp ;
	HDB_CUR		*hcp ;
	int		rs = SR_OK ;
	int		rs1 ;
#if	CF_DEBUGS
	debugprintf("mfsbuilt_curend: ent\n") ;
#endif
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	dbp = &op->db ;
	hcp = &curp->hcur ;
	rs1 = hdb_curend(dbp,hcp) ;
	if (rs >= 0) rs = rs1 ;
	curp->magic = 0 ;
#if	CF_DEBUGS
	debugprintf("mfsbuilt_curend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfsbuilt_curend) */


int mfsbuilt_enum(MFSBUILT *op,MFSBUILT_CUR *curp,char *rbuf,int rlen)
{
	HDB		*dbp ;
	HDB_CUR		*hcp ;
	HDB_DATUM	k, v ;
	int		rs ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	dbp = &op->db ;
	hcp = &curp->hcur ;
	if ((rs = hdb_enum(dbp,hcp,&k,&v)) >= 0) {
	    const int	sl = k.len ;
	    cchar	*sp = (cchar *) k.buf ;
	    rs = snwcpy(rbuf,rlen,sp,sl) ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
#if	CF_DEBUGS
	debugprintf("mfsbuilt_enum: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (mfsbuilt_enum) */


int mfsbuilt_strsize(MFSBUILT *op)
{
	HDB		*dbp ;
	HDB_CUR		hcur ;
	HDB_DATUM	k, v ;
	int		rs ;
	int		rs1 ;
	int		size = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	dbp = &op->db ;
	if ((rs = hdb_curbegin(dbp,&hcur)) >= 0) {
	    while ((rs1 = hdb_enum(dbp,&hcur,&k,&v)) >= 0) {
	        cchar	*sp = (cchar *) k.buf ;
		size += (strlen(sp)+1) ;
	    } /* end while */
	    if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	    rs1 = hdb_curend(dbp,&hcur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (hdb-cur) */
	return (rs >= 0) ? size : rs ;
}
/* end subroutine (mfsbuilt_strsize) */


int mfsbuilt_strvec(MFSBUILT *op,cchar **va,char *rbuf,int rlen)
{
	HDB		*dbp ;
	HDB_CUR		hcur ;
	HDB_DATUM	k, v ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	int		rl = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (va == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	dbp = &op->db ;
	if ((rs = hdb_curbegin(dbp,&hcur)) >= 0) {
	    char	*bp = rbuf ;
	    while ((rs1 = hdb_enum(dbp,&hcur,&k,&v)) >= 0) {
	        cchar	*sp = (cchar *) k.buf ;
		va[c++] = bp ;
		rs = sncpy1(bp,(rlen-rl),sp) ;
		rl += (rs+1) ;
		bp += (rs+1) ;
		if (rs < 0) break ;
	    } /* end while */
	    va[c] = NULL ;
	    if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	    rs1 = hdb_curend(dbp,&hcur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (hdb-cur) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mfsbuilt_strvec) */


int mfsbuilt_check(MFSBUILT *op,time_t dt)
{
	const int	to = MFSBUILT_INTCHECK ;
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	if (dt == 0) dt = time(NULL) ;
	if ((dt - op->ti_check) >= to) {
	    op->ti_check = dt ;
	    if ((rs = mfsbuilt_entprune(op)) >= 0) {
		rs = mfsbuilt_entload(op) ;
	    }
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mfsbuilt_check) */


/* private subroutines */


static int mfsbuilt_entprune(MFSBUILT *op)
{
	HDB		*dbp = &op->db ;
	HDB_DATUM	k, v ;
	HDB_CUR		c ;
	int		rs ;
	int		rs1 ;
	int		rs2 ;

#if	CF_DEBUGS
	debugprintf("mfsbuilt_entprune: ent\n") ;
#endif

	if ((rs = hdb_curbegin(dbp,&c)) >= 0) {
	    ENT		*ep ;
	    const int	rsn = SR_NOTFOUND ;
	    int		i ;
	    for (i = 0 ; (rs2 = hdb_enum(dbp,&c,&k,&v)) >= 0 ; i += 1) {
		ep = (ENT *) v.buf ;
		if (ep != NULL) {
		    if ((rs = ent_isnotloaded(ep)) > 0) {
		        rs1 = hdb_delcur(dbp,&c,0) ;
		        if (rs >= 0) rs = rs1 ;
		        rs1 = ent_finish(ep) ;
		        if (rs >= 0) rs = rs1 ;
		        rs1 = uc_free(ep) ;
		        if (rs >= 0) rs = rs1 ;
		    }
		}
	    } /* end for */
	    if ((rs >= 0) && (rs2 != rsn)) rs = rs2 ;
	    rs1 = hdb_curend(dbp,&c) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (hdb-cur) */

#if	CF_DEBUGS
	debugprintf("mfsbuilt_entprunce: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mfsbuilt_entprune) */


static int mfsbuilt_entload(MFSBUILT *op)
{
	FSDIR		d ;
	FSDIR_ENT	de ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
#if	CF_DEBUGS
	debugprintf("mfsbuilt_entload: ent dn=%s\n",op->dname) ;
#endif
	if ((rs = fsdir_open(&d,op->dname)) >= 0) {
	    char	pbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath1(pbuf,op->dname)) >= 0) {
	        const int	plen = rs ;
	        while ((rs = fsdir_read(&d,&de)) > 0) {
		    cchar	*ep = de.name ;
		    int		el = rs ;
		    if (hasNotDots(ep,el)) {
		        int	sl ;
		        if ((sl = hasFileType(ep,el)) > 0) {
			    if ((rs = pathadd(pbuf,plen,ep)) >= 0) {
				rs = mfsbuilt_ent(op,ep,sl,pbuf,rs) ;
				c += rs ;
			    }
		        }
		    }
		    if (rs < 0) break ;
	        } /* end while */
	    } /* end if (mkpath) */
	    rs1 = fsdir_close(&d) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (fsdir) */
#if	CF_DEBUGS
	debugprintf("mfsbuilt_entload: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mfsbuilt_entload) */


static int mfsbuilt_ent(MFSBUILT *op,cchar *sp,int sl,cchar *pp,int pl)
{
	HDB		*dbp = &op->db ;
	HDB_DATUM	k, v ;
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	int		c = 0 ;
#if	CF_DEBUGS
	debugprintf("mfsbuilt_ent: ent s=%t\n",sp,sl) ;
	debugprintf("mfsbuilt_ent: p=%t\n",pp,pl) ;
#endif
	k.buf = sp ;
	k.len = sl ;
	if ((rs = hdb_fetch(dbp,k,NULL,&v)) == rsn) {
	    const int	am = (R_OK|X_OK) ;
	    if ((rs = uc_access(pp,am)) >= 0) {
	        ENT		*ep ;
	        const int	esize = sizeof(ENT) ;
	        if ((rs = uc_malloc(esize,&ep)) >= 0) {
	            if ((rs = ent_start(ep,sp,sl,pp,pl)) >= 0) {
		        cchar	*svc ;
		        if ((rs = ent_getsvc(ep,&svc)) >= 0) {
			    sl = rs ;
		            c += 1 ;
		            k.buf = svc ;
		            k.len = sl ;
		            v.buf = ep ;
		            v.len = esize ;
		            rs = hdb_store(dbp,k,v) ;
		            if (rs < 0)
		                ent_finish(ep) ;
	                } /* end if (ent_getsvc) */
	            } /* end if (ent_start) */
	            if (rs < 0)
		        uc_free(ep) ;
	        } /* end if (m-a) */
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    } /* end if (uc_access) */
	} /* end if (hdb_fetch) */
#if	CF_DEBUGS
	debugprintf("mfsbuilt_ent: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (mfsbuilt_ent) */


static int mfsbuilt_fins(MFSBUILT *op)
{
	HDB		*dbp = &op->db ;
	HDB_DATUM	k, v ;
	HDB_CUR		c ;
	int		rs ;
	int		rs1 ;
	int		rs2 ;

#if	CF_DEBUGS
	debugprintf("mfsbuilt_fins: ent\n") ;
#endif

	if ((rs = hdb_curbegin(dbp,&c)) >= 0) {
	    ENT		*ep ;
	    const int	rsn = SR_NOTFOUND ;
	    int		i ;
	    for (i = 0 ; (rs2 = hdb_enum(dbp,&c,&k,&v)) >= 0 ; i += 1) {
		ep = (ENT *) v.buf ;
		if (ep != NULL) {
#ifdef	OPTIONAL
		    rs1 = hdb_delcur(dbp,&c,0) ; /* why is this needed? */
		    if (rs >= 0) rs = rs1 ;
#endif
		    rs1 = ent_finish(ep) ;
		    if (rs >= 0) rs = rs1 ;
		    rs1 = uc_free(ep) ;
		    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGS
		    debugprintf("mfsbuilt_fins: for-bot rs=%d\n",rs) ;
#endif
		}
	    } /* end for */
	    if ((rs >= 0) && (rs2 != rsn)) rs = rs2 ;
	    rs1 = hdb_curend(dbp,&c) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (hdb-cur) */

#if	CF_DEBUGS
	debugprintf("mfsbuilt_fins: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mfsbuilt_fins) */


static int ent_start(ENT *ep,cchar *sp,int sl,cchar *fp,int fl)
{
	int		rs ;
	int		size = 0 ;
	char		*bp ;
	if (sl < 0) sl = strlen(sp) ;
	if (fl < 0) fl = strlen(fp) ;
	size += (sl+1) ;
	size += (fl+1) ;
	memset(ep,0,sizeof(ENT)) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    ep->a = bp ;
	    ep->svc = bp ;
	    bp = (strwcpy(bp,sp,sl)+1) ;
	    ep->fname = bp ;
	    bp = (strwcpy(bp,fp,fl)+1) ;
	}
	return rs ;
}
/* end subroutine (ent_start) */


static int ent_finish(ENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep->sop != NULL) {
	    dlclose(ep->sop) ;
	    ep->sop = NULL ;
	    ep->rcount = 0 ;
	}

	if (ep->a != NULL) {
	    rs1 = uc_free(ep->a) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->a = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("ent_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ent_finish) */


static int ent_getsvc(ENT *ep,cchar **rpp)
{
	int		rs ;
	rs = strlen(ep->svc) ;
	if (rpp != NULL) *rpp = ep->svc ;
	return rs ;
}
/* end subroutine (ent_getsvc) */


/* ARGSUSED */
static int ent_loadbegin(ENT *ep,MFSERVE_INFO *ip)
{
	int		rs = SR_OK ;
#if	CF_DEBUGS
	debugprintf("ent_loadbegin: ent\n") ;
	debugprintf("ent_loadbegin: fn=%s\n",ep->fname) ;
#endif
	if (ep->sop == NULL) {
	    const int	m = RTLD_LAZY ;
	    void	*sop ;
	    if ((sop = dlopen(ep->fname,m)) != NULL) {
		ep->sop = sop ;
		ep->rcount = 1 ;
		if ((rs = ent_loadinfo(ep)) >= 0) {
	    	    rs = ent_getinfo(ep,ip) ;
		}
	    } else {
#if	CF_DEBUGS
		{
		    cchar	*sp = dlerror() ;
		    debugprintf("ent_loadbegin: dlopen-failed\n") ;
		    debugprintf("ent_loadbegin: err=%s\n",sp) ;
		}
#endif
		rs = SR_LIBACC ;
	    }
	} else {
	    ep->rcount += 1 ; /* reference count */
	    rs = ent_getinfo(ep,ip) ;
	}
#if	CF_DEBUGS
	    debugprintf("ent_loadbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (ent_loadbegin) */


static int ent_loadend(ENT *ep)
{
	int		rs = SR_OK ;
	if ((ep->sop != NULL) && (ep->rcount > 0)) {
	    ep->rcount -= 1 ;
	    if (ep->rcount == 0) {
		dlclose(ep->sop) ;
		ep->sop = NULL ;
	    }
	}
	return rs ;
}
/* end subroutine (ent_loadend) */


static int ent_loadinfo(ENT *ep)
{
	int		rs = SR_OK ;
	cchar		*svc = ep->svc ;
	void		*p ;
#if	CF_DEBUGS
	debugprintf("ent_loadinfo: ent svc=%s\n",svc) ;
#endif
	if ((p = dlsym(ep->sop,svc)) != NULL) {
	    MFSERVE_MOD	*mp = (MFSERVE_MOD *) p ;
	    int		i ;
	    ep->info.objsize = mp->objsize ;
	    for (i = 0 ; (rs >= 0) && (methods[i] != NULL) ; i += 1) {
	    	const int	slen = SYMBUFLEN ;
		cchar		*m = methods[i] ;
	    	char		sbuf[SYMBUFLEN+1] ;
		if ((rs = sncpy3(sbuf,slen,svc,"_",m)) >= 0) {
	    	    caddr_t	symp ;
		    if ((symp = (caddr_t) dlsym(ep->sop,sbuf)) != NULL) {
#if	CF_DEBUGS
	    		debugprintf("ent_loadinfo: sn=%s symp{%p}\n",
				sbuf,symp) ;
#endif
		        switch (i) {
		        case 0:
			    ep->info.start = (objstart_t) symp ;
		            break ;
		        case 1:
			    ep->info.check = (objcheck_t) symp ;
		            break ;
		        case 2:
			    ep->info.abort = (objabort_t) symp ;
		            break ;
		        case 3:
			    ep->info.finish = (objfinish_t) symp ;
		            break ;
		        } /* end switch */
		    } else {
#if	CF_DEBUGS
	    		debugprintf("ent_loadinfo: no-sym\n") ;
#endif
			rs = SR_LIBACC ;
		    }
		} /* end if (sncpy) */
	    } /* end for */
	} else {
#if	CF_DEBUGS
	debugprintf("ent_loadinfo: no OBJ sym\n") ;
#endif
	    rs = SR_LIBACC ;
	}
#if	CF_DEBUGS
	debugprintf("ent_loadinfo: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (ent_loadinfo) */


static int ent_getinfo(ENT *ep,MFSERVE_INFO *ip)
{
	int		rs = SR_OK ;
	if (ep->info.objsize != 0) {
	    *ip = ep->info ;
	    rs = 1 ;
	} else {
	    memset(ip,0,sizeof(MFSERVE_INFO)) ;
	}
	return rs ;
}
/* end subroutine (ent_getinfo) */


static int ent_isnotloaded(ENT *ep)
{
	return (ep->info.objsize == 0) ;
}
/* end subrouine (ent_isnotloaded) */


#ifdef	COMMENT
static int mkfile(cchar *template,cchar **rpp)
{
	int		rs ;
	int		tl = 0 ;
	char		tbuf[MAXPATHLEN + 1] ;

	tbuf[0] = '\0' ;
	if ((rs = mktmpfile(tbuf,0666,template)) >= 0) {
	    int	tl = rs ;
	    rs = uc_mallocstrw(tbuf,tl,rpp) ;
	    if (rs < 0) {
	        u_unlink(tbuf) ;
		*rpp = NULL ;
	    } /* end if (error-recovery) */
	} /* end if (mktmpfile) */

	return (rs >= 0) ? tl : rs ;
}
/* end subroutines (mkfile) */
#endif /* COMMENT */


static int hasFileType(cchar *sp,int sl)
{
	cchar		*tp ;
	if (sl < 0) sl = strlen(sp) ;
	if ((tp = strnchr(sp,sl,'.')) != NULL) {
	    cchar	*ep = (tp+1) ;
	    const int	el = ((sp+sl)-(tp+1)) ;
	    sl = (tp-sp) ;
	    if (matstr(exts,ep,el) < 0) {
		sl = 0 ;
	    }
	}
	return sl ;
}
/* end subroutine (hasFileType) */


