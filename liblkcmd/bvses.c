/* bvses */

/* read or audit a BVSES (Bible Verse Structure) database */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine opens and allows for reading or auditing of a BVSES
	database.

	Synopsis:

	int bvses_open(op,pr,dbname)
	BVSES		*op ;
	const char	pr[] ;
	const char	dbname[] ;

	Arguments:

	- op		object pointer
	- pr		program root
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
#include	<vecstr.h>
#include	<char.h>
#include	<endianstr.h>
#include	<localmisc.h>

#include	"bvses.h"
#include	"bvshdr.h"
#include	"bvsbook.h"


/* local defines */

#define	BVSES_FMI	struct bvses_fmi
#define	BVSES_IDNAME	"var/bvses"

#define	SHIFTINT	(6 * 60)	/* possible time-shift */

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	TO_CHECK	4


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
extern char	*timestr_log(time_t,char *) ;
#endif


/* external variables */


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

static int	bvses_loadbegin(BVSES *,time_t) ;
static int	bvses_loadend(BVSES *) ;
static int	bvses_mapbegin(BVSES *,time_t) ;
static int	bvses_mapend(BVSES *) ;
static int	bvses_proc(BVSES *,time_t) ;
static int	bvses_verify(BVSES *,time_t) ;
static int	bvses_auditbt(BVSES *) ;
static int	bvses_auditct(BVSES *) ;
static int	bvses_checkupdate(BVSES *,time_t) ;


/* local variables */


/* exported variables */

BVSES_OBJ	bvses = {
	"bvses",
	sizeof(BVSES),
	0
} ;


/* exported subroutines */


