/* mfsbuilt */

/* built-in services */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine was originally written.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module is responsible for providing some built-in services
	for the MFSERVE server.


*******************************************************************************/


#define	MFSBUILT_MASTER	0


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
#include	<fsdir.h>
#include	<localmisc.h>

#include	"mfsbuilt.h"


/* local defines */

#define	ENT		struct mfsbuilt_ent


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	pathadd(char *,int,cchar *) ;
extern int	mktmpfile(char *,mode_t,cchar *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	chmods(const char *,mode_t) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	hasNotDots(cchar *,int) ;
extern int	isNotPresent(int) ;

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
	int		rcount ;	/* reference count */
} ;


/* forward references */

static int	mfsbuilt_entdump(MFSBUILT *) ;
static int	mfsbuilt_entload(MFSBUILT *) ;
static int	mfsbuilt_ent(MFSBUILT *,cchar *,int,cchar *,int) ;
static int	mfsbuilt_fins(MFSBUILT *) ;

static int	ent_start(ENT *,cchar *,int,cchar *,int) ;
static int	ent_load(ENT *,void *) ;
static int	ent_release(ENT *) ;
static int	ent_finish(ENT *) ;

#ifdef	COMMENT
static int	mkfile(cchar *,cchar **) ;
#endif /* COMMENT */

static int	hasService(cchar *,int) ;


/* local variables */

static cchar	*exts[] = {
	"so",
	"o",
	NULL
} ;


/* exported subroutines */


