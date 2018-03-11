/* bvsmk */

/* make a BVS database */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module creates a BVS database file.

	Synopsis:

	int bvsmk_open(op,pr,dbname,...)
	BVSMK		*op ;
	const char	pr[] ;
	const char	dbname[] ;

	Arguments:

	- op		object pointer
	- pr		program-root
	- dbname	name of DB

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
	__________________________________

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


#define	BVSMK_MASTER	0


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
#include	<nulstr.h>
#include	<localmisc.h>

#include	"bvsmk.h"
#include	"bvshdr.h"
#include	"bvsbook.h"


/* local defines */

#define	BVSMK_NENTRIES	(19 * 1024)
#define	BVSMK_IDNAME	"var/bvses"
#define	BVSMK_IDMODE	0777

#define	HDRBUFLEN	(sizeof(BVSHDR) + 128)

#define	FSUF_IDX	"bvs"

#define	TO_OLDFILE	(5 * 60)


/* external subroutines */

extern int	sfdirname(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	getpwd(char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	mktmpfile(char *,mode_t,cchar *) ;
extern int	filebuf_writefill(FILEBUF *,const char *,int) ;
extern int	filebuf_writealign(FILEBUF *,int,uint) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* exported variables */

BVSMK_OBJ	bvsmk = {
	"bvsmk",
	sizeof(BVSMK)
} ;


/* local structures */


/* forward references */

static int	bvsmk_filesbegin(BVSMK *) ;
static int	bvsmk_filesbeginc(BVSMK *) ;
static int	bvsmk_filesbeginwait(BVSMK *) ;
static int	bvsmk_filesbegincreate(BVSMK *,cchar *,int,mode_t) ;
static int	bvsmk_filesend(BVSMK *) ;
static int	bvsmk_listbegin(BVSMK *,int) ;
static int	bvsmk_listend(BVSMK *) ;
static int	bvsmk_mkidx(BVSMK *) ;
static int	bvsmk_mkidxwrmain(BVSMK *,BVSHDR *) ;
static int	bvsmk_mkidxwrhdr(BVSMK *,BVSHDR *,FILEBUF *) ;
static int	bvsmk_mkidxchaptab(BVSMK *,BVSHDR *,FILEBUF *,int) ;
static int	bvsmk_mkidxbooktab(BVSMK *,BVSHDR *,FILEBUF *,int) ;
static int	bvsmk_nidxopen(BVSMK *) ;
static int	bvsmk_nidxclose(BVSMK *) ;
static int	bvsmk_renamefiles(BVSMK *) ;

static int	mkdname(cchar *,mode_t) ;
static int	mknifname(char *,int,cchar *,cchar *,cchar *) ;
static int	unlinkstale(cchar *,int) ;


/* local variables */


/* exported subroutines */


int bvsmk_open(BVSMK *op,cchar *pr,cchar db[],int of,mode_t om)
{
	const int	n = BVSMK_NENTRIES ;
	int		rs ;
	int		size = 0 ;
	int		c = 0 ;
	char		*bp ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;
	if (db == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;
	if (db[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("bvsmk_open: ent pr=%s\n",pr) ;
	debugprintf("bvsmk_open: dbname=%s\n",dbname) ;
#endif /* CF_DEBUGS */

	memset(op,0,sizeof(BVSMK)) ;
	op->om = om ;
	op->nfd = -1 ;

	op->f.ofcreat = MKBOOL(of & O_CREAT) ;
	op->f.ofexcl = MKBOOL(of & O_EXCL) ;

	size += (strlen(pr)+1) ;
	size += (strlen(db)+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    op->a = bp ;
	    op->pr = bp ;
	    bp = (strwcpy(bp,pr,-1)+1) ;
	    op->db = bp ;
	    bp = (strwcpy(bp,db,-1)+1) ;
	        if ((rs = bvsmk_filesbegin(op)) >= 0) {
		    c = rs ;
		    if ((rs = bvsmk_listbegin(op,n)) >= 0) {
			op->magic = BVSMK_MAGIC ;
		    }
		    if (rs < 0)
			bvsmk_filesend(op) ;
		} /* end if (files-begin) */
		if (rs < 0) {
		    uc_free(op->a) ;
		    op->a = NULL ;
		}
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("bvsmk_open: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bvsmk_open) */


int bvsmk_close(BVSMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		nverses = 0 ;
	int		f_go = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVSMK_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("bvsmk_close: nverses=%u\n",op->nverses) ;
	debugprintf("bvsmk_close: nzverses=%u\n",op->nzverses) ;
#endif

	f_go = (! op->f.abort) ;
	nverses = op->nverses ;
	if (nverses > 0) {
	    rs1 = bvsmk_mkidx(op) ;
	    if (rs >= 0) rs = rs1 ;
	    f_go = f_go && (rs1 >= 0) ;
	}

#if	CF_DEBUGS
	debugprintf("bvsmk_close: bvsmk_mkidx() rs=%d\n",rs) ;
#endif

	if (op->nfd >= 0) {
	    rs1 = u_close(op->nfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfd = -1 ;
	}

	rs1 = bvsmk_listend(op) ;
	if (rs >= 0) rs = rs1 ;
	f_go = f_go && (rs1 >= 0) ;

#if	CF_DEBUGS
	debugprintf("bvsmk_close: bvsmk_listend() rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (nverses > 0) && f_go) {
	    rs1 = bvsmk_renamefiles(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("bvsmk_close: bvsmk_renamefiles() rs=%d\n",rs) ;
#endif

	rs1 = bvsmk_filesend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("bvsmk_close: bvsmk_filesend() rs=%d\n",rs) ;
#endif

	if (op->db != NULL) {
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("bvsmk_close: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bvsmk_close) */


int bvsmk_add(BVSMK *op,int book,uchar *ap,int al)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (ap == NULL) return SR_FAULT ;

	if (op->magic != BVSMK_MAGIC) return SR_NOTOPEN ;

	if ((book >= 0) && (al >= 0)) {
	    if (al > 0) {
		BVSBOOK		be ;
		const int	size = (al * sizeof(uchar)) ;
		uchar		*bp ;
		memset(&be,0,sizeof(BVSBOOK)) ;
	        if ((rs = uc_malloc(size,&bp)) >= 0) {
	            uint	nzverses ;
	            uint	nverses = 0 ;
	            int		i = 0 ;
	            for (i = 0 ; i < al ; i += 1) {
	                bp[i] = ap[i] ;
	                nverses += ap[i] ;
	            }
	            nzverses = (nverses - ap[0]) ;
	            be.book = book ;
	            be.nverses = nverses ;
	            be.nzverses = nzverses ;
	            be.al = (uchar) al ;
	            be.ap = bp ;
	            if ((rs = vecobj_add(&op->books,&be)) >= 0) {
		        op->nverses += nverses ;
		        if (book > 0) op->nzverses += nzverses ;
	            }
	            if (rs < 0)
	                uc_free(bp) ;
	        } /* end if (memory-allocation) */
	    } /* end if (greater) */
	} else
	    rs = SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("bvsmk_add: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvsmk_add) */


int bvsmk_abort(BVSMK *op,int f)
{
	op->f.abort = f ;
	return SR_OK ;
}
/* end subroutine (bvsmk_abort) */


/* private subroutines */


static int bvsmk_filesbegin(BVSMK *op)
{
	int		rs ;
	int		dnl ;
	int		c = 0 ;
	cchar		*idname = BVSMK_IDNAME ;
	const char	*dnp ;
	char		tbuf[MAXPATHLEN + 1] ;

	dnp = tbuf ;
	if ((rs = mkpath2(tbuf,op->pr,idname)) >= 0) {
	    const mode_t	dm = 0777 ;
	    dnl = rs ;
	    if ((rs = mkdname(tbuf,dm)) >= 0) {
	        const char	*cp ;
	        if ((rs = uc_mallocstrw(dnp,dnl,&cp)) >= 0) {
	            op->idname = cp ;
	            if (op->f.ofcreat) {
	                rs = bvsmk_filesbeginc(op) ;
	            } else {
	                rs = bvsmk_filesbeginwait(op) ;
	                c = rs ;
	            }
	            if (rs < 0) {
	                uc_free(op->idname) ;
	                op->idname = NULL ;
	            }
	        } /* end if (memory-allocation) */
	    } /* end if (mkdname) */
	} /* end if (mkpath) */

#if	CF_DEBUGS
	debugprintf("bvsmk_filesbegin: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bvsmk_filesbegin) */


static int bvsmk_filesbeginc(BVSMK *op)
{
	const int	type = (op->f.ofcreat && (! op->f.ofexcl)) ;
	int		rs ;
	cchar		*id = op->idname ;
	cchar		*db = op->db;
	cchar		*suf = FSUF_IDX	 ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mknifname(tbuf,type,id,db,suf)) >= 0) {
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
	        rs = bvsmk_filesbegincreate(op,tfn,of,om) ;
		if ((rs < 0) && type) {
		    uc_unlink(rbuf) ;
		}
	    } /* end if (ok) */
	} /* end if (mknifname) */
	return rs ;
}
/* end subroutine (bvsmk_filesbeginc) */


static int bvsmk_filesbeginwait(BVSMK *op)
{
	int		rs ;
	int		c = 0 ;
	cchar		*id = op->idname ;
	cchar		*db = op->db ;
	cchar		*suf = FSUF_IDX	 ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mknifname(tbuf,FALSE,id,db,suf)) >= 0) {
	    const mode_t	om = op->om ;
	    const int		to_stale = BVSMK_INTSTALE ;
	    const int		nrs = SR_EXISTS ;
	    const int		of = (O_CREAT|O_WRONLY|O_EXCL) ;
	    int			to = BVSMK_INTOPEN ;
	    while ((rs = bvsmk_filesbegincreate(op,tbuf,of,om)) == nrs) {
#if	CF_DEBUGS
	        debugprintf("bvsmk_filesbeginwait: loop ret rs=%d\n",rs) ;
#endif
	        c = 1 ;
	        sleep(1) ;
	        unlinkstale(tbuf,to_stale) ;
	        if (to-- == 0) break ;
	    } /* end while (db exists) */
	    if (rs == nrs) {
	        op->f.ofcreat = FALSE ;
	        c = 0 ;
	        rs = bvsmk_filesbeginc(op) ;
	    }
	} /* end if (mknifname) */
#if	CF_DEBUGS
	debugprintf("bvsmk_filesbeginwait: ret ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bvsmk_filesbeginwait) */


static int bvsmk_filesbegincreate(BVSMK *op,cchar *tfn,int of,mode_t om)
{
	int		rs ;
#if	CF_DEBUGS
	{
	    char	obuf[100+1] ;
	    snopenflags(obuf,100,of) ;
	    debugprintf("bvsmk_filesbegincreate: ent of=%s\n",obuf) ;
	    debugprintf("bvsmk_filesbegincreate: om=%05o\n",om) ;
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
	debugprintf("bvsmk_filesbegincreate: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvsmk_filesbegincreate) */


static int bvsmk_filesend(BVSMK *op)
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
	debugprintf("bvsmk_filesend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvsmk_filesend) */


static int bvsmk_listbegin(BVSMK *op,int n)
{
	int		rs ;
	int		size ;
	int		opts ;

	opts = 0 ;
	opts |= VECOBJ_OCOMPACT ;
	opts |= VECOBJ_OORDERED ;
	opts |= VECOBJ_OSTATIONARY ;
	size = sizeof(BVSBOOK) ;
	rs = vecobj_start(&op->books,size,n,opts) ;

	return rs ;
}
/* end subroutine (bvsmk_listbegin) */


static int bvsmk_listend(BVSMK *op)
{
	BVSBOOK		*bep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(&op->books,i,&bep) >= 0 ; i += 1) {
	    if (bep != NULL) {
	        if (bep->ap != NULL) {
		    rs1 = uc_free(bep->ap) ;
		    if (rs >= 0) rs = rs1 ;
		}
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("bvsmk_listend: mid rs=%d\n",rs) ;
#endif

	rs1 = vecobj_finish(&op->books) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("bvsmk_listend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvsmk_listend) */


static int bvsmk_mkidx(BVSMK *op)
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	debugprintf("bvsmk_mkidx: ent\n") ;
#endif

	if ((rs = bvsmk_nidxopen(op)) >= 0) {
	    BVSHDR	hdr ;

	    memset(&hdr,0,sizeof(BVSHDR)) ;
	hdr.vetu[0] = BVSHDR_VERSION ;
	hdr.vetu[1] = ENDIAN ;
	hdr.vetu[2] = 0 ;
	hdr.vetu[3] = 0 ;
	hdr.wtime = (uint) time(NULL) ;
	hdr.nverses = op->nverses ;
	hdr.nzverses = op->nverses ;

	    if ((rs = bvsmk_mkidxwrmain(op,&hdr)) >= 0) {
	        const int	hlen = HDRBUFLEN ;
	        char		hbuf[HDRBUFLEN+1] ;
	        hdr.fsize = rs ;
	        wlen = rs ;

	        if ((rs = bvshdr(&hdr,0,hbuf,hlen)) >= 0) {
	            const int	bl = rs ;
	            if ((rs = u_pwrite(op->nfd,hbuf,bl,0L)) >= 0) {
	                const mode_t	om = op->om ;
	                rs = uc_fminmod(op->nfd,om) ;
	            }
	        }

	    } /* end if (bvsmk_mkidxwrmain) */

	    rs1 = bvsmk_nidxclose(op) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bvsmk_nidx) */

#if	CF_DEBUGS
	debugprintf("bvsmk_mkidx: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bvsmk_mkidx) */


static int bvsmk_mkidxwrmain(BVSMK *op,BVSHDR *hdrp)
{
	FILEBUF		hf, *hfp = &hf ;
	const int	nfd = op->nfd ;
	const int	ps = getpagesize() ;
	int		bsize ;
	int		rs ;
	int		rs1 ;
	int		foff = 0 ;
	bsize = (ps * 4) ;
	if ((rs = filebuf_start(hfp,nfd,0,bsize,0)) >= 0) {
	    if ((rs = bvsmk_mkidxwrhdr(op,hdrp,hfp)) >= 0) {
	        foff += rs ;
		op->maxbook = 0 ;
	        if (rs >= 0) {
	            rs = bvsmk_mkidxchaptab(op,hdrp,hfp,foff) ;
	            foff += rs ;
	        }
	        if (rs >= 0) {
	            rs = bvsmk_mkidxbooktab(op,hdrp,hfp,foff) ;
	            foff += rs ;
	        }
	    } /* end if (bvsmk_mkidxwrhdr) */
	    rs1 = filebuf_finish(hfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */
	return (rs >= 0) ? foff : rs ;
}
/* end subroutine (bvsmk_mkidxwrmain) */


/* ARGSUSED */
static int bvsmk_mkidxwrhdr(BVSMK *op,BVSHDR *hdrp,FILEBUF *hfp)
{
	const int	hlen = HDRBUFLEN ;
	int		rs ;
	int		wlen = 0 ;
	char		hbuf[HDRBUFLEN+1] ;
	if (op == NULL) return SR_FAULT ; /* LINT */
	if ((rs = bvshdr(hdrp,0,hbuf,hlen)) >= 0) {
	    rs = filebuf_writefill(hfp,hbuf,rs) ;
	    wlen += rs ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bvsmk_mkidxwrhdr) */


static int bvsmk_mkidxchaptab(BVSMK *op,BVSHDR *hdrp,FILEBUF *hfp,int foff)
{
	BVSBOOK		*bep ;
	vecobj		*blp = &op->books ;
	int		rs = SR_OK ;
	int		ctlen = 0 ;
	int		n = 0 ;
	int		i ;
	int		wlen = 0 ;
	hdrp->ctoff = foff ;
	for (i = 0 ; vecobj_get(blp,i,&bep) >= 0 ; i += 1) {
	    if (bep != NULL) {
	        if (bep->book > 0) n += 1 ;
		if (bep->book > op->maxbook) op->maxbook = bep->book ;
		ctlen += bep->al ;
	        bep->ci = (foff - hdrp->ctoff) ;
	        rs = filebuf_write(hfp,bep->ap,bep->al) ;
	        foff += rs ;
		wlen += rs ;
	        if (rs < 0) break ;
	    }
	} /* end for */
	hdrp->nzbooks = n ;
	hdrp->ctlen = ctlen ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bvsmk_mkidxchaptab) */


static int bvsmk_mkidxbooktab(BVSMK *op,BVSHDR *hdrp,FILEBUF *hfp,int foff)
{
	const int	n = (op->maxbook + 1) ;
	int		rs ;
	int		wlen = 0 ;

	if ((rs = filebuf_writealign(hfp,sizeof(int),foff)) >= 0) {
	    const int	size = (n * 4) * sizeof(ushort) ;
	    ushort	(*array)[4] = NULL ;
	    wlen += rs ;
	    foff += rs ;
	    if ((rs = uc_malloc(size,&array)) >= 0) {
	        BVSBOOK	*bep ;
	        int	i ;
	        memset(array,0,size) ;

	    for (i = 0 ; vecobj_get(&op->books,i,&bep) >= 0 ; i += 1) {
	        if (bep != NULL) {
	            rs = bvsbook_set(bep,array[bep->book]) ;
	        }
		if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
	        hdrp->btoff = foff ;
	        hdrp->btlen = n ;
	        rs = filebuf_write(hfp,array,size) ;
	        wlen += rs ;
	    }

	    uc_free(array) ;
	} /* end if (memory allocation) */
	} /* end if (filebuf_writealign) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bvsmk_mkidxbooktab) */


static int bvsmk_nidxopen(BVSMK *op)
{
	const mode_t	om = op->om ;
	int		rs ;
	int		fd = -1 ;
	int		of = (O_CREAT|O_WRONLY) ;
#if	CF_DEBUGS
	debugprintf("bvsmk_nidxopen: ent nidxfname=%s\n",op->nidxfname) ;
#endif
	if (op->nidxfname == NULL) {
	    const int	type = (op->f.ofcreat && (! op->f.ofexcl)) ;
	    cchar	*id = op->idname ;
	    cchar	*db = op->db ;
	    cchar	*suf = FSUF_IDX ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mknifname(tbuf,type,id,db,suf)) >= 0) {
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
		    if (rs < 0) {
			u_close(fd) ;
			op->nfd = -1 ;
			if (type) u_unlink(rbuf) ;
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
	debugprintf("bvsmk_nidxopen: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (bvsmk_nidxopen) */


static int bvsmk_nidxclose(BVSMK *op)
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
/* end subroutine (bvsmk_nidxclose) */


static int bvsmk_renamefiles(BVSMK *op)
{
	const int	clen = MAXNAMELEN ;
	int		rs ;
	cchar		*suf = FSUF_IDX ;
	cchar		*end = ENDIANSTR ;
	char		cbuf[MAXNAMELEN+1] ;

#if	CF_DEBUGS
	debugprintf("bvsmk_renamefiles: ent\n") ;
	debugprintf("bvsmk_renamefiles: idname=%s\n",op->idname) ;
	debugprintf("bvsmk_renamefiles: dbname=%s\n",op->db) ;
	debugprintf("bvsmk_renamefiles: nfname=%s\n",op->nidxfname) ;
#endif

	if ((rs = sncpy4(cbuf,clen,op->db,".",suf,end)) >= 0) {
	    char	idxfname[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(idxfname,op->idname,cbuf)) >= 0) {
	        if ((rs = u_rename(op->nidxfname,idxfname)) >= 0) {
	            op->nidxfname[0] = '\0' ;
	        } else {
	            u_unlink(op->nidxfname) ;
	            op->nidxfname[0] = '\0' ;
	        }
	    } /* end if (mkpath) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("bvsmk_renamefiles: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvsmk_renamefiles) */


static int mkdname(cchar *dname,mode_t dm)
{
	    struct ustat	sb ;
	    const int		nrs = SR_NOENT ;
	int		rs ;
	    if ((rs = u_stat(dname,&sb)) == nrs) {
		dm |= (BVSMK_IDMODE | 0555) ;
		if ((rs = mkdirs(dname,dm)) >= 0) {
	            rs = uc_minmod(dname,dm) ;
		}
	    } /* end if (creating directory) */

#if	CF_DEBUGS
	debugprintf("mkdname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mkdname) */


static int mknifname(char *rbuf,int type,cchar *id,cchar *db,cchar *suf)
{
	int		rs ;
	cchar		*end = ENDIANSTR ;
	cchar		*fin = (type) ? "xXXXX" : "n" ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mkpath2(tbuf,id,db)) >= 0) {
	    rs = mkfnamesuf3(rbuf,tbuf,suf,end,fin) ;
	}
	return rs ;
}
/* end subroutine (mknifname) */


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


