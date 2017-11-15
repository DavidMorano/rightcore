/* fsdirtree */

/* file-system directory tree (traversing) */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGAUDIT	0		/* perform some auditing */


/* revision history:

	= 2000-04-27, David A­D­ Morano
	I wanted an interative enumeration.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is not similar to the 'ftw' subroutine. That subroutine
        "walks" the directory tree and calls a user-supplied function at each
        entry. This module is quite different from 'ftw' and 'wdt' in that it
        allows the caller to intermix directory functions with other caller
        activities. It allows this by "reading" and entry in a similar way as
        FSDIR allows.

        The caller "opens" an object and then performs operations on it like
        reading a directory entry. After the caller is finished with all
        directory functions on this object, the object should be "closed".

	Synopsis:

	int fsdirtree_open(op,dname,opts)
	FSDIRTREE	*op ;
	const char	dname[] ;
	int		opts ;

	Arguments:

	op		object pointer
	dname		directory name to process
	opts		options for processing

	Returns:

	>=0		good
	<0		bad in some way!


*******************************************************************************/


#define	FSDIRTREE_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<fsdir.h>
#include	<fifostr.h>
#include	<localmisc.h>

#include	"fsdirtree.h"


/* local defines */

#define	FSDIRTREE_MTYPE1	(FSDIRTREE_MREG | FSDIRTREE_MBLOCK)
#define	FSDIRTREE_MTYPE2	(FSDIRTREE_MPIPE | FSDIRTREE_MSOCK)
#define	FSDIRTREE_MTYPE3	(FSDIRTREE_MDIR | FSDIRTREE_MCHAR)
#define	FSDIRTREE_MTYPE		\
	    (FSDIRTREE_MTYPE1 | FSDIRTREE_MTYPE2 | FSDIRTREE_MTYPE3)

#undef	DIRID
#define	DIRID		struct fsdirtree_dirid

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 10),2048)
#endif

