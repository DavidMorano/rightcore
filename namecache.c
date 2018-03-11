/* namecache */

/* real-name cache (from UNIX® System PASSWD database) */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_FULLNAME	0		/* use fullname? */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object provides a crude cache for PASSWD-DB real-names.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<realname.h>
#include	<localmisc.h>

#include	"namecache.h"


/* local defines */

#ifndef	NAMECACHE_ENT
#define	NAMECACHE_ENT	struct namecache_e
#endif

#ifndef	GNAMELEN
#define	GNAMELEN	100		/* GECOS name length */
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real name length */
#endif

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */


/* external subroutines */

extern int	getgecosname(const char *,int,const char **) ;
extern int	mkgecosname(char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;

#if	CF_DEBUGS
extern char	*timestr_log(time_t,char *) ;
#endif


/* local structures */

struct namecache_e {
	const char	*username ;
	const char	*realname ;
	char		*a ;
	time_t		ti_init ;
	time_t		ti_access ;
	int		realnamelen ;
} ;


/* forward references */

static int	namecache_newentry(NAMECACHE *,NAMECACHE_ENT **,
			cchar *,cchar *,int) ;
static int	namecache_repentry(NAMECACHE *,NAMECACHE_ENT **,
			cchar *,cchar *,int) ;
static int	namecache_entfins(NAMECACHE *) ;

static int	entry_start(NAMECACHE_ENT *,const char *,const char *,int) ;
static int	entry_update(NAMECACHE_ENT *,const char *,int) ;
static int	entry_finish(NAMECACHE_ENT *) ;
static int	entry_loadnames(NAMECACHE_ENT *,cchar *,cchar *,int) ;

static int	mkaname(char *,int,const char *) ;


/* local variables */


/* exported subroutines */


int namecache_start(NAMECACHE *op,cchar *varname,int max,int ttl)
{
	int		rs ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (varname == NULL) return SR_FAULT ;

	if (varname[0] == '\0') return SR_INVALID ;

	if (max < 3) 
	    max = NAMECACHE_DEFMAX ;

	if (ttl < 1)
	    ttl = NAMECACHE_DEFTO ;

	memset(op,0,sizeof(NAMECACHE)) ;

	if ((rs = uc_mallocstrw(varname,-1,&cp)) >= 0) {
	    const int	n = NAMECACHE_DEFENTS ;
	    op->varname = cp ;
	    if ((rs = hdb_start(&op->db,n,1,NULL,NULL)) >= 0) {
	        op->max = max ;
	        op->ttl = ttl ;
	        op->magic = NAMECACHE_MAGIC ;
	    } /* end if (hdb-start) */
	    if (rs < 0) {
		uc_free(op->varname) ;
		op->varname = NULL ;
	    }
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (namecache_start) */


int namecache_finish(NAMECACHE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != NAMECACHE_MAGIC) return SR_NOTOPEN ;

/* loop freeing up all cache entries */

	rs1 = namecache_entfins(op) ;
	if (rs >= 0) rs = rs1 ;

/* free up everything else */

	rs1 = hdb_finish(&op->db) ;
	if (rs >= 0) rs = rs1 ;

	if (op->varname != NULL) {
	    rs1 = uc_free(op->varname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->varname = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (namecache_finish) */


int namecache_add(NAMECACHE *op,cchar *un,cchar *rnp,int rnl)
{
	HDB_DATUM	key, val ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;
	if (rnp == NULL) return SR_FAULT ;

	if (op->magic != NAMECACHE_MAGIC) return SR_NOTOPEN ;

	if (un[0] == '\0') return SR_INVALID ;

	key.buf = un ;
	key.len = strlen(un) ;

	if ((rs = hdb_fetch(&op->db,key,NULL,&val)) >= 0) {
	    NAMECACHE_ENT	*ep ;

	    ep = (NAMECACHE_ENT *) val.buf ;
	    rs = entry_update(ep,rnp,rnl) ;

	} else if (rs == SR_NOTFOUND) {

	    if ((rs = hdb_count(&op->db)) >= op->max) {
	        rs = namecache_repentry(op,NULL,un,rnp,rnl) ;
	    } else {
	        rs = namecache_newentry(op,NULL,un,rnp,rnl) ;
	    }

	} /* end if (hdb-fetch) */

	return rs ;
}
/* end subroutine (namecache_add) */


int namecache_lookup(NAMECACHE *op,cchar *un,cchar **rpp)
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	int		rl = 0 ;
	const char	*rp = NULL ;
	char		*pwbuf ;

	if (op == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (op->magic != NAMECACHE_MAGIC) return SR_NOTOPEN ;

	if (un[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("namecache_lookup: u=>%s<\n",un) ;
#endif

	if (rpp != NULL) *rpp = NULL ;

	op->s.total += 1 ;
	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    HDB_DATUM		key, val ;
	    NAMECACHE_ENT	*ep ;
	    const time_t	dt = time(NULL) ;
	    const int		rlen = REALNAMELEN ;
	    char		rbuf[REALNAMELEN + 1] ;

	    key.buf = un ;
	    key.len = strlen(un) ;
	    if ((rs = hdb_fetch(&op->db,key,NULL,&val)) >= 0) {

	        ep = (NAMECACHE_ENT *) val.buf ;

	        ep->ti_access = dt ;
	        if (dt > (ep->ti_init + op->ttl)) {

#if	CF_DEBUGS
	            {
		        char	timebuf[TIMEBUFLEN + 1] ;
		        debugprintf("namecache_lookup: need update to=%u\n",
			    op->ttl) ;
		        debugprintf("namecache_lookup: daytime=%s\n",
			    timestr_log(dt,timebuf)) ;
		        debugprintf("namecache_lookup: ti_init=%s\n",
			    timestr_log(ep->ti_init,timebuf)) ;
	            }
#endif /* CF_DEBUGS */

		    ep->ti_init = dt ;
	            if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,un)) >= 0) {
		        cchar	*gecos = pw.pw_gecos ;
	                if ((rs = mkaname(rbuf,rlen,gecos)) >= 0) {
		            rl = rs ;
			    rs = entry_update(ep,rbuf,rl) ;
		        }
		    } /* end if (get-pwname) */

	        } /* end if (expiration) */

	        rp = ep->realname ;
	        rl = ep->realnamelen ;

	        if (rs >= 0) {
		    if (rl > 0) {
		        op->s.phits += 1 ;
		    } else {
		        op->s.nhits += 1 ;
		    }
	        }

	    } else if (rs == SR_NOTFOUND) {

	        if ((rs = GETPW_NAME(&pw,pwbuf,pwlen,un)) >= 0) {
		    cchar	*gecos = pw.pw_gecos ;

	            if ((rs = mkaname(rbuf,rlen,gecos)) >= 0) {
		        rl = rs ;

/* enter this name into the cache */

	                if ((rs = hdb_count(&op->db)) >= op->max) {
	                    rs = namecache_repentry(op,&ep,un,rbuf,rl) ;
	                } else if (rs >= 0) {
	                    rs = namecache_newentry(op,&ep,un,rbuf,rl) ;
		        }
	                if ((rs >= 0) && (ep != NULL)) {
		            rp = ep->realname ;
		            rl = ep->realnamelen ;
	                }

		    } /* end if (real-name) */

	        } /* end if (getpw_name) */

	    } /* end if (hdb_fetch) */

	    uc_free(pwbuf) ;
	} /* end if (memory-allocation) */

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? rp : NULL ;
	}

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (namecache_lookup) */


int namecache_stats(NAMECACHE *op,NAMECACHE_STATS *sp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != NAMECACHE_MAGIC) return SR_NOTOPEN ;

	if ((rs = hdb_count(&op->db)) >= 0) {
	    *sp = op->s ;
	    sp->nentries = rs ;
	}

	return rs ;
}
/* end subroutine (namecache_stats) */


/* private subroutines */


static int namecache_newentry(NAMECACHE *op,NAMECACHE_ENT **epp,cchar *un,
		cchar *np,int nl)
{
	NAMECACHE_ENT	*ep ;
	const int	msize = sizeof(NAMECACHE_ENT) ;
	int		rs ;

	if (epp != NULL)
	    *epp = NULL ;

	if ((rs = uc_malloc(msize,&ep)) >= 0) {
	    if ((rs = entry_start(ep,un,np,nl)) >= 0) {
	        HDB_DATUM	key, val ;
	        key.buf = ep->username ;
	        key.len = strlen(ep->username) ;
	        val.buf = ep ;
	        val.len = msize ;
	        if ((rs = hdb_store(&op->db,key,val)) >= 0) {
		    if (epp != NULL) *epp = ep ;
		} /* end if (hdb-store) */
		if (rs < 0)
		    entry_finish(ep) ;
	    } /* end if (entry-start) */
	    if (rs < 0)
		uc_free(ep) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (namecache_newentry) */


static int namecache_repentry(NAMECACHE *op,NAMECACHE_ENT **epp,cchar *un,
		cchar *np,int nl)
{
	NAMECACHE_ENT	*ep = NULL ;
	HDB		*dbp = &op->db ;
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	int		rs ;

	if (epp != NULL)
	    *epp = NULL ;

	if ((rs = hdb_curbegin(dbp,&cur)) >= 0) {
	    NAMECACHE_ENT	*tep = NULL ;

	    while ((rs = hdb_enum(dbp,&cur,&key,&val)) >= 0) {
		tep = (NAMECACHE_ENT *) val.buf ;
		if ((ep == NULL) || (ep->ti_access < tep->ti_access)) {
		    ep = tep ;
		}
	    } /* end while */

	    hdb_curend(dbp,&cur) ;
	} /* end if (cursor) */

	if ((rs >= 0) && (ep != NULL)) {
	    entry_finish(ep) ;
	    rs = entry_start(ep,un,np,nl) ;
	} else {
	    rs = SR_NOANODE ;
	}

	if (epp != NULL)
	    *epp = ep ;

	return rs ;
}
/* end subroutine (namecache_repentry) */


static int namecache_entfins(NAMECACHE *op)
{
	HDB		*elp = &op->db ;
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	int		rs = SR_OK ;
	int		rs1 ;
	if ((rs1 = hdb_curbegin(elp,&cur)) >= 0) {
	    NAMECACHE_ENT	*ep ;
	    while (hdb_enum(elp,&cur,&key,&val) >= 0) {
	        ep = (NAMECACHE_ENT *) val.buf ;
	        rs1 = entry_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = uc_free(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end while */
	    rs1 = hdb_curend(elp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (namecache_entfins) */


static int entry_start(NAMECACHE_ENT *ep,cchar *up,cchar *rp,int rl)
{
	const time_t	dt = time(NULL) ;
	int		rs ;

	if (ep == NULL) return SR_FAULT ;
	if (up == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (up[0] == '\0') return SR_INVALID ;

	if (rl < 0) rl = strlen(rp) ;

	memset(ep,0,sizeof(NAMECACHE_ENT)) ;
	ep->ti_init = dt ;
	ep->ti_access = dt ;
	ep->realnamelen = rl ;

	rs = entry_loadnames(ep,up,rp,rl) ;

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(NAMECACHE_ENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->a != NULL) {
	    rs1 = uc_free(ep->a) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->a = NULL ;
	}

	ep->username = NULL ;
	ep->realname = NULL ;
	return rs ;
}
/* end subroutine (entry_finish) */


static int entry_update(NAMECACHE_ENT *ep,cchar *rp,int rl)
{
	time_t		dt = 0 ;
	int		rs = SR_OK ;
	int		f_changed = FALSE ;

	if (ep == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (rl < 0) rl = strlen(rp) ;

#if	CF_DEBUGS
	debugprintf("entry_update: u=%s\n",ep->username) ;
#endif

	f_changed = ((strncmp(ep->realname,rp,rl) == 0) &&
		(ep->realname[rl] == '\0')) ;

	if (f_changed) {

	    if (dt == 0) dt = time(NULL) ;
	    ep->ti_init = dt ;
	    ep->realnamelen = rl ;

	    {
		const int	ulen = USERNAMELEN ;
		char		ubuf[USERNAMELEN+1] ;
		strdcpy1(ubuf,ulen,ep->username) ;
	        rs = entry_loadnames(ep,ubuf,rp,rl) ;
	    }

	} /* end if (changed) */

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (entry_update) */


static int entry_loadnames(NAMECACHE_ENT *ep,cchar *up,cchar *rp,int rl)
{
	int		rs ;
	int		size = 0 ;
	char		*bp ;
	if (ep->a != NULL) {
	    uc_free(ep->a) ;
	    ep->a = NULL ;
	}
	size += (strlen(up)+1) ;
	size += (strnlen(rp,rl)+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    ep->a = bp ;
	    ep->username = bp ;
	    bp = (strwcpy(bp,up,-1)+1) ;
	    ep->realname = bp ;
	    bp = (strwcpy(bp,rp,rl)+1) ;
	} /* end if (memory-allocation) */
	return rs ;
}
/* end subroutine (entry_loadnames) */


/* make a real name from a GECOS name */
static int mkaname(char *nbuf,int nlen,cchar *gecos)
{
	int		rs ;
	int		rl = 0 ;
	const char	*gp ;

	nbuf[0] = '\0' ;
	if ((rs = getgecosname(gecos,-1,&gp)) > 0) {
	    REALNAME	rn ;
	    int		gl = rs ;
	    if ((rs = realname_start(&rn,gp,gl)) >= 0) {

#if	CF_FULLNAME
	        rl = realname_fullname(&rm,mbuf,nlen) ;
#else
	        rl = realname_name(&rn,nbuf,nlen) ;
#endif

	        realname_finish(&rn) ;
	    } /* end if (realname) */
	} /* end if (gecos-name) */

	return (rs >= 0) ? rl : 0 ;
}
/* end subroutine (mkaname) */