int mfsbuilt_start(MFSBUILT *op,cchar *dname)
{
	int		rs ;
	cchar		*cp ;

	if (op == NULL) return SR_FAULT ;
	if (dname == NULL) return SR_FAULT ;

	memset(op,0,sizeof(MFSBUILT)) ;

	if ((rs = uc_mallocstrw(dname,-1,&cp)) >= 0) {
	    HDB		*dbp = &op->db ;
	    const int	n = MFSBUILT_NENTS ;
	    const int	at = FALSE ;
	    op->dname = cp ;
	    if ((rs = hdb_start(dbp,n,at,NULL,NULL)) >= 0) {
		if ((rs = mfsbuilt_entdump(op)) >= 0) {
		    if ((rs = mfsbuilt_entload(op)) >= 0) {
		        op->magic = MFSBUILT_MAGIC ;
		    }
		}
		if (rs < 0) {
		    mfsbuilt_fins(op) ;
		    hdb_finish(dbp) ;
		}
	    }
	    if (rs < 0) {
		uc_free(op->dname) ;
		op->dname = NULL ;
	    }
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (mfsbuilt_start) */


int mfsbuilt_finish(MFSBUILT *op)
{
	HDB		*dbp ;
	int		rs = SR_OK ;
	int		rs1 ;

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

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (mfsbuilt_finish) */


int mfsbuilt_load(MFSBUILT *op,cchar *sp,int sl,void *vrp)
{
	HDB		*dbp ;
	HDB_DATUM	k, v ;
	int		rs ;
	int		rl = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (vrp == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	dbp = &op->db ;
	if (sl < 0) sl = strlen(sp) ;
	k.buf = sp ;
	k.len = sl ;
	if ((rs = hdb_fetch(dbp,k,NULL,&v)) >= 0) {
	    rl = v.len ;
	    if (vrp != NULL) {
		ENT	*ep = (ENT *) v.buf ;
		rs = ent_load(ep,vrp) ;
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (mfsbuilt_load) */


int mfsbuilt_release(MFSBUILT *op,cchar *sp,int sl)
{
	HDB		*dbp ;
	HDB_DATUM	k, v ;
	int		rs ;
	int		rl = 0 ;
	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	dbp = &op->db ;
	if (sl < 0) sl = strlen(sp) ;
	k.buf = sp ;
	k.len = sl ;
	if ((rs = hdb_fetch(dbp,k,NULL,&v)) >= 0) {
	    ENT	*ep = (ENT *) v.buf ;
	    rs = ent_release(ep) ;
	    rl = rs ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (mfsbuilt_release) */


int mfsbuilt_have(MFSBUILT *op,cchar *sp,int sl)
{
	return mfsbuilt_load(op,sp,sl,NULL) ;
}
/* end subroutine (mfsbuilt_have) */


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
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	dbp = &op->db ;
	hcp = &curp->hcur ;
	if ((rs = hdb_curbegin(dbp,hcp)) >= 0) {
	    curp->magic = MFSBUILT_MAGIC ;
	}
	return rs ;
}
/* end subroutine (mfsbuilt_curbegin) */


int mfsbuilt_curend(MFSBUILT *op,MFSBUILT_CUR *curp)
{
	HDB		*dbp ;
	HDB_CUR		*hcp ;
	int		rs = SR_OK ;
	int		rs1 ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (op->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != MFSBUILT_MAGIC) return SR_NOTOPEN ;
	dbp = &op->db ;
	hcp = &curp->hcur ;
	rs1 = hdb_curend(dbp,hcp) ;
	if (rs >= 0) rs = rs1 ;
	curp->magic = 0 ;
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
	    cchar	*sp = (cchar *) k.buf ;
	    rs = snwcpy(rbuf,rlen,sp,k.len) ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
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
	    f = TRUE ;
	    if ((rs = mfsbuilt_entdump(op)) >= 0) {
		rs = mfsbuilt_entload(op) ;
	    }
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mfsbuilt_check) */


/* private subroutines */


static int mfsbuilt_entdump(MFSBUILT *op)
{
	int		rs ;

	rs = mfsbuilt_fins(op) ;

	return rs ;
}
/* end subroutine (mfsbuilt_entdump) */


static int mfsbuilt_entload(MFSBUILT *op)
{
	FSDIR		d ;
	FSDIR_ENT	de ;
	int		rs ;
	int		rs1 ;
	if ((rs = fsdir_open(&d,op->dname)) >= 0) {
	    char	pbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath1(pbuf,op->dname)) >= 0) {
	        const int	plen = rs ;
	        while ((rs = fsdir_read(&d,&de)) > 0) {
		    cchar	*ep = de.name ;
		    int		el = rs ;
		    if (hasNotDots(ep,el)) {
		        int	sl ;
		        if ((sl = hasService(ep,el)) > 0) {
			    if ((rs = pathadd(pbuf,plen,ep)) >= 0) {
				rs = mfsbuilt_ent(op,ep,sl,pbuf,rs) ;
			    }
		        }
		    }
		    if (rs < 0) break ;
	        } /* end while */
	    } /* end if (mkpath) */
	    rs1 = fsdir_close(&d) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (fsdir) */
	return rs ;
}
/* end subroutine (mfsbuilt_entload) */


static int mfsbuilt_ent(MFSBUILT *op,cchar *sp,int sl,cchar *pp,int pl)
{
	ENT		*ep ;
	const int	esize = sizeof(ENT) ;
	int		rs ;
	if ((rs = uc_malloc(esize,&ep)) >= 0) {
	    if ((rs = ent_start(ep,sp,sl,pp,pl)) >= 0) {
		HDB		*dbp = &op->db ;
		HDB_DATUM	k, v ;
		k.buf = sp ;
		k.len = sl ;
		v.buf = ep ;
		v.len = esize ;
		rs = hdb_store(dbp,k,v) ;
		if (rs < 0)
		    ent_finish(ep) ;
	    }
	    if (rs < 0)
		uc_free(ep) ;
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (mfsbuilt_ent) */


static int mfsbuilt_fins(MFSBUILT *op)
{
	HDB		*dbp = &op->db ;
	HDB_DATUM	k, v ;
	HDB_CUR		c ;
	int		rs ;
	int		rs1 ;

	if ((rs = hdb_curbegin(dbp,&c)) >= 0) {
	    ENT		*ep ;
	    int		i ;
	    for (i = 0 ; hdb_enum(dbp,&c,&k,&v) >= 0 ; i += 1) {
		ep = (ENT *) v.buf ;
		if (ep != NULL) {
		    rs1 = ent_finish(ep) ;
		    if (rs >= 0) rs = rs1 ;
		    rs1 = uc_free(ep) ;
		    if (rs >= 0) rs = rs1 ;
		}
	    } /* end for */
	    rs1 = hdb_curend(dbp,&c) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (hdb-cur) */

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

	return rs ;
}
/* end subroutine (ent_finish) */


/* ARGSUSED */
static int ent_load(ENT *ep,void *vrp)
{
	int		rs = SR_OK ;
	void		**rpp = (void **) vrp ;
	if (ep->sop == NULL) {
	    void	*sop ;
	    if ((sop = dlopen(ep->fname,0)) != NULL) {
		ep->sop = sop ;
		ep->rcount = 1 ;
	        *rpp = ep->sop ;
	    } else {
		rs = SR_LIBACC ;
		*rpp = NULL ;
	    }
	} else {
	    ep->rcount += 1 ; /* reference count */
	    *rpp = ep->sop ;
	}
	return rs ;
}
/* end subroutine (ent_load) */


static int ent_release(ENT *ep)
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
/* end subroutine (ent_release) */


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


static int hasService(cchar *sp,int sl)
{
	cchar		*tp ;
	if (sl < 0) sl = strlen(sp) ;
	if ((tp = strnchr(sp,sl,'.')) != NULL) {
	    cchar	*ep = (tp+1) ;
	    const int	el = ((sp+sl)-(tp+1)) ;
	    if (matstr(exts,ep,el) < 0) {
		sl = 0 ;
	    }
	}
	return sl ;
}
/* end subroutine (hasService) */