#ifndef	MAXLINKLEN
#define	MAXLINKLEN	(4*MAXPATHLEN)
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mknpath1(char *,int,const char *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	pathadd(char *,int,const char *) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	hasNotDots(const char *,int) ;
extern int	isNotPresent(int) ;
extern int	isDotDir(const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct fsdirtree_dirid {
	uino_t		ino ;
	dev_t		dev ;
} ;


/* forward references */

static uint	diridhash(const void *,int) ;

static int	fsdirtree_dirbegin(FSDIRTREE *) ;
static int	fsdirtree_diradd(FSDIRTREE *,dev_t,uino_t) ;
static int	fsdirtree_dirhave(FSDIRTREE *,dev_t,uino_t,DIRID **) ;
static int	fsdirtree_dirend(FSDIRTREE *) ;

static int	dirid_start(DIRID *,dev_t,uino_t) ;
static int	dirid_finish(DIRID *) ;

static int	diridcmp(DIRID *,DIRID *,int) ;
static int	interested(int,mode_t) ;


/* local variables */


/* exported subroutines */


int fsdirtree_open(FSDIRTREE *op,cchar dname[],int opts)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("fsdirtree_open: dname=%s\n",dname) ;
	debugprintf("fsdirtree_open: opts=%04x\n",opts) ;
#endif

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(FSDIRTREE)) ;
	op->opts = opts ;

	if ((rs = fifostr_start(&op->dirq)) >= 0) {
	    cchar	*bdp = dname ;
	    if ((bdp == NULL) || (strcmp(bdp,".") == 0)) bdp = "" ;
	    if (bdp[0] != '/') {
	        if ((rs = uc_getcwd(op->basedname,MAXPATHLEN)) >= 0) {
	            op->bdnlen = rs ;
	            if (bdp[0] != '\0') {
	                op->basedname[op->bdnlen++] = '/' ;
	            }
	        }
	    }
	    if (rs >= 0) {
	        if (bdp[0] != '\0') {
	            int		cl = (MAXPATHLEN - op->bdnlen) ;
	            rs = sncpy1((op->basedname + op->bdnlen),cl,bdp) ;
	            op->bdnlen += rs ;
	        }
	        if (rs >= 0) {
	            if ((rs = fsdir_open(&op->dir,op->basedname)) >= 0) {
	                op->f.dir = TRUE ;
	                if (op->basedname[op->bdnlen - 1] != '/') {
	                    op->basedname[op->bdnlen++] = '/' ;
	                }
	                if (opts & FSDIRTREE_MUNIQ) {
	                    if ((rs = fsdirtree_dirbegin(op)) >= 0) {
	                        struct ustat	sb ;
	                        if ((rs = uc_stat(op->basedname,&sb)) >= 0) {
	                            uino_t	ino = sb.st_ino ;
	                            dev_t	dev = sb.st_dev ;
	                            rs = fsdirtree_diradd(op,dev,ino) ;
	                        } /* end if (stat) */
	                        if (rs < 0)
	                            fsdirtree_dirend(op) ;
	                    } /* end if (dir-tracking) */
	                } /* end if (uniq traversal requested) */
	                if (rs >= 0) {
	                    op->cdnlen = op->bdnlen ;
	                    op->magic = FSDIRTREE_MAGIC ;
	                }
	                if (rs < 0)
	                    fsdir_close(&op->dir) ;
	            } /* end if (fsdir) */
	        } /* end if (ok) */
	    } /* end if (ok) */
	    if (rs < 0)
	        fifostr_finish(&op->dirq) ;
	} /* end if (fifostr) */

#if	CF_DEBUGS
	debugprintf("fsdirtree_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (fsdirtree_open) */


int fsdirtree_close(FSDIRTREE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != FSDIRTREE_MAGIC) return SR_NOTOPEN ;

	if (op->f.dirids) {
	    rs1 = fsdirtree_dirend(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->f.dir) {
	    op->f.dir = FALSE ;
	    rs1 = fsdir_close(&op->dir) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = fifostr_finish(&op->dirq) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (fsdirtree_close) */


/* read a directory entry-name and its status */
int fsdirtree_read(FSDIRTREE *op,FSDIRTREE_STAT *sbp,char *rbuf,int rlen)
{
	struct ustat	se ;
	FSDIR_ENT	de ;
	int		rs = SR_OK ;
	int		mlen ;
	int		clen, flen ;
	int		ndir = 0 ;
	int		len = 0 ;
	const char	*enp ;
	char		*fnp ;
	char		*cdnp = NULL ;

#if	CF_DEBUGS
	debugprintf("fsdirtree_read: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if ((sbp == NULL) || (rbuf == NULL)) return SR_FAULT ;

	if (op->magic != FSDIRTREE_MAGIC) return SR_NOTOPEN ;

	if (rlen < 0) rlen = MAXPATHLEN ;

	while ((rs >= 0) && (! op->f.eof)) {
	    cdnp = NULL ;

	    if ((rs = fsdir_read(&op->dir,&de)) > 0) {
	        int	enl = rs ;
	        int	f_proc = TRUE ;

/* do not play with the "special" entries! (this is a fast way of deciding) */

	        enp = de.name ;
	        if (hasNotDots(enp,enl)) {

#if	CF_DEBUGS
	            debugprintf("fsdirtree_read: non-trivial\n") ;
#endif

	            if (op->prune != NULL) {
	                f_proc = (matstr(op->prune,enp,enl) < 0) ;
	            }

	            if (f_proc) {

/* form the full filepath */

	                if ((op->cdnlen > 0) && 
	                    (op->basedname[op->cdnlen - 1] != '/')) {
	                    op->basedname[op->cdnlen++] = '/' ;
	                }

	                fnp = op->basedname + op->cdnlen ;
	                mlen = MAXPATHLEN - op->cdnlen ;
	                flen = strwcpy(fnp,enp,mlen) - fnp ;

#if	CF_DEBUGS
	                debugprintf("fsdirtree_read: bpath=%s\n",
	                    op->basedname) ;
#endif

/* if we cannot 'stat' it, then it cannot be too important to us! */

	                if ((rs = u_lstat(op->basedname,sbp)) >= 0) {
	                    if (S_ISLNK(sbp->st_mode)) {

#if	CF_DEBUGS
	                        debugprintf("fsdirtree_read: ISLNK\n") ;
#endif

	                        if (op->opts & FSDIRTREE_MFOLLOW) {
	                            const int	llen = MAXLINKLEN ;
	                            cchar	*fn = op->basedname ;
	                            char	lbuf[MAXLINKLEN+1] ;
	                            if ((rs = uc_readlink(fn,lbuf,llen)) >= 0) {
	                                if (! isDotDir(lbuf)) {
	                                    if ((rs = uc_stat(fn,&se)) >= 0) {
	                                        sbp = &se ;
	                                    } else if (rs == SR_NOENT) {
	                                        rs = SR_OK ;
	                                    }
	                                } /* end if (not-dots) */
	                            } /* end if (uc_readlink) */
	                        } /* end if (follow-option specified) */

	                    } /* end if (is-link) */
	                } /* end if (u_lstat) */

	            } /* end if (not pruned) */

	        } else {
	            f_proc = FALSE ;
	        }

#if	CF_DEBUGS
	        debugprintf("fsdirtree_read: read-mid rs=%d\n",rs) ;
#endif

/* directory-uniqueness check */

	        if ((rs >= 0) && f_proc) {
		    const int	m = FSDIRTREE_MUNIQ ;
	            if (S_ISDIR(sbp->st_mode) && (op->opts & m)) {
	                dev_t	dev = sbp->st_dev ;
	                uino_t	ino = sbp->st_ino ;
	                int	rs1 = fsdirtree_dirhave(op,dev,ino,NULL) ;
	                if (rs1 >= 0) {
	                    f_proc = FALSE ;
	                } else if (rs1 == SR_NOTFOUND) {
	                    rs = fsdirtree_diradd(op,dev,ino) ;
	                } else {
	                    rs = rs1 ;
	                }
	            } /* end if (directory-uniqueness check) */
	        } /* end if (directory-uniqueness check) */

	        if ((rs >= 0) && f_proc) {

/* identify the filepath part in addition to the base */

	            cdnp = op->basedname + op->bdnlen ;
	            clen = op->cdnlen - op->bdnlen + flen ;

#if	CF_DEBUGS
	            debugprintf("fsdirtree_read: clen=%d\n",clen) ;
	            debugprintf("fsdirtree_read: cdnp=%s\n",cdnp) ;
#endif

	            if (S_ISDIR(sbp->st_mode)) {

#if	CF_DEBUGS
	                debugprintf("fsdirtree_read: ISDIR\n") ;
#endif

/* OK, we are free, prepare to write out information on this directory */

	                ndir += 1 ;

/* write out the directory name as a coded line */

#if	CF_DEBUGS
	                debugprintf("fsdirtree_read: writing=%t\n",
	                    cdnp,clen) ;
#endif

	                rs = fifostr_add(&op->dirq,cdnp,clen) ;

#if	CF_DEBUGS
	                debugprintf("fsdirtree_read: fifostr_add() rs=%d\n",
	                    rs) ;
#endif

	                if (rs < 0)
	                    break ;

#if	CF_DEBUGS && CF_DEBUGAUDIT
	                {
	                    int	rs1 = fsdir_audit(&op->dir) ;
	                    debugprintf("fsdirtree_read: fsdir_audit() rs=%d\n",
	                        rs1) ;
	                }
#endif

	            } /* end if (special handling for directories) */

#if	CF_DEBUGS
	            debugprintf("fsdirtree_read: ISallother\n") ;
#endif

/* OK, should we return this to the caller? */

	            if ((op->opts & FSDIRTREE_MTYPE) == 0)
	                break ;

	            if (interested(op->opts,sbp->st_mode))
	                break ;

#if	CF_DEBUGS
	            debugprintf("fsdirtree_read: not interested\n") ;
#endif

	        } else if ((rs == SR_NOENT) || (rs == SR_ACCESS)) {

#if	CF_DEBUGS
	            debugprintf("fsdirtree_read: NOENT-ACCESS\n") ;
#endif

	            if (op->opts & FSDIRTREE_MNOENT) break ;
	            rs = SR_OK ;
	        } /* end if */

	    } else if (rs == 0) { /* EOF on directory read */

	        len = 0 ;
	        op->f.dir = FALSE ;
	        rs = fsdir_close(&op->dir) ;

#if	CF_DEBUGS
	        debugprintf("fsdirtree_read: fsdir_close() rs=%d\n",rs) ;
#endif

	        if ((op->bdnlen > 0) && 
	            (op->basedname[op->bdnlen - 1] != '/'))
	            op->basedname[op->bdnlen++] = '/' ;

	        while (rs >= 0) {

	            cdnp = op->basedname + op->bdnlen ;
	            mlen = MAXPATHLEN - op->bdnlen ;
	            rs = fifostr_remove(&op->dirq,cdnp,mlen) ;
	            len = rs ;

#if	CF_DEBUGS
	            debugprintf("fsdirtree_read: fifostr_remove() rs=%d\n",rs) ;
#endif

	            if ((rs < 0) && (rs != SR_NOTFOUND))
	                break ;

	            if (rs == SR_NOTFOUND) {
	                cdnp = NULL ;
	                rs = SR_OK ;
	                len = 0 ;
	                op->f.eof = TRUE ;
	                break ;
	            }

	            cdnp[len] = '\0' ; /* not needed? */
	            op->cdnlen = op->bdnlen + len ;

#if	CF_DEBUGS
	            debugprintf("fsdirtree_read: read=%s\n",cdnp) ;
	            debugprintf("fsdirtree_read: opening=%s\n",op->basedname) ;
#endif

	            if ((rs = fsdir_open(&op->dir,op->basedname)) >= 0) {
	                op->f.dir = TRUE ;
	                break ;
	            } else if (isNotPresent(rs)) {
	                rs = SR_OK ;
	            }

	        } /* end while */

	    } /* end if (directory-read) */

	} /* end while (outer) */

#if	CF_DEBUGS
	debugprintf("fsdirtree_read: while-out rs=%d\n",rs) ;
#endif

	rbuf[0] = '\0' ;
	if ((rs >= 0) && (cdnp != NULL) && (! op->f.eof)) {
	    rs = mknpath1(rbuf,rlen,cdnp) ;
	    len = rs ;
	}

#if	CF_DEBUGS
	debugprintf("fsdirtree_read: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (fsdirtree_read) */


int fsdirtree_prune(FSDIRTREE *op,cchar **prune)
{
	if (op == NULL) return SR_FAULT ;
	if (prune == NULL) return SR_FAULT ;
	if (op->magic != FSDIRTREE_MAGIC) return SR_NOTOPEN ;
	op->prune = prune ;
	return SR_OK ;
}
/* end subroutine (fsdirtree_prune) */


/* private subroutines */


static int fsdirtree_dirbegin(FSDIRTREE *pip)
{
	HDB		*dbp = &pip->dirids ;
	const int	n = 50 ;
	const int	at = 1 ;	/* use 'lookaside(3dam)' */
	int		rs ;

	rs = hdb_start(dbp,n,at,diridhash,diridcmp) ;
	pip->f.dirids = (rs >= 0) ;

	return rs ;
}
/* end subroutine (fsdirtree_dirbegin) */


static int fsdirtree_dirend(FSDIRTREE *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->f.dirids) {
	    HDB		*dbp = &pip->dirids ;
	    HDB_CUR	cur ;
	    HDB_DATUM	key, val ;

	    pip->f.dirids = FALSE ;
	    if ((rs1 = hdb_curbegin(dbp,&cur)) >= 0) {
	        DIRID	*dip ;

	        while (hdb_enum(dbp,&cur,&key,&val) >= 0) {
	            dip = (DIRID *) val.buf ;

	            if (dip != NULL) {
	                rs1 = dirid_finish(dip) ;
	                if (rs >= 0) rs = rs1 ;
	                rs1 = uc_free(dip) ;
	                if (rs >= 0) rs = rs1 ;
	            }

	        } /* end while */

	        hdb_curend(dbp,&cur) ;
	    } /* end if */
	    if (rs >= 0) rs = rs1 ;

	    rs1 = hdb_finish(&pip->dirids) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (was activated) */

	return rs ;
}
/* end subroutine (fsdirtree_dirend) */


static int fsdirtree_diradd(FSDIRTREE *pip,dev_t dev,uino_t ino)
{
	DIRID		*dip ;
	const int	size = sizeof(DIRID) ;
	int		rs ;

	if ((rs = uc_malloc(size,&dip)) >= 0) {
	    HDB		*dbp = &pip->dirids ;
	    HDB_DATUM	key, val ;
	    if ((rs = dirid_start(dip,dev,ino)) >= 0) {
	        key.buf = dip ;
	        key.len = sizeof(uino_t) + sizeof(dev_t) ;
	        val.buf = dip ;
	        val.len = size ;
	        rs = hdb_store(dbp,key,val) ;
	        if (rs < 0)
	            dirid_finish(dip) ;
	    } /* end if (dirid-start) */
	    if (rs < 0)
	        uc_free(dip) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (fsdirtree_diradd) */


static int fsdirtree_dirhave(FSDIRTREE *pip,dev_t d,uino_t ino,DIRID **rpp)
{
	HDB		*dbp = &pip->dirids ;
	HDB_DATUM	key, val ;
	DIRID		did ;
	int		rs ;

	did.ino = ino ;
	did.dev = d ;

	key.buf = &did ;
	key.len = sizeof(uino_t) + sizeof(dev_t) ;
	if ((rs = hdb_fetch(dbp,key,NULL,&val)) >= 0) {
	    if (rpp != NULL) *rpp = (DIRID *) val.buf ;
	}

	return rs ;
}
/* end subroutine (fsdirtree_dirhave) */


static int dirid_start(DIRID *dip,dev_t dev,uino_t ino)
{
	int		rs = SR_OK ;

	dip->dev = dev ;
	dip->ino = ino ;

	return rs ;
}
/* end subroutine (dirid_start) */


static int dirid_finish(DIRID *dip)
{
	if (dip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (dirid_finish) */


static uint diridhash(const void *vp,int vl)
{
	uint		h = 0 ;
	ushort		*sa = (ushort *) vp ;

	h = h ^ ((sa[1] << 16) | sa[0]) ;
	h = h ^ ((sa[0] << 16) | sa[1]) ;
	if (vl > sizeof(uint)) {
	    h = h ^ ((sa[3] << 16) | sa[2]) ;
	    h = h ^ ((sa[2] << 16) | sa[3]) ;
	    if (vl > sizeof(ULONG)) {
	        h = h ^ ((sa[5] << 16) | sa[4]) ;
	        h = h ^ ((sa[4] << 16) | sa[5]) ;
	        if (vl > (4*3)) {
	            h = h ^ ((sa[7] << 16) | sa[6]) ;
	            h = h ^ ((sa[6] << 16) | sa[7]) ;
	        }
	    }
	}

	return h ;
}
/* end subroutine (diridhash) */


/* ARGSUSED */
static int diridcmp(DIRID *e1p,DIRID *e2p,int len)
{
	int64_t		d ;
	int		rc = 0 ;

	if ((d = (e1p->dev - e2p->dev)) == 0) {
	    d = (e1p->ino - e2p->ino) ;
	}

	if (d > 0) rc = 1 ;
	else if (d < 0) rc = -1 ;

	return rc ;
}
/* end subroutine (diridcmp) */


static int interested(int opts,mode_t mode)
{
	int		f = FALSE ;

	f = f || ((opts & FSDIRTREE_MREG) && S_ISREG(mode)) ;
	f = f || ((opts & FSDIRTREE_MDIR) && S_ISDIR(mode)) ;
	f = f || ((opts & FSDIRTREE_MLINK) && S_ISLNK(mode)) ;
	f = f || ((opts & FSDIRTREE_MPIPE) && S_ISFIFO(mode)) ;
	f = f || ((opts & FSDIRTREE_MSOCK) && S_ISSOCK(mode)) ;
	f = f || ((opts & FSDIRTREE_MCHAR) && S_ISCHR(mode)) ;
	f = f || ((opts & FSDIRTREE_MBLOCK) && S_ISBLK(mode)) ;

	return f ;
}
/* end subroutine (interested) */


