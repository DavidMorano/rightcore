/* cmimk */

/* make a CMI database */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module creates a CMI database file.

	Synopsis:

	int cmimk_open(op,dbname)
	CMIMK		*op ;
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


#define	CMIMK_MASTER	0


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
#include	<estrings.h>
#include	<vecobj.h>
#include	<filebuf.h>
#include	<char.h>
#include	<localmisc.h>

#include	"cmimk.h"
#include	"cmihdr.h"


/* local defines */

#define	CMIMK_NENTS	(19 * 1024)
#define	CMIMK_NSKIP	5
#define	HDRBUFLEN	(sizeof(CMIHDR) + 128)

#define	BUFLEN		(sizeof(CMIHDR) + 128)

#define	FSUF_IDX	"cmi"

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

CMIMK_OBJ	cmimk = {
	"cmimk",
	sizeof(CMIMK)
} ;


/* local structures */

struct cmentry {
	uint		eoff ;
	uint		elen ;
	uint		li ;		/* index-number of first line-entry */
	ushort		nlines ;
	ushort		cn ;		/* command-number */
} ;

struct blentry {
	uint		loff ;
	uint		llen ;
} ;


/* forward references */

static int	cmimk_filesbegin(CMIMK *) ;
static int	cmimk_filesbeginc(CMIMK *) ;
static int	cmimk_filesbeginwait(CMIMK *) ;
static int	cmimk_filesbegincreate(CMIMK *,cchar *,int,mode_t) ;
static int	cmimk_filesend(CMIMK *) ;
static int	cmimk_listbegin(CMIMK *,int) ;
static int	cmimk_listend(CMIMK *) ;
static int	cmimk_mkidx(CMIMK *) ;
static int	cmimk_mkidxwrmain(CMIMK *,CMIHDR *) ;
static int	cmimk_mkidxwrhdr(CMIMK *,CMIHDR *,FILEBUF *) ;
static int	cmimk_mkidxwrents(CMIMK *,CMIHDR *,FILEBUF *,int) ;
static int	cmimk_mkidxwrlines(CMIMK *,CMIHDR *,FILEBUF *,int) ;
static int	cmimk_nidxopen(CMIMK *) ;
static int	cmimk_nidxclose(CMIMK *) ;
static int	cmimk_renamefiles(CMIMK *) ;

static int	mknewfname(char *,int,cchar *,cchar *) ;
static int	unlinkstale(cchar *,int) ;

static int	vvecmp(const void *,const void *) ;


/* local variables */


/* exported subroutines */


