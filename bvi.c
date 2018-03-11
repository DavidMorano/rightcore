/* bvi */

/* read or audit a BVI database */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SEARCH	1		/* use 'bsearch(3c)' */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine opens and allows for reading or auditing of a BVI
	(Bible Verse Index) database.

	Synopsis:

	int bvi_open(op,dbname)
	BVI		*op ;
	const char	dbname[] ;

	Arguments:

	- op		object pointer
	- dbname	name of (path-to) DB

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<endianstr.h>
#include	<localmisc.h>

#include	"bvi.h"
#include	"bvihdr.h"
#include	"bvcitekey.h"


/* local defines */

#define	BVI_FMI		struct bvi_fmi

#define	BVI_KA		sizeof(BVI_LINE)
#define	BVI_BO(v)	((BVI_KA - ((v) % BVI_KA)) % BVI_KA)

#define	SHIFTINT	(6 * 60)	/* possible time-shift */

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	TO_CHECK	4


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* exported variables */

BVI_OBJ	bvi = {
	"bvi",
	sizeof(BVI),
	sizeof(BVI_CUR)
} ;


/* local structures */

struct chapters {
	uchar		*ap ;
	int		al ;
	int		ci ;
} ;


/* forward references */

static int	bvi_loadbegin(BVI *,time_t) ;
static int	bvi_loadend(BVI *) ;
static int	bvi_mapcreate(BVI *,time_t) ;
static int	bvi_mapdestroy(BVI *) ;
static int	bvi_proc(BVI *,time_t) ;
static int	bvi_verify(BVI *,time_t) ;
static int	bvi_auditvt(BVI *) ;
static int	bvi_checkupdate(BVI *,time_t) ;
static int	bvi_search(BVI *,BVI_QUERY *) ;
static int	bvi_loadbve(BVI *,BVI_VERSE *,char *,int,int) ;
static int	bvi_loadchapters(BVI *,int,uchar *,int) ;

static int	chapters_start(struct chapters *,uchar *,int) ;
static int	chapters_set(struct chapters *,int,int) ;
static int	chapters_finish(struct chapters *) ;

static int	mkcitekey(BVI_QUERY *,uint *) ;

#if	CF_SEARCH
static int	vtecmp(const void *,const void *) ;
#endif

#if	CF_DEBUGS
static int	debugpresent(cchar *,const void *) ;
#endif


/* local variables */


/* exported subroutines */


