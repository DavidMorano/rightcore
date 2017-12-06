/* bpimk */

/* make a BPI database */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module creates a BPI database file.

	Synopsis:

	int bpimk_open(op,dbname,...)
	BPIMK		*op ;
	const char	dbname[] ;

	Arguments:

	- op		object pointer
	- dbname	name of (path-to) DB

	Returns:

	>=0		OK
	<0		error code


	Notes:

	= possible returns to an open attempt

	- OK (creating)
	- already exists
	- doesn't exist but is in progress
	- exists and is in progress

	= open-flags

			if DB exits	if NDB exists	returns
	___________________________________________________________________

	-		no		no		SR_OK (created)
	-		no		yes		SR_INPROGRESS
	-		yes		no		SR_OK
	-		yes		yes		SR_INPROGRESS

	O_CREAT|O_EXCL	no		no		SR_OK (created)
	O_CREAT|O_EXCL	no		yes		SR_INPROGRESS
	O_CREAT|O_EXCL	yes		no		SR_EXIST
	O_CREAT|O_EXCL	yes		yes		SR_INPROGRESS

	O_CREAT		x		x		SR_OK (created)


*******************************************************************************/


#define	BPIMK_MASTER	0


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endian.h>
#include	<endianstr.h>
#include	<vecobj.h>
#include	<filebuf.h>
#include	<char.h>
#include	<localmisc.h>

#include	"bpimk.h"
#include	"bpihdr.h"


/* local defines */

#define	BPIMK_NENTRIES	(19 * 1024)
#define	BPIMK_NSKIP	5
#define	HDRBUFLEN	(sizeof(BPIHDR) + 128)

#define	FSUF_IDX	"bpi"