int cmimk_open(CMIMK *op,cchar *dbname,int of,mode_t om)
{
	const int	n = CMIMK_NENTS ;
	int		rs ;
	int		c = 0 ;
	cchar		*cp ;

	if (op == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	{
	    char	obuf[100+1] ;
	    snopenflags(obuf,100,of) ;
	    debugprintf("cmimk_open: ent dbname=%s\n",dbname) ;
	    debugprintf("cmimk_open: of=%s\n",obuf) ;
	    debugprintf("cmimk_open: om=%05o\n",om) ;
	}
#endif /* CF_DEBUGS */

	if (dbname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(CMIMK)) ;
	op->om = (om|0600) ;
	op->nfd = -1 ;

	op->f.ofcreat = MKBOOL(of & O_CREAT) ;
	op->f.ofexcl = MKBOOL(of & O_EXCL) ;

	if ((rs = uc_mallocstrw(dbname,-1,&cp)) >= 0) {
	    op->dbname = cp ;
	    if ((rs = cmimk_filesbegin(op)) >= 0) {
	        c = rs ;
	        if ((rs = cmimk_listbegin(op,n)) >= 0) {
	            op->magic = CMIMK_MAGIC ;
	        }
	        if (rs < 0)
	            cmimk_filesend(op) ;
	    } /* end if (nvimk) */
	    if (rs < 0) {
	        uc_free(op->dbname) ;
	        op->dbname = NULL ;
	    }
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("cmimk_open: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (cmimk_open) */


int cmimk_close(CMIMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		nents = 0 ;
	int		f_go = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CMIMK_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("cmimk_close: nents=%u\n",op->nents) ;
#endif
	f_go = (! op->f.abort) ;
	if (op->f.notsorted) {
	    vecobj_sort(&op->ents,vvecmp) ;
	}

	nents = op->nents ;
	if (nents > 0) {
	    rs1 = cmimk_mkidx(op) ;
	    if (rs >= 0) rs = rs1 ;
	    f_go = f_go && (rs1 >= 0) ;
	}

#if	CF_DEBUGS
	debugprintf("cmimk_close: cmimk_mkidx() rs=%d\n",rs) ;
#endif

	if (op->nfd >= 0) {
	    rs1 = u_close(op->nfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfd = -1 ;
	}

	rs1 = cmimk_listend(op) ;
	if (rs >= 0) rs = rs1 ;
	f_go = f_go && (rs1 >= 0) ;

	if ((rs >= 0) && (nents > 0) && f_go) {
	    rs1 = cmimk_renamefiles(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("cmimk_close: cmimk_renamefiles() rs=%d\n",rs) ;
#endif

	rs1 = cmimk_filesend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->dbname != NULL) {
	    rs1 = uc_free(op->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("cmimk_close: ret rs=%d nv=%u\n",rs,nents) ;
#endif

	op->magic = 0 ;
	return (rs >= 0) ? nents : rs ;
}
/* end subroutine (cmimk_close) */


int cmimk_setdb(CMIMK *op,size_t size_db,time_t ti_db)
{
	if (op == NULL) return SR_FAULT ;
	op->size_db = size_db ;
	op->ti_db = ti_db ;
	return SR_OK ;
}
/* end subroutine (cmimk_setdb) */


int cmimk_add(CMIMK *op,CMIMK_ENT *bvp)
{
	uint		li = UINT_MAX ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (bvp == NULL) return SR_FAULT ;

	if (op->magic != CMIMK_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("cmimk_add: cn=%u\n",bvp->cn) ;
#endif

	if ((bvp->lines != NULL) && (bvp->nlines > 0)) {
	    struct blentry	ble ;
	    int			i ;
	    for (i = 0 ; i < bvp->nlines ; i += 1) {

	        ble.loff = bvp->lines[i].loff ;
	        ble.llen = bvp->lines[i].llen ;
	        rs = vecobj_add(&op->lines,&ble) ;
	        if (i == 0) li = rs ;

	        if (rs < 0) break ;
	    } /* end for */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("cmimk_add: li=%u\n",li) ;
#endif

	if (rs >= 0) {
	    struct cmentry	cme ;

	    cme.eoff = bvp->eoff ;
	    cme.elen = bvp->elen ;
	    cme.li = li ;
	    cme.nlines = bvp->nlines ;
	    cme.cn = bvp->cn ;

	    {
		uint	cn = bvp->cn ;
	        if (cn < op->pcn) op->f.notsorted = TRUE ;
	        op->pcn = cn ;
	    }

	    if ((rs = vecobj_add(&op->ents,&cme)) >= 0) {
	        op->nents += 1 ;
	        if (cme.cn > op->maxent) op->maxent = cme.cn ;
	    } /* end if (add) */

	} /* end if (ok) */

#if	CF_DEBUGS && 0
	debugprintf("cmimk_add: ret=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cmimk_add) */


int cmimk_abort(CMIMK *op,int f)
{
	op->f.abort = f ;
	return SR_OK ;
}
/* end subroutine (cmimk_abort) */


int cmimk_info(CMIMK *op,CMIMK_INFO *bip)
{
	int		rs = SR_OK ;
	int		nents ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CMIMK_MAGIC) return SR_NOTOPEN ;

	nents = op->nents ;
	if (bip != NULL) {
	    bip->maxent = op->maxent ;
	    bip->nents = op->nents ;
	}

	return (rs >= 0) ? nents : rs ;
}
/* end subroutine (cmimk_info) */


/* private subroutines */


static int cmimk_filesbegin(CMIMK *op)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (op->f.ofcreat) {
	    rs = cmimk_filesbeginc(op) ;
	} else {
	    rs = cmimk_filesbeginwait(op) ;
	    c = rs ;
	}
#if	CF_DEBUGS
	debugprintf("cmimk_filesbegin: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (cmimk_filesbegin) */


static int cmimk_filesbeginc(CMIMK *op)
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
	        rs = cmimk_filesbegincreate(op,tfn,of,om) ;
		if ((rs < 0) && type) {
		    uc_unlink(rbuf) ;
		}
	    } /* end if (ok) */
	} /* end if (mknewfname) */
	return rs ;
}
/* end subroutine (cmimk_filesbeginc) */


static int cmimk_filesbeginwait(CMIMK *op)
{
	int		rs ;
	int		c = 0 ;
	cchar		*dbn = op->dbname ;
	cchar		*suf = FSUF_IDX	 ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mknewfname(tbuf,FALSE,dbn,suf)) >= 0) {
	    const mode_t	om = op->om ;
	    const int		to_stale = CMIMK_INTSTALE ;
	    const int		nrs = SR_EXISTS ;
	    const int		of = (O_CREAT|O_WRONLY|O_EXCL) ;
	    int			to = CMIMK_INTOPEN ;
	    while ((rs = cmimk_filesbegincreate(op,tbuf,of,om)) == nrs) {
#if	CF_DEBUGS
	        debugprintf("cmimk_filesbeginwait: loop ret rs=%d\n",rs) ;
#endif
	        c = 1 ;
	        sleep(1) ;
	        unlinkstale(tbuf,to_stale) ;
	        if (to-- == 0) break ;
	    } /* end while (db exists) */
	    if (rs == nrs) {
	        op->f.ofcreat = FALSE ;
	        c = 0 ;
	        rs = cmimk_filesbeginc(op) ;
	    }
	} /* end if (mknewfname) */
#if	CF_DEBUGS
	debugprintf("cmimk_filesbeginwait: ret ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (cmimk_filesbeginwait) */


static int cmimk_filesbegincreate(CMIMK *op,cchar *tfn,int of,mode_t om)
{
	int		rs ;
#if	CF_DEBUGS
	{
	    char	obuf[100+1] ;
	    snopenflags(obuf,100,of) ;
	    debugprintf("cmimk_filesbegincreate: ent of=%s\n",obuf) ;
	    debugprintf("cmimk_filesbegincreate: om=%05o\n",om) ;
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
	debugprintf("cmimk_filesbegincreate: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cmimk_filesbegincreate) */


static int cmimk_filesend(CMIMK *op)
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
	debugprintf("cmimk_filesend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cmimk_filesend) */


static int cmimk_listbegin(CMIMK *op,int n)
{
	int		rs ;
	int		size ;
	int		opts ;

	opts = 0 ;
	opts |= VECOBJ_OCOMPACT ;
	opts |= VECOBJ_OORDERED ;
	opts |= VECOBJ_OSTATIONARY ;
	size = sizeof(struct cmentry) ;
	if ((rs = vecobj_start(&op->ents,size,n,opts)) >= 0) {
	    rs = vecobj_start(&op->lines,size,(n * 2),opts) ;
	    if (rs < 0)
	        vecobj_finish(&op->ents) ;
	}

	return rs ;
}
/* end subroutine (cmimk_listbegin) */


static int cmimk_listend(CMIMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecobj_finish(&op->lines) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->ents) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (cmimk_listend) */


static int cmimk_mkidx(CMIMK *op)
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	debugprintf("cmimk_mkidx: ent\n") ;
#endif

	if ((rs = cmimk_nidxopen(op)) >= 0) {
	    CMIHDR	hdr ;

	    memset(&hdr,0,sizeof(CMIHDR)) ;
	    hdr.vetu[0] = CMIHDR_VERSION ;
	    hdr.vetu[1] = ENDIAN ;
	    hdr.vetu[2] = 0 ;
	    hdr.vetu[3] = 0 ;
	    hdr.dbsize = (uint) op->size_db ;
	    hdr.dbtime = (uint) op->ti_db ;
	    hdr.nents = op->nents ;
	    hdr.maxent = op->maxent ;

	    if ((rs = cmimk_mkidxwrmain(op,&hdr)) >= 0) {
	        const int	hlen = HDRBUFLEN ;
	        char		hbuf[HDRBUFLEN+1] ;

	        hdr.idxtime = (uint) time(NULL) ;
	        hdr.idxsize = (uint) rs ;
	        wlen = rs ;
	        if ((rs = cmihdr(&hdr,0,hbuf,hlen)) >= 0) {
	            const int	bl = rs ;
	            if ((rs = u_pwrite(op->nfd,hbuf,bl,0L)) >= 0) {
	                const mode_t	om = op->om ;
	                rs = uc_fminmod(op->nfd,om) ;
	            }
	        }

	    } /* end if (cmimk_mkidxwrmain) */

	    rs1 = cmimk_nidxclose(op) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cmimk_nidx) */

#if	CF_DEBUGS
	debugprintf("cmimk_mkidx: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cmimk_mkidx) */


static int cmimk_mkidxwrmain(CMIMK *op,CMIHDR *hdrp)
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
	    if ((rs = cmimk_mkidxwrhdr(op,hdrp,hfp)) >= 0) {
	        off += rs ;
	        if (rs >= 0) {
	            rs = cmimk_mkidxwrents(op,hdrp,hfp,off) ;
	            off += rs ;
	        }
	        if (rs >= 0) {
	            rs = cmimk_mkidxwrlines(op,hdrp,hfp,off) ;
	            off += rs ;
	        }
	    } /* end if (cmimk_mkidxwrhdr) */
	    rs1 = filebuf_finish(hfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */
	return (rs >= 0) ? off : rs ;
}
/* end subroutine (cmimk_mkidxwrmain) */


/* ARGSUSED */
static int cmimk_mkidxwrhdr(CMIMK *op,CMIHDR *hdrp,FILEBUF *hfp)
{
	const int	hlen = HDRBUFLEN ;
	int		rs ;
	int		wlen = 0 ;
	char		hbuf[HDRBUFLEN+1] ;
	if (op == NULL) return SR_FAULT ; /* LINT */
	if ((rs = cmihdr(hdrp,0,hbuf,hlen)) >= 0) {
	    rs = filebuf_writefill(hfp,hbuf,rs) ;
	    wlen += rs ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cmimk_mkidxwrhdr) */


static int cmimk_mkidxwrents(CMIMK *op,CMIHDR *hdrp,FILEBUF *hfp,int off)
{
	struct cmentry	*cmep ;
	uint		a[4] ;
	const int	size = (4 * sizeof(uint)) ;
	int		rs = SR_OK ;
	int		wlen = 0 ;
	int		n = 0 ;
	int		i ;
	hdrp->vioff = off ;
	for (i = 0 ; vecobj_get(&op->ents,i,&cmep) >= 0 ; i += 1) {
	    if (cmep != NULL) {
	        a[0] = cmep->eoff ;
	        a[1] = cmep->elen ;
	        a[2] = cmep->li ;
	        a[3] = ((cmep->nlines << 16) | (cmep->cn & UINT_MAX)) ;
	        n += 1 ;
	        rs = filebuf_write(hfp,a,size) ;
	        wlen += rs ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	hdrp->vilen = n ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cmimk_mkidxwrents) */


static int cmimk_mkidxwrlines(CMIMK *op,CMIHDR *hdrp,FILEBUF *hfp,int off)
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
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cmimk_mkidxwrlines) */


static int cmimk_nidxopen(CMIMK *op)
{
	const mode_t	om = op->om ;
	int		rs ;
	int		fd = 0 ;
	int		of = (O_CREAT|O_WRONLY) ;
#if	CF_DEBUGS
	debugprintf("cmimk_nidxopen: ent nidxfname=%s\n",op->nidxfname) ;
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
	debugprintf("cmimk_nidxopen: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (cmimk_nidxopen) */


static int cmimk_nidxclose(CMIMK *op)
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
/* end subroutine (cmimk_nidxclose) */


static int cmimk_renamefiles(CMIMK *op)
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
/* end subroutine (cmimk_renamefiles) */


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
	struct cmentry	**e1pp = (struct cmentry **) v1p ;
	struct cmentry	**e2pp = (struct cmentry **) v2p ;
	int		rc = 0 ;

	if (*e1pp != NULL) {
	    if (*e2pp != NULL) {
	        uint	vc1 = (*e1pp)->cn ;
	        uint	vc2 = (*e2pp)->cn ;
	        rc = (vc1 - vc2) ;
	    } else
	        rc = -1 ;
	} else
	    rc = 1 ;

	return rc ;
}
/* end subroutine (vvecmp) */


