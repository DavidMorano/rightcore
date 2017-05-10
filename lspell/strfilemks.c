/* strfilemks */

/* make a STRFILE database */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_FIRSTHASH	0		/* arrange for first-attempt hashing */
#define	CF_MINMOD	1		/* ensure minimum file mode */
#define	CF_LATE		0		/* late open */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a STRFILE database file.

	Synopsis:

	int strfilemks_open(op,dbname,oflags,om,n)
	STRFILEMKS	*op ;
	const char	dbname[] ;
	int		oflags ;
	mode_t		om ;
	int		n ;

	Arguments:

	op		object pointer
	dbname		name of (path-to) DB
	oflags		open-flags
	om		open-mode
	n		starting estimate of numbers of variables

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


#define	STRFILEMKS_MASTER	0


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
#include	<ctype.h>

#include	<vsystem.h>
#include	<nulstr.h>
#include	<vecobj.h>
#include	<strtab.h>
#include	<filebuf.h>
#include	<endidanstr.h>
#include	<localmisc.h>

#include	"strfilemks.h"
#include	"strlisthdr.h"


/* local defines */

#define	STRFILEMKS_SIZEMULT	4
#define	STRFILEMKS_NSKIP	5
#define	STRFILEMKS_INDPERMS	0664
#define	STRLISTMKS_FSUF		STRLISTHDR_FSUF

#define	RECMGR		struct recmgr
#define	RECMGR_ENT	struct recmgr_e
#define	MAPFILE		struct mapfile
#define	IDX		struct idx 
#define	IDX_FL		struct idx_flags

#undef	RECTAB
#define	RECTAB		struct strfilemks_rectab

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	120
#endif

#define	BUFLEN		(sizeof(STRLISTHDR) + 128)

#define	FSUF_IND	STRLISTHDR_FSUF

#define	TO_OLDFILE	(5 * 60)

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	DEBFNAME	"strfilemks.deb"


/* external subroutines */

extern uint	hashelf(const char *,int) ;
extern uint	hashagain(uint,int,int) ;
extern uint	nextpowtwo(uint) ;

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,const char *,
			const char *) ;
extern int	sncpy5(char *,int,const char *,const char *,const char *,
			const char *,const char *) ;
