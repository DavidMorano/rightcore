/* bpi */

/* read or audit a BPI (Bible Paragraph Index) database */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGENTS	0		/* mode debugging */
#define	CF_SEARCH	1		/* use 'bsearch(3c)' */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine opens and allows for reading or auditing of a BPI (Bible
        Paragraph Index) database.

	Synopsis:

	int bpi_open(op,dbname)
	BPI		*op ;
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
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<limits.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<char.h>
#include	<endianstr.h>
#include	<localmisc.h>

#include	"bpi.h"
#include	"bpihdr.h"
#include	"bvcitekey.h"


/* local defines */

#undef	COMMENT

#define	BPI_MAGIC	0x88773422
#define	BPI_FMI		struct bpi_fmi

#define	BPI_KA		sizeof(BPI_LINE)
#define	BPI_BO(v)	((BPI_KA - ((v) % BPI_KA)) % BPI_KA)

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

BPI_OBJ	bpi = {
	"bpi",
	sizeof(BPI),
	sizeof(BPI_CUR)
} ;


/* local structures */


/* forward references */

static int	bpi_loadbegin(BPI *,time_t) ;
static int	bpi_loadend(BPI *) ;
static int	bpi_mapcreate(BPI *,time_t) ;
static int	bpi_mapdestroy(BPI *) ;
static int	bpi_proc(BPI *,time_t) ;
static int	bpi_verify(BPI *,time_t) ;
static int	bpi_auditvt(BPI *) ;
static int	bpi_checkupdate(BPI *,time_t) ;
static int	bpi_search(BPI *,BPI_QUERY *) ;
static int	bpi_loadbve(BPI *,BPI_VERSE *,int) ;

static int	mkcitekey(BPI_QUERY *,uint *) ;

#if	CF_SEARCH
static int	vtecmp(const void *,const void *) ;
#endif


/* local variables */


/* exported subroutines */