int bvses_open(BVSES *op,cchar pr[],cchar dbname[])
{
	int		rs ;
	cchar		*cp ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;
	if (dbname[0] == '\0') return SR_INVALID ;

#ifdef	COMMENT
	if (strchr(dbname,'/') != NULL) return SR_INVALID ;
#endif

#if	CF_DEBUGS
	debugprintf("bvses_open: dbname=%s\n",dbname) ;
#endif

	memset(op,0,sizeof(BVSES)) ;

	if ((rs = uc_mallocstrw(pr,-1,&cp)) >= 0) {
	    op->pr = cp ;
	    if ((rs = uc_mallocstrw(dbname,-1,&cp)) >= 0) {
	        const int	clen = MAXNAMELEN ;
	        const char	*suf = BVSES_SUF ;
	        const char	*end = ENDIANSTR ;
	        char		cbuf[MAXNAMELEN+1] ;
	        op->dbname = cp ;
	        if ((rs = sncpy4(cbuf,clen,op->dbname,".",suf,end)) >= 0) {
	            const char	*ind = BVSES_IDNAME ;
	            char	tbuf[MAXPATHLEN+1] ;
	            if ((rs = mkpath3(tbuf,op->pr,ind,cbuf)) >= 0) {
	                int	tl = rs ;
	                if ((rs = uc_mallocstrw(tbuf,tl,&cp)) >= 0) {
	                    const time_t	dt = time(NULL) ;
	                    op->fname = cp ;
	                    if ((rs = bvses_loadbegin(op,dt)) >= 0) {
	                        op->ti_lastcheck = dt ;
	                        op->magic = BVSES_MAGIC ;
	                    }
	                    if (rs < 0) {
	                        uc_free(op->fname) ;
	                        op->fname = NULL ;
	                    }
	                } /* end if (memory-allocation) */
	            } /* end if (mkpath) */
	        } /* end if (sncpy) */
	        if (rs < 0) {
	            uc_free(op->dbname) ;
	            op->dbname = NULL ;
	        }
	    } /* end if (memory-allocation) */
	    if (rs < 0) {
	        uc_free(op->pr) ;
	        op->pr = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("bvses_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvses_open) */


int bvses_close(BVSES *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVSES_MAGIC) return SR_NOTOPEN ;

	rs1 = bvses_loadend(op) ;
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

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (bvses_close) */


int bvses_audit(BVSES *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVSES_MAGIC) return SR_NOTOPEN ;

/* verify that all list pointers and list entries are valid */

	if ((rs = bvses_auditbt(op)) >= 0) {
	    rs = bvses_auditct(op) ;
	}

#if	CF_DEBUGS
	debugprintf("bvses_audit: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvses_audit) */


int bvses_count(BVSES *op)
{
	BVSHDR		*hip ;
	int		rs = SR_OK ;
	int		nverses = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVSES_MAGIC) return SR_NOTOPEN ;

	hip = &op->fhi ;
	nverses = hip->nverses ;

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (bvses_count) */


int bvses_info(BVSES *op,BVSES_INFO *ip)
{
	BVSHDR		*hip ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BVSES_MAGIC) return SR_NOTOPEN ;

	hip = &op->fhi ;

	if (ip != NULL) {
	    memset(ip,0,sizeof(BVSES_INFO)) ;
	    ip->mtime = op->fmi.ti_mod ;
	    ip->ctime = (time_t) hip->wtime ;
	    ip->nzbooks = hip->nzbooks ;
	    ip->nbooks = hip->btlen ;
	    ip->nchapters = hip->ctlen ;
	    ip->nverses = hip->nverses ;
	    ip->nzverses = hip->nzverses ;
	}

	return (rs >= 0) ? hip->nverses : rs ;
}
/* end subroutine (bvses_info) */


int bvses_mkmodquery(BVSES *op,BVSES_VERSE *bvep,int mjd)
{
	BVSBOOK		be ;
	BVSES_FMI	*mip ;
	BVSHDR		*hip ;
	ushort		(*bt)[4] ;
	uint		b, c, v ;
	uint		nzverses ;
	uchar		*ct ;
	int		rs = SR_OK ;
	int		ci, cl ;
	int		vi = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (bvep == NULL) return SR_FAULT ;

	if (op->magic != BVSES_MAGIC) return SR_NOTOPEN ;

	if (mjd < 0) return SR_INVALID ;

	mip = &op->fmi ;
	hip = &op->fhi ;

#if	CF_DEBUGS
	debugprintf("bvses_mkmodquery: mjd=%u\n",mjd) ;
	debugprintf("bvses_mkmodquery: nzv=%u\n",hip->nzverses) ;
#endif

	bt = mip->bt ;

/* check for update */

	if (op->ncursors == 0) {
	    rs = bvses_checkupdate(op,0) ;
	}

	if (rs >= 0) {

/* perform the calculation */

	v = (mjd % hip->nzverses) ;
#ifdef	COMMENT /* what is this statement doing? */
	if (v == 0) v = hip->nzverses ;		/* not (nzverses-1) */
#endif
	vi = v ;			/* Verse-Index; return this value */

#if	CF_DEBUGS
	debugprintf("bvses_mkmodquery: vi=%u\n",vi) ;
#endif

	for (b = 1 ; b < hip->btlen ; b += 1) {
	    bvsbook_get(&be,bt[b]) ;

#if	CF_DEBUGS
	    debugprintf("bvses_mkmodquery: b=%u ci=%u al=%u nv=%u nzv=%u\n",
	        b,be.ci,be.al,be.nverses,be.nzverses) ;
#endif

	    if (be.al > 0) {
	        ci = be.ci ;
	        ct = (mip->ct + ci) ;
	        nzverses = (be.nverses - ct[0]) ;
	        if (v < nzverses) break ;
	        v -= nzverses ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("bvses_mkmodquery: mid b=%u v=%u\n",b,v) ;
#endif

/* loop through chapters reducing 'v' to as close to *zero* as possible */

	bvsbook_get(&be,bt[b]) ;
	ci = be.ci ;
	cl = hip->ctlen ;
	ct = (mip->ct + ci) ;
	for (c = 1 ; (c < cl) && (c < be.al) && (v >= ct[c]) ; c += 1) {
#if	CF_DEBUGS
	    debugprintf("bvses_mkmodquery: c=%u ncv=%u\n",c,ct[c]) ;
#endif
	    v -= ct[c] ;
	}

#if	CF_DEBUGS
	debugprintf("bvses_mkmodquery: fin c=%u ncv=%u v=%u\n",c,ct[c],v) ;
#endif

	memset(bvep,0,sizeof(BVSES_VERSE)) ;
	bvep->b = b ;
	bvep->c = c ;
	bvep->v = (v+1) ;

	} /* end if (ok ) */

#if	CF_DEBUGS
	debugprintf("bvses_get: ret rs=%d vi=%u\n",rs,vi) ;
#endif

	return (rs >= 0) ? vi : rs ;
}
/* end subroutine (bvses_get) */


/* private subroutines */


static int bvses_loadbegin(BVSES *op,time_t dt)
{
	int		rs ;
	int		fsize = 0 ;

	if ((rs = bvses_mapbegin(op,dt)) >= 0) {
	    fsize = rs ;
	    rs = bvses_proc(op,dt) ;
	    if (rs < 0)
	        bvses_mapend(op) ;
	} /* end if (loadbegin) */

#if	CF_DEBUGS
	debugprintf("bvses_loadbegin: ret rs=%d fsize=%u\n",rs,fsize) ;
#endif

	return (rs >= 0) ? fsize : rs ;
}
/* end subroutine (bvses_loadbegin) */


static int bvses_loadend(BVSES *op)
{
	BVSES_FMI	*mip ;
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = bvses_mapend(op) ;
	if (rs >= 0) rs = rs1 ;

	mip = &op->fmi ;
	mip->bt = NULL ;
	mip->ct = NULL ;
	return rs ;
}
/* end subroutine (bvses_loadend) */


static int bvses_mapbegin(BVSES *op,time_t dt)
{
	BVSES_FMI	*mip = &op->fmi ;
	int		rs ;
	int		fsize = 0 ;

#if	CF_DEBUGS
	debugprintf("bvses_mapbegin: fname=%s\n",op->fname) ;
#endif

	if (op->fname == NULL)
	    return SR_FAULT ;

	if ((rs = u_open(op->fname,O_RDONLY,0666)) >= 0) {
	    struct ustat	sb ;
	    int			fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {

	        fsize = (sb.st_size & UINT_MAX) ;
	        if (fsize > 0) {
	            size_t	ms = (size_t) fsize ;
	            int		mp = PROT_READ ;
	            int		mf = MAP_SHARED ;
	            void	*md ;
	            if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	                mip->mapdata = md ;
	                mip->mapsize = ms ;
	                mip->ti_mod = sb.st_mtime ;
	                mip->ti_map = dt ;
	            } /* end if (u_mamp) */

#if	CF_DEBUGS
	            debugprintf("bvses_mapbegin: u_mmap() rs=%d \n",rs) ;
#endif

	        } else
	            rs = SR_UNATCH ;

	    } /* end if (fstat) */
	    u_close(fd) ;
	} /* end if (file-process) */

#if	CF_DEBUGS
	debugprintf("bvses_mapbegin: ret rs=%d fsize=%u\n",rs,fsize) ;
#endif

	return (rs >= 0) ? fsize : rs ;
}
/* end subroutine (bvses_mapbegin) */


static int bvses_mapend(BVSES *op)
{
	BVSES_FMI	*mip = &op->fmi ;
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
/* end subroutine (bvses_mapend) */


static int bvses_checkupdate(BVSES *op,time_t dt)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

	if (op->ncursors == 0) {
	    if (dt <= 0) dt = time(NULL) ;
	    if ((dt - op->ti_lastcheck) >= TO_CHECK) {
	        struct ustat	sb ;
	        BVSES_FMI	*mip = &op->fmi ;
	        op->ti_lastcheck = dt ;
	        if ((rs1 = u_stat(op->fname,&sb)) >= 0) {
	            f = FALSE ;
	            f = f || (sb.st_mtime > mip->ti_mod) ;
	            f = f || (sb.st_mtime > mip->ti_map) ;
	            if (f) {
	                rs1 = bvses_loadend(op) ;
	                if (rs >= 0) rs = rs1 ;
	                if (rs >= 0) {
	                    rs = bvses_loadbegin(op,dt) ;
	  	        }
	            } /* end if (update) */
	        } /* end if (stat) */
	    } /* end if (time-out) */
	} /* end if (no cursors out) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (bvses_checkupdate) */


static int bvses_proc(BVSES *op,time_t dt)
{
	BVSES_FMI	*mip = &op->fmi ;
	BVSHDR		*hip = &op->fhi ;
	int		rs ;

	if ((rs = bvshdr(hip,1,mip->mapdata,mip->mapsize)) >= 0) {
	    if ((rs = bvses_verify(op,dt)) >= 0) {
	            mip->bt = (ushort (*)[4]) (mip->mapdata + hip->btoff) ;
	            mip->ct = (uchar *) (mip->mapdata + hip->ctoff) ;
	    } /* end if */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("bvses_proc: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvses_proc) */


static int bvses_verify(BVSES *op,time_t dt)
{
	BVSES_FMI	*mip = &op->fmi ;
	BVSHDR		*hip = &op->fhi ;
	uint		dtime = (uint) dt ;
	int		rs = SR_OK ;
	int		size ;
	int		f = TRUE ;

#if	CF_DEBUGS
	debugprintf("bvses_verify: ent nverses=%u\n",hip->nverses) ;
#endif

	f = f && (hip->nverses > 0) ;

#if	CF_DEBUGS
	debugprintf("bvses_verify: fsize=%u mapsize=%u\n",
	   hip->fsize,mip->mapsize) ;
#endif

	f = f && (hip->fsize == mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("bvses_verify: fsize=%u f=%u\n",
	    hip->fsize,f) ;
#endif

	f = f && (hip->wtime > 0) && (hip->wtime <= (dtime + SHIFTINT)) ;

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("bvses_verify: wtime=%s f=%u\n",
	        timestr_log(((time_t) hip->wtime),timebuf),f) ;
	}
#endif

/* alignment restriction */
	f = f && ((hip->btoff & (sizeof(uint)-1)) == 0) ;

#if	CF_DEBUGS
	debugprintf("bvses_verify: 2 f=%u\n",f) ;
	debugprintf("bvses_verify: btoff=%u\n",hip->btoff) ;
#endif

/* size restrictions */
	f = f && (hip->btoff <= mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("bvses_verify: 3 f=%u\n",f) ;
#endif

	size = hip->btlen * 4 * sizeof(ushort) ;
	f = f && ((hip->btoff + size) <= mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("bvses_verify: 4 f=%u\n",f) ;
#endif

/* size restrictions */
	f = f && (hip->ctoff <= mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("bvses_verify: 5 f=%u\n",f) ;
#endif

	size = hip->ctlen * 1 * sizeof(uchar) ;
	f = f && ((hip->ctoff + size) <= mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("bvses_verify: 5 f=%u\n",f) ;
#endif

/* size restrictions */
	f = f && (hip->btlen <= hip->ctlen) ;

#if	CF_DEBUGS
	debugprintf("bvses_verify: 6 f=%u\n",f) ;
#endif

/* get out */

	if (! f)
	    rs = SR_BADFMT ;

#if	CF_DEBUGS
	debugprintf("bvses_verify: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvses_verify) */


static int bvses_auditbt(BVSES *op)
{
	BVSES_FMI	*mip = &op->fmi ;
	BVSBOOK		be ;
	BVSHDR		*hip = &op->fhi ;
	ushort		(*bt)[4] ;
	int		rs = SR_OK ;
	int		i ;

	bt = mip->bt ;

/* "book" table */

	for (i = 0 ; (rs >= 0) && (i < hip->btlen) ; i += 1) {

	    rs = bvsbook_get(&be,bt[i]) ;

	    if ((rs >= 0) && (be.al >= hip->ctlen))
	        rs = SR_BADFMT ;

	    if ((rs >= 0) && (be.ci >= hip->ctlen))
	        rs = SR_BADFMT ;

	    if ((rs >= 0) && (be.nverses >= hip->nverses))
	        rs = SR_BADFMT ;

	} /* end for (record table entries) */

#if	CF_DEBUGS
	debugprintf("bvses_auditbt: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvses_auditbt) */


static int bvses_auditct(BVSES *op)
{
	BVSES_FMI	*mip = &op->fmi ;
	BVSHDR		*hip = &op->fhi ;
	uchar		*ct ;
	int		rs = SR_OK ;
	int		nverses = 0 ;
	int		i ;

	ct = mip->ct ;
	for (i = 0 ; (rs >= 0) && (i < hip->ctlen) ; i += 1) {
	    nverses += (ct[i] & UCHAR_MAX) ;
	} /* end for (record table entries) */

	if ((rs >= 0) && (nverses > hip->nverses))
	    rs = SR_BADFMT ;

#if	CF_DEBUGS
	debugprintf("bvses_auditct: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bvses_auditct) */