extern int	sncpy6(char *,int,const char *,const char *,const char *,
			const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf3(char *,const char *,const char *,const char *,
			const char *) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	strnnlen(const char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	getpwd(char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	hasuc(const char *,int) ;
extern int	vstrkeycmp(const char *,const char *) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* external variables */


/* exported variables */

STRFILEMKS_OBJ	strfilemks = {
	"strfilemks",
	sizeof(STRFILEMKS)
} ;


/* local structures */

struct recmgr {
	vecobj		recs ;
	int		bso ;		/* base string-offiset */
	int		so ;		/* string-offset */
} ;

struct recmgr_e {
	int		si ;
	int		sl ;
} ;

struct idx_flags {
	uint		fb:1 ;
}

struct idx {
	const char	*idname ;	/* idx dir-name */
	const char	*ibname ;	/* idx base-name */
	const char	*nfname ;	/* new idx file-name */
	const char	*ai ;
	STRLISTHDR	hdr ;
	IDX_FL		f ;
	FILEBUF		fb ;
	int		fd ;
	uint		fo ;
} ;

struct mapfile {
	void		*mdata ;
	size_t		msize ;
} ;

struct strentry {
	uint	khash ;
	uint	ri ;
	uint	ki ;
	uint	hi ;
} ;


/* forward references */

static int	strfilemks_recbegin(STRFILEMKS *) ;
static int	strfilemks_recend(STRFILEMKS *) ;

static int	strfilemks_idxbegin(STRFILEMKS *,const char *) ;
static int	strfilemks_idxend(STRFILEMKS *) ;

static int	strfilemks_filesbegin(STRFILEMKS *) ;
static int	strfilemks_filesend(STRFILEMKS *,int) ;

static int	strfilemks_listbegin(STRFILEMKS *,int) ;
static int	strfilemks_listend(STRFILEMKS *) ;

static int	strfilemks_nfcreate(STRFILEMKS *,const char *) ;
static int	strfilemks_nfcreatecheck(STRFILEMKS *,
			const char *,const char *) ;
static int	strfilemks_nfdestroy(STRFILEMKS *) ;
static int	strfilemks_nfstore(STRFILEMKS *,const char *) ;
static int	strfilemks_fexists(STRFILEMKS *) ;

static int	strfilemks_addfiler(STRFILEMKS *,MAPFILE *) ;

static int	strfilemks_mkvarfile(STRFILEMKS *) ;
static int	strfilemks_wrvarfile(STRFILEMKS *) ;
static int	strfilemks_mkind(STRFILEMKS *,const char *,uint (*)[3],int) ;
static int	strfilemks_renamefiles(STRFILEMKS *) ;

static int	rectab_start(RECTAB *,int) ;
static int	rectab_add(RECTAB *,uint,uint) ;
static int	rectab_done(RECTAB *) ;
static int	rectab_getvec(RECTAB *,uint (**)[2]) ;
static int	rectab_extend(RECTAB *) ;
static int	rectab_finish(RECTAB *) ;

#ifdef	COMMENT
static int	rectab_count(RECTAB *) ;
#endif

static int	mapfile_start(MAPFILE *,int,const char *,int) ;
static int	mapfile_end(MAPFILE *) ;

static int	filebuf_writefill(FILEBUF *,const char *,int) ;

static int	indinsert(uint (*rt)[2],uint (*it)[3],int,struct strentry *) ;
static int	hashindex(uint,int) ;


/* local variables */

static const char	zerobuf[4] = {
	0, 0, 0, 0 
} ;


/* exported subroutines */


int strfilemks_open(op,dbname,oflags,om,n)
STRFILEMKS	*op ;
const char	dbname[] ;
int		oflags ;
mode_t		om ;
int		n ;
{
	int	rs ;
	const char	*cp ;


#if	CF_DEBUGS && defined(DEBFNAME)
	{
	    int	dfd = debuggetfd() ;
	    nprintf(DEBFNAME,"strfilemks_open: ent dfd=%d\n",dfd) ;
	}
#endif /* DEBFNAME */

	if (op == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("strfilemks_open: ent dbname=%s\n",dbname) ;
#endif /* CF_DEBUGS */

	if (dbname == NULL) return SR_FAULT ;

	if (dbname[0] == '\0') return SR_INVALID ;

	if (n < STRFILEMKS_NENTRIES)
	    n = STRFILEMKS_NENTRIES ;

	memset(op,0,sizeof(STRFILEMKS)) ;
	op->om = om ;
	op->nfd = -1 ;
	op->gid = -1 ;
	op->pagesize = getpagesize() ;

	op->f.creat = (oflags & O_CREAT) ;
	op->f.excl = (oflags & O_EXCL) ;
	op->f.none = (! op->f.creat) && (! op->f.excl) ;

	if ((rs = strfilemks_recbegin(op)) >= 0) {
	    if ((rs = uc_mallocstrw(dbname,-1,&cp)) >= 0) {
		op->dbname = cp ;
		if ((rs = strfilemks_filesbegin(op)) >= 0) {
		    if ((rs = strfilemks_listbegin(op,n)) >= 0) {
			op->magic = STRFILEMKS_MAGIC ;
		    }
		    if (rs < 0)
			strfilemks_filesend(op,FALSE) ;
		} /* end if */
		if (rs < 0) {
	    	    uc_free(op->dbname) ;
	    	    op->dbname = NULL ;
		}
	    } /* end if (memory-allocation) */
	    if (rs < 0)
	        strfilemks_recend(op) ;
	} /* end if (recmgr) */

#if	CF_DEBUGS
	debugprintf("strfilemks_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (strfilemks_open) */


int strfilemks_close(op)
STRFILEMKS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	f_remove = TRUE ;
	int	nvars = 0 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != STRFILEMKS_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("strfilemks_close: nvars=%u\n",op->nvars) ;
#endif

	nvars = op->nvars ;
	if (! op->f.abort) {
	    rs1 = strfilemks_mkvarfile(op) ;
	    f_remove = (rs1 < 0) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("strfilemks_close: strfilemks_mkvarfile() rs=%d\n",rs) ;
#endif

	if (op->nfd >= 0) {
	    rs1 = u_close(op->nfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfd = -1 ;
	}

	rs1 = strfilemks_listend(op) ;
	if (! f_remove) f_remove = (rs1 < 0) ;
	if (rs >= 0) rs = rs1 ;

	if ((rs >= 0) && (! op->f.abort)) {
	    rs1 = strfilemks_renamefiles(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("strfilemks_close: strfilemks_renamefiles() rs=%d\n",rs) ;
#endif

	rs1 = strfilemks_filesend(op,f_remove) ;
	if (rs >= 0) rs = rs1 ;

	if (op->dbname != NULL) {
	    rs1 = uc_free(op->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("strfilemks_close: ret=%d\n",rs) ;
#endif /* CF_DEBUGS */

	op->magic = 0 ;
	return (rs >= 0) ? nvars : rs ;
}
/* end subroutine (strfilemks_close) */


int strfilemks_addfile(op,sp,sl)
STRFILEMKS	*op ;
const char	sp[] ;
int		sl ;
{
	const int	ms = STEFILEMK_MAXFILESIZE ;
	int	rs ;


	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != STRFILEMKS_MAGIC) return SR_NOTOPEN ;

	if ((rs = mapfile_start(&fm,ms,sp,sl)) >= 0) {

	    rs = strfilemks_addfiler(op,&fm) ;

	    mapfile_finish(&fm) ;
	} /* end if (mapfile) */
#if	CF_DEBUGS
	debugprintf("strfilemks_addfile: ret=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (strfilemks_addfile) */


static int strfilemks_addfiler(STRFILEMKS *op,MAPFILE *mfp)
{
	RECMGR		*rmp = op->recorder ;
	int		rs ;
	int		c = 0 ;

	if ((rs = recmgr_grpbegin(rmp)) >= 0) {
	    int		ml = mfp->msize ;
	    int		ll ;
	    const char	*tp, *lp ;
	    const char	*mp = mfp->mdata ;

	    while ((tp = strnpbrk(mp,ml,"\n#")) != NULL) {
	        lp = mp ;
	        ll = (tp - mp) ;
	        len = ((tp + 1) - mp) ;

	        if (*tp == '#') {
	            if ((tp = strnchr((tp+1),(mp+ml-(tp+1)),'\n')) != NULL)
	                len = ((tp + 1) - mp) ;
	        }

		if (ll > 0) {
		    int	mo = (mfp->msize - ml) ;
		    if ((rs = recmgr_grpadd(rmp,mo,ll)) >= 0) {
			if ((rs = idx_bufstr(op->idx,lp,ll)) >= 0) {
				c += 1 ;
			}
		    }
		}

	        mp += len ;
	        ml -= len ;
	        if (rs < 0) break ;
	    } /* end while (lines) */

	    recmgr_grpend(rmp) ;
	} /* end if (recgrp) */

	op->nstrs += c ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (strfilemks_addfiler) */


static int recmgr_start(RECMGR *rmp)
{
	const int	esize = sizeof(RECMGR_ENT) ;
	memset(rmp,0,sizeof(RECMGR)) ;
	return vecobj_start(&rmp->recs,esize,100,0) ;
}
/* end subroutine (strfilemks_start) */


static int recmgr_finish(RECMGR *rmp)
{
	return vecobj_finish(&rmp->recs) ;
}
/* end subroutine (strfilemks_finish) */


static int recmgr_grpbegin(RECMGR *rmp)
{
	rmp->bso = rmp->so ;
	return SR_OK ;
}
/* end subroutine (strfilemks_grpbegin) */


/* nothing to do */
static int recmgr_grpend(RECMGR *rmp)
{
	if (rmp == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (strfilemks_grpend) */


static int recmgr_grpadd(RECMGR *rmp,int si,int sl)
{
	RECMGR_ENT	e ;
	e.si = (si + rmp->bso) ;
	s.sl = sl ;
	rmp->so += (si+1) ;
	return vecobj_add(&rmp->recs,&e) ;
}
/* end subroutine (strfilemks_grpadd) */


int strfilemks_abort(op)
STRFILEMKS	*op ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != STRFILEMKS_MAGIC)
	    return SR_NOTOPEN ;

	op->f.abort = TRUE ;
	return SR_OK ;
}
/* end subroutine (strfilemks_abort) */


int strfilemks_chgrp(op,gid)
STRFILEMKS	*op ;
gid_t		gid ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != STRFILEMKS_MAGIC)
	    return SR_NOTOPEN ;

	op->gid = gid ;
	return SR_OK ;
}
/* end subroutine (strfilemks_chgrp) */


/* private subroutines */


static int strfilemks_recbegin(STRFILEMKS *op)
{
	const int	rsize = sizeof(RECMGR) ;
	int	rs ;
	void	*p ;
	if ((rs = uc_malloc(rsize,&p)) >= 0) {
	    RECMGR	*rmp = p ;
	    op->recorder = p ;
	    rs = recmgr_start(rmp) ;
	    if (rs < 0) {
		uc_free(op->recorder) ;
		op->recorder = NULL ;
	    }
	} /* end if (memory-allocation) */
	return rs ;
}
/* end subroutine (strfilemks_recbegin) */


static int strfilemks_recend(STRFILEMKS *op)
{
	int	rs = SR_OK ;
	int	rs1 ;

	rs1 = recmg_finish(&op->recorder) ;
	if (rs >= 0) rs = rs1 ;

	if (op->recorder != NULL) {
	    rs1 = uc_free(op->recorder) ;
	    if (rs >= 0) rs = rs1 ;
	    op->recorder = NULL ;
	}

	return rs ;
}
/* end subroutine (strfilemks_recend) */


static int strfilemks_idxbegin(STRFILEMKS *op,const char *dbname)
{
	const int	isize = sizeof(IDX) ;
	int	rs ;
	void	*p ;
	if ((rs = uc_malloc(isize,&p)) >= 0) {
	    IDX		*ixp = p ;
	    op->idx = p ;
	    rs = idx_start(op->idx,dbname) ;
	    if (rs < 0) {
		uc_free(op->idx) ;
		op->idx = NULL ;
	    }
	} /* end if (memory-allocation) */
	return rs ;
}
/* end subroutine (strfilemks_idxbegin) */


static int strfilemks_idxend(STRFILEMKS *op)
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (op->idx != NULL) {
	    rs1 = idx_finish(op->idx) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(op->idx) ;
	    if (rs >= 0) rs = rs1 ;
	    op->idx = NULL ;
	}

	return rs ;
}
/* end subroutine (strfilemks_idxend) */


static int idx_start(IDX *ixp,const char *dbname)
{
	int	rs = SR_OK ;
	int	dnl ;

	const char	*dnp ;

	memset(ixp,0,sizeof(IDX)) ;
	ixp->fd = -1 ;
	if ((dnl = sfdirname(dbname,-1,&dnp)) >= 0) {
	    int		bnl ;
	    const char	*bnp ;
	    if ((bnl = sfbasename(dbname,-1,&bnp)) > 0) {
		int	size = 0 ;
		char	*bp ;
		size += (dnl+1) ;
		size += (bnl+1) ;
		if ((rs = uc_malloc(size,&bp)) >= 0) {
		    ixp->ai = bp ;
		    op->idname = bp ;
		    bp = (strwcpy(bp,dnp,dnl) + 1) ;
		    ixp->ibname = bp ;
		    bp = (strwcpy(bp,bnp,bnl) + 1) ;
		    if ((rs = idx_dirwritable(ixp)) >= 0) {
			const char	*fsuf = STRLISTMKS_FSUF ;
		        rs = idx_create(ixp,fsuf) ;
		    }
		    if (rs < 0) {
			uc_free(idx->ai) ;
			idx->ai = NULL ;
			idx->idname = NULL ;
			idx->ibname = NULL ;
		    }
		} /* end if (memory-allocation) */
	    } else
		rs = SR_INVALID ;
	} else
	    rs = SR_BADFMT ;

	return rs ;
}
/* end subroutine (idx_start) */


static int idx_finish(IDX *ixp)
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (ixp->nfname != NULL) {
	    rs1 = uc_free(ixp->nfname) ;
	    if (rs >= 0) rs = rs1 ;
	    ixp->nfname = NULL ;
	}

	if (ixp->ai != NULL) {
	    rs1 = uc_free(ixp->ai) ;
	    if (rs >= 0) rs = rs1 ;
	    ixp->ai = NULL ;
	    idx->idname = NULL ;
	    idx->ibname = NULL ;
	}

	return rs ;
}
/* end subroutine (idx_finish) */


static int idx_dirwritable(IDX *ixp)
{
	const int	am = (X_OK|W_OK) ;
	int	rs ;
	const char	*dname = ixp->idname ;

	if (dname[0] == '\0') dname = "." ;

	rs = perm(dname,-1,-1,NULL,am) ;

	return rs ;
}
/* end subroutine (idx_dirwritable) */


static int idx_create(IDX *ixp,const char *fsuf)
{
	time_t		dt = time(NULL) ;
	const int	clen = MAXNAMELEN ;
	int		rs ;
	const char	*ibname = ixp->ibname ;
	const char	*end = ENDIANSTR ;
	char		cbuf[MAXNAMELEN+1] ;

	if ((rs = sncpy5(cbuf,clen,ibname,".",fsuf,end,"n")) >= 0) {
	    const char	*nfname = cbuf ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if (op->idname[0] != '\0') {
		rs = mkpath2(tbuf,op->idname,cbuf) ;
		nfname = tbuf ;
	    }
	    if ((rs = idx_nfopen(ixp,nfname)) == SR_EXISTS) {
		if ((rs = idx_nfold(ixp,dt,nfname)) > 0) {
	    	    rs = idx_nfopen(ixp,nfname) ;
	 	} else (rs >= 0) {
	    	    op->f.inprogress = TRUE ;
		    rs = idx_nfopentmp(ixp,fsuf) ;
		}
	    }
	    if (rs >= 0) {
		if ((rs = idx_bufbegin(ixp)) >= 0) {
		    rs = idx_creator(ixp) ;
		    if (rs < 0)
			idx_bufend(ixp) ;
		}
		if (rs < 0) {
		    idx_destroy(ixp) ;
		}
	    } /* end if (ok) */
	} /* end if (making component name) */

	return rs ;
}
/* end subroutine (idx_create) */


static int idx_creator(IDX *ixp)
{
	int	rs ;

	if ((rs = idx_mapbegin(ixp)) >= 0) {
	    rs = idx_bufhdr(ixp) ;
	}

	return rs ;
}
/* end subroutine (idx_creator) */


static int idx_destroy(IDX *ixp)
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (ixp->fd >= 0) {
	    rs1 = u_close(ixp->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    ixp->fd = -1 ;
	}

	if (ixp->nfname != NULL) {
	    if (ixp->nfname[0] != '\0') u_unlink(izp->nfname) ;
	    rs1 = uc_free(ixp->nfname) ;
	    if (rs >= 0) rs = rs1 ;
	    ixp->nfname = NULL ;
	}

	return rs ;
}
/* end subroutine (idx_destroy) */


static int idx_nfopen(IDX *ixp,const char *nfname)
{
	const int	of = (O_CREAT | O_EXCL | O_WRONLY) ;
	int	rs ;

	if ((rs = u_open(nfname,of,op->om)) >= 0) {
	    op->nfd = rs ;
	}

	return rs ;
}
/* end subroutine (idx_nfopen) */


static int idx_nfclose(IDX *ixp)
{
	int	rs = SR_OK ;
	int	rs1 ;
	if (ixp->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	}
	return rs ;
}
/* end subroutine (idx_nfclose) */


static int idx_nfold(IDX *ixp,time_t dt,const char *nfname)
{
	struct ustat	sb ;
	int	rs ;
	int	f = FALSE ;
	if ((rs = u_stat(nfname,&sb)) >= 0) {
	    if ((dt-sb.st_mtime) >= TO_OLD) {
		if (u_unlink(nfname) >= 0) f = TRUE ;
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	    f = TRUE ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (idx_nfold) */


static int idx_nfopentmp(IDX *ixp,const char *fsuf)
{
	const int	of = (O_WRONLY | O_CREAT) ;
	const int	clen = MAXNAMELEN ;
	int		rs = SR_OK ;
	const char	*fpre = "sm" ;
	const char	*xxx = "XXXXXXXX" ;
	const char	*end = ENDIANSTR ;
	char	cbuf[MAXNAMELEN + 1] ;
	if ((rs = sncpy6(cbuf,clen,fpre,xxx,".",fsuf,end,"n")) >= 0) {
	    char	infname[MAXPATHLEN + 1] ;
	    if (op->idname[0] != '\0') {
		rs = mkpath2(infname,op->idname,cbuf) ;
	    } else
		rs = mkpath1(infname,cbuf) ;
	    if (rs >= 0) {
	        char	obuf[MAXPATHLEN + 1] = { 0 } ;
		if ((rs = opentmpfile(infname,of,op->om,obuf)) >= 0) {
	                ixp->nfd = rs ;
		        rs = idx_nfstore(op,obuf) ;
		        if (rs < 0) {
		            if (obuf[0] != '\0') u_unlink(obuf) ;
			    u_close(ixp->nfd) ;
			    ixp->fd = -1 ;
		        }
		} /* end if (opentmpfile) */
	    } /* end if (ok) */
	} /* end if (making file-name) */
	return rs ;
}
/* end subroutine (idx_nfopentmp) */


static int idx_nfstore(IDX *op,const char *nf)
{
	int	rs ;

	const char	*cp ;

	if (op->nfname != NULL) {
	    uc_free(op->nfname) ;
	    op->nfname = NULL ;
	}

	rs = uc_mallocstrw(nf,-1,&cp) ;
	if (rs >= 0) op->nfname = cp ;

	return rs ;
}
/* end subroutine (idx_nfstore) */


static int idx_bufbegin(IDX *ixp)
{
	int	rs ;

	if ((rs = filebuf_start(&ixp->fb,ixp->fd,0L,0)) >= 0) {
	    ixp->f.fb = TRUE ;
	}

	return rs ;
}
/* end subroutine (idx_bufbegin) */


static int idx_bufend(IDX *ixp)
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (ixp->f.fb) {
	    ixp->f.fb = FALSE ;
	    rs1 = filebuf_finish(&ixp->fb) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (idx_bufend) */


static int idx_bufwrite(IDX *ixp,const void *wbuf,int wlen)
{
	int	rs = filebuf_write(&ixp->db,wbuf,wlen) ;
	ixp->fo += rs ;
	return rs ;
}
/* end subroutine (idx_bufwrite) */


static int idx_bufhdr(IDX *ixp)
{
	const int	hlen = sizeof(STRLISTHDR) ;
	int	rs ;

	rs = filebuf_write(&ixp->db,&ixp->hdr,hlen) ;
	ixp->fo += rs ;
	ixp->hdr.stoff = ixp->fo ;
	return rs ;
}
/* end subroutine (idx_bufhdr) */


static int idx_bufstr(IDX *ixp,const char *lp,int ll)
{
	FILEBUF	*fbp = &ixp->fb ;
	int	rs ;
	rs = filebuf_print(fbp,lp,ll) ;
	return rs ;
}
/* end subroutine (idx_bufstr) */



static int strfilemks_filesbegin(op)
STRFILEMKS	*op ;
{
	int	rs = SR_INVALID ;
	int	dnl ;

	const char	*dnp ;

	char	tmpdname[MAXPATHLEN + 1] ;


	if ((dnl = sfdirname(op->dbname,-1,&dnp)) >= 0) {
	    const char	*cp ;
	    if ((rs = uc_mallocstrw(dnp,dnl,&cp)) >= 0) {
	        op->idname = cp ;
	        if (dnl == 0) {
	            rs = getpwd(tmpdname,MAXPATHLEN) ;
	            dnl = rs ;
	        } else
	            rs = mkpath1w(tmpdname,dnp,dnl) ;
	        if (rs >= 0) {
	            int	operm = (X_OK | W_OK) ;
	            rs = perm(tmpdname,-1,-1,NULL,operm) ;
	        }
	        if (rs >= 0) {
	            if ((rs = strfilemks_nfcreate(op,FSUF_IND)) >= 0) {
	                if (op->f.creat && op->f.excl) {
	                    rs = strfilemks_fexists(op) ;
	                }
	                if (rs < 0)
		            strfilemks_nfdestroy(op) ;
	            } /* end if (nfcreate) */
	        }
	        if (rs < 0) {
		    if (op->idname != NULL) {
	    	        uc_free(op->idname) ;
	    	        op->idname = NULL ;
		    }
	        }
	    } /* end if (memory-allocation) */
	} /* end if (sfdirname) */

#if	CF_DEBUGS
	debugprintf("strfilemks_filesbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (strfilemks_filesbegin) */


static int strfilemks_filesend(op,f)
STRFILEMKS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op->nfname != NULL) {
	    if (f && (op->nfname[0] != '\0')) {
	        u_unlink(op->nfname) ;
	    }
	    rs1 = uc_free(op->nfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfname = NULL ;
	}

	if (op->idname != NULL) {
	    rs1 = uc_free(op->idname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->idname = NULL ;
	}

	return rs ;
}
/* end subroutine (strfilemks_filesend) */


/* exclusively create this new file */
static int strfilemks_nfcreate(op,fsuf)
STRFILEMKS	*op ;
const char	fsuf[] ;
{
	struct ustat	sb ;

	const int	to_old = TO_OLDFILE ;

	int	rs ;
	int	rs1 ;
	int	nfl ;
	int	oflags = (O_CREAT | O_EXCL | O_WRONLY) ;

	const char	*end = ENDIANSTR ;
	const char	*cp ;

	char	nfname[MAXPATHLEN + 1] ;


	rs = mkfnamesuf3(nfname,op->dbname,fsuf,end,"n") ;
	nfl = rs ;
	if (rs >= 0) {
	    rs = uc_mallocstrw(nfname,nfl,&cp) ;
	    if (rs >= 0) op->nfname = (char *) cp ;
	}

	if (rs < 0) goto ret0 ;

again:
	rs = u_open(op->nfname,oflags,op->om) ;
	op->nfd = rs ;

#if	CF_LATE
	if (rs >= 0) {
	    u_close(op->nfd) ;
	    op->nfd = -1 ;
	}
#endif /* CF_LATE */

	if (rs == SR_EXIST) {
	    time_t	daytime = time(NULL) ;
	    int		f_inprogress ;
	    rs1 = u_stat(op->nfname,&sb) ;
	    if ((rs1 >= 0) && ((daytime - sb.st_mtime) > to_old)) {
		u_unlink(op->nfname) ;
		goto again ;
	    }
	    op->f.inprogress = TRUE ;
	    f_inprogress = op->f.none ;
	    f_inprogress = f_inprogress || (op->f.creat && op->f.excl) ;
	    rs = (f_inprogress) ? SR_INPROGRESS : SR_OK ;
	} /* end if */

	if (rs >= 0) {
	    op->f.created = TRUE ;
	} else {
	    if (op->nfname != NULL) {
	        uc_free(op->nfname) ;
	        op->nfname = NULL ;
	    }
	}

ret0:
	return rs ;
}
/* end subroutine (txindexmks_nfcreate) */


static int strfilemks_nfcreatecheck(op,fpre,fsuf)
STRFILEMKS	*op ;
const char	fpre[] ;
const char	fsuf[] ;
{
	int	rs = SR_OK ;


#if	CF_DEBUGS
	debugprintf("strfilemks_nfcreatecheck: nfd=%d\n",op->nfd) ;
	debugprintf("strfilemks_nfcreatecheck: f_inprogress=%u\n",
		op->f.inprogress) ;
#endif

	if ((op->nfd < 0) || op->f.inprogress) {
	    int	oflags = O_WRONLY | O_CREAT ;
	    if (op->nfd >= 0) {
		u_close(op->nfd) ;
		op->nfd = -1 ;
	    }
	    if (op->f.inprogress) {
		char	cname[MAXNAMELEN + 1] ;
		char	infname[MAXPATHLEN + 1] ;
		char	outfname[MAXPATHLEN + 1] ;
		outfname[0] = '\0' ;
		rs = sncpy6(cname,MAXNAMELEN,
			fpre,"XXXXXXXX",".",fsuf,ENDIANSTR,"n") ;
		if (rs >= 0) {
		    if ((op->idname != NULL) && (op->idname[0] != '\0')) {
		        rs = mkpath2(infname,op->idname,cname) ;
		    } else
		        rs = mkpath1(infname,cname) ;
		}
		if (rs >= 0) {
		    rs = opentmpfile(infname,oflags,op->om,outfname) ;
	            op->nfd = rs ;
		    op->f.created = (rs >= 0) ;
		}
		if (rs >= 0)
		    rs = strfilemks_nfstore(op,outfname) ;
		if (rs < 0) {
		    if (outfname[0] != '\0')
			u_unlink(outfname) ;
		}
	    } else {
	        rs = u_open(op->nfname,oflags,op->om) ;
	        op->nfd = rs ;
		op->f.created = (rs >= 0) ;
	    }
	    if (rs < 0) {
		if (op->nfd >= 0) {
		    u_close(op->nfd) ;
		    op->nfd = -1 ;
		}
	        if (op->nfname != NULL) {
		    if (op->nfname[0] != '\0') {
			u_unlink(op->nfname) ;
		    }
	            uc_free(op->nfname) ;
	            op->nfname = NULL ;
		}
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (strfilemks_nfcreatecheck) */


static int strfilemks_nfdestroy(op)
STRFILEMKS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op->nfd >= 0) {
	    rs1 = u_close(op->nfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfd = -1 ;
	}

	if (op->nfname != NULL) {
	    if (op->nfname[0] != '\0') {
		rs1 = u_unlink(op->nfname) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = uc_free(op->nfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfname = NULL ;
	}

	return rs ;
}
/* end subroutine (strfilemks_nfdestroy) */


static int strfilemks_nfstore(op,outfname)
STRFILEMKS	*op ;
const char	outfname[] ;
{
	int	rs ;

	const char	*cp ;


	if (op->nfname != NULL) {
	    uc_free(op->nfname) ;
	    op->nfname = NULL ;
	}

	rs = uc_mallocstrw(outfname,-1,&cp) ;
	if (rs >= 0) op->nfname = (char *) cp ;

	return rs ;
}
/* end subroutine (strfilemks_nfstore) */


static int strfilemks_fexists(op)
STRFILEMKS	*op ;
{
	int	rs = SR_OK ;

	if (op->f.creat && op->f.excl && op->f.inprogress) {
	    const char	*suf = FSUF_IND ;
	    const char	*end = ENDIANSTR ;
	    char	hfname[MAXPATHLEN + 1] ;
	    if ((rs = mkfnamesuf2(hfname,op->dbname,suf,end)) >= 0) {
		struct ustat	sb ;
	        int	rs1 = u_stat(hfname,&sb) ;
	        if (rs1 >= 0) rs = SR_EXIST ;
	    }
	}

	return rs ;
}
/* end subroutine (strfilemks_fexists) */


static int strfilemks_listbegin(op,n)
STRFILEMKS	*op ;
int		n ;
{
	int	rs ;
	int	size ;


	size = (n * STRFILEMKS_SIZEMULT) ;
	if ((rs = strtab_start(&op->strs,size)) >= 0) {
	        rs = rectab_start(&op->rectab,n) ;
	    if (rs < 0)
		strtab_finish(&op->strs) ;
	} /* end if (strtab-keys) */

	return rs ;
}
/* end subroutine (strfilemks_listbegin) */


static int strfilemks_listend(op)
STRFILEMKS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	rs1 = rectab_finish(&op->rectab) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = strtab_finish(&op->strs) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (strfilemks_listend) */


static int strfilemks_mkvarfile(op)
STRFILEMKS	*op ;
{
	int	rs = SR_OK ;
	int	rtl ;


	rtl = rectab_done(&op->rectab) ;

	if (rtl == (op->nvars + 1)) {
	    rs = strfilemks_wrvarfile(op) ;
	} else
	    rs = SR_BUGCHECK ;

	return (rs >= 0) ? op->nvars : rs ;
}
/* end subroutine (strfilemks_mkvarfile) */


static int strfilemks_wrvarfile(op)
STRFILEMKS	*op ;
{
	STRLISTGDR	hf ;

	FILEBUF	varfile ;

	STRTAB	*ksp = &op->strs ;

	time_t	daytime = time(NULL) ;

	uint	fileoff = 0 ;

	uint	(*rt)[2] ;

	const int	pagesize = getpagesize() ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	rtl ;
	int	itl ;
	int	size ;
	int	bl ;

	char	buf[BUFLEN + 1] ;


	rtl = rectab_getvec(&op->rectab,&rt) ;

/* open (create) the STRFILE file */

	rs = strfilemks_nfcreatecheck(op,"nv",FSUF_IND) ;
	if (rs < 0)
	    goto ret0 ;

	op->f.viopen = TRUE ;
	size = (pagesize * 4) ;
	rs = filebuf_start(&varfile,op->nfd,0,size,0) ;
	if (rs < 0)
	    goto ret1 ;

/* prepare the file-header */

	memset(&hf,0,sizeof(STRLISTHDR)) ;

	hf.vetu[0] = STRLISTHDR_VERSION ;
	hf.vetu[1] = ENDIAN ;
	hf.vetu[2] = 0 ;
	hf.vetu[3] = 0 ;
	hf.wtime = (uint) daytime ;
	hf.nvars = op->nvars ;
	hf.nskip = STRFILEMKS_NSKIP ;

/* create the file-header */

	rs = strlisthdr(&hf,0,buf,BUFLEN) ;
	bl = rs ;
	if (rs < 0)
	    goto ret2 ;

/* write header */

	if (rs >= 0) {
	    rs = filebuf_writefill(&varfile,buf,bl) ;
	    fileoff += rs ;
	}

	if (rs < 0)
	    goto ret2 ;

/* write the record table */

	hf.rtoff = fileoff ;
	hf.rtlen = rtl ;

	size = (rtl + 1) * 2 * sizeof(uint) ;
	rs = filebuf_write(&varfile,rt,size) ;
	fileoff += rs ;

/* make and write out key-string table */

	if (rs >= 0) {
	    char	*kstab = NULL ;

	    size = strtab_strsize(ksp) ;

	    hf.ksoff = fileoff ;
	    hf.kslen = size ;

	    if ((rs = uc_malloc(size,&kstab)) >= 0) {

	        rs = strtab_strmk(ksp,kstab,size) ;

/* write out the key-string table */

	        if (rs >= 0) {
	            rs = filebuf_write(&varfile,kstab,size) ;
	            fileoff += rs ;
	        }

/* make and write out the record-index table */

	        if (rs >= 0) {
		    uint	(*indtab)[3] = NULL ;

	            itl = nextpowtwo(rtl) ;

	            hf.itoff = fileoff ;
	            hf.itlen = itl ;

	            size = (itl + 1) * 3 * sizeof(int) ;

	            if ((rs = uc_malloc(size,&indtab)) >= 0) {

			memset(indtab,0,size) ;

#if	CF_DEBUGS
	debugprintf("strfilemks_wrvarfile: _mkind() \n") ;
#endif

	                rs = strfilemks_mkind(op,kstab,indtab,itl) ;

	                if (rs >= 0) {
	                    rs = filebuf_write(&varfile,indtab,size) ;
	                    fileoff += rs ;
	                }

	                uc_free(indtab) ;
	            } /* end if (memory allocation) */

	        } /* end if (record-index table) */

	        uc_free(kstab) ;
	    } /* end if (memory allocation) */

	} /* end if (key-string table) */

/* write out the header -- again! */
ret2:
	filebuf_finish(&varfile) ;

	if (rs >= 0) {

	    hf.fsize = fileoff ;

	    rs = strlisthdr(&hf,0,buf,BUFLEN) ;
	    bl = rs ;
	    if (rs >= 0)
	        rs = u_pwrite(op->nfd,buf,bl,0L) ;

#if	CF_MINMOD
	if (rs >= 0)
	    rs = uc_fminmod(op->nfd,op->om) ;
#endif /* CF_MINMOD */

	    if ((rs >= 0) && (op->gid >= 0)) {
#if	CF_DEBUGS
		debugprintf("strfilemks_wrvarfile: gid=%d\n",op->gid) ;
#endif
		rs = u_fchown(op->nfd,-1,op->gid) ;
	    }

	} /* end if (succeeded?) */

/* we're out of here */
ret1:
	op->f.viopen = FALSE ;
	rs1 = u_close(op->nfd) ;
	if (rs >= 0) rs = rs1 ;
	op->nfd = -1 ;

	if ((rs < 0) && (op->nfname[0] != '\0')) {
	    u_unlink(op->nfname) ;
	    op->nfname[0] = '\0' ;
	}

ret0:
	return rs ;
}
/* end subroutine (strfilemks_wrvarfile) */


/* make an index table of the record table */
int strfilemks_mkind(op,kst,it,il)
STRFILEMKS	*op ;
const char	kst[] ;
uint		(*it)[3] ;
int		il ;
{
	struct strentry	ve ;

	uint	ri, ki, hi ;
	uint	khash ;
	uint	(*rt)[2] ;

	int	rs = SR_OK ;
	int	rtl ;
	int	sc = 0 ;

	const char	*kp ;


	rtl = rectab_getvec(&op->rectab,&rt) ;

#if	CF_DEBUGS
	debugprintf("strfilemks_mkind: rtl=%u\n",rtl) ;
#endif

#if	CF_FIRSTHASH
	{
	    struct strentry	*vep ;
	    VECOBJ	ves ;
	    int		size, opts ;

	    size = sizeof(struct strentry) ;
	    opts = VECOBJ_OCOMPACT ;
	    if ((rs = vecobj_start(&ves,size,rtl,opts)) >= 0) {

	    for (ri = 1 ; ri < rtl ; ri += 1) {

	        ki = rt[ri][0] ;
	        kp = kst + ki ;
	        khash = hashelf(kp,-1) ;

	        hi = hashindex(khash,il) ;

	        if (it[hi][0] == 0) {
	            it[hi][0] = ri ;
	            it[hi][1] = (khash & INT_MAX) ;
	            it[hi][2] = 0 ;
	            sc += 1 ;
	        } else {
		    ve.ri = ri ;
		    ve.ki = ki ;
	            ve.khash = chash ;
	            ve.hi = hi ;
	            rs = vecobj_add(&ves,&ve) ;
	        }

	        if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
		int	i ;
	        for (i = 0 ; vecobj_get(&ves,i,&vep) >= 0 ; i += 1) {
	            sc += indinsert(rt,it,il,vep) ;
	        } /* end for */
	    }

	    vecobj_finish(&ves) ;
	    } /* end if (ves) */

	}
#else /* CF_FIRSTHASH */
	{
	for (ri = 1 ; ri < rtl ; ri += 1) {

	    ki = rt[ri][0] ;
	    kp = kst + ki ;

#if	CF_DEBUGS
	debugprintf("strfilemks_mkind: ri=%u k=%s\n",ri,
		kp,strnlen(kp,20)) ;
#endif

	    khash = hashelf(kp,-1) ;

	    hi = hashindex(khash,il) ;

	    ve.ri = ri ;
	    ve.ki = ki ;
	    ve.khash = khash ;
	    ve.hi = hi ;
	    sc += indinsert(rt,it,il,&ve) ;

	} /* end for */
	}
#endif /* CF_FIRSTHASH */

	it[il][0] = UINT_MAX ;
	it[il][1] = 0 ;
	it[il][2] = 0 ;

	if (sc < 0)
	    sc = 0 ;

#if	CF_DEBUGS
	debugprintf("strfilemks_mkind: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? sc : rs ;
}
/* end subroutine (strfilemks_mkind) */


static int strfilemks_renamefiles(op)
STRFILEMKS	*op ;
{
	int	rs ;

	const char	*end = ENDIANSTR ;

	char	hashfname[MAXPATHLEN + 1] ;


	if ((rs = mkfnamesuf2(hashfname,op->dbname,FSUF_IND,end)) >= 0) {
	    if ((rs = u_rename(op->nfname,hashfname)) >= 0)
	        op->nfname[0] = '\0' ;
	    if (op->nfname[0] != '\0') {
	        u_unlink(op->nfname) ;
	        op->nfname[0] = '\0' ;
	    }
	}

	return rs ;
}
/* end subroutine (strfilemks_renamefiles) */


static int rectab_start(rtp,n)
RECTAB		*rtp ;
int		n ;
{
	int	rs = SR_OK ;
	int	size ;

	void	*p ;


	if (n < 10)
	    n = 10 ;

	rtp->i = 0 ;
	rtp->n = n ;
	size = (n + 1) * 2 * sizeof(int) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    rtp->rectab = p ;
	    rtp->rectab[0][0] = 0 ;
	    rtp->rectab[0][1] = 0 ;
	    rtp->i = 1 ;
	}

	return rs ;
}
/* end subroutine (rectab_start) */


static int rectab_finish(rtp)
RECTAB		*rtp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (rtp->rectab != NULL) {
	    rs1 = uc_free(rtp->rectab) ;
	    if (rs >= 0) rs = rs1 ;
	    rtp->rectab = NULL ;
	}

	return rs ;
}
/* end subroutine (rectab_finish) */


static int rectab_add(rtp,ki,vi)
RECTAB		*rtp ;
uint		ki, vi ;
{
	int	rs = SR_OK ;
	int	i ;


	i = rtp->i ;
	if ((i + 1) > rtp->n)
	    rs = rectab_extend(rtp) ;

	if (rs >= 0) {
	    rtp->rectab[i][0] = ki ;
	    rtp->rectab[i][1] = vi ;
	    rtp->i += 1 ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (rectab_add) */


static int rectab_extend(rtp)
RECTAB		*rtp ;
{
	int	rs = SR_OK ;

	if ((rtp->i + 1) > rtp->n) {
	    uint	(*va)[2] ;
	    int		nn ;
	    int		size ;
	    nn = (rtp->n + 1) * 2 ;
	    size = (nn + 1) * 2 * sizeof(int) ;
	    if ((rs = uc_realloc(rtp->rectab,size,&va)) >= 0) {
	        rtp->rectab = va ;
	        rtp->n = nn ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (rectab_extend) */


static int rectab_done(rtp)
RECTAB		*rtp ;
{
	int	i = rtp->i ;


	rtp->rectab[i][0] = UINT_MAX ;
	rtp->rectab[i][1] = 0 ;
	return rtp->i ;
}
/* end subroutine (rectab_done) */


#ifdef	COMMENT
static int rectab_count(rtp)
RECTAB		*rtp ;
{

	return rtp->i ;
}
/* end subroutine (rectab_count) */
#endif /* COMMENT */


static int rectab_getvec(rtp,rpp)
RECTAB		*rtp ;
uint		(**rpp)[2] ;
{

	*rpp = rtp->rectab ;
	return rtp->i ;
}
/* end subroutine (rectab_getvec) */


static int mapfile_start(MAPFILE *mfp,int max,const char *sp,int sl)
{
	NULSTR	fn ;
	int	rs ;
	const char	*fname ;

	memset(mfp,0,sizeof(MAPFILE)) ;

	if ((rs = nulstr_start(&fn,sp,sl,&fname)) >= 0) {
	    const int	of = O_RDONLY ;
	if ((rs = uc_open(fname,of,0666)) >= 0) {
	    struct ustat	db ;
	    int	fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
		if (S_ISREG(sb.st_mode)) {
		    const size_t	ps = op->pagesize ;
		    if ((max > 0) && (sb.st_size <= max)) {
	    	        size_t	ms = MAX(ps,sbp->st_size) ;
	    	        int	mp = PROT_READ ;
	    	        int	mf = MAP_SHARED ;
	    	        void	*md ;
		        if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
			    const int		madv = MADV_SEQUENTIAL ;
			    const caddr_t	ma = md ;
	    		    if ((rs = uc_madvise(ma,ms,madv)) >= 0) {
			         mfp->mdata = md ;
			         mfp->msize = ms ;
			    }
			    if (rs < 0)
				u_munmap(md,ms) ;
	    	        } /* end if (mmap) */
		    } else
	    	        rs = SR_TOOBIG ;
	        } else
	            rs = SR_PROTO ;
	    } /* end if (stat) */
	    u_close(fd) ;
	} /* end if (file-open) */
	    nulstr_finish(&fn) ;
	} /* end if (file-name) */

	return rs ;
}
/* end subroutine (mapfile_begin) */


static int mapfile_end(MAPFILE *mfp)
{
	int	rs = SR_OK ;

	if (mfp->mapdata != NULL) {
	    size_t	ms = fmp->mapsize ;
	    const void	*md = fmp->mapdata ;
	    rs = u_munmap(md,ms) ;
	    mfp->mapdata = NULL ;
	    mfp->mapsize = 0 ;
	}

	return rs ;
}
/* end subroutine (mapfile_end) */


static int filebuf_writefill(bp,wbuf,wlen)
FILEBUF		*bp ;
const char	wbuf[] ;
int		wlen ;
{
	int	rs ;
	int	r, nzero ;
	int	len ;


	if (wlen < 0)
	    wlen = (strlen(wbuf) + 1) ;

	rs = filebuf_write(bp,wbuf,wlen) ;
	len = rs ;

	r = (wlen & 3) ;
	if ((rs >= 0) && (r > 0)) {
	    nzero = (4 - r) ;
	    if (nzero > 0) {
	        rs = filebuf_write(bp,zerobuf,nzero) ;
	        len += rs ;
	    }
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (filebuf_writefill) */


static int indinsert(rt,it,il,vep)
uint		(*rt)[2] ;
uint		(*it)[3] ;
int		il ;
struct strentry	*vep ;
{
	uint	nhash, chash ;

	uint	ri, ki ;
	uint	lhi, nhi, hi ;

	int	c = 0 ;


	hi = vep->hi ;
	nhash = vep->khash ;
	chash = (nhash & INT_MAX) ;

#if	CF_DEBUGS
	debugprintf("indinsert: ve ri=%u ki=%u khash=%08X hi=%u\n",
		vep->ri,vep->ki,vep->khash,vep->hi) ;
	debugprintf("indinsert: il=%u loop 1\n",il) ;
#endif

/* CONSTCOND */
	while (TRUE) {

#if	CF_DEBUGS
	debugprintf("indinsert: it%u ri=%u nhi=%u\n",
		hi,it[hi][0],it[hi][2]) ;
#endif

	    if (it[hi][0] == 0)
		break ;

	    ri = it[hi][0] ;
	    ki = rt[ri][0] ;
	    if (ki == vep->ki)
		break ;

	    it[hi][1] |= (~ INT_MAX) ;
	    nhash = hashagain(nhash,c++,STRFILEMKS_NSKIP) ;

	    hi = hashindex(nhash,il) ;

#if	CF_DEBUGS
	debugprintf("indinsert: nhash=%08X nhi=%u\n",nhash,hi) ;
#endif

	} /* end while */

	if (it[hi][0] > 0) {

#if	CF_DEBUGS
	debugprintf("indinsert: loop 2\n") ;
#endif

	    lhi = hi ;
	    while ((nhi = it[lhi][2]) > 0)
	        lhi = nhi ;

	    hi = hashindex((lhi + 1),il) ;

#if	CF_DEBUGS
	debugprintf("indinsert: loop 3 lhi=%u\n",lhi) ;
#endif

	    while (it[hi][0] > 0)
	        hi = hashindex((hi + 1),il) ;

	    it[lhi][2] = hi ;

#if	CF_DEBUGS
	debugprintf("indinsert: loop 3 it%u ki=%u nhi=%u\n",lhi,
		it[lhi][0],hi) ;
#endif

	} /* end if (same-key continuation) */

	it[hi][0] = vep->ri ;
	it[hi][1] = chash ;
	it[hi][2] = 0 ;

#if	CF_DEBUGS
	debugprintf("indinsert: ret hi=%u c=%u\n",hi,c) ;
#endif

	return c ;
}
/* end subroutine (indinsert) */


static int hashindex(uint i,int n)
{
	int	hi = MODP2(i,n) ;
	if (hi == 0) hi = 1 ;
	return hi ;
}
/* end subroutine (hashindex) */