int bpi_open(BPI *op,cchar *dbname)
{
	const time_t	dt = time(NULL) ;
	int		rs = SR_OK ;
	int		tl ;
	int		nverses = 0 ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (dbname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(BPI)) ;

	if ((rs = uc_mallocstrw(dbname,-1,&cp)) >= 0) {
	    const char	*suf = BPI_SUF ;
	    const char	*end = ENDIANSTR ;
	    char	tbuf[MAXPATHLEN + 1] ;
	    op->dbname = cp ;
	    if ((rs = mkfnamesuf2(tbuf,op->dbname,suf,end)) >= 0) {
	        tl = rs ;
	        if ((rs = uc_mallocstrw(tbuf,tl,&cp)) >= 0) {
	            op->fname = cp ;
	            if ((rs = bpi_loadbegin(op,dt)) >= 0) {
	                op->ti_lastcheck = dt ;
	                op->magic = BPI_MAGIC ;
	            }
	            if (rs < 0) {
	                uc_free(op->fname) ;
	                op->fname = NULL ;
	            }
	        } /* end if (m-a) */
	    } /* end if (mkfnamesuf) */
	    if (rs < 0) {
	        uc_free(op->dbname) ;
	        op->dbname = NULL ;
	    }
	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("bpi_open: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bpi_open) */


int bpi_close(BPI *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BPI_MAGIC) return SR_NOTOPEN ;

	rs1 = bpi_loadend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

	if (op->dbname != NULL) {
	    rs1 = uc_free(op->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (bpi_close) */


int bpi_audit(BPI *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BPI_MAGIC) return SR_NOTOPEN ;

/* verify that all list pointers and list entries are valid */

	rs = bpi_auditvt(op) ;

#if	CF_DEBUGS
	debugprintf("bpi_audit: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bpi_audit) */


int bpi_count(BPI *op)
{
	BPIHDR		*hip ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BPI_MAGIC) return SR_NOTOPEN ;

	hip = &op->fhi ;
	return (rs >= 0) ? hip->nverses : rs ;
}
/* end subroutine (bpi_count) */


int bpi_info(BPI *op,BPI_INFO *ip)
{
	BPIHDR		*hip ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BPI_MAGIC) return SR_NOTOPEN ;

	hip = &op->fhi ;

	if (ip != NULL) {
	    memset(ip,0,sizeof(BPI_INFO)) ;
	    ip->mtime = op->fmi.ti_mod ;
	    ip->ctime = (time_t) hip->wtime ;
	    ip->maxbook = hip->maxbook ;
	    ip->maxchapter = hip->maxchapter ;
	    ip->count = hip->nverses ;
	    ip->nzverses = hip->nzverses ;
	}

	return (rs >= 0) ? hip->nverses : rs ;
}
/* end subroutine (bpi_info) */


int bpi_get(BPI *op,BPI_QUERY *qp)
{
	int		rs = SR_OK ;
	int		vi = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;

	if (op->magic != BPI_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("bpi_get: q=%u:%u:%u\n",qp->b,qp->c,qp->v) ;
#endif

/* check for update */

	if (op->ncursors == 0) {
	    rs = bpi_checkupdate(op,0) ;
	}

	if (rs >= 0) {
	    rs = bpi_search(op,qp) ;
	    vi = rs ;
	}

#if	CF_DEBUGS
	debugprintf("bpi_get: search rs=%d vi=%u\n",rs,vi) ;
	debugprintf("bpi_get: ret rs=%d vi=%u\n",rs,vi) ;
#endif

	return (rs >= 0) ? vi : rs ;
}
/* end subroutine (bpi_get) */


int bpi_curbegin(BPI *op,BPI_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != BPI_MAGIC) return SR_NOTOPEN ;

	curp->i = 0 ;
	op->ncursors += 1 ;

	return SR_OK ;
}
/* end subroutine (bpi_curbegin) */


int bpi_curend(BPI *op,BPI_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != BPI_MAGIC) return SR_NOTOPEN ;

	curp->i = 0 ;
	if (op->ncursors > 0)
	    op->ncursors -= 1 ;

	return SR_OK ;
}
/* end subroutine (bpi_curend) */


int bpi_enum(BPI *op,BPI_CUR *curp,BPI_VERSE *bvep)
{
	BPIHDR		*hip ;
	int		rs = SR_OK ;
	int		vi ;
	int		vtlen ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (bvep == NULL) return SR_FAULT ;

	if (op->magic != BPI_MAGIC) return SR_NOTOPEN ;

	if (op->ncursors == 0) return SR_INVALID ;

	vi = (curp->i < 0) ? 0 : (curp->i + 1) ;

	hip = &op->fhi ;
	vtlen = hip->vilen ;
	if (vi >= vtlen) rs = SR_NOTFOUND ;

	if (rs >= 0) {
	    if ((rs = bpi_loadbve(op,bvep,vi)) >= 0) {
	    	curp->i = vi ;
	    }
	}

	return rs ;
}
/* end subroutine (bpi_enum) */


/* private subroutines */


static int bpi_loadbegin(BPI *op,time_t daytime)
{
	int		rs ;
	int		nverses = 0 ;

	if ((rs = bpi_mapcreate(op,daytime)) >= 0) {
	    rs = bpi_proc(op,daytime) ;
	    nverses = rs ;
	    if (rs < 0)
		bpi_mapdestroy(op) ;
	} /* end if (map) */

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bpi_loadbegin) */


static int bpi_loadend(BPI *op)
{
	BPI_FMI		*mip ;
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = bpi_mapdestroy(op) ;
	if (rs >= 0) rs = rs1 ;

	mip = &op->fmi ;
	mip->vt = NULL ;
	return rs ;
}
/* end subroutine (bpi_loadend) */


static int bpi_mapcreate(BPI *op,time_t daytime)
{
	BPI_FMI		*mip = &op->fmi ;
	int		rs ;

	if (op->fname == NULL) return SR_FAULT ;

	if ((rs = u_open(op->fname,O_RDONLY,0666)) >= 0) {
	    struct ustat	sb ;
	    const int		fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
	        const size_t	fsize = (sb.st_size & UINT_MAX) ;
	        if (fsize > 0) {
	            size_t	ms = (size_t) fsize ;
	            int		mp = PROT_READ ;
	            int		mf = MAP_SHARED ;
	            void	*md ;
	            if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	                mip->mapdata = md ;
	                mip->mapsize = ms ;
	                mip->ti_mod = sb.st_mtime ;
	                mip->ti_map = daytime ;
	            } /* end if (u_mmap) */
	        } else
		    rs = SR_UNATCH ;
	    } /* end if (stat) */
	    u_close(fd) ;
	} /* end if (file-open) */

	return rs ;
}
/* end subroutine (bpi_mapcreate) */


static int bpi_mapdestroy(BPI *op)
{
	BPI_FMI		*mip = &op->fmi ;
	int		rs = SR_OK ;

	if (mip->mapdata != NULL) {
	    rs = u_munmap(mip->mapdata,mip->mapsize) ;
	    mip->mapdata = NULL ;
	    mip->mapsize = 0 ;
	    mip->ti_map = 0 ;
	}

	return rs ;
}
/* end subroutine (bpi_mapdestroy) */


static int bpi_checkupdate(BPI *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (op->ncursors == 0) {
	    if (dt <= 0) dt = time(NULL) ;
	    if ((dt - op->ti_lastcheck) >= TO_CHECK) {
	        struct ustat	sb ;
	        BPI_FMI		*mip = &op->fmi ;
	        op->ti_lastcheck = dt ;
	        if ((rs = u_stat(op->fname,&sb)) >= 0) {
	            f = f || (sb.st_mtime > mip->ti_mod) ;
	            f = f || (sb.st_mtime > mip->ti_map) ;
	            if (f) {
	                bpi_loadend(op) ;
	                rs = bpi_loadbegin(op,dt) ;
	            } /* end if (update) */
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	    } /* end if (needed checking) */
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (bpi_checkupdate) */


static int bpi_proc(BPI *op,time_t daytime)
{
	BPI_FMI		*mip = &op->fmi ;
	BPIHDR		*hip = &op->fhi ;
	int		rs ;
	int		nverses = 0 ;

	rs = bpihdr(hip,1,mip->mapdata,mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("bpi_proc: bpihdr() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    rs = bpi_verify(op,daytime) ;
	    nverses = hip->nverses ;
	}

#if	CF_DEBUGS
	debugprintf("bpi_proc: bpi_verify() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    mip->vt = (uint (*)[1]) (mip->mapdata + hip->vioff) ;
#if	CF_DEBUGS
	debugprintf("bpi_proc: vt={%p}\n",mip->vt) ;
#endif
	}

#if	CF_DEBUGS
	debugprintf("bpi_proc: ret rs=%d nverses=%u\n",rs,nverses) ;
#endif

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bpi_proc) */


static int bpi_verify(BPI *op,time_t daytime)
{
	BPI_FMI		*mip = &op->fmi ;
	BPIHDR		*hip = &op->fhi ;
	uint		utime = (uint) daytime ;
	int		rs = SR_OK ;
	int		size ;
	int		f = TRUE ;

	f = f && (hip->fsize == mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("bpi_verify: fsize=%u f=%u\n",
		hip->fsize,f) ;
#endif

	f = f && (hip->wtime > 0) && (hip->wtime <= (utime + SHIFTINT)) ;

#if	CF_DEBUGS
	{
	char	timebuf[TIMEBUFLEN + 1] ;
	debugprintf("bpi_verify: wtime=%s f=%u\n",
		timestr_log(((time_t) hip->wtime),timebuf),f) ;
	}
#endif

/* alignment restriction */

#if	CF_DEBUGS
	debugprintf("bpi_verify: vioff=%u\n",hip->vioff) ;
	debugprintf("bpi_verify: vilen=%u\n",hip->vilen) ;
#endif

	f = f && ((hip->vioff & (sizeof(int)-1)) == 0) ;

#if	CF_DEBUGS
	debugprintf("bpi_verify: 1 f=%u\n",f) ;
#endif

/* size restrictions */

	f = f && (hip->vioff <= mip->mapsize) ;
	size = (hip->vilen * 1) * sizeof(uint) ;
	f = f && ((hip->vioff + size) <= mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("bpi_verify: 2 f=%u\n",f) ;
#endif

/* something restriction? */

	f = f && (hip->vilen == hip->nverses) ;

#if	CF_DEBUGS
	debugprintf("bpi_verify: 3 f=%u\n",f) ;
#endif

#if	CF_DEBUGS && CF_DEBUGENTS
	{
		uint	(*vt)[1] = (uint (*)[1]) (mip->mapdata + hip->vioff) ;
		int	vtlen = hip->vilen ;
		int	i ;
		for (i = 0 ; i < vtlen ; i += 1)
		debugprintf("bpi_verify: vt[%u][0]=%08X\n",i,vt[i][0]) ;
	}
#endif /* CF_DEBUGENTS */

/* get out */

	if (! f)
	    rs = SR_BADFMT ;

#if	CF_DEBUGS
	debugprintf("bpi_verify: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bpi_verify) */


static int bpi_auditvt(BPI *op)
{
	BPI_FMI		*mip = &op->fmi ;
	BPIHDR		*hip = &op->fhi ;
	uint		(*vt)[1] ;
	uint		pcitcmpval = 0 ;
	uint		citcmpval ;
	int		rs = SR_OK ;
	int		i ;

	vt = mip->vt ;

/* "verses" table */

	for (i = 1 ; (rs >= 0) && (i < hip->vilen) ; i += 1) {

/* verify all entries are ordered w/ increasing citations */

	    citcmpval = vt[i][0] & 0x00FFFFFF ;
	    if (citcmpval < pcitcmpval) {
	        rs = SR_BADFMT ;
	        break ;
	    }
	    pcitcmpval = citcmpval ;

	} /* end for (record table entries) */

#if	CF_DEBUGS
	debugprintf("bpi_auditvt: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bpi_auditvt) */


static int bpi_search(BPI *op,BPI_QUERY *qp)
{
	BPI_FMI		*mip = &op->fmi ;
	BPIHDR		*hip = &op->fhi ;
	uint		(*vt)[1] ;
	uint		citekey ;
	uint		vte[1] ;
	int		rs = SR_OK ;
	int		vtlen ;
	int		vi = 0 ;

	vt = mip->vt ;
	vtlen = hip->vilen ;

/* search for entry */

	mkcitekey(qp,&citekey) ;

#if	CF_DEBUGS
	debugprintf("bpi_search: vtlen=%u\n",vtlen) ;
	debugprintf("bpi_search: citekey=%08X\n",citekey) ;
#endif

	vte[0] = citekey ;

#if	CF_SEARCH
	{
	    uint	*vtep ;
	    int		vtesize = (1 * sizeof(uint)) ;

	    vtep = (uint *) bsearch(vte,vt,vtlen,vtesize,vtecmp) ;

	    rs = (vtep != NULL) ? ((vtep - vt[0]) >> 2) : SR_NOTFOUND ;
	    vi = rs ;
	}
#else /* CF_SEARCH */
	{
	    for (vi = 0 ; vi < vtlen ; vi += 1) {
	        if ((vt[vi][0] & 0x00FFFFFF) == citekey)
		    break ;
	    }
	    rs = (vi < vtlen) ? vi : SR_NOTFOUND ;
	}
#endif /* CF_SEARCH */

	return (rs >= 0) ? vi : rs ;
}
/* end subroutine (bpi_search) */


static int bpi_loadbve(BPI *op,BPI_VERSE *bvep,int vi)
{
	BPI_FMI		*mip = &op->fmi ;
	BPIHDR		*hip = &op->fhi ;
	uint		*vte ;
	int		rs = SR_OK ;
	int		vtlen ;

	if (bvep == NULL) return SR_FAULT ;

	memset(bvep,0,sizeof(BPI_VERSE)) ;

	vtlen = hip->vilen ;
	if (vi >= vtlen) rs = SR_NOANODE ;

/* load the basic stuff */

	if (rs >= 0) {
	    vte = mip->vt[vi] ;
	    bvep->nlines = (vte[0] >> 24) & 0xFF ;
	    bvep->b = (vte[0] >> 16) & 0xFF ;
	    bvep->c = (vte[0] >> 8) & 0xFF ;
	    bvep->v = (vte[0] >> 0) & 0xFF ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("bpi_loadbve: ret rs=%d\n") ;
#endif

	return rs ;
}
/* end subroutine (bpi_loadbve) */


static int mkcitekey(BPI_QUERY *bvp,uint *cip)
{
	uint	ci = 0 ;

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

	c1 = vte1[0] & 0x00FFFFFF ;
	c2 = vte2[0] & 0x00FFFFFF ;
	return (c1 - c2) ;
}
/* end subroutine (vtecmp) */
#endif /* CF_SEARCH */


