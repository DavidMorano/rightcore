/* cyi */

/* read or audit a CYI database */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SEARCH	1		/* use 'bsearch(3c)' */
#define	CF_ISOUR	0		/* isOur */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine opens and allows for reading or auditing of a VAR
	database (which currently consists of two files).

	Synopsis:

	int cyi_open(op,year,dname,cname)
	CYI		*op ;
	int		year ;
	cchar		dname[] ;
	cchar		cname[] ;

	Arguments:

	op		object pointer
	year		year
	dnames		list of (pointers to) directories to search
	cnames		list of (pointers to) calendar files to use

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
#include	<endianstr.h>
#include	<vecstr.h>
#include	<ids.h>
#include	<storebuf.h>
#include	<char.h>
#include	<localmisc.h>

#include	"cyi.h"
#include	"cyihdr.h"


/* local defines */

#define	CYI_FMI		struct cyi_fmi
#define	CYI_KA		sizeof(CYI_LINE)
#define	CYI_FSUF	"cyi"
#define	CYI_FSUFLEN	10
#define	CYI_BO(v)	((CYI_KA - ((v) % CYI_KA)) % CYI_KA)

#define	FE_CYI		"cyi"		/* variable-index */

#define	SHIFTINT	(6 * 60)	/* possible time-shift */

