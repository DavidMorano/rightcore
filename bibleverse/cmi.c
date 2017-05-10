/* cmi */

/* read or audit a ComMandment Index (CMI) database */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SEARCH	1		/* use 'bsearch(3c)' */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine opens and allows for reading or auditing of a CMI
	(ComMandment Index) database.

	Synopsis:

	int cmi_open(op,dbname)
	CMI		*op ;
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

#include	"cmi.h"
#include	"cmihdr.h"
#include	"bvcitekey.h"


/* local defines */

#define	CMI_FMI		struct cmi_fmi

#define	CMI_KA		sizeof(CMI_LINE)
#define	CMI_BO(v)	((CMI_KA - ((v) % CMI_KA)) % CMI_KA)

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
extern int	debugprintf(const char *,...) ;
extern char	*timestr_log(time_t,char *) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* exported variables */

CMI_OBJ	cmi = {
	"cmi",
	sizeof(CMI),
	sizeof(CMI_CUR)
} ;


/* local structures */


/* forward references */

static int	cmi_loadbegin(CMI *,time_t) ;
static int	cmi_loadend(CMI *) ;
static int	cmi_mapcreate(CMI *,time_t) ;
static int	cmi_mapdestroy(CMI *) ;
static int	cmi_proc(CMI *,time_t) ;
static int	cmi_verify(CMI *,time_t) ;
static int	cmi_auditvt(CMI *) ;
static int	cmi_checkupdate(CMI *,time_t) ;
static int	cmi_search(CMI *,uint) ;
static int	cmi_loadcmd(CMI *,CMI_ENT *,char *,int,int) ;

#if	CF_SEARCH
static int	vtecmp(const void *,const void *) ;
#endif

#if	CF_DEBUGS
static int	debugpresent(cchar *,const void *) ;
#endif


/* local variables */


/* exported subroutines */