#define	TO_OLDFILE	(5 * 60)

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	BVENTRY		struct bventry


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,cchar *,cchar *) ;
extern int	sncpy5(char *,int,cchar *,cchar *,cchar *,cchar *,cchar *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf3(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	getpwd(char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	mktmpfile(char *,mode_t,cchar *) ;
extern int	filebuf_writefill(FILEBUF *,const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	snopenflags(char *,int,int) ;
extern int	debugprintf(cchar *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* exported variables */

BPIMK_OBJ	bpimk = {
	"bpimk",
	sizeof(BPIMK)
} ;


/* local structures */

struct bventry {
	uint	citation ;		/* (nlines, b, c, v) */
} ;


/* forward references */

static int	bpimk_filesbegin(BPIMK *) ;
static int	bpimk_filesbeginc(BPIMK *) ;
static int	bpimk_filesbeginwait(BPIMK *) ;
static int	bpimk_filesbegincreate(BPIMK *,cchar *,int,mode_t) ;
static int	bpimk_filesend(BPIMK *) ;
static int	bpimk_listbegin(BPIMK *,int) ;
static int	bpimk_listend(BPIMK *) ;
static int	bpimk_mkidx(BPIMK *) ;
static int	bpimk_mkidxwrmain(BPIMK *,BPIHDR *) ;
static int	bpimk_mkidxwrhdr(BPIMK *,BPIHDR *,FILEBUF *,int) ;
static int	bpimk_mkidxwrtab(BPIMK *,BPIHDR *,FILEBUF *,int) ;
static int	bpimk_nidxopen(BPIMK *) ;
static int	bpimk_nidxclose(BPIMK *) ;
static int	bpimk_renamefiles(BPIMK *) ;

static int	mkcitation(uint *,BPIMK_VERSE *) ;
static int	mknewfname(char *,int,cchar *,cchar *) ;
static int	unlinkstale(cchar *,int) ;

static int	vvecmp(const void *,const void *) ;


/* local variables */


/* exported subroutines */


int bpimk_open(BPIMK *op,cchar dbname[],int of,mode_t om)
{
	const int	n = BPIMK_NENTRIES ;
	int		rs ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (dbname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(BPIMK)) ;
	op->om = (om|0600) ;
	op->nfd = -1 ;

	op->f.ofcreat = MKBOOL(of & O_CREAT) ;
	op->f.ofexcl = MKBOOL(of & O_EXCL) ;

	if ((rs = uc_mallocstrw(dbname,-1,&cp)) >= 0) {
	    op->dbname = cp ;
	    if ((rs = bpimk_filesbegin(op)) >= 0) {
	        if ((rs = bpimk_listbegin(op,n)) >= 0) {
	            op->magic = BPIMK_MAGIC ;
	        }
	        if (rs < 0)
	            bpimk_filesend(op) ;
	    } /* end if (files) */
	    if (rs < 0) {
	        if (op->dbname != NULL) {
	            uc_free(op->dbname) ;
	            op->dbname = NULL ;
	        }
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("bpimk_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bpimk_open) */


int bpimk_close(BPIMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		nverses = 0 ;
	int		f_go  ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BPIMK_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("bpimk_close: nverses=%u\n",op->nverses) ;
#endif

	f_go = (! op->f.abort) ;
	if (op->f.notsorted) {
	    vecobj_sort(&op->verses,vvecmp) ;
	}

	nverses = op->nverses ;
	if (nverses > 0) {
	    rs1 = bpimk_mkidx(op) ;
	    if (rs >= 0) rs = rs1 ;
	    f_go = f_go && (rs1 >= 0) ;
	}

#if	CF_DEBUGS
	debugprintf("bpimk_close: bpimk_mkidx() rs=%d\n",rs) ;
#endif

	if (op->nfd >= 0) {
	    rs1 = u_close(op->nfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfd = -1 ;
	}

	rs1 = bpimk_listend(op) ;
	if (rs >= 0) rs = rs1 ;
	f_go = f_go && (rs1 >= 0) ;

	if ((rs >= 0) && (nverses > 0) && f_go) {
	    rs1 = bpimk_renamefiles(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("bpimk_close: bpimk_renamefiles() rs=%d\n",rs) ;
#endif

	rs1 = bpimk_filesend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->dbname != NULL) {
	    rs1 = uc_free(op->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

	op->magic = 0 ;
	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bpimk_close) */


int bpimk_add(BPIMK *op,BPIMK_VERSE *bvp)
{
	struct bventry	bve ;
	uint		citcmpval ;
	uint		v ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (bvp == NULL) return SR_FAULT ;

	if (op->magic != BPIMK_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("bpimk_add: q=%u:%u:%u\n",
	    bvp->b,bvp->c,bvp->v) ;
#endif

	mkcitation(&bve.citation,bvp) ;

	citcmpval = (bve.citation & 0x00FFFFFF) ;
	if (citcmpval < op->pcitation)
	    op->f.notsorted = TRUE ;

	op->pcitation = citcmpval ;

	if ((rs = vecobj_add(&op->verses,&bve)) >= 0) {
	    op->nverses += 1 ;
	    if ((bvp->b > 0) && (bvp->c > 0) && (bvp->v > 0)) {
	        op->nzverses += 1 ;
	    }
	    v = bvp->b ;
	    if (v > op->maxbook) op->maxbook = v ;
	    v = bvp->c ;
	    if (v > op->maxchapter) op->maxchapter = v ;
	    v = bvp->v ;
	    if (v > op->maxverse) op->maxverse = v ;
	} /* end if (vecobj_add) */

#if	CF_DEBUGS && 0
	debugprintf("bpimk_add: ret=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bpimk_add) */


int bpimk_abort(BPIMK *op,int f)
{
	op->f.abort = f ;
	return SR_OK ;
}
/* end subroutine (bpimk_abort) */


int bpimk_info(BPIMK *op,BPIMK_INFO *bip)
{
	int		rs = SR_OK ;
	int		nverses = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (bip == NULL) return SR_FAULT ;

	if (op->magic != BPIMK_MAGIC) return SR_NOTOPEN ;

	if (bip != NULL) {
	    bip->maxbook = op->maxbook ;
	    bip->maxchapter = op->maxchapter ;
	    bip->maxverse = op->maxverse ;
	    bip->nverses = op->nverses ;
	    bip->nzverses = op->nzverses ;
	}

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bpimk_info) */


/* private subroutines */


static int bpimk_filesbegin(BPIMK *op)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (op->f.ofcreat) {
	    rs = bpimk_filesbeginc(op) ;
	} else {
	    rs = bpimk_filesbeginwait(op) ;
	    c = rs ;
	}
#if	CF_DEBUGS
	debugprintf("bpimk_filesbegin: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bpimk_filesbegin) */


static int bpimk_filesbeginc(BPIMK *op)
{
	const int	type = (op->f.ofcreat && (! op->f.ofexcl)) ;
	int		rs ;
	cchar		*dbn = op->dbname ;
	cchar		*suf = FSUF_IDX	 ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mknewfname(tbuf,type,dbn,suf)) >= 0) {
	    const mode_t	om = op->om ;
	    cchar		*tfn = tbuf ;
	    char		rbuf[MAXPATHLEN+1] ;
	    if (type) {
	        if ((rs = mktmpfile(rbuf,om,tbuf)) >= 0) {
	            op->f.created = TRUE ;
	            tfn = rbuf ;
	        }
	    }
	    if (rs >= 0) {
	        int	of = O_CREAT ;
	        if (op->f.ofexcl) of |= O_EXCL ;
	        rs = bpimk_filesbegincreate(op,tfn,of,om) ;
		if ((rs < 0) && type) {
		    uc_unlink(rbuf) ;
		}
	    } /* end if (ok) */
	} /* end if (mknewfname) */
	return rs ;
}
/* end subroutine (bpimk_filesbeginc) */


static int bpimk_filesbeginwait(BPIMK *op)
{
	int		rs ;
	int		c = 0 ;
	cchar		*dbn = op->dbname ;
	cchar		*suf = FSUF_IDX	 ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mknewfname(tbuf,FALSE,dbn,suf)) >= 0) {
	    const mode_t	om = op->om ;
	    const int		to_stale = BPIMK_INTSTALE ;
	    const int		nrs = SR_EXISTS ;
	    const int		of = (O_CREAT|O_WRONLY|O_EXCL) ;
	    int			to = BPIMK_INTOPEN ;
	    while ((rs = bpimk_filesbegincreate(op,tbuf,of,om)) == nrs) {
	        c = 1 ;
	        sleep(1) ;
	        unlinkstale(tbuf,to_stale) ;
	        if (to-- == 0) break ;
	    } /* end while (db exists) */
	    if (rs == nrs) {
	        op->f.ofcreat = FALSE ;
	        c = 0 ;
	        rs = bpimk_filesbeginc(op) ;
	    }
	} /* end if (mknewfname) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bpimk_filesbeginwait) */


static int bpimk_filesbegincreate(BPIMK *op,cchar *tfn,int of,mode_t om)
{
	int		rs ;
#if	CF_DEBUGS
	{
	    char	obuf[100+1] ;
	    snopenflags(obuf,100,of) ;
	    debugprintf("bpimk_filesbegincreate: ent of=%s\n",obuf) ;
	    debugprintf("bpimk_filesbegincreate: om=%05o\n",om) ;
	}
#endif
	if ((rs = uc_open(tfn,of,om)) >= 0) {
	    const int	fd = rs ;
	    cchar	*cp ;
	    op->f.created = TRUE ;
	    if ((rs = uc_mallocstrw(tfn,-1,&cp)) >= 0) {
	        op->nidxfname = (char *) cp ;
	    }
	    u_close(fd) ;
	} /* end if (create) */

#if	CF_DEBUGS
	debugprintf("bpimk_filesbegincreate: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bpimk_filesbegincreate) */


static int bpimk_filesend(BPIMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->nidxfname != NULL) {
	    if (op->f.created && (op->nidxfname[0] != '\0')) {
	        u_unlink(op->nidxfname) ;
	    }
	    rs1 = uc_free(op->nidxfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nidxfname = NULL ;
	}

	if (op->idname != NULL) {
	    rs1 = uc_free(op->idname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->idname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("bpimk_filesend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bpimk_filesend) */


static int bpimk_listbegin(BPIMK *op,int n)
{
	int		rs ;
	int		size ;
	int		opts ;

	opts = 0 ;
	opts |= VECOBJ_OCOMPACT ;
	opts |= VECOBJ_OORDERED ;
	opts |= VECOBJ_OSTATIONARY ;
	size = sizeof(struct bventry) ;
	rs = vecobj_start(&op->verses,size,n,opts) ;

	return rs ;
}
/* end subroutine (bpimk_listbegin) */


static int bpimk_listend(BPIMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecobj_finish(&op->verses) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (bpimk_listend) */


static int bpimk_mkidx(BPIMK *op)
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = bpimk_nidxopen(op)) >= 0) {
	    BPIHDR	hdr ;

	    memset(&hdr,0,sizeof(BPIHDR)) ;
	    hdr.vetu[0] = BPIHDR_VERSION ;
	    hdr.vetu[1] = ENDIAN ;
	    hdr.vetu[2] = 0 ;
	    hdr.vetu[3] = 0 ;
	    hdr.wtime = (uint) time(NULL) ;
	    hdr.nverses = op->nverses ;
	    hdr.nzverses = op->nzverses ;
	    hdr.maxbook = op->maxbook ;
	    hdr.maxchapter = op->maxchapter ;

	    if ((rs = bpimk_mkidxwrmain(op,&hdr)) >= 0) {
	        const int	hlen = HDRBUFLEN ;
	        char		hbuf[HDRBUFLEN+1] ;
	        hdr.fsize = rs ;
	        wlen = rs ;

	        if ((rs = bpihdr(&hdr,0,hbuf,hlen)) >= 0) {
	            const int	bl = rs ;
	            if ((rs = u_pwrite(op->nfd,hbuf,bl,0L)) >= 0) {
	                const mode_t	om = op->om ;
	                rs = uc_fminmod(op->nfd,om) ;
	            }
	        }

	    } /* end if (bpimk_mkidxwrmain) */

	    rs1 = bpimk_nidxclose(op) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bpimk_nidx) */

#if	CF_DEBUGS
	debugprintf("bpimk_mkidx: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bpimk_mkidx) */


static int bpimk_mkidxwrmain(BPIMK *op,BPIHDR *hdrp)
{
	FILEBUF		hf, *hfp = &hf ;
	const int	nfd = op->nfd ;
	const int	ps = getpagesize() ;
	int		bsize ;
	int		rs ;
	int		rs1 ;
	int		off = 0 ;
	bsize = (ps * 4) ;
	if ((rs = filebuf_start(hfp,nfd,0,bsize,0)) >= 0) {
	    if ((rs = bpimk_mkidxwrhdr(op,hdrp,hfp,off)) >= 0) {
	        off += rs ;
	        if (rs >= 0) {
	            rs = bpimk_mkidxwrtab(op,hdrp,hfp,off) ;
	            off += rs ;
	        }
	    } /* end if (bpimk_mkidxwrhdr) */
	    rs1 = filebuf_finish(hfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */
	return (rs >= 0) ? off : rs ;
}
/* end subroutine (bpimk_mkidxwrmain) */


/* ARGSUSED */
static int bpimk_mkidxwrhdr(BPIMK *op,BPIHDR *hdrp,FILEBUF *hfp,int off)
{
	const int	hlen = HDRBUFLEN ;
	int		rs ;
	int		wlen = 0 ;
	char		hbuf[HDRBUFLEN+1] ;
	if (op == NULL) return SR_FAULT ; /* LINT */
	if ((rs = bpihdr(hdrp,0,hbuf,hlen)) >= 0) {
	    const int	bl = rs ;
	    rs = filebuf_writefill(hfp,hbuf,bl) ;
	    wlen += rs ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bpimk_mkidxwrhdr) */


static int bpimk_mkidxwrtab(BPIMK *op,BPIHDR *hdrp,FILEBUF *hfp,int off)
{
	struct bventry	*bvep ;
	uint		a[4] ;
	const int	size = (1 * sizeof(uint)) ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	int		n = 0 ;
	int		i ;
	hdrp->vioff = off ;
	for (i = 0 ; vecobj_get(&op->verses,i,&bvep) >= 0 ; i += 1) {
	    if (bvep != NULL) {
	        a[0] = bvep->citation ;
	        rs = filebuf_write(hfp,a,size) ;
	        wlen += rs ;
	        n += 1 ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	hdrp->vilen = n ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bpimk_mkidxwrtab) */


static int bpimk_nidxopen(BPIMK *op)
{
	const mode_t	om = op->om ;
	int		rs ;
	int		fd = -1 ;
	int		of = (O_CREAT|O_WRONLY) ;
	if (op->nidxfname == NULL) {
	    const int	type = (op->f.ofcreat && (! op->f.ofexcl)) ;
	    cchar	*dbn = op->dbname ;
	    cchar	*suf = FSUF_IDX ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mknewfname(tbuf,type,dbn,suf)) >= 0) {
	        cchar	*tfn = tbuf ;
	        char	rbuf[MAXPATHLEN+1] ;
	        if (type) {
	            rs = opentmpfile(tbuf,of,om,rbuf) ;
	            op->nfd = rs ;
		    fd = rs ;
	            tfn = rbuf ;
	        } else {
	            if (op->f.ofexcl) of |= O_EXCL ;
	            rs = uc_open(tbuf,of,om) ;
	            op->nfd = rs ;
		    fd = rs ;
	        }
	        if (rs >= 0) {
	            cchar	*cp ;
	            if ((rs = uc_mallocstrw(tfn,-1,&cp)) >= 0) {
	                op->nidxfname = (char *) cp ;
	            }
	        } /* end if (ok) */
	    } /* end if (mknewfname) */
	} else {
	    if (op->f.ofexcl) of |= O_EXCL ;
	    rs = uc_open(op->nidxfname,of,om) ;
	    op->nfd = rs ;
	    fd = rs ;
	}
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (bpimk_nidxopen) */


static int bpimk_nidxclose(BPIMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->nfd >= 0) {
	    rs1 = u_close(op->nfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfd = -1 ;
	}
	return rs ;
}
/* end subroutine (bpimk_nidxclose) */


static int bpimk_renamefiles(BPIMK *op)
{
	int		rs ;
	const char	*suf = FSUF_IDX ;
	const char	*end = ENDIANSTR ;
	char		idxfname[MAXPATHLEN + 1] ;

	if ((rs = mkfnamesuf2(idxfname,op->dbname,suf,end)) >= 0) {
	    if ((rs = u_rename(op->nidxfname,idxfname)) >= 0) {
	        op->nidxfname[0] = '\0' ;
	    } else {
	        u_unlink(op->nidxfname) ;
	        op->nidxfname[0] = '\0' ;
	    }
	} /* end if (mkfnamesuf) */

	return rs ;
}
/* end subroutine (bpimk_renamefiles) */


static int mkcitation(uint *cip,BPIMK_VERSE *bvp)
{
	uint		ci = 0 ;
	uint		nlines = 0 ;

	ci |= (nlines & UCHAR_MAX) ;

	ci = (ci << 8) ;
	ci |= (bvp->b & UCHAR_MAX) ;

	ci = (ci << 8) ;
	ci |= (bvp->c & UCHAR_MAX) ;

	ci = (ci << 8) ;
	ci |= (bvp->v & UCHAR_MAX) ;

	*cip = ci ;
	return SR_OK ;
}
/* end subroutine (mkcitation) */


static int mknewfname(char *tbuf,int type,cchar *dbn,cchar *suf)
{
	cchar		*end = ENDIANSTR ;
	cchar		*fin = (type) ? "xXXXX" : "n" ;
	return mkfnamesuf3(tbuf,dbn,suf,end,fin) ;
}
/* end subroutine (mknewfname) */


static int unlinkstale(cchar *fn,int to)
{
	struct ustat	sb ;
	const time_t	dt = time(NULL) ;
	int		rs ;
	if ((rs = uc_stat(fn,&sb)) >= 0) {
	    if ((dt-sb.st_mtime) >= to) {
	        uc_unlink(fn) ;
	        rs = 1 ;
	    } else {
	        rs = 0 ;
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
	return rs ;
}
/* end subroutine (unlinkstale) */


static int vvecmp(const void *v1p,const void *v2p)
{
	struct bventry	**e1pp = (struct bventry **) v1p ;
	struct bventry	**e2pp = (struct bventry **) v2p ;
	int		rc = 0 ;

	if (*e1pp != NULL) {
	    if (*e2pp != NULL) {
	        uint	vc1 = (*e1pp)->citation & 0x00FFFFFF ;
	        uint	vc2 = (*e2pp)->citation & 0x00FFFFFF ;
	        rc = (vc1 - vc2) ;
	    } else
	        rc = -1 ;
	} else
	    rc = 1 ;

	return rc ;
}
/* end subroutine (vvecmp) */