int bvi_open(BVI *op,cchar *dbname)
{
	const time_t	dt = time(NULL) ;
	int		rs ;
	int		nverses = 0 ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (dbname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("bvi_open: dbname=%s\n",dbname) ;
#endif
	memset(op,0,sizeof(BVI)) ;

	if ((rs = uc_mallocstrw(dbname,-1,&cp)) >= 0) {
	    cchar	*es = ENDIANSTR ;
	    char	tmpfname[MAXPATHLEN + 1] ;
	    op->dbname = cp ;
	    if ((rs = mkfnamesuf2(tmpfname,op->dbname,BVI_SUF,es)) >= 0) {
	        const int	tl = rs ;
	        if ((rs = uc_mallocstrw(tmpfname,tl,&cp)) >= 0) {
	            op->fname = cp ;
#if	CF_DEBUGS
	            debugpresent("bvi_open: present{fname}=%d\n",op->fname) ;
#endif
	            if ((rs = bvi_loadbegin(op,dt)) >= 0) {
	                nverses = rs ;
	                op->ti_lastcheck = dt ;
	                op->magic = BVI_MAGIC ;
	            } /* end if (loadbegin) */
	            if (rs < 0) {
	                if (op->fname != NULL) {
	                    uc_free(op->fname) ;
	                    op->fname = NULL ;
	                }
	            }
	        } /* end if (memory-allocation) */
	    } /* end if (mkfnamesuf2) */
	    if (rs < 0) {
	        if (op->dbname != NULL) {
	            uc_free(op->dbname) ;
	            op->dbname = NULL ;
	        }
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("bvi_open: ret rs=%d bv=%u\n",rs,nverses) ;
#endif

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bvi_open) */


int bvi_close(BVI *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVI_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("bvi_close: ent\n") ;
#endif

#if	CF_DEBUGS
	rs1 = uc_mallpresent(op->fname) ;
	if (rs >= 0) rs = rs1 ;
	debugprintf("bvi_close: 0 rs=%d\n",rs) ;
#endif

	rs1 = bvi_loadend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("bvi_close: 1 rs=%d\n",rs) ;
	debugprintf("bvi_close: fname{%p}\n",op->fname) ;
	debugprintf("bvi_close: fname=%s\n",op->fname) ;
	debugpresent("bvi_close: present{fname}=%d\n",op->fname) ;
#endif

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("bvi_close: 2 rs=%d\n",rs) ;
#endif

	if (op->dbname != NULL) {
	    rs1 = uc_free(op->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("bvi_close: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (bvi_close) */


int bvi_audit(BVI *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVI_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	if (rs >= 0) {
	    rs = uc_mallpresent(op->fname) ;
	}
#endif

/* verify that all list pointers and list entries are valid */

	if (rs >= 0) {
	    rs = bvi_auditvt(op) ;
	}

#if	CF_DEBUGS
	debugprintf("bvi_audit: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvi_audit) */


int bvi_count(BVI *op)
{
	BVIHDR		*hip ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVI_MAGIC) return SR_NOTOPEN ;

	hip = &op->fhi ;
	return (rs >= 0) ? hip->nverses : rs ;
}
/* end subroutine (bvi_count) */


int bvi_info(BVI *op,BVI_INFO *ip)
{
	BVIHDR		*hip ;
	int		rs = SR_OK ;
	int		nv = 0 ;

#if	CF_DEBUGS
	debugprintf("bvi_info: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVI_MAGIC) return SR_NOTOPEN ;

	hip = &op->fhi ;
	nv = hip->nverses ;

	if (ip != NULL) {
	    memset(ip,0,sizeof(BVI_INFO)) ;
	    ip->mtime = op->fmi.ti_mod ;
	    ip->ctime = (time_t) hip->wtime ;
	    ip->maxbook = hip->maxbook ;
	    ip->maxchapter = hip->maxchapter ;
	    ip->count = hip->nverses ;
	    ip->nzverses = hip->nzverses ;
	}

#if	CF_DEBUGS
	debugprintf("bvi_info: ret rs=%d nv=%u\n",rs,nv) ;
#endif

	return (rs >= 0) ? nv : rs ;
}
/* end subroutine (bvi_info) */


int bvi_read(BVI *op,BVI_VERSE *bvep,char *vbuf,int vlen,BVI_QUERY *qp)
{
	int		rs = SR_OK ;
	int		vi = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;
	if (bvep == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;

	if (op->magic != BVI_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("bvi_get: q=%u:%u:%u\n",qp->b,qp->c,qp->v) ;
	rs = debugpresent("bvi_get: present{fname}=%d\n",op->fname) ;
#endif

/* check for update */

	if ((rs >= 0) && (op->ncursors == 0)) {
	    rs = bvi_checkupdate(op,0) ;
	}
	if (rs >= 0) {
	    if ((rs = bvi_search(op,qp)) >= 0) {
	        vi = rs ;
	        rs = bvi_loadbve(op,bvep,vbuf,vlen,vi) ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("bvi_get: ret rs=%d vi=%u\n",rs,vi) ;
#endif

	return (rs >= 0) ? vi : rs ;
}
/* end subroutine (bvi_read) */


int bvi_get(BVI *op,BVI_QUERY *qp,BVI_VERSE *bvep,char *vbuf,int vlen)
{
	return bvi_read(op,bvep,vbuf,vlen,qp) ;
}
/* end subroutine (bvi_get) */


int bvi_curbegin(BVI *op,BVI_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != BVI_MAGIC) return SR_NOTOPEN ;

	curp->i = 0 ;
	op->ncursors += 1 ;

	return SR_OK ;
}
/* end subroutine (bvi_curbegin) */


int bvi_curend(BVI *op,BVI_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != BVI_MAGIC) return SR_NOTOPEN ;

	curp->i = 0 ;
	if (op->ncursors > 0) {
	    op->ncursors -= 1 ;
	}

	return SR_OK ;
}
/* end subroutine (bvi_curend) */


int bvi_enum(BVI *op,BVI_CUR *curp,BVI_VERSE *bvep,char *vbuf,int vlen)
{
	BVIHDR		*hip ;
	int		rs = SR_OK ;
	int		vi ;
	int		nlines = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (bvep == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;

	if (op->magic != BVI_MAGIC) return SR_NOTOPEN ;

	if (op->ncursors == 0) return SR_INVALID ;

	vi = (curp->i < 0) ? 0 : (curp->i + 1) ;

	hip = &op->fhi ;
	if (vi < hip->vilen) {
	    if ((rs = bvi_loadbve(op,bvep,vbuf,vlen,vi)) >= 0) {
		nlines = rs ;
	        curp->i = vi ;
	    }
	} else {
	    rs = SR_NOTFOUND ;
	}

	return (rs >= 0) ? nlines : rs ;
}
/* end subroutine (bvi_enum) */


/* retrieve a table w/ the number of verses per chapter for a book */
int bvi_chapters(BVI *op,int book,uchar *ap,int al)
{
	BVI_QUERY	q ;
	int		rs ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVI_MAGIC) return SR_NOTOPEN ;

	if (book < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("bvi_chapters: b=%u al=%u\n",book,al) ;
#endif

	q.b = (book & UCHAR_MAX) ;
	q.c = 1 ;
	q.v = 1 ;
	if ((rs = bvi_search(op,&q)) >= 0) {
	    int	vi = rs ;
	    rs = bvi_loadchapters(op,vi,ap,al) ;
	    n += rs ;
	}

#if	CF_DEBUGS
	debugprintf("bvi_chapters: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (bvi_chapters) */


/* private subroutines */


static int bvi_loadbegin(BVI *op,time_t dt)
{
	int		rs ;
	int		nverses = 0 ;

	if ((rs = bvi_mapcreate(op,dt)) >= 0) {
	    rs = bvi_proc(op,dt) ;
	    nverses = rs ;
	    if (rs < 0)
	        bvi_mapdestroy(op) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("bvi_loadbegin: ret rs=%d nv=%u\n",rs,nverses) ;
#endif

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bvi_loadbegin) */


static int bvi_loadend(BVI *op)
{
	BVI_FMI		*mip ;
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = bvi_mapdestroy(op) ;
	if (rs >= 0) rs = rs1 ;

	mip = &op->fmi ;
	mip->vt = NULL ;
	mip->lt = NULL ;
	return rs ;
}
/* end subroutine (bvi_loadend) */


static int bvi_mapcreate(BVI *op,time_t dt)
{
	BVI_FMI		*mip = &op->fmi ;
	int		rs ;

	if (op->fname == NULL) return SR_BUGCHECK ;

	if ((rs = u_open(op->fname,O_RDONLY,0666)) >= 0) {
	    USTAT	sb ;
	    int		fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
	        const size_t	fsize = (sb.st_size & UINT_MAX) ;
	        if (fsize > 0) {
	            size_t	ms = fsize ;
	            int		mp = PROT_READ ;
	            int		mf = MAP_SHARED ;
	            void	*md ;
	            if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	                mip->mapdata = md ;
	                mip->mapsize = ms ;
	                mip->ti_mod = sb.st_mtime ;
	                mip->ti_map = dt ;
	                rs = fsize ;
	            } /* end if (mmap) */
	        } else
	            rs = SR_UNATCH ;
	    } /* end if (stat) */
	    u_close(fd) ;
	} /* end if (open) */

	return rs ;
}
/* end subroutine (bvi_mapcreate) */


static int bvi_mapdestroy(BVI *op)
{
	BVI_FMI		*mip = &op->fmi ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (mip->mapdata != NULL) {
	    rs1 = u_munmap(mip->mapdata,mip->mapsize) ;
	    if (rs >= 0) rs = rs1 ;
	    mip->mapdata = NULL ;
	    mip->mapsize = 0 ;
	    mip->ti_map = 0 ;
	}

	return rs ;
}
/* end subroutine (bvi_mapdestroy) */


static int bvi_checkupdate(BVI *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (op->ncursors == 0) {
	    if (dt <= 0) dt = time(NULL) ;
	    if ((dt - op->ti_lastcheck) >= TO_CHECK) {
	        USTAT		sb ;
	        BVI_FMI		*mip = &op->fmi ;
	        op->ti_lastcheck = dt ;
	        if ((rs = u_stat(op->fname,&sb)) >= 0) {
	            f = f || (sb.st_mtime > mip->ti_mod) ;
	            f = f || (sb.st_mtime > mip->ti_map) ;
	            if (f) {
	                bvi_loadend(op) ;
	                rs = bvi_loadbegin(op,dt) ;
	            } /* end if (update) */
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	    } /* end if (time-checked out) */
	} /* end if (no cursors out) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (bvi_checkupdate) */


static int bvi_proc(BVI *op,time_t dt)
{
	BVI_FMI		*mip = &op->fmi ;
	BVIHDR		*hip = &op->fhi ;
	int		rs ;
	int		nverses = 0 ;

	if ((rs = bvihdr(hip,1,mip->mapdata,mip->mapsize)) >= 0) {
	    nverses = hip->nverses ;
	    if ((rs = bvi_verify(op,dt)) >= 0) {
	        mip->vt = (uint (*)[4]) (mip->mapdata + hip->vioff) ;
	        mip->lt = (uint (*)[2]) (mip->mapdata + hip->vloff) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("bvi_proc: ret rs=%d nv=%u\n",rs,nverses) ;
#endif

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bvi_proc) */


static int bvi_verify(BVI *op,time_t dt)
{
	BVI_FMI		*mip = &op->fmi ;
	BVIHDR		*hip = &op->fhi ;
	int		rs = SR_OK ;
	int		size ;
	int		f = TRUE ;

	f = f && (hip->fsize == mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("bvi_verify: fsize=%u ms=%u f=%u\n",
	    hip->fsize,mip->mapsize,f) ;
#endif

#if	CF_DEBUGS
	{
	    const uint	utime = (uint) dt ;
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("bvi_verify: utime=%s sh=%u\n",
	        timestr_log(((time_t) utime),timebuf),SHIFTINT) ;
	}
#endif

	f = f && (hip->wtime > 0) ;
	if (f) {
	    time_t	tt = (time_t) hip->wtime ;
	    f = (dt >= tt) ;
	}

#ifdef	COMMENT
	{
	    const uint	utime = (uint) dt ;
	    f = f && (hip->wtime <= (utime + SHIFTINT)) ;
	}
#endif

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("bvi_verify: wtime=%s f=%u\n",
	        timestr_log(((time_t) hip->wtime),timebuf),f) ;
	}
#endif

/* alignment restriction */

#if	CF_DEBUGS
	debugprintf("bvi_verify: vioff=%u\n",hip->vioff) ;
#endif

	f = f && ((hip->vioff & (sizeof(int)-1)) == 0) ;

#if	CF_DEBUGS
	debugprintf("bvi_verify: 1 f=%d\n",f) ;
#endif

/* size restrictions */

	f = f && (hip->vioff <= mip->mapsize) ;
	size = hip->vilen * 4 * sizeof(uint) ;
	f = f && ((hip->vioff + size) <= mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("bvi_verify: 2 f=%d\n",f) ;
#endif

/* alignment restriction */

#if	CF_DEBUGS
	debugprintf("bvi_verify: vloff=%u\n",hip->vloff) ;
#endif

	f = f && ((hip->vloff & (sizeof(int)-1)) == 0) ;

#if	CF_DEBUGS
	debugprintf("bvi_verify: 3 f=%d\n",f) ;
#endif

/* size restrictions */

	f = f && (hip->vloff <= mip->mapsize) ;
	size = (hip->vllen * 2 * sizeof(uint)) ;
	f = f && ((hip->vloff + size) <= mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("bvi_verify: 4 f=%d\n",f) ;
#endif

/* size restrictions */
	f = f && (hip->vilen == hip->nverses) ;

#if	CF_DEBUGS
	debugprintf("bvi_verify: 5 f=%d\n",f) ;
#endif

/* get out */

	if (! f)
	    rs = SR_BADFMT ;

#if	CF_DEBUGS
	debugprintf("bvi_verify: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvi_verify) */


static int bvi_auditvt(BVI *op)
{
	BVI_FMI		*mip = &op->fmi ;
	BVIHDR		*hip = &op->fhi ;
	uint		(*vt)[4] ;
	uint		pcitcmpval = 0 ;
	uint		citcmpval ;
	int		rs = SR_OK ;
	int		i, li ;

	vt = mip->vt ;

/* "verses" table */

	for (i = 1 ; (rs >= 0) && (i < hip->vilen) ; i += 1) {

/* verify no line-index is longer than the "lines" table itself */

	    li = vt[i][2] ;
	    if (li >= hip->vllen) {
	        rs = SR_BADFMT ;
	        break ;
	    }

/* verify all entries are ordered w/ increasing citations */

	    citcmpval = vt[i][3] & 0x00FFFFFF ;
	    if (citcmpval < pcitcmpval) {
	        rs = SR_BADFMT ;
	        break ;
	    }
	    pcitcmpval = citcmpval ;

	} /* end for (record table entries) */

#if	CF_DEBUGS
	debugprintf("bvi_auditvt: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvi_auditvt) */


static int bvi_search(BVI *op,BVI_QUERY *qp)
{
	BVI_FMI		*mip ;
	BVIHDR		*hip ;
	uint		(*vt)[4] ;
	uint		citekey ;
	uint		vte[4] ;
	int		rs = SR_OK ;
	int		vtlen ;
	int		vi = 0 ;

	mip = &op->fmi ;
	hip = &op->fhi ;

	vt = mip->vt ;
	vtlen = hip->vilen ;

/* search for entry */

	mkcitekey(qp,&citekey) ;

#if	CF_DEBUGS
	debugprintf("bvi_search: citekey=%08X\n",citekey) ;
	debugprintf("bvi_search: vtlen=%u\n",vtlen) ;
#endif

	vte[3] = citekey ;

#if	CF_SEARCH
	{
	    uint	*vtep ;
	    int		vtesize = (4 * sizeof(uint)) ;

	    vtep = (uint *) bsearch(vte,vt,vtlen,vtesize,vtecmp) ;

	    rs = (vtep != NULL) ? ((vtep - vt[0]) >> 2) : SR_NOTFOUND ;
	    vi = rs ;
	}
#else /* CF_SEARCH */
	{
	    for (vi = 0 ; vi < vtlen ; vi += 1) {
	        if ((vt[vi][3] & 0x00FFFFFF) == citekey) {
	            break ;
	        }
	    }
	    rs = (vi < vtlen) ? vi : SR_NOTFOUND ;
	}
#endif /* CF_SEARCH */

	return (rs >= 0) ? vi : rs ;
}
/* end subroutine (bvi_search) */


static int bvi_loadbve(BVI *op,BVI_VERSE *bvep,char *ebuf,int elen,int vi)
{
	BVI_LINE	*lines ;
	BVI_FMI		*mip ;
	BVIHDR		*hip ;
	ulong		uebuf = (ulong) ebuf ;
	uint		*vte ;
	uint		(*lt)[2] ;
	uint		li ;
	uint		bo ;
	int		rs = SR_OK ;
	int		i ;
	int		linesize ;
	int		nlines ;

	if (bvep == NULL) return SR_FAULT ;
	if (ebuf == NULL) return SR_FAULT ;

	if (elen <= 0) return SR_OVERFLOW ;

	mip = &op->fmi ;
	hip = &op->fhi ;

	vte = mip->vt[vi] ;

/* load the basic stuff */

	memset(bvep,0,sizeof(BVI_VERSE)) ;
	bvep->voff = vte[0] ;
	bvep->vlen = vte[1] ;
	bvep->nlines = (vte[3] >> 24) & 0xFF ;
	bvep->b = (vte[3] >> 16) & 0xFF ;
	bvep->c = (vte[3] >> 8) & 0xFF ;
	bvep->v = (vte[3] >> 0) & 0xFF ;

/* load the lines */

	li = vte[2] ;
	nlines = bvep->nlines ;

	if (li < hip->vllen) {

#if	CF_DEBUGS
	    debugprintf("bvi_loadbve: li=%u\n",li) ;
#endif

	    bo = BVI_BO(uebuf) ;

#if	CF_DEBUGS
	    debugprintf("bvi_loadbve: nlines=%u\n",nlines) ;
	    debugprintf("bvi_loadbve: q=%u:%u:%u\n",
	        bvep->b,bvep->c,bvep->v) ;
#endif

	    linesize = ((nlines + 1) * sizeof(BVI_LINE)) ;
	    if (linesize <= (elen - (bo-uebuf))) {

	        lt = (uint (*)[2]) (mip->mapdata + hip->vloff) ;
	        lines = (BVI_LINE *) (uebuf + bo) ;
	        bvep->lines = lines ;

	        for (i = 0 ; i < nlines ; i += 1) {
	            lines[i].loff = lt[li+i][0] ;
	            lines[i].llen = lt[li+i][1] ;
	        } /* end for */

	        if (rs >= 0) {
	            lines[i].loff = 0 ;
	            lines[i].llen = 0 ;
	        }

	    } else {
	        rs = SR_OVERFLOW ;
	    }

	} else {
	    rs = SR_BADFMT ;
	}

#if	CF_DEBUGS
	debugprintf("bvi_loadbve: ret rs=%d nlines=%u\n",rs,nlines) ;
#endif

	return (rs >= 0) ? nlines : rs ;
}
/* end subroutine (bvi_loadbve) */


static int bvi_loadchapters(BVI *op,int vi,uchar *ap,int al)
{
	struct chapters	cm ;
	BVCITEKEY	ck ;
	BVI_FMI		*mip ;
	BVIHDR		*hip ;
	uint		(*vt)[4] ;
	uint		b, c, v ;
	int		rs = SR_OK ;
	int		i ;
	int		vtlen ;
	int		n = 0 ;

#if	CF_DEBUGS
	debugprintf("bvi_loadchapters: vi=%u al=%d\n",vi,al) ;
	debugprintf("bvi_loadchapters: ap(%p)\n",ap) ;
#endif

	mip = &op->fmi ;
	hip = &op->fhi ;

	vt = mip->vt ;
	vtlen = hip->vilen ;

#if	CF_DEBUGS
	debugprintf("bvi_loadchapters: vtlen=%u\n",vtlen) ;
#endif

	if (vi < vtlen) {

#if	CF_DEBUGS
	    debugprintf("bvi_loadchapters: citekey=%08x\n",vt[vi][3]) ;
#endif

	    bvcitekey_get(&ck,(vt[vi]+3)) ;
	    b = ck.b ;
	    c = ck.c ;

#if	CF_DEBUGS
	    debugprintf("bvi_loadchapters: b=%u c=%u\n",b,c) ;
#endif

	    if (ap != NULL) rs = chapters_start(&cm,ap,al) ;

	    if (rs >= 0) {

	        v = 0 ;
	        for (i = vi ; (rs >= 0) && (i < vtlen) ; i += 1) {
	            bvcitekey_get(&ck,(vt[i]+3)) ;
	            if (b != ck.b)
	                break ;
	            if (c != ck.c) {
	                if (ap != NULL) rs = chapters_set(&cm,c,v) ;
	                c = ck.c ;
	            }
	            v = ck.v ;
	        } /* end for */

	        if (rs >= 0) {
	            if (ap != NULL) rs = chapters_set(&cm,c,v) ;
	            c += 1 ;
	        }

	        n = c ;
	        if (ap != NULL) chapters_finish(&cm) ;

	    } /* end if (ok) */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("bvi_loadchapters: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (bvi_loadchapters) */


static int chapters_start(struct chapters *cp,uchar *ap,int al)
{

	cp->ap = ap ;
	cp->al = al ;
	cp->ci = 0 ;
	return SR_OK ;
}
/* end subroutine (chapters_start) */


static int chapters_set(struct chapters *cp,int ci,int nv)
{

	if (nv > UCHAR_MAX)
	    return SR_RANGE ;

	if (ci >= cp->al)
	    return SR_OVERFLOW ;

	while (cp->ci < ci) {
	    cp->ap[cp->ci++] = 0 ;
	}

	if (cp->ci == ci) {
	    cp->ap[cp->ci++] = nv ;
	}

	return cp->ci ;
}
/* end subroutine (chapters_set) */


static int chapters_finish(struct chapters *cp)
{
	int		ci = cp->ci ;
	return ci ;
}
/* end subroutine (chapters_finish) */


static int mkcitekey(BVI_QUERY *bvp,uint *cip)
{
	uint		ci = 0 ;

	ci |= (bvp->b & UCHAR_MAX) ;

	ci = (ci << 8) ;
	ci |= (bvp->c & UCHAR_MAX) ;

	ci = (ci << 8) ;
	ci |= (bvp->v & UCHAR_MAX) ;

	*cip = ci ;
	return SR_OK ;
}
/* end subroutine (mkcitekey) */


#if	CF_SEARCH
static int vtecmp(const void *v1p,const void *v2p)
{
	uint		*vte1 = (uint *) v1p ;
	uint		*vte2 = (uint *) v2p ;
	uint		c1, c2 ;
	c1 = vte1[3] & 0x00FFFFFF ;
	c2 = vte2[3] & 0x00FFFFFF ;
	return (c1 - c2) ;
}
/* end subroutine (vtecmp) */
#endif /* CF_SEARCH */


#if	CF_DEBUGS
static int debugpresent(cchar *s,const void *a)
{
	int	rs = uc_mallpresent(a) ;
	debugprintf(s,rs) ;
	return rs ;
}
/* end subroutine (debugpresent) */
#endif /* CF_DEBUGS */