int cmi_open(CMI *op,cchar *dbname)
{
	const time_t	dt = time(NULL) ;
	int		rs ;
	int		nents = 0 ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (dbname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("cmi_open: dbname=%s\n",dbname) ;
#endif
	memset(op,0,sizeof(CMI)) ;

	if ((rs = uc_mallocstrw(dbname,-1,&cp)) >= 0) {
	    const char	*es = ENDIANSTR ;
	    char	tmpfname[MAXPATHLEN + 1] ;
	    op->dbname = cp ;
	    if ((rs = mkfnamesuf2(tmpfname,op->dbname,CMI_SUF,es)) >= 0) {
	        const int	tl = rs ;
	        if ((rs = uc_mallocstrw(tmpfname,tl,&cp)) >= 0) {
	            op->fname = cp ;
#if	CF_DEBUGS
	            debugpresent("cmi_open: present{fname}=%d\n",op->fname) ;
#endif
	            if ((rs = cmi_loadbegin(op,dt)) >= 0) {
	                nents = rs ;
	                op->ti_lastcheck = dt ;
	                op->magic = CMI_MAGIC ;
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
	debugprintf("cmi_open: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? nents : rs ;
}
/* end subroutine (cmi_open) */


int cmi_close(CMI *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CMI_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("cmi_close: ent\n") ;
#endif

#if	CF_DEBUGS
	rs1 = uc_mallpresent(op->fname) ;
	if (rs >= 0) rs = rs1 ;
	debugprintf("cmi_close: 0 rs=%d\n",rs) ;
#endif

	rs1 = cmi_loadend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("cmi_close: 1 rs=%d\n",rs) ;
	debugprintf("cmi_close: fname{%p}\n",op->fname) ;
	debugprintf("cmi_close: fname=%s\n",op->fname) ;
	debugpresent("cmi_close: present{fname}=%d\n",op->fname) ;
#endif

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("cmi_close: 2 rs=%d\n",rs) ;
#endif

	if (op->dbname != NULL) {
	    rs1 = uc_free(op->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("cmi_close: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (cmi_close) */


int cmi_audit(CMI *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CMI_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	if (rs >= 0) {
	    rs = uc_mallpresent(op->fname) ;
	}
#endif

/* verify that all list pointers and list entries are valid */

	if (rs >= 0) {
	    rs = cmi_auditvt(op) ;
	}

#if	CF_DEBUGS
	debugprintf("cmi_audit: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cmi_audit) */


int cmi_count(CMI *op)
{
	CMIHDR		*hip ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CMI_MAGIC) return SR_NOTOPEN ;

	hip = &op->fhi ;
	return (rs >= 0) ? hip->nents : rs ;
}
/* end subroutine (cmi_count) */


/* this is so vital to normal operation! (no joke) */
int cmi_info(CMI *op,CMI_INFO *ip)
{
	CMIHDR		*hip ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CMI_MAGIC) return SR_NOTOPEN ;

	hip = &op->fhi ;

	if (ip != NULL) {
	    memset(ip,0,sizeof(CMI_INFO)) ;
	    ip->idxmtime = op->fmi.ti_mod ;
	    ip->idxctime = (time_t) hip->idxtime ;
	    ip->dbtime = (time_t) hip->dbtime ;
	    ip->dbsize = (size_t) hip->dbsize ;
	    ip->idxsize = (size_t) hip->idxsize ;
	    ip->nents = hip->nents ;
	    ip->maxent = hip->maxent ;
	}

	return (rs >= 0) ? hip->nents : rs ;
}
/* end subroutine (cmi_info) */


int cmi_read(CMI *op,CMI_ENT *bvep,char *vbuf,int vlen,uint cn)
{
	int		rs = SR_OK ;
	int		vi = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (bvep == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;

	if (op->magic != CMI_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("cmi_get: ent cn=%u\n",cn) ;
#endif

/* check for update */

	if ((rs >= 0) && (op->ncursors == 0)) {
	    rs = cmi_checkupdate(op,0) ;
	}
	if (rs >= 0) {
	    if ((rs = cmi_search(op,cn)) >= 0) {
	        vi = rs ;
	        rs = cmi_loadcmd(op,bvep,vbuf,vlen,vi) ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("cmi_get: ret rs=%d vi=%u\n",rs,vi) ;
#endif

	return (rs >= 0) ? vi : rs ;
}
/* end subroutine (cmi_read) */


int cmi_curbegin(CMI *op,CMI_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != CMI_MAGIC) return SR_NOTOPEN ;

	curp->i = 0 ;
	op->ncursors += 1 ;

	return SR_OK ;
}
/* end subroutine (cmi_curbegin) */


int cmi_curend(CMI *op,CMI_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != CMI_MAGIC) return SR_NOTOPEN ;

	curp->i = 0 ;
	if (op->ncursors > 0) {
	    op->ncursors -= 1 ;
	}

	return SR_OK ;
}
/* end subroutine (cmi_curend) */


int cmi_enum(CMI *op,CMI_CUR *curp,CMI_ENT *bvep,char *vbuf,int vlen)
{
	CMIHDR		*hip ;
	int		rs = SR_OK ;
	int		vi ;
	int		nlines = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (bvep == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;

	if (op->magic != CMI_MAGIC) return SR_NOTOPEN ;

	if (op->ncursors == 0) return SR_INVALID ;

	vi = (curp->i < 0) ? 0 : (curp->i + 1) ;
	hip = &op->fhi ;

#if	CF_DEBUGS
	debugprintf("cmi_enum: ent vi=%d\n",vi) ;
	debugprintf("cmi_enum: vilen=%d\n",hip->vilen) ;
	debugprintf("cmi_enum: vllen=%d\n",hip->vllen) ;
#endif

	if (vi < hip->vilen) {
	    if ((rs = cmi_loadcmd(op,bvep,vbuf,vlen,vi)) >= 0) {
		nlines = rs ;
	        curp->i = vi ;
	    }
	} else {
	    rs = SR_NOTFOUND ;
	}

#if	CF_DEBUGS
	debugprintf("cmi_enum: ret rs=%d nl=%u\n",rs,nlines) ;
#endif

	return (rs >= 0) ? nlines : rs ;
}
/* end subroutine (cmi_enum) */


/* private subroutines */


static int cmi_loadbegin(CMI *op,time_t dt)
{
	int		rs ;
	int		nents = 0 ;

	if ((rs = cmi_mapcreate(op,dt)) >= 0) {
	    rs = cmi_proc(op,dt) ;
	    nents = rs ;
	    if (rs < 0)
	        cmi_mapdestroy(op) ;
	} /* end if */

	return (rs >= 0) ? nents : rs ;
}
/* end subroutine (cmi_loadbegin) */


static int cmi_loadend(CMI *op)
{
	CMI_FMI		*mip ;
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = cmi_mapdestroy(op) ;
	if (rs >= 0) rs = rs1 ;

	mip = &op->fmi ;
	mip->vt = NULL ;
	mip->lt = NULL ;
	return rs ;
}
/* end subroutine (cmi_loadend) */


static int cmi_mapcreate(CMI *op,time_t dt)
{
	CMI_FMI		*mip = &op->fmi ;
	int		rs ;

	if (op->fname == NULL) return SR_BUGCHECK ;

	if ((rs = u_open(op->fname,O_RDONLY,0666)) >= 0) {
	    struct ustat	sb ;
	    int			fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
	        size_t	fsize = (sb.st_size & UINT_MAX) ;
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
	        } else {
	            rs = SR_UNATCH ;
	        }
	    } /* end if (stat) */
	    u_close(fd) ;
	} /* end if (open) */

	return rs ;
}
/* end subroutine (cmi_mapcreate) */


static int cmi_mapdestroy(CMI *op)
{
	CMI_FMI		*mip = &op->fmi ;
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
/* end subroutine (cmi_mapdestroy) */


static int cmi_checkupdate(CMI *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (op->ncursors == 0) {
	    if (dt <= 0) dt = time(NULL) ;
	    if ((dt - op->ti_lastcheck) >= TO_CHECK) {
	        struct ustat	sb ;
	        CMI_FMI		*mip = &op->fmi ;
	        op->ti_lastcheck = dt ;
	        if ((rs = u_stat(op->fname,&sb)) >= 0) {
	            f = f || (sb.st_mtime > mip->ti_mod) ;
	            f = f || (sb.st_mtime > mip->ti_map) ;
	            if (f) {
	                cmi_loadend(op) ;
	                rs = cmi_loadbegin(op,dt) ;
	            } /* end if (update) */
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        }
	    } /* end if (time-checked out) */
	} /* end if (no cursors out) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (cmi_checkupdate) */


static int cmi_proc(CMI *op,time_t dt)
{
	CMI_FMI		*mip = &op->fmi ;
	CMIHDR		*hip = &op->fhi ;
	int		rs ;
	int		nents = 0 ;

	if ((rs = cmihdr(hip,1,mip->mapdata,mip->mapsize)) >= 0) {
	    if ((rs = cmi_verify(op,dt)) >= 0) {
	        mip->vt = (uint (*)[4]) (mip->mapdata + hip->vioff) ;
	        mip->lt = (uint (*)[2]) (mip->mapdata + hip->vloff) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("cmi_proc: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? nents : rs ;
}
/* end subroutine (cmi_proc) */


static int cmi_verify(CMI *op,time_t dt)
{
	CMI_FMI		*mip = &op->fmi ;
	CMIHDR		*hip = &op->fhi ;
	int		rs = SR_OK ;
	int		size ;
	int		f = TRUE ;

	f = f && (hip->idxsize == mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("cmi_verify: fsize=%u ms=%u f=%u\n",
	    hip->idxsize,mip->mapsize,f) ;
#endif

#if	CF_DEBUGS
	{
	    const uint	utime = (uint) dt ;
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("cmi_verify: utime=%s sh=%u\n",
	        timestr_log(((time_t) utime),timebuf),SHIFTINT) ;
	}
#endif

	f = f && (hip->idxtime > 0) ;
	if (f) {
	    time_t	tt = (time_t) hip->idxtime ;
	    f = (dt >= tt) ;
	}

#ifdef	COMMENT
	{
	    const uint	utime = (uint) dt ;
	    f = f && (hip->idxtime <= (utime + SHIFTINT)) ;
	}
#endif

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("cmi_verify: wtime=%s f=%u\n",
	        timestr_log(((time_t) hip->idxtime),timebuf),f) ;
	}
#endif

/* alignment restriction */

#if	CF_DEBUGS
	debugprintf("cmi_verify: vioff=%u\n",hip->vioff) ;
#endif

	f = f && ((hip->vioff & (sizeof(int)-1)) == 0) ;

#if	CF_DEBUGS
	debugprintf("cmi_verify: 1 f=%d\n",f) ;
#endif

/* size restrictions */

	f = f && (hip->vioff <= mip->mapsize) ;
	size = hip->vilen * 4 * sizeof(uint) ;
	f = f && ((hip->vioff + size) <= mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("cmi_verify: 2 f=%d\n",f) ;
#endif

/* alignment restriction */

#if	CF_DEBUGS
	debugprintf("cmi_verify: vloff=%u\n",hip->vloff) ;
#endif

	f = f && ((hip->vloff & (sizeof(int)-1)) == 0) ;

#if	CF_DEBUGS
	debugprintf("cmi_verify: 3 f=%d\n",f) ;
#endif

/* size restrictions */

	f = f && (hip->vloff <= mip->mapsize) ;
	size = (hip->vllen * 2 * sizeof(uint)) ;
	f = f && ((hip->vloff + size) <= mip->mapsize) ;

#if	CF_DEBUGS
	debugprintf("cmi_verify: 4 f=%d\n",f) ;
#endif

/* size restrictions */
	f = f && (hip->vilen == hip->nents) ;

#if	CF_DEBUGS
	debugprintf("cmi_verify: 5 f=%d\n",f) ;
#endif

/* get out */

	if (! f)
	    rs = SR_BADFMT ;

#if	CF_DEBUGS
	debugprintf("cmi_verify: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cmi_verify) */


static int cmi_auditvt(CMI *op)
{
	CMI_FMI		*mip = &op->fmi ;
	CMIHDR		*hip = &op->fhi ;
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
	debugprintf("cmi_auditvt: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cmi_auditvt) */


static int cmi_search(CMI *op,uint cn)
{
	CMI_FMI		*mip = &op->fmi ;
	CMIHDR		*hip = &op->fhi ;
	uint		(*vt)[4] ;
	uint		vte[4] ;
	uint		citekey = (uint) (cn & USHORT_MAX) ;
	int		rs = SR_OK ;
	int		vtlen ;
	int		vi = 0 ;

#if	CF_DEBUGS
	debugprintf("cmi_search: ent cn=%u\n",cn) ;
#endif

	vt = mip->vt ;
	vtlen = hip->vilen ;

/* search for entry */

#if	CF_DEBUGS
	debugprintf("cmi_search: vtlen=%u\n",vtlen) ;
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
	        const ushort	vkey = ((vt[vi][3] >> 16) & USHORT_MAX) ;
	        if (vkey == citekey) break ;
	    }
	    rs = (vi < vtlen) ? vi : SR_NOTFOUND ;
	}
#endif /* CF_SEARCH */

#if	CF_DEBUGS
	debugprintf("cmi_search: ret rs=%d vi=%u\n",rs,vi) ;
#endif

	return (rs >= 0) ? vi : rs ;
}
/* end subroutine (cmi_search) */


static int cmi_loadcmd(CMI *op,CMI_ENT *bvep,char *ebuf,int elen,int vi)
{
	CMI_LINE	*lines ;
	CMI_FMI		*mip ;
	CMIHDR		*hip ;
	ulong		uebuf = (ulong) ebuf ;
	uint		*vte ;
	uint		(*lt)[2] ;
	uint		li ;
	int		rs = SR_OK ;
	int		bo, i ;
	int		linesize ;
	int		nlines ;

	if (bvep == NULL) return SR_FAULT ;
	if (ebuf == NULL) return SR_FAULT ;

	if (elen <= 0) return SR_OVERFLOW ;

#if	CF_DEBUGS
	    debugprintf("cmi_loadcmd: ent vi=%d\n",vi) ;
#endif

	mip = &op->fmi ;
	hip = &op->fhi ;

	vte = mip->vt[vi] ;

/* load the basic stuff */

	memset(bvep,0,sizeof(CMI_ENT)) ;
	bvep->eoff = vte[0] ;
	bvep->elen = vte[1] ;
	bvep->nlines = ((vte[3] >> 16) & USHORT_MAX) ;
	bvep->cn = ((vte[3] >> 0) & USHORT_MAX) ;

#if	CF_DEBUGS
	    debugprintf("cmi_loadcmd: cn=%u\n",bvep->cn) ;
#endif

/* load the lines */

	li = vte[2] ;
	nlines = bvep->nlines ;

#if	CF_DEBUGS
	    debugprintf("cmi_loadcmd: mid1 li=%u nlines=%u\n",li,nlines) ;
	    debugprintf("cmi_loadcmd: vllen=%u\n",hip->vllen) ;
#endif

	if (li < hip->vllen) {

#if	CF_DEBUGS
	    debugprintf("cmi_loadcmd: li=%u\n",li) ;
#endif

	    bo = CMI_BO(uebuf) ;

#if	CF_DEBUGS
	    debugprintf("cmi_loadcmd: nlines=%u\n",nlines) ;
	    debugprintf("cmi_loadcmd: cn=%u\n",bvep->cn) ;
#endif

	    linesize = ((nlines + 1) * sizeof(CMI_LINE)) ;
	    if (linesize <= (elen - (bo-uebuf))) {

	        lt = (uint (*)[2]) (mip->mapdata + hip->vloff) ;
	        lines = (CMI_LINE *) (uebuf + bo) ;
	        bvep->lines = lines ;

	        for (i = 0 ; i < nlines ; i += 1) {
	            lines[i].loff = lt[li+i][0] ;
	            lines[i].llen = lt[li+i][1] ;
	        } /* end for */

	        if (rs >= 0) {
	            lines[i].loff = 0 ;
	            lines[i].llen = 0 ;
	        }

	    } else
	        rs = SR_OVERFLOW ;

	} else
	    rs = SR_BADFMT ;

#if	CF_DEBUGS
	debugprintf("cmi_loadcmd: ret rs=%d nl=%u\n",rs,nlines) ;
#endif

	return (rs >= 0) ? nlines : rs ;
}
/* end subroutine (cmi_loadcmd) */


#if	CF_SEARCH
static int vtecmp(const void *v1p,const void *v2p)
{
	uint		*vte1 = (uint *) v1p ;
	uint		*vte2 = (uint *) v2p ;
	uint		cn1, cn2 ;
	cn1 = ((vte1[3] >> 0) & USHORT_MAX) ;
	cn2 = ((vte2[3] >> 0) & USHORT_MAX) ;
	return (cn1 - cn2) ;
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


