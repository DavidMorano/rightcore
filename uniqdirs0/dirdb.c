/* dirdb */

/* handle directory list operations */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
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
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"dirdb.h"


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	dirdb_alreadyentry(DIRDB *,DIRDB_ENT *) ;
static int	dirdb_alreadyname(DIRDB *,const char *,int) ;

static int	entry_start(DIRDB_ENT *,cchar *,int,struct ustat *,int) ;
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


int dirdb_add(DIRDB *dbp,cchar *name,int dlen)
{
	struct ustat	sb ;
	DIRDB_ENT	*ep ;
	int		rs ;
	int		size ;
	int		nl, cl ;
	int		dbi ;
	int		f_new = FALSE ;
	const char	*np, *dnamep ;
	const char	*cp ;

	if (dbp == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	if (dbp->magic != DIRDB_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	    debugprintf("dirdb_add: name=%t\n",name,dlen) ;
#endif

	{
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath1w(tbuf,name,dlen)) >= 0)
	        rs = u_stat(tbuf,&sb) ;
	}

#if	CF_DEBUGS
	debugprintf("dirdb_add: u_stat() rs=%d\n",rs) ;
#endif


#if	CF_NOEXIST
	if (rs < 0) {
	    rs = SR_OK ;
	    goto ret0 ;
	}

	if (! S_ISDIR(sb.st_mode))
	    goto ret0 ;

#else /* CF_NOEXIST */

	if (rs < 0)
	    goto ret0 ;

	if (! S_ISDIR(sb.st_mode)) {
	    rs = SR_NOTDIR ;
	    goto ret0 ;
	}

#endif /* CF_NOEXIST */

#if	CF_DEBUGS
	    debugprintf("dirdb_add: got a dir\n") ;
#endif

	dnamep = name ;
	if (dlen < 0) dlen = strlen(name) ;

#if	CF_DEBUGS
	    debugprintf("dirdb_add: dlen=%d\n",dlen) ;
#endif

	while ((dlen > 0) && (dnamep[dlen - 1] == '/'))
	    dlen -= 1 ;

	np = dnamep ;
	nl = dlen ;

	if (dirdb_alreadyname(dbp,dnamep,dlen) >= 0)
	    goto ret0 ;

	while ((cl = sfdirname(dnamep,dlen,&cp)) > 0) {

#if	CF_DEBUGS
	    debugprintf("dirdb_add: dname=%t\n",cp,cl) ;
#endif

	    if (dirdb_alreadyname(dbp,cp,cl) >= 0)
	        goto ret0 ;

	    dnamep = cp ;
	    dlen = cl ;

	} /* end while */

/* enter this into the database list */

#if	CF_DEBUGS
	    debugprintf("dirdb_add: adding to DB\n") ;
#endif

	f_new = TRUE ;

#if	CF_DEBUGS
	    debugprintf("dirdb_add: id=%d\n",dbp->count) ;
#endif

	size = sizeof(DIRDB_ENT) ;
	if ((rs = uc_malloc(size,&ep)) >= 0) {
	    if ((rs = entry_start(ep,np,nl,&sb,dbp->count)) >= 0) {
	        if ((rs = vechand_add(&dbp->dlist,ep)) >= 0) {
		    HDB_DATUM	key, val ;
	            dbi = rs ;
	            key.buf = &ep->fid ;
	            key.len = sizeof(DIRDB_FID) ;
	            val.buf = ep ;
	            val.len = size ;
	            if ((rs = hdb_store(&dbp->db,key,val)) >= 0) {
	                dbp->count += 1 ;
	            } /* end if (hdb_store) */
	            if (rs < 0)
		        vechand_del(&dbp->dlist,dbi) ;
	        } /* end if (vechand_add) */
	        if (rs < 0)
	            entry_finish(ep) ;
	    } /* end if (entry_start) */
	    if (rs < 0)
	        uc_free(ep) ;
	} /* end if (m-a) */

ret0:

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

	    hdb_curend(&dbp->db,&cur) ;
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


static int dirdb_alreadyentry(DIRDB *dbp,DIRDB_ENT *ep)
{
	int		rs = SR_NOTFOUND ;
	int		dlen, cl ;
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

	    if (dirdb_alreadyname(dbp,cp,cl) >= 0) {
	        rs = SR_OK ;
	        break ;
	    }

	    dnamep = cp ;
	    dlen = cl ;

	} /* end while */

#if	CF_DEBUGS
	    debugprintf("dirdb_alreadyentry: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dirdb_alreadyentry) */


/* do we already have this name? */
static int dirdb_alreadyname(DIRDB *dbp,cchar *name,int nlen)
{
	struct ustat	sb ;
	int		rs ;
	char		tmpdname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	    if (nlen < 0) nlen = strlen(name) ;
	    debugprintf("dirdb_alreadyname: name=%t\n",name,nlen) ;
#endif

	if (nlen < 0)
	    nlen = strlen(name) ;

	snwcpy(tmpdname,MAXPATHLEN,name,nlen) ;

	if ((rs = u_stat(tmpdname,&sb)) >= 0) {
	    DIRDB_FID	fid ;
	    HDB_DATUM	key, val ;
	    memset(&fid,0,sizeof(DIRDB_FID)) ;
	    fid.ino = sb.st_ino ;
	    fid.dev = sb.st_dev ;
	    key.buf = &fid ;
	    key.len = sizeof(DIRDB_FID) ;
	    rs = hdb_fetch(&dbp->db,key,NULL,&val) ;
	} else
	    rs = SR_OK ;

#if	CF_DEBUGS
	    debugprintf("dirdb_alreadyname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dirdb_alreadyname) */


static int entry_start(DIRDB_ENT *ep,cchar *np,int nl,struct ustat *sbp,
		int count)
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
static uint statcmp(e1pp,e2pp)
struct ustat	**e1pp, **e2pp ;
{
	struct ustat	*e1p, *e2p ;

	e1p = *e1pp ;
	e2p = *e2pp ;

	if (e1p->st_ino != e2p->st_ino)
	    return 1 ;

	if (e1p->st_dev != e2p->st_dev)
	    return 1 ;

	return 0 ;
}
#endif /* CF_STATCMP */