#define	TO_CHECK	4


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	pathadd(char *,int,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;

#if	CF_DEBUGS
extern char	*timestr_log(time_t,char *) ;
#endif


/* external variables */


/* exported variables */

CYI_OBJ	cyi = {
	"cyi",
	sizeof(CYI),
	sizeof(CYI_CUR)
} ;


/* local structures */

struct bventry {
	uint	voff ;
	uint	vlen ;
	uint	li ;			/* index-number of first line-entry */
	uint	hash ;
	uint	citation ;		/* (nlines, m, d) */
} ;

struct blentry {
	uint	loff ;
	uint	llen ;
} ;


/* forward references */

static int	cyi_dbfind(CYI *,time_t,cchar *,cchar *,int) ;
static int	cyi_dbfindname(CYI *,IDS *,time_t,char *,cchar *,cchar *,int) ;
static int	cyi_dbfindone(CYI *,time_t,cchar *,cchar *) ;
static int	cyi_dblose(CYI *) ;

static int	cyi_loadbegin(CYI *,time_t,const char *) ;
static int	cyi_loadend(CYI *) ;
static int	cyi_mapcreate(CYI *,time_t,const char *) ;
static int	cyi_mapdestroy(CYI *) ;
static int	cyi_proc(CYI *,time_t) ;
static int	cyi_verify(CYI *,time_t) ;
static int	cyi_auditvt(CYI *) ;
static int	cyi_checkupdate(CYI *,time_t) ;
static int	cyi_loadbve(CYI *,CYI_ENT *,char *,int,uint *) ;

#if	CF_SEARCH
static int	cyi_bsearch(CYI *,uint (*)[5],int,uint *) ;
#else
static int	cyi_lsearch(CYI *,uint (*)[5],int,uint *) ;
#endif

static int	mkydname(char *,cchar *,int) ;
static int	mkcitekey(uint *,CYI_QUERY *) ;

#if	CF_SEARCH
static int	vtecmp(const void *,const void *) ;
#endif

#if	CF_ISOUR
static int	isOurSuffix(const char *,const char *) ;
static int	isNotOurFile(int) ;
#endif


/* local variables */


/* exported subroutines */


int cyi_open(CYI *op,int year,cchar dname[],cchar cname[])
{
	const time_t	dt = time(NULL) ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (dname == NULL) return SR_FAULT ;
	if (cname == NULL) return SR_FAULT ;

	if (dname[0] == '\0') return SR_INVALID ;
	if (cname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(CYI)) ;

#if	CF_DEBUGS
	debugprintf("cyi_open: dname=%s\n",dname) ;
	debugprintf("cyi_open: cname=%s\n",cname) ;
	debugprintf("cyi_open: year=%d\n",year) ;
#endif

	if ((rs = cyi_dbfind(op,dt,dname,cname,year)) >= 0) {
	    op->ti_lastcheck = dt ;
	    op->year = year ;
	    op->magic = CYI_MAGIC ;
	}

#if	CF_DEBUGS
	debugprintf("cyi_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cyi_open) */


int cyi_close(CYI *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CYI_MAGIC) return SR_NOTOPEN ;

	rs1 = cyi_dblose(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (cyi_close) */


int cyi_audit(CYI *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CYI_MAGIC) return SR_NOTOPEN ;

/* verify that all list pointers and list entries are valid */

	rs = cyi_auditvt(op) ;

#if	CF_DEBUGS
	debugprintf("cyi_audit: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cyi_audit) */


int cyi_count(CYI *op)
{
	CYIHDR		*hip ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CYI_MAGIC) return SR_NOTOPEN ;

	hip = &op->fhi ;
	return (rs >= 0) ? hip->nentries : rs ;
}
/* end subroutine (cyi_count) */


int cyi_info(CYI *op,CYI_INFO *ip)
{
	CYIHDR		*hip ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CYI_MAGIC) return SR_NOTOPEN ;

	hip = &op->fhi ;
	if (ip != NULL) {
	    memset(ip,0,sizeof(CYI_INFO)) ;
	    ip->mtime = op->fmi.ti_mod ;
	    ip->ctime = (time_t) hip->wtime ;
	    ip->count = hip->nentries ;
	    ip->year = hip->year ;
	}

	return (rs >= 0) ? hip->nentries : rs ;
}
/* end subroutine (cyi_info) */


int cyi_curbegin(CYI *op,CYI_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != CYI_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(CYI_CUR)) ;
	curp->citekey = UINT_MAX ;
	curp->i = -1 ;
	curp->magic = CYI_MAGIC ;
	op->ncursors += 1 ;

	return SR_OK ;
}
/* end subroutine (cyi_curbegin) */


int cyi_curend(CYI *op,CYI_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != CYI_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != CYI_MAGIC) return SR_NOTOPEN ;

	if (op->ncursors > 0)
	    op->ncursors -= 1 ;

	curp->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (cyi_curend) */


int cyi_lookcite(CYI *op,CYI_CUR *curp,CYI_QUERY *qp)
{
	CYI_FMI		*mip ;
	CYIHDR		*hip ;
	uint		(*vt)[5] ;
	uint		vte[5] ;
	uint		citekey ;
	int		rs = SR_OK ;
	int		vtlen ;
	int		vi = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (qp == NULL) return SR_FAULT ;

	if (op->magic != CYI_MAGIC) return SR_NOTOPEN ;
	if (curp->magic != CYI_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("cyi_lookcite: q=(%u:%u)\n",qp->m,qp->d) ;
#endif

	mip = &op->fmi ;
	hip = &op->fhi ;

	vt = mip->vt ;
	vtlen = hip->vilen ;

/* check for update */

	if (op->ncursors == 0) {
	    rs = cyi_checkupdate(op,0) ;
	}

	if (rs >= 0) {

/* search for entry */

	mkcitekey(&citekey,qp) ;

#if	CF_DEBUGS
	debugprintf("cyi_lookcite: citekey=%08X\n",citekey) ;
	debugprintf("cyi_lookcite: vtlen=%u\n",vtlen) ;
#endif

	vte[3] = citekey ;

#if	CF_SEARCH
	{
	rs = cyi_bsearch(op,vt,vtlen,vte) ;
	vi = rs ;
	}
#else /* CF_SEARCH */
	{
	rs = cyi_lsearch(op,vt,vtlen,vte) ;
	vi = rs ;
	}
#endif /* CF_SEARCH */

#if	CF_DEBUGS
	debugprintf("cyi_lookcite: search rs=%d vi=%u\n",rs,vi) ;
#endif

	if (rs >= 0) {
	    curp->citekey = citekey ;
	    curp->i = vi ;
	}

	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("cyi_lookcite: ret rs=%d vi=%u\n",rs,vi) ;
#endif

	return (rs >= 0) ? vi : rs ;
}
/* end subroutine (cyi_lookcite) */


int cyi_read(CYI *op,CYI_CUR *curp,CYI_ENT *ep,char ebuf[],int elen)
{
	CYI_FMI		*mip ;
	CYIHDR		*hip ;
	uint		vi ;
	uint		(*vt)[5] ;
	uint		*vtep ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (ebuf == NULL) return SR_FAULT ;

	if (op->magic != CYI_MAGIC) return SR_NOTOPEN ;

	if (op->ncursors == 0) return SR_INVALID ;

	if (curp->i < 0)
	    return SR_NOTFOUND ;

	vi = curp->i ;

	mip = &op->fmi ;
	hip = &op->fhi ;

#if	CF_DEBUGS
	debugprintf("cyi_read: ent elen=%d\n",elen) ;
	debugprintf("cyi_read: vilen=%u vi=%u\n",hip->vilen,vi) ;
#endif

	if (vi < hip->vllen) {

	    vt = mip->vt ;
	    vtep = vt[vi] ;

#if	CF_DEBUGS
	debugprintf("cyi_read: c_cite=%08x\n",curp->citekey) ;
	debugprintf("cyi_read: vte[3]=%08x\n", vtep[3]) ;
#endif

	    if (curp->citekey != UINT_MAX) {

		if (curp->citekey != (vtep[3] & 0xffff))
		    rs = SR_NOTFOUND ;

	    } /* end if */

	} else
	    rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("cyi_read: mid rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    rs = cyi_loadbve(op,ep,ebuf,elen,vtep) ;

#if	CF_DEBUGS
	debugprintf("cyi_read: cyi_loadbve() rs=%d\n",rs) ;
#endif

	}

	if (rs >= 0)
	    curp->i = (vi + 1) ;

#if	CF_DEBUGS
	debugprintf("cyi_read: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cyi_read) */


int cyi_enum(CYI *op,CYI_CUR *curp,CYI_ENT *bvep,char ebuf[],int elen)
{

	return cyi_read(op,curp,bvep,ebuf,elen) ;
}
/* end subroutine (cyi_enum) */


/* private subroutines */


static int cyi_dbfind(CYI *op,time_t dt,cchar dname[],cchar cname[],int y)
{
	IDS		id ;
	int		rs ;
	int		tl = 0 ;

#if	CF_DEBUGS
	debugprintf("cyi_dbfind: ent\n") ;
#endif

	if ((rs = ids_load(&id)) >= 0) {
	    char	tbuf[MAXPATHLEN + 1] ;
	    if ((rs = cyi_dbfindname(op,&id,dt,tbuf,dname,cname,y)) >= 0) {
	        cchar	*cp ;
#if	CF_DEBUGS
		debugprintf("cyi_dbfind: tl=%d\n",rs) ;
		debugprintf("cyi_dbfind: tb=%s\n",tbuf) ;
#endif
	        tl = rs ;
	        if ((rs = uc_mallocstrw(tbuf,tl,&cp)) >= 0) {
	            op->fname = cp ;
		}
	    }
	    ids_release(&id) ;
	} /* end if (ids) */

#if	CF_DEBUGS
	debugprintf("cyi_dbfind: ret rs=%d tl=%u\n",rs,tl) ;
#endif

	return (rs >= 0) ? tl : rs ;
}
/* end subroutine (cyi_dbfind) */


static int cyi_dbfindname(CYI *op,IDS *idp,time_t dt,char *tbuf,
		cchar *dname,cchar *cal,int y)
{
	int		rs ;
	int		tl = 0 ;
	char		ydname[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	debugprintf("cyi_dbfindname: ent\n") ;
	debugprintf("cyi_dbfindname: dname=%s\n",dname) ;
	debugprintf("cyi_dbfindname: cal=%s\n",cal) ;
#endif

	if ((rs = mkydname(ydname,dname,y)) >= 0) {
	    const int	flen = CYI_FSUFLEN ;
	    cchar	*suf = CYI_FSUF ;
	    cchar	*end = ENDIANSTR ;
	    char	fsuf[CYI_FSUFLEN + 1] ;
#if	CF_DEBUGS
	debugprintf("cyi_dbfindname: ydname=%s\n",ydname) ;
#endif
	    if ((rs = sncpy2(fsuf,flen,suf,end)) >= 0) {
	        char	cname[MAXNAMELEN + 1] ;
#if	CF_DEBUGS
	debugprintf("cyi_dbfindname: fsuf=%s\n",fsuf) ;
#endif
	        if ((rs = snsds(cname,MAXNAMELEN,cal,fsuf)) >= 0) {
#if	CF_DEBUGS
	debugprintf("cyi_dbfindname: cname=%s\n",cname) ;
#endif
		    if ((rs = mkpath2(tbuf,ydname,cname)) >= 0) {
			struct ustat	sb ;
#if	CF_DEBUGS
	debugprintf("cyi_dbfindname: tb=%s\n",tbuf) ;
#endif
			if ((rs = uc_stat(tbuf,&sb)) >= 0) {
			    const int	am = (R_OK) ;
			    if ((rs = sperm(idp,&sb,am)) >= 0) {
	                        tl = rs ;
	                        rs = cyi_dbfindone(op,dt,cal,tbuf) ;
			    }
			}
#if	CF_DEBUGS
			debugprintf("cyi_dbfindname: stat-out rs=%d\n",rs) ;
#endif
		    } /* end if (mkpath) */
	        } /* end if (snsds) */
	    } /* end if (sncpy) */
	} /* end if (mkydname) */

	return (rs >= 0) ? tl : rs ;
}
/* end subroutine (cyi_dbfindname) */


static int cyi_dbfindone(CYI *op,time_t dt,cchar *cal,cchar *tmpfname)
{
	CYI_FMI		*mip = &op->fmi ;
	CYIHDR		*hip = &op->fhi ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("cyi_dbfindone: cal=%s\n",cal) ;
	debugprintf("cyi_dbfindone: tfn=%s\n",tmpfname) ;
#endif

	if ((rs = cyi_loadbegin(op,dt,tmpfname)) >= 0) {
	    caddr_t	md = (caddr_t) mip->mapdata ;
	    const int	mnl = MAXNAMELEN ;
	    const char	*cp ;
	    cp = (md + hip->caloff) ;
	    if (strncmp(cp,cal,mnl) != 0) rs = SR_NOMSG ;
	    if (rs < 0) {
		cyi_loadend(op) ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("cyi_dbfindone: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cyi_dbfindone) */


static int cyi_dblose(CYI *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = cyi_loadend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

	return rs ;
}
/* end subroutine (cyi_dblose) */


static int cyi_loadbegin(CYI *op,time_t dt,cchar fname[])
{
	int		rs ;
	int		fsize = 0 ;

#if	CF_DEBUGS
	debugprintf("cyi_loadbegin: fn=%s\n",fname) ;
#endif

	if ((rs = cyi_mapcreate(op,dt,fname)) >= 0) {
	    fsize = rs ;
	    rs = cyi_proc(op,dt) ;
	    if (rs < 0) {
		cyi_mapdestroy(op) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("cyi_loadbegin: ret rs=%d fsize=%u\n",rs,fsize) ;
#endif

	return (rs >= 0) ? fsize : rs ;
}
/* end subroutine (cyi_loadbegin) */


static int cyi_loadend(CYI *op)
{
	CYI_FMI		*mip ;
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = cyi_mapdestroy(op) ;
	if (rs >= 0) rs = rs1 ;

	{
	    mip = &op->fmi ;
	    mip->vt = NULL ;
	    mip->lt = NULL ;
	}

	return rs ;
}
/* end subroutine (cyi_loadend) */


static int cyi_mapcreate(CYI *op,time_t dt,cchar fname[])
{
	int		rs ;
	int		fsize = 0 ;	/* subroutine return value */

	if ((rs = u_open(fname,O_RDONLY,0666)) >= 0) {
	    struct ustat	sb ;
	    const int		fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
		fsize = (sb.st_size & INT_MAX) ;
	        if (fsize > 0) {
	    	    size_t	ms = (size_t) fsize ;
	    	    int		mp = PROT_READ ;
	    	    int		mf = MAP_SHARED ;
	    	    void	*md ;
	    	    if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
			CYI_FMI	*mip = &op->fmi ;
			mip->mapdata = md ;
	        	mip->mapsize = ms ;
	        	mip->ti_mod = sb.st_mtime ;
	        	mip->ti_map = dt ;
	    	    } /* end if (u_mmap) */
		} else
	    	    rs = SR_NOCSI ;
	    } /* end if (stat) */
	    u_close(fd) ;
	} /* end if (file) */

#if	CF_DEBUGS
	debugprintf("cyi_mapcreate: ret rs=%d fsize=%u\n",rs,fsize) ;
#endif

	return (rs >= 0) ? fsize : rs ;
}
/* end subroutine (cyi_mapcreate) */


static int cyi_mapdestroy(CYI *op)
{
	CYI_FMI		*mip = &op->fmi ;
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
/* end subroutine (cyi_mapdestroy) */


static int cyi_checkupdate(CYI *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("cyi_checkupdate: ncursors=%u\n",op->ncursors) ;
#endif

	if (op->ncursors == 0) {
	    if (dt <= 0) dt = time(NULL) ;
	    if ((dt - op->ti_lastcheck) >= TO_CHECK) {
	        struct ustat	sb ;
	        CYI_FMI		*mip = &op->fmi ;
	        op->ti_lastcheck = dt ;
	        if ((rs = u_stat(op->fname,&sb)) >= 0) {
	            f = f || (sb.st_mtime > mip->ti_mod) ;
	            f = f || (sb.st_mtime > mip->ti_map) ;
	            if (f) {
	                cyi_loadend(op) ;
	                rs = cyi_loadbegin(op,dt,op->fname) ;
	            } /* end if (update) */
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	    } /* end if (timed out) */
	} /* end if (no cursors out) */

#if	CF_DEBUGS
	debugprintf("cyi_checkupdate: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (cyi_checkupdate) */


static int cyi_proc(CYI *op,time_t dt)
{
	CYI_FMI		*mip = &op->fmi ;
	CYIHDR		*hip = &op->fhi ;
	int		rs ;

	if ((rs = cyihdr(hip,1,mip->mapdata,mip->mapsize)) >= 0) {
	    if ((rs = cyi_verify(op,dt)) >= 0) {
	        caddr_t		ma = (caddr_t) mip->mapdata ;
	        mip->vt = (uint (*)[5]) (ma + hip->vioff) ;
	        mip->lt = (uint (*)[2]) (ma + hip->vloff) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("cyi_proc: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cyi_proc) */


static int cyi_verify(CYI *op,time_t dt)
{
	CYI_FMI		*mip = &op->fmi ;
	CYIHDR		*hip = &op->fhi ;
	uint		utime = (uint) dt ;
	int		rs = SR_OK ;
	int		size ;
	int		f = TRUE ;

	f = f && (hip->fsize == mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("cyi_verify: fsize=%u f=%u\n",
		hip->fsize,f) ;
#endif

	f = f && (hip->wtime > 0) && (hip->wtime <= (utime + SHIFTINT)) ;

#if	CF_DEBUGS
	{
	char	timebuf[TIMEBUFLEN + 1] ;
	debugprintf("cyi_verify: wtime=%s f=%u\n",
		timestr_log(((time_t) hip->wtime),timebuf),f) ;
	}
#endif

	f = f && (hip->vioff <= mip->mapsize) ;
	size = hip->vilen * 5 * sizeof(uint) ;
	f = f && ((hip->vioff + size) <= mip->mapsize) ;

	f = f && (hip->vloff <= mip->mapsize) ;
	size = hip->vllen * 2 * sizeof(uint) ;
	f = f && ((hip->vloff + size) <= mip->mapsize) ;

	f = f && (hip->vilen == hip->nentries) ;

/* get out */

	if (! f)
	    rs = SR_BADFMT ;

#if	CF_DEBUGS
	debugprintf("cyi_verify: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cyi_verify) */


static int cyi_auditvt(CYI *op)
{
	CYI_FMI		*mip = &op->fmi ;
	CYIHDR		*hip = &op->fhi ;
	uint		(*vt)[5] ;
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

	    citcmpval = vt[i][3] & 0x0000FFFF ;
	    if (citcmpval < pcitcmpval) {
	        rs = SR_BADFMT ;
	        break ;
	    }
	    pcitcmpval = citcmpval ;

	} /* end for (record table entries) */

#if	CF_DEBUGS
	debugprintf("cyi_auditvt: VT rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cyi_auditvt) */


#if	CF_SEARCH

static int cyi_bsearch(op,vt,vtlen,vte)
CYI		*op ;
uint		(*vt)[5] ;
int		vtlen ;
uint		vte[5] ;
{
	uint		citekey ;
	uint		(*vtep)[5] ;
	int		rs ;
	int		vtesize = (5 * sizeof(uint)) ;
	int		vi ;

	if (op == NULL) return SR_FAULT ;

	citekey = (vte[3] & 0xffff) ;
	vtep = (uint (*)[5]) bsearch(vte,vt,vtlen,vtesize,vtecmp) ;

#if	CF_DEBUGS
	debugprintf("cyi_bsearch: bsearch() vtep=%p\n",vtep) ;
#endif

	rs = (vtep != NULL) ? (vtep - vt) : SR_NOTFOUND ;
	vi = rs ;

#if	CF_DEBUGS
	debugprintf("cyi_bsearch: rs=%d \n",rs) ;
#endif

	if (rs >= 0) {
	    while ((vi > 0) && ((vt[vi-1][3] & 0x0000FFFF) == citekey)) {
		    vi -= 1 ;
	    }
	} /* end while */

#if	CF_DEBUGS
	debugprintf("cyi_bsearch: ret rs=%d vi=%u\n",rs,vi) ;
#endif

	return (rs >= 0) ? vi : rs ;
}
/* end subroutine (cyi_bsearch) */

#else /* CF_SEARCH */

static int cyi_lsearch(op,vt,vtlen,vte)
CYI		*op ;
uint		(*vt)[5] ;
int		vtlen ;
uint		vte[5] ;
{
	uint		citekey ;
	int		rs ;
	int		vi ;

#if	CF_DEBUGS
	debugprintf("cyi_lsearch: vtlen=%u\n",vtlen) ;
#endif

	citekey = (vte[3] & 0xffff) ;
	for (vi = 0 ; vi < vtlen ; vi += 1) {

#if	CF_DEBUGS
	debugprintf("cyi_lsearch: cite%02u=%08x\n",vi,
	    (vt[vi][3] & 0x0000FFFF)) ;
#endif

	    if ((vt[vi][3] & 0x0000FFFF) == citekey)
		break ;
	}
	rs = (vi < vtlen) ? vi : SR_NOTFOUND ;

	return (rs >= 0) ? vi : rs ;
}
/* end subroutine (cyi_lsearch) */

#endif /* CF_SEARCH */


static int cyi_loadbve(op,bvep,ebuf,ebuflen,vte)
CYI		*op ;
CYI_ENT		*bvep ;
char		ebuf[] ;
int		ebuflen ;
uint		vte[5] ;
{
	CYIHDR		*hip = &op->fhi ;
	int		rs = SR_OK ;
	uint		li ;
	int		nlines ;
	int		rlen = 0 ;

	if (bvep == NULL) return SR_FAULT ;
	if (ebuf == NULL) return SR_FAULT ;
	if (vte == NULL) return SR_FAULT ;

	if (ebuflen <= 0) return SR_OVERFLOW ;

/* load the basic stuff */

	memset(bvep,0,sizeof(CYI_ENT)) ;
	bvep->voff = vte[0] ;
	bvep->vlen = vte[1] ;
	bvep->nlines = (vte[3] >> 24) & UCHAR_MAX ;
	bvep->m = (vte[3] >> 8) & UCHAR_MAX ;
	bvep->d = (vte[3] >> 0) & UCHAR_MAX ;
	bvep->hash = vte[4] ;

/* load the lines */

	li = vte[2] ;
	nlines = bvep->nlines ;

	if (li < hip->vllen) {
	    CYI_FMI	*mip = &op->fmi ;
	    const int	bo = CYI_BO((ulong) ebuf) ;
	    const int	linesize = ((nlines + 1) * sizeof(CYI_LINE)) ;

#if	CF_DEBUGS
	debugprintf("cyi_loadbve: li=%u\n",li) ;
#endif

#if	CF_DEBUGS
	    debugprintf("cyi_loadbve: nlines=%u\n",nlines) ;
	    debugprintf("cyi_loadbve: q=%u:%u\n",
		bvep->m,bvep->d) ;
#endif

	    if (linesize <= (ebuflen - bo)) {
	        CYI_LINE	*lines ;
	        caddr_t		ma = (caddr_t) mip->mapdata ;
	        uint		(*lt)[2] ;
	        int		i ;

	        lt = (uint (*)[2]) (ma + hip->vloff) ;
	        lines = (CYI_LINE *) (ebuf + bo) ;
	        bvep->lines = lines ;

	        for (i = 0 ; i < nlines ; i += 1) {
	            lines[i].loff = lt[li+i][0] ;
	            lines[i].llen = lt[li+i][1] ;
#if	CF_DEBUGS
		    debugprintf("cyi_loadbve: loff[%u]=%u\n",i,lt[li+i][0]) ;
		    debugprintf("cyi_loadbve: llen[%u]=%u\n",i,lt[li+i][1]) ;
#endif
	        } /* end for */

	        if (rs >= 0) {
	            lines[i].loff = 0 ;
	            lines[i].llen = 0 ;
	            rlen = (linesize + bo) ;
	        }

	    } else {
	        rs = SR_OVERFLOW ;
	    }
	} else {
	    rs = SR_BADFMT ;
	}

#if	CF_DEBUGS
	debugprintf("cyi_loadbve: ret rs=%d rlen=%u\n",rs,rlen) ;
#endif

	return (rs >= 0) ? rlen : rs ;
}
/* end subroutine (cyi_loadbve) */


static int mkydname(char *rbuf,cchar *dname,int year)
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;
	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,dname,-1) ;
	    i += rs ;
	}
	if (rs >= 0) {
	    rs = storebuf_char(rbuf,rlen,i,'/') ;
	    i += rs ;
	}
	if (rs >= 0) {
	    rs = storebuf_char(rbuf,rlen,i,'y') ;
	    i += rs ;
	}
	if (rs >= 0) {
	    rs = storebuf_deci(rbuf,rlen,i,year) ;
	    i += rs ;
	}
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkydname) */


static int mkcitekey(uint *cip,CYI_QUERY *bvp)
{
	uint		ci = 0 ;

	ci |= ((bvp->m & UCHAR_MAX) << 8) ;
	ci |= ((bvp->d & UCHAR_MAX) << 0) ;

	*cip = ci ;
	return SR_OK ;
}
/* end subroutine (mkcitekey) */


#if	CF_SEARCH
static int vtecmp(const void *v1p,const void *v2p)
{
	uint		*vte1 = (uint *) v1p ;
	uint		*vte2 = (uint *) v2p ;
	int		c1, c2 ;

	c1 = vte1[3] & 0x0000FFFF ;
	c2 = vte2[3] & 0x0000FFFF ;
	return (c1 - c2) ;
}
/* end subroutine (vtecmp) */
#endif /* CF_SEARCH */


#if	CF_ISOUR
static int isOurSuffix(const char *name,const char *fsuf)
{
	int		f = FALSE ;
	const char	*tp ;
	if (((tp = strchr(name,'.')) != NULL) && (strcmp((tp+1),fsuf) == 0)) {
	    f = TRUE ;
	}
	return f ;
}
/* end subroutine (isOurSuffix) */
#endif /* CF_ISOUR */


#if	CF_ISOUR
static int isNotOurFile(int rs)
{
	int	f = FALSE ;
	f = f || isNotPresent(rs) ;
	f = f || (rs == SR_NOMSG) ;
	f = f || (rs == SR_NOCSI) ;
	return f ;
}
/* end subroutine (isNotOurFile) */
#endif /* CF_ISOUR */


