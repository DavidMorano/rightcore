/* dirdb */

/* handle directory list operations */


#define	CF_DEBUGS	1		/* compile-time debug print-outs */
#define	CF_NOEXIST	1		/* handle names that don't exist */
#define	CF_STATCMP	0		/* compare only parts of STAT */


/* revision history:

	= 1996-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is an object but it is tied to the main program so it is really not
        independent. I do not know what it would take to make this an
        independent object at this point.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<localmisc.h>

#include	"dirdb.h"


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	dirdb_alreadyentry(DIRDB *,DIRDB_ENT *) ;
static int	dirdb_alreadyname(DIRDB *,cchar *,int) ;
static int	dirdb_adding(DIRDB *,USTAT *,cchar *,int) ;

static int	entry_start(DIRDB_ENT *,cchar *,int,USTAT *,int) ;
static int	entry_finish(DIRDB_ENT *) ;

#if	CF_STATCMP
static uint	statcmp(struct ustat **,struct ustat **) ;
#endif


/* local variables */


/* exported subroutines */


int dirdb_start(DIRDB *dbp,int n)
{
	const int	size = sizeof(DIRDB) ;
	int		rs ;

	if (dbp == NULL) return SR_FAULT ;

	if (n < DIRDB_NDEF) n = DIRDB_NDEF ;

	memset(dbp,0,size) ;
	dbp->count = 0 ;

	if ((rs = vechand_start(&dbp->dlist,n,0)) >= 0) {
	    if ((rs = hdb_start(&dbp->db,n,1,NULL,NULL)) >= 0) {
	        dbp->magic = DIRDB_MAGIC ;
	    }
	    if (rs < 0)
		vechand_finish(&dbp->dlist) ;
	} /* end if (vechand_start) */

	return rs ;
}
/* end subroutine (dirdb_start) */


