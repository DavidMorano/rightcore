/* bvimk */

/* make a BVI database */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module creates a BVI database file.

	Synopsis:

	int bvimk_open(op,dbname,...)
	BVIMK		*op ;
	cchar		dbname[] ;

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


#define	BVIMK_MASTER	0


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
#include	<estrings.h>
#include	<vecobj.h>
#include	<filebuf.h>
#include	<char.h>
#include	<localmisc.h>

#include	"bvimk.h"
#include	"bvihdr.h"


/* local defines */

#define	BVIMK_NENTRIES	(19 * 1024)
#define	BVIMK_NSKIP	5
#define	HDRBUFLEN	(sizeof(BVIHDR) + 128)

#define	BUFLEN		(sizeof(BVIHDR) + 128)

#define	FSUF_IDX	"bvi"

#define	TO_OLDFILE	(5 * 60)

#define	MODP2(v,n)	((v) & ((n) - 1))


/* external subroutines */

extern uint	uceil(uint,int) ;

extern int	mkfnamesuf2(char *,cchar *,cchar *,cchar *) ;
extern int	mkfnamesuf3(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfhexi(cchar *,int,uint *) ;
extern int	getpwd(char *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	mktmpfile(char *,mode_t,cchar *) ;
extern int	filebuf_writefill(FILEBUF *,const void *,int) ;
extern int	iceil(int,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	snopenflags(char *,int,int) ;
extern int	debugprintf(cchar *,...) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;


/* external variables */


/* exported variables */

BVIMK_OBJ	bvimk = {
	"bvimk",
	sizeof(BVIMK)
} ;


/* local structures */

struct bventry {
	uint	voff ;
	uint	vlen ;
	uint	li ;			/* index-number of first line-entry */
	uint	citation ;		/* (nlines, b, c, v) */
} ;

struct blentry {
	uint	loff ;
	uint	llen ;
} ;


/* forward references */

static int	bvimk_filesbegin(BVIMK *) ;
static int	bvimk_filesbeginc(BVIMK *) ;
static int	bvimk_filesbeginwait(BVIMK *) ;
static int	bvimk_filesbegincreate(BVIMK *,cchar *,int,mode_t) ;
static int	bvimk_filesend(BVIMK *) ;
static int	bvimk_listbegin(BVIMK *,int) ;
static int	bvimk_listend(BVIMK *) ;
static int	bvimk_mkidx(BVIMK *) ;
static int	bvimk_mkidxwrmain(BVIMK *,BVIHDR *) ;
static int	bvimk_mkidxwrhdr(BVIMK *,BVIHDR *,FILEBUF *) ;
static int	bvimk_mkidxwrverses(BVIMK *,BVIHDR *,FILEBUF *,int) ;
static int	bvimk_mkidxwrlines(BVIMK *,BVIHDR *,FILEBUF *,int) ;
static int	bvimk_nidxopen(BVIMK *) ;
static int	bvimk_nidxclose(BVIMK *) ;
static int	bvimk_renamefiles(BVIMK *) ;

static int	mkcitation(uint *,BVIMK_VERSE *) ;
static int	mknewfname(char *,int,cchar *,cchar *) ;
static int	unlinkstale(cchar *,int) ;

static int	vvecmp(const void *,const void *) ;


/* local variables */


/* exported subroutines */


int bvimk_open(BVIMK *op,cchar *dbname,int of,mode_t om)
{
	const int	n = BVIMK_NENTRIES ;
	int		rs ;
	int		c = 0 ;
	cchar		*cp ;

	if (op == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	{
	    char	obuf[100+1] ;
	    snopenflags(obuf,100,of) ;
	    debugprintf("bvimk_open: ent dbname=%s\n",dbname) ;
	    debugprintf("bvimk_open: of=%s\n",obuf) ;
	    debugprintf("bvimk_open: om=%05o\n",om) ;
	}
#endif /* CF_DEBUGS */

	if (dbname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(BVIMK)) ;
	op->om = (om|0600) ;
	op->nfd = -1 ;

	op->f.ofcreat = MKBOOL(of & O_CREAT) ;
	op->f.ofexcl = MKBOOL(of & O_EXCL) ;

	if ((rs = uc_mallocstrw(dbname,-1,&cp)) >= 0) {
	    op->dbname = cp ;
	    if ((rs = bvimk_filesbegin(op)) >= 0) {
	        c = rs ;
	        if ((rs = bvimk_listbegin(op,n)) >= 0) {
	            op->magic = BVIMK_MAGIC ;
	        }
	        if (rs < 0)
	            bvimk_filesend(op) ;
	    } /* end if (nvimk) */
	    if (rs < 0) {
	        uc_free(op->dbname) ;
	        op->dbname = NULL ;
	    }
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("bvimk_open: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bvimk_open) */


int bvimk_close(BVIMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		nverses = 0 ;
	int		f_go = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVIMK_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("bvimk_close: nverses=%u\n",op->nverses) ;
#endif
	f_go = (! op->f.abort) ;
	if (op->f.notsorted) {
	    vecobj_sort(&op->verses,vvecmp) ;
	}

	nverses = op->nverses ;
	if (nverses > 0) {
	    rs1 = bvimk_mkidx(op) ;
	    if (rs >= 0) rs = rs1 ;
	    f_go = f_go && (rs1 >= 0) ;
	}

#if	CF_DEBUGS
	debugprintf("bvimk_close: bvimk_mkidx() rs=%d\n",rs) ;
#endif

	if (op->nfd >= 0) {
	    rs1 = u_close(op->nfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfd = -1 ;
	}

	rs1 = bvimk_listend(op) ;
	if (rs >= 0) rs = rs1 ;
	f_go = f_go && (rs1 >= 0) ;

	if ((rs >= 0) && (nverses > 0) && f_go) {
	    rs1 = bvimk_renamefiles(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("bvimk_close: bvimk_renamefiles() rs=%d\n",rs) ;
#endif

	rs1 = bvimk_filesend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->dbname != NULL) {
	    rs1 = uc_free(op->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("bvimk_close: ret rs=%d nv=%u\n",rs,nverses) ;
#endif

	op->magic = 0 ;
	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bvimk_close) */


int bvimk_add(BVIMK *op,BVIMK_VERSE *bvp)
{
	struct bventry	bve ;
	struct blentry	ble ;
	uint		li = UINT_MAX ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (bvp == NULL) return SR_FAULT ;

	if (op->magic != BVIMK_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("bvimk_add: q=%u:%u:%u\n",
	    bvp->b,bvp->c,bvp->v) ;
#endif

	if ((bvp->lines != NULL) && (bvp->nlines > 0)) {
	    int	i ;
	    for (i = 0 ; i < bvp->nlines ; i += 1) {

	        ble.loff = bvp->lines[i].loff ;
	        ble.llen = bvp->lines[i].llen ;
	        rs = vecobj_add(&op->lines,&ble) ;
	        if (i == 0) li = rs ;

	        if (rs < 0) break ;
	    } /* end for */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("bvimk_add: li=%u\n",li) ;
#endif

	if (rs >= 0) {
	    uint	citcmpval ;

	    bve.voff = bvp->voff ;
	    bve.vlen = bvp->vlen ;
	    bve.li = li ;
	    mkcitation(&bve.citation,bvp) ;

	    citcmpval = (bve.citation & 0x00FFFFFF) ;
	    if (citcmpval < op->pcitation) {
	        op->f.notsorted = TRUE ;
	    }

	    op->pcitation = citcmpval ;

	    if ((rs = vecobj_add(&op->verses,&bve)) >= 0) {
		uint	v ;
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
	    } /* end if (add) */

	} /* end if (ok) */

#if	CF_DEBUGS && 0
	debugprintf("bvimk_add: ret=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvimk_add) */


int bvimk_abort(BVIMK *op,int f)
{
	op->f.abort = f ;
	return SR_OK ;
}
/* end subroutine (bvimk_abort) */


int bvimk_info(BVIMK *op,BVIMK_INFO *bip)
{
	int		rs = SR_OK ;
	int		nverses ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVIMK_MAGIC) return SR_NOTOPEN ;

	nverses = op->nverses ;
	if (bip != NULL) {
	    bip->maxbook = op->maxbook ;
	    bip->maxchapter = op->maxchapter ;
	    bip->maxverse = op->maxverse ;
	    bip->nverses = op->nverses ;
	    bip->nzverses = op->nzverses ;
	}

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bvimk_info) */


/* private subroutines */


static int bvimk_filesbegin(BVIMK *op)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (op->f.ofcreat) {
	    rs = bvimk_filesbeginc(op) ;
	} else {
	    rs = bvimk_filesbeginwait(op) ;
	    c = rs ;
	}
#if	CF_DEBUGS
	debugprintf("bvimk_filesbegin: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bvimk_filesbegin) */


static int bvimk_filesbeginc(BVIMK *op)
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
	        mode_t	om = op->om ;
	        int	of = O_CREAT ;
	        if (op->f.ofexcl) of |= O_EXCL ;
	        rs = bvimk_filesbegincreate(op,tfn,of,om) ;
		if ((rs < 0) && type) {
		    uc_unlink(rbuf) ;
		}
	    } /* end if (ok) */
	} /* end if (mknewfname) */
	return rs ;
}
/* end subroutine (bvimk_filesbeginc) */


static int bvimk_filesbeginwait(BVIMK *op)
{
	int		rs ;
	int		c = 0 ;
	cchar		*dbn = op->dbname ;
	cchar		*suf = FSUF_IDX	 ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mknewfname(tbuf,FALSE,dbn,suf)) >= 0) {
	    const mode_t	om = op->om ;
	    const int		to_stale = BVIMK_INTSTALE ;
	    const int		nrs = SR_EXISTS ;
	    const int		of = (O_CREAT|O_WRONLY|O_EXCL) ;
	    int			to = BVIMK_INTOPEN ;
	    while ((rs = bvimk_filesbegincreate(op,tbuf,of,om)) == nrs) {
#if	CF_DEBUGS
	        debugprintf("bvimk_filesbeginwait: loop ret rs=%d\n",rs) ;
#endif
	        c = 1 ;
	        sleep(1) ;
	        unlinkstale(tbuf,to_stale) ;
	        if (to-- == 0) break ;
	    } /* end while (db exists) */
	    if (rs == nrs) {
	        op->f.ofcreat = FALSE ;
	        c = 0 ;
	        rs = bvimk_filesbeginc(op) ;
	    }
	} /* end if (mknewfname) */
#if	CF_DEBUGS
	debugprintf("bvimk_filesbeginwait: ret ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bvimk_filesbeginwait) */


static int bvimk_filesbegincreate(BVIMK *op,cchar *tfn,int of,mode_t om)
{
	int		rs ;
#if	CF_DEBUGS
	{
	    char	obuf[100+1] ;
	    snopenflags(obuf,100,of) ;
	    debugprintf("bvimk_filesbegincreate: ent of=%s\n",obuf) ;
	    debugprintf("bvimk_filesbegincreate: om=%05o\n",om) ;
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
	debugprintf("bvimk_filesbegincreate: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvimk_filesbegincreate) */


static int bvimk_filesend(BVIMK *op)
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
	debugprintf("bvimk_filesend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvimk_filesend) */


static int bvimk_listbegin(BVIMK *op,int n)
{
	int		rs ;
	int		size ;
	int		opts ;

	opts = 0 ;
	opts |= VECOBJ_OCOMPACT ;
	opts |= VECOBJ_OORDERED ;
	opts |= VECOBJ_OSTATIONARY ;
	size = sizeof(struct bventry) ;
	if ((rs = vecobj_start(&op->verses,size,n,opts)) >= 0) {
	    rs = vecobj_start(&op->lines,size,(n * 2),opts) ;
	    if (rs < 0)
	        vecobj_finish(&op->verses) ;
	}

	return rs ;
}
/* end subroutine (bvimk_listbegin) */


static int bvimk_listend(BVIMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecobj_finish(&op->lines) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->verses) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (bvimk_listend) */


static int bvimk_mkidx(BVIMK *op)
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	debugprintf("bvimk_mkidx: ent\n") ;
#endif

	if ((rs = bvimk_nidxopen(op)) >= 0) {
	    BVIHDR	hdr ;

	    memset(&hdr,0,sizeof(BVIHDR)) ;
	    hdr.vetu[0] = BVIHDR_VERSION ;
	    hdr.vetu[1] = ENDIAN ;
	    hdr.vetu[2] = 0 ;
	    hdr.vetu[3] = 0 ;
	    hdr.wtime = (uint) time(NULL) ;
	    hdr.nverses = op->nverses ;
	    hdr.nzverses = op->nzverses ;
	    hdr.maxbook = op->maxbook ;
	    hdr.maxchapter = op->maxchapter ;

	    if ((rs = bvimk_mkidxwrmain(op,&hdr)) >= 0) {
	        const int	hlen = HDRBUFLEN ;
	        char		hbuf[HDRBUFLEN+1] ;
	        hdr.fsize = rs ;
	        wlen = rs ;

	        if ((rs = bvihdr(&hdr,0,hbuf,hlen)) >= 0) {
	            const int	bl = rs ;
	            if ((rs = u_pwrite(op->nfd,hbuf,bl,0L)) >= 0) {
	                const mode_t	om = op->om ;
	                rs = uc_fminmod(op->nfd,om) ;
	            }
	        }

	    } /* end if (bvimk_mkidxwrmain) */

	    rs1 = bvimk_nidxclose(op) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bvimk_nidx) */

#if	CF_DEBUGS
	debugprintf("bvimk_mkidx: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bvimk_mkidx) */


static int bvimk_mkidxwrmain(BVIMK *op,BVIHDR *hdrp)
{
	FILEBUF		hf, *hfp = &hf ;
	const int	nfd = op->nfd ;
	const int	ps = getpagesize() ;
	int		bsize ;
	int		rs ;
	int		rs1 ;
	int		off = 0 ;
#if	CF_DEBUGS
	debugprintf("bvimk_mkidxwrmain: ent\n") ;
#endif
	bsize = (ps * 4) ;
	if ((rs = filebuf_start(hfp,nfd,0,bsize,0)) >= 0) {
	    if ((rs = bvimk_mkidxwrhdr(op,hdrp,hfp)) >= 0) {
	        off += rs ;
	        if (rs >= 0) {
	            rs = bvimk_mkidxwrverses(op,hdrp,hfp,off) ;
	            off += rs ;
	        }
	        if (rs >= 0) {
	            rs = bvimk_mkidxwrlines(op,hdrp,hfp,off) ;
	            off += rs ;
	        }
	    } /* end if (bvimk_mkidxwrhdr) */
	    rs1 = filebuf_finish(hfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */
#if	CF_DEBUGS
	debugprintf("bvimk_mkidxwrmain: ret rs=%d off=%u\n",rs,off) ;
#endif
	return (rs >= 0) ? off : rs ;
}
/* end subroutine (bvimk_mkidxwrmain) */


/* ARGSUSED */
static int bvimk_mkidxwrhdr(BVIMK *op,BVIHDR *hdrp,FILEBUF *hfp)
{
	const int	hlen = HDRBUFLEN ;
	int		rs ;
	int		wlen = 0 ;
	char		hbuf[HDRBUFLEN+1] ;
#if	CF_DEBUGS
	debugprintf("bvimk_mkidxwrhdr: ent\n") ;
#endif
	if (op == NULL) return SR_FAULT ; /* LINT */
	if ((rs = bvihdr(hdrp,0,hbuf,hlen)) >= 0) {
	    rs = filebuf_writefill(hfp,hbuf,rs) ;
	    wlen += rs ;
#if	CF_DEBUGS
	debugprintf("bvimk_mkidxwrhdr: filebuf_writefill() rs=%d\n",rs) ;
#endif
	}
#if	CF_DEBUGS
	debugprintf("bvimk_mkidxwrhdr: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bvimk_mkidxwrhdr) */


static int bvimk_mkidxwrverses(BVIMK *op,BVIHDR *hdrp,FILEBUF *hfp,int off)
{
	struct bventry	*bvep ;
	uint		a[4] ;
	const int	size = (4 * sizeof(uint)) ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	int		n = 0 ;
	int		i ;
	hdrp->vioff = off ;
	for (i = 0 ; vecobj_get(&op->verses,i,&bvep) >= 0 ; i += 1) {
	    if (bvep != NULL) {
	        a[0] = bvep->voff ;
	        a[1] = bvep->vlen ;
	        a[2] = bvep->li ;
	        a[3] = bvep->citation ;
	        n += 1 ;
	        rs = filebuf_write(hfp,a,size) ;
	        wlen += rs ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	hdrp->vilen = n ;
#if	CF_DEBUGS
	debugprintf("bvimk_mkidxwrverses: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bvimk_mkidxwrverses) */


static int bvimk_mkidxwrlines(BVIMK *op,BVIHDR *hdrp,FILEBUF *hfp,int off)
{
	struct blentry	*blep ;
	uint		a[4] ;
	const int	size = (2 * sizeof(uint)) ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	int		n = 0 ;
	int		i ;
	hdrp->vloff = off ;
	for (i = 0 ; vecobj_get(&op->lines,i,&blep) >= 0 ; i += 1) {
	    if (blep != NULL) {
	        a[0] = blep->loff ;
	        a[1] = blep->llen ;
	        n += 1 ;
	        rs = filebuf_write(hfp,a,size) ;
	        wlen += rs ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	hdrp->vllen = n ;
#if	CF_DEBUGS
	debugprintf("bvimk_mkidxwrlines: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bvimk_mkidxwrlines) */


static int bvimk_nidxopen(BVIMK *op)
{
	const mode_t	om = op->om ;
	int		rs ;
	int		fd = -1 ;
	int		of = (O_CREAT|O_WRONLY) ;
#if	CF_DEBUGS
	debugprintf("bvimk_nidxopen: ent nidxfname=%s\n",op->nidxfname) ;
#endif
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
#if	CF_DEBUGS
	debugprintf("bvimk_nidxopen: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (bvimk_nidxopen) */


static int bvimk_nidxclose(BVIMK *op)
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
/* end subroutine (bvimk_nidxclose) */


static int bvimk_renamefiles(BVIMK *op)
{
	int		rs ;
	cchar		*suf = FSUF_IDX ;
	cchar		*end = ENDIANSTR ;
	char		idxfname[MAXPATHLEN + 1] ;

	if ((rs = mkfnamesuf2(idxfname,op->dbname,suf,end)) >= 0) {
	    if ((rs = u_rename(op->nidxfname,idxfname)) >= 0) {
	        op->nidxfname[0] = '\0' ;
	    } else {
	        u_unlink(op->nidxfname) ;
	        op->nidxfname[0] = '\0' ;
	    } /* end if (rename) */
	} /* end if (mkfnamesuf) */

	return rs ;
}
/* end subroutine (bvimk_renamefiles) */


static int mkcitation(uint *cip,BVIMK_VERSE *bvp)
{
	uint		ci = 0 ;
	uint		nlines = 0 ;

	if (bvp->lines != NULL)
	    nlines = bvp->nlines ;

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