int dirdb_finish(DIRDB *dbp)
{
	DIRDB_ENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (dbp == NULL) return SR_FAULT ;

	if (dbp->magic != DIRDB_MAGIC) return SR_NOTOPEN ;

/* free all entries */

	for (i = 0 ; vechand_get(&dbp->dlist,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs1 = entry_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = uc_free(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

/* free up object items */

	rs1 = hdb_finish(&dbp->db) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&dbp->dlist) ;
	if (rs >= 0) rs = rs1 ;

	dbp->count = 0 ;
	dbp->magic = 0 ;
	return rs ;
}
/* end subroutine (dirdb_finish) */


int dirdb_add(DIRDB *dbp,cchar *dp,int dl)
{
	int		rs ;
	int		f_new = FALSE ;
	char		tbuf[MAXPATHLEN+1] ;

	if (dbp == NULL) return SR_FAULT ;
	if (dp == NULL) return SR_FAULT ;

	if (dbp->magic != DIRDB_MAGIC) return SR_NOTOPEN ;

	if (dp[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("dirdb_add: name=%t\n",dp,dl) ;
#endif

	if (dl < 0) dl = strlen(dp) ;

	if ((rs = mkpath1w(tbuf,dp,dl)) >= 0) {
	    USTAT	sb ;
	    if ((rs = u_stat(tbuf,&sb)) >= 0) {
		if (S_ISDIR(sb.st_mode)) {
		    int		f_add = TRUE ;
		    int		nl ;
		    const char	*np ;
#if	CF_DEBUGS
	    	    debugprintf("dirdb_add: dl=%d\n",dl) ;
#endif
		    while ((dl > 0) && (dp[dl-1] == '/')) {
	    	        dl -= 1 ;
		    }
		    np = dp ;
		    nl = dl ;
		    if ((rs = dirdb_alreadyname(dbp,dp,dl)) == 0) {
	    	        int		cl ;
  	    		cchar		*cp ;
			while ((cl = sfdirname(dp,dl,&cp)) > 0) {
#if	CF_DEBUGS
	    		    debugprintf("dirdb_add: dname=%t\n",cp,cl) ;
#endif
	    		    if ((rs = dirdb_alreadyname(dbp,cp,cl)) > 0) {
				f_add = FALSE ;
				break ;
			    }
	    		    dp = cp ;
	    		    dl = cl ;
		        } /* end while */
/* enter this into the database list */
#if	CF_DEBUGS
		        debugprintf("dirdb_add: adding to DB\n") ;
#endif
#if	CF_DEBUGS
	    	        debugprintf("dirdb_add: id=%d\n",dbp->count) ;
#endif
		        if ((rs >= 0) && f_add) {
		            f_new = TRUE ;
	    	            rs = dirdb_adding(dbp,&sb,np,nl) ;
		        } /* end if (ok) */
		    } /* end if (dirdb_alreadyname) */
		} /* end if (is-dir) */
	    } else if (isNotPresent(rs)) {
		rs = SR_OK ;
	    } /* end if (stat) */
	} /* end if (mkpath) */

#if	CF_DEBUGS
	debugprintf("dirdb_add: ret rs=%d f_new=%u\n",rs,f_new) ;
#endif

	return (rs >= 0) ? f_new : rs ;
}
/* end subroutine (dirdb_add) */


int dirdb_clean(DIRDB *dbp)
{
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	int		rs ;
	int		rs1 ;

	if (dbp == NULL) return SR_FAULT ;

	if (dbp->magic != DIRDB_MAGIC) return SR_NOTOPEN ;

	if ((rs = hdb_curbegin(&dbp->db,&cur)) >= 0) {
	    DIRDB_ENT	*ep ;

	    while (hdb_enum(&dbp->db,&cur,&key,&val) >= 0) {

	        ep = (DIRDB_ENT *) val.buf ;
	        if ((rs = dirdb_alreadyentry(dbp,ep)) >= 0) {

	            rs1 = hdb_delcur(&dbp->db,&cur,0) ;
		    if (rs >= 0) rs = rs1 ;

	            rs1 = entry_finish(ep) ;
		    if (rs >= 0) rs = rs1 ;

	            rs1 = uc_free(ep) ;
		    if (rs >= 0) rs = rs1 ;

	        } else if (isNotPresent(rs)) {
		    rs = SR_OK ;
	        } /* end if */

	    } /* end while */

	    rs1 = hdb_curend(&dbp->db,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (kdb-cursor) */

	return rs ;
}
/* end subroutine (dirdb_clean) */


int dirdb_curbegin(DIRDB *dbp,DIRDB_CUR *curp)
{

	if (dbp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (dbp->magic != DIRDB_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (dirdb_curbegin) */


int dirdb_curend(DIRDB *dbp,DIRDB_CUR *curp)
{

	if (dbp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (dbp->magic != DIRDB_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (dirdb_curend) */


int dirdb_enum(DIRDB *dbp,DIRDB_CUR *curp,DIRDB_ENT **epp)
{
	int		rs ;
	int		i ;
	int		len = 0 ;

	if (dbp == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (dbp->magic != DIRDB_MAGIC) return SR_NOTOPEN ;

	i = (curp->i < 0) ? 0 : (curp->i + 1) ;
	if ((rs = vechand_get(&dbp->dlist,i,epp)) >= 0) {
	    curp->i = i ;
	    len = strlen((*epp)->name) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (dirdb_enum) */


/* private subroutines */


static int dirdb_adding(DIRDB *op,USTAT *sbp,cchar *np,int nl)
{
	DIRDB_ENT	*ep ;
	const int	size = sizeof(DIRDB_ENT) ;
	int		rs ;
	if ((rs = uc_malloc(size,&ep)) >= 0) {
	    if ((rs = entry_start(ep,np,nl,sbp,op->count)) >= 0) {
	        if ((rs = vechand_add(&op->dlist,ep)) >= 0) {
		    HDB_DATUM	key, val ;
	            int		dbi = rs ;
	            key.buf = &ep->fid ;
	            key.len = sizeof(DIRDB_FID) ;
	            val.buf = ep ;
	            val.len = size ;
	            if ((rs = hdb_store(&op->db,key,val)) >= 0) {
	                op->count += 1 ;
	            } /* end if (hdb_store) */
	            if (rs < 0)
		        vechand_del(&op->dlist,dbi) ;
	        } /* end if (vechand_add) */
	        if (rs < 0)
	            entry_finish(ep) ;
	    } /* end if (entry_start) */
	    if (rs < 0)
	        uc_free(ep) ;
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (dirdb_adding) */


static int dirdb_alreadyentry(DIRDB *dbp,DIRDB_ENT *ep)
{
	int		rs = SR_OK ;
	int		dlen ;
	int		cl ;
	int		f = FALSE ;
	const char	*dnamep ;
	const char	*cp ;

#if	CF_DEBUGS
	debugprintf("dirdb_alreadyentry: ent\n") ;
#endif

	dnamep = ep->name ;
	dlen = strlen(dnamep) ;

	while ((cl = sfdirname(dnamep,dlen,&cp)) > 0) {
#if	CF_DEBUGS
	    debugprintf("dirdb_alreadyentry: dname=%t\n",cp,cl) ;
#endif
	    if ((rs = dirdb_alreadyname(dbp,cp,cl)) > 0) {
		f = TRUE ;
	        break ;
	    }
	    dnamep = cp ;
	    dlen = cl ;
	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUGS
	    debugprintf("dirdb_alreadyentry: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (dirdb_alreadyentry) */


/* do we already have this name? */
static int dirdb_alreadyname(DIRDB *dbp,cchar *name,int nlen)
{
	int		rs ;
	int		f = FALSE ;
	char		tbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	if (nlen < 0) nlen = strlen(name) ;
	debugprintf("dirdb_alreadyname: name=%t\n",name,nlen) ;
#endif

	if (nlen < 0)
	    nlen = strlen(name) ;

	if ((rs = mkpath1w(tbuf,name,nlen)) >= 0) {
	    USTAT	sb ;
	    if ((rs = u_stat(tbuf,&sb)) >= 0) {
	        DIRDB_FID	fid ;
	        HDB_DATUM	key, val ;
	        memset(&fid,0,sizeof(DIRDB_FID)) ;
	        fid.ino = sb.st_ino ;
	        fid.dev = sb.st_dev ;
	        key.buf = &fid ;
	        key.len = sizeof(DIRDB_FID) ;
	        if ((rs = hdb_fetch(&dbp->db,key,NULL,&val)) >= 0) {
		    f = TRUE ;
	        } else if (rs == SR_NOTFOUND) {
		    rs = SR_OK ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	} /* end if (mkpath) */

#if	CF_DEBUGS
	    debugprintf("dirdb_alreadyname: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (dirdb_alreadyname) */


static int entry_start(DIRDB_ENT *ep,cchar *np,int nl,USTAT *sbp,int count)
{
	int		rs ;
	const char	*cp ;

	if (ep == NULL) return SR_FAULT ;

	if (nl < 0) nl = strlen(np) ;

	memset(ep,0,sizeof(DIRDB_ENT)) ;
	ep->fid.ino = sbp->st_ino ;
	ep->fid.dev = sbp->st_dev ;
	ep->count = count ;

	if ((rs = uc_mallocstrw(np,nl,&cp)) >= 0) {
	    ep->name = cp ;
	}

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(DIRDB_ENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->name != NULL) {
	    rs1 = uc_free(ep->name) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->name = NULL ;
	}

	return rs ;
}
/* end subroutine (entry_finish) */


#if	CF_STATCMP
static uint statcmp(const USTAT **e1pp,const USTAT **e2pp)
{
	const USTAT	*e1p = *e1pp ;
	const USTAT	*e2p = *e2pp ;
	int		rc = (e1p->st_ino - e2p->st_ino) ;
	if (rc == 0) {
	    rc = (e1p->st_dev - e2p->st_dev)
	}
	return rc ;
}
#endif /* CF_STATCMP */


