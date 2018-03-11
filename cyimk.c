/* cyimk */

/* make a CYI database */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module creates a CYI database file.

	Synopsis:

	int cyimk_open(op,year,dname,cname,of,om)
	CYIMK		*op ;
	int		year ;
	const char	dname[] ;
	const char	cname[] ;
	int		of ;
	mode_t		om ;

	Arguments:

	op		object pointer
	dname		directory path
	cname		name of calendar
	of		open-flags
	om		open (create) file permissions 
	year		year

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


#define	CYIMK_MASTER	0


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<endian.h>
#include	<endianstr.h>
#include	<estrings.h>
#include	<ids.h>
#include	<storebuf.h>
#include	<filebuf.h>
#include	<char.h>
#include	<tmtime.h>
#include	<localmisc.h>

#include	"cyimk.h"
#include	"cyihdr.h"


/* local defines */

#define	CYIMK_DEFENTS	1024
#define	CYIMK_NSKIP	5

#define	HDRBUFLEN	(sizeof(CYIHDR) + 128)
#define	BUFLEN		(sizeof(CYIHDR) + 128)

#define	FSUF_IDX	"cyi"

#define	TO_OLDFILE	(5 * 60)

#define	MODP2(v,n)	((v) & ((n) - 1))


/* external subroutines */

extern uint	hashelf(const char *,int) ;

extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	filebuf_writefill(FILEBUF *,const void *,int) ;
extern int	iceil(int,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* exported variables */

CYIMK_OBJ	cyimk = {
	"cyimk",
	sizeof(CYIMK)
} ;


/* local structures */

struct bventry {
	uint		voff ;
	uint		vlen ;
	uint		li ;		/* index-number of first line-entry */
	uint		hash ;
	uint		citation ;	/* (nlines, m, d) */
} ;

struct blentry {
	uint		loff ;
	uint		llen ;
} ;


/* forward references */

static int	cyimk_idbegin(CYIMK *,cchar *,int) ;
static int	cyimk_idend(CYIMK *) ;
static int	cyimk_idxdir(CYIMK *,IDS *,cchar *) ;
static int	cyimk_minown(CYIMK *,cchar *,mode_t) ;

static int	cyimk_filesbegin(CYIMK *) ;
static int	cyimk_filesbeginc(CYIMK *) ;
static int	cyimk_filesbeginwait(CYIMK *) ;
static int	cyimk_filesbegincreate(CYIMK *,cchar *,int,mode_t) ;
static int	cyimk_filesend(CYIMK *) ;

static int	cyimk_listbegin(CYIMK *,int) ;
static int	cyimk_listend(CYIMK *) ;

static int	cyimk_mkidx(CYIMK *) ;
static int	cyimk_mkidxmain(CYIMK *,CYIHDR *) ;
static int	cyimk_mkidxhdr(CYIMK *,CYIHDR *,FILEBUF *) ;
static int	cyimk_mkidxstrs(CYIMK *,CYIHDR *,FILEBUF *,int) ;
static int	cyimk_mkidxents(CYIMK *,CYIHDR *,FILEBUF *,int) ;
static int	cyimk_mkidxlines(CYIMK *,CYIHDR *,FILEBUF *,int) ;
static int	cyimk_nidxopen(CYIMK *) ;
static int	cyimk_nidxclose(CYIMK *) ;
static int	cyimk_renamefiles(CYIMK *) ;

static int	mkydname(char *,cchar *,int) ;
static int	mkcitation(uint *,CYIMK_ENT *) ;
static int	mknewfname(char *,int,cchar *,cchar *) ;
static int	unlinkstale(cchar *,int) ;

static int	vvecmp(const void *,const void *) ;


/* local variables */


/* exported subroutines */


int cyimk_open(CYIMK *op,int year,cchar dname[],cchar cname[],int of,mode_t om)
{
	const int	n = CYIMK_DEFENTS ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (dname == NULL) return SR_FAULT ;
	if (cname == NULL) return SR_FAULT ;

	if (dname[0] == '\0') return SR_INVALID ;
	if (cname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("cyimk_open: ent\n") ;
	debugprintf("cyimk_open: dname=%s\n",dname) ;
	debugprintf("cyimk_open: cname=%s\n",cname) ;
#endif

	if (year <= 0) {
	    TMTIME	tm ;
	    time_t	dt = time(NULL) ;
	    rs = tmtime_localtime(&tm,dt) ;
	    year = (tm.year + TM_YEAR_BASE) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("cyimk_open: y=%u\n",year) ;
#endif

	memset(op,0,sizeof(CYIMK)) ;
	op->om = (om|0600) ;
	op->nfd = -1 ;
	op->year = year ;

	op->f.ofcreat = MKBOOL(of & O_CREAT) ;
	op->f.ofexcl = MKBOOL(of & O_EXCL) ;
	op->f.none = (! op->f.ofcreat) && (! op->f.ofexcl) ;

	if (rs >= 0) {
	    if ((rs = cyimk_idbegin(op,dname,year)) >= 0) {
	        cchar	*cp ;
	        if ((rs = uc_mallocstrw(cname,-1,&cp)) >= 0) {
	            op->cname = cp ;
	            if ((rs = cyimk_filesbegin(op)) >= 0) {
	                c = rs ;
	                if ((rs = cyimk_listbegin(op,n)) >= 0) {
	                    op->magic = CYIMK_MAGIC ;
	                }
	                if (rs < 0)
	                    cyimk_filesend(op) ;
	            }
	            if (rs < 0) {
	                if (op->cname != NULL) {
	                    uc_free(op->cname) ;
	                    op->cname = NULL ;
	                }
	            }
	        } /* end if (memory-allocation) */
	        if (rs < 0)
	            cyimk_idend(op) ;
	    } /* end if (cyim_idbegin) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("cyimk_open: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (cyimk_open) */


int cyimk_close(CYIMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		n = 0 ;
	int		f_go = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CYIMK_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("cyimk_close: nentries=%u\n",op->nentries) ;
#endif

	f_go = (! op->f.abort) ;
	n = op->nentries ;
	if (n > 0) {
	    if (op->f.notsorted) {
	        vecobj_sort(&op->verses,vvecmp) ;
	    }
	    rs1 = cyimk_mkidx(op) ;
	    if (rs >= 0) rs = rs1 ;
	    f_go = f_go && (rs1 >= 0) ;
	}

#if	CF_DEBUGS
	debugprintf("cyimk_close: cyimk_mkidx() rs=%d\n",rs) ;
#endif

	rs1 = cyimk_listend(op) ;
	if (rs >= 0) rs = rs1 ;
	f_go = f_go && (rs1 >= 0) ;

	if ((rs >= 0) && f_go) {
	    rs1 = cyimk_renamefiles(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("cyimk_close: cyimk_renamefiles() rs=%d\n",rs) ;
#endif

	rs1 = cyimk_filesend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->cname != NULL) {
	    rs1 = uc_free(op->cname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->cname = NULL ;
	}

	rs1 = cyimk_idend(op) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("cyimk_close: ret rs=%d n=%u\n",rs,n) ;
#endif

	op->magic = 0 ;
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (cyimk_close) */


int cyimk_add(CYIMK *op,CYIMK_ENT *bvp)
{
	uint		li = UINT_MAX ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (bvp == NULL) return SR_FAULT ;

	if (op->magic != CYIMK_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("cyimk_add: q=%u:%u\n",bvp->m,bvp->d) ;
#endif

	if ((bvp->lines != NULL) && (bvp->nlines > 0)) {
	    struct blentry	ble ;
	    const int		imax = UCHAR_MAX ;
	    int			i ;
	    for (i = 0 ; (i < bvp->nlines) && (i < imax) ; i += 1) {

	        ble.loff = bvp->lines[i].loff ;
	        ble.llen = bvp->lines[i].llen ;
	        rs = vecobj_add(&op->lines,&ble) ;
	        if (i == 0) li = rs ;

	        if (rs < 0) break ;
	    } /* end for */
	} /* end if */

	if (rs >= 0) {
	    struct bventry	bve ;
	    uint		citcmpval ;
	    bve.voff = bvp->voff ;
	    bve.vlen = bvp->vlen ;
	    bve.li = li ;
	    bve.hash = bvp->hash ;
	    mkcitation(&bve.citation,bvp) ;
	    citcmpval = (bve.citation & 0x0000FFFF) ;
	    if (citcmpval < op->pcitation) op->f.notsorted = TRUE ;
	    op->pcitation = citcmpval ;
	    rs = vecobj_add(&op->verses,&bve) ;
	    op->nentries += 1 ;
	}

#if	CF_DEBUGS && 0
	debugprintf("cyimk_add: ret=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cyimk_add) */


int cyimk_abort(CYIMK *op,int f)
{
	op->f.abort = f ;
	return SR_OK ;
}
/* end subroutine (cyimk_abort) */


/* private subroutines */


static int cyimk_idbegin(CYIMK *op,cchar *dname,int year)
{
	struct ustat	sb ;
	int		rs ;
#if	CF_DEBUGS
	debugprintf("cyimk_idbegin: dname=%s\n",dname) ;
#endif
	if ((rs = uc_stat(dname,&sb)) >= 0) {
	    IDS		id ;
	    const int	am = (W_OK|X_OK) ;
	    op->gid = sb.st_gid ;
	    op->uid = sb.st_uid ;
	    if ((rs = ids_load(&id)) >= 0) {
	        if ((rs = sperm(&id,&sb,am)) >= 0) {
	            char	ydname[MAXPATHLEN+1] ;
	            if ((rs = mkydname(ydname,dname,year)) >= 0) {
		        if ((rs = cyimk_idxdir(op,&id,ydname)) >= 0) {
	                    cchar	*cp ;
	                    if ((rs = uc_mallocstrw(ydname,-1,&cp)) >= 0) {
	                        op->idname = cp ;
	                    }
		        } /* end if (cyimk_idxdir) */
	            } /* end if (mkydname) */
		} /* end if (sperm) */
		ids_release(&id) ;
	    } /* end if (ids) */
	} /* end if */
#if	CF_DEBUGS
	debugprintf("cyimk_idbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (cyimk_idbegin) */


static int cyimk_idxdir(CYIMK *op,IDS *idp,cchar *ydname)
{
	struct ustat	sb ;
	const mode_t	dm = 0777 ;
	const int	nrs = SR_NOENT ;
	int		rs = SR_OK ;
#if	CF_DEBUGS
	debugprintf("cyimk_idxdir: ydname=%s\n",ydname) ;
#endif
	if ((rs = uc_stat(ydname,&sb)) >= 0) {
	    const int	am = (W_OK|X_OK) ;
	    uid_t	uid_yd = sb.st_uid ;
	    gid_t	gid_yd = sb.st_gid ;
	    if ((rs = sperm(idp,&sb,am)) >= 0) {
		uid_t	uid = geteuid() ;
#if	CF_DEBUGS
	debugprintf("cyimk_idxdir: perm() rs=%d\n",rs) ;
#endif
		if (uid == uid_yd) {
		    if ((uid_yd != op->uid) || (gid_yd != op->gid)) {
			rs = cyimk_minown(op,ydname,dm) ;
		    }
		}
	    } /* end if (sperm) */
	} else if (rs == nrs) {
	    if ((rs = mkdirs(ydname,dm)) >= 0) {
		rs = cyimk_minown(op,ydname,dm) ;
	    } /* end if (mkdirs) */
#if	CF_DEBUGS
	debugprintf("cyimk_idxdir: mkdirs() rs=%d\n",rs) ;
#endif
	}
#if	CF_DEBUGS
	debugprintf("cyimk_idxdir: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (cyimk_idxdir) */


static int cyimk_minown(CYIMK *op,cchar *dname,mode_t dm)
{
	int		rs ;
		if ((rs = uc_minmod(dname,dm)) >= 0) {
		    gid_t	gid = op->gid ;
		    uid_t	uid = op->uid ;
		    uc_chown(dname,uid,gid) ;
		}
	return rs ;
}
/* end subroutine (cyimk_minown) */


static int cyimk_idend(CYIMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->idname != NULL) {
	    rs1 = uc_free(op->idname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->idname = NULL ;
	}
	return rs ;
}
/* end subroutine (cyimk_idend) */


static int cyimk_filesbegin(CYIMK *op)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	if (op->f.ofcreat) {
	    rs = cyimk_filesbeginc(op) ;
	} else {
	    rs = cyimk_filesbeginwait(op) ;
	    c = rs ;
	}
#if	CF_DEBUGS
	debugprintf("cyimk_filesbegin: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (cyimk_filesbegin) */


static int cyimk_filesbeginc(CYIMK *op)
{
	const int	type = (op->f.ofcreat && (! op->f.ofexcl)) ;
	int		rs ;
	char		dbn[MAXPATHLEN+1] ;
	if ((rs = mkpath2(dbn,op->idname,op->cname)) >= 0) {
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
	        rs = cyimk_filesbegincreate(op,tfn,of,om) ;
	        if ((rs < 0) && type) {
	            uc_unlink(rbuf) ;
	        }
	    } /* end if (ok) */
	} /* end if (mknewfname) */
	} /* end if (mkpath) */
	return rs ;
}
/* end subroutine (cyimk_filesbeginc) */


static int cyimk_filesbeginwait(CYIMK *op)
{
	int		rs ;
	int		c = 0 ;
	char		dbn[MAXPATHLEN+1] ;
	if ((rs = mkpath2(dbn,op->idname,op->cname)) >= 0) {
	cchar		*suf = FSUF_IDX	 ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mknewfname(tbuf,FALSE,dbn,suf)) >= 0) {
	    const mode_t	om = op->om ;
	    const int		to_stale = CYIMK_INTSTALE ;
	    const int		nrs = SR_EXISTS ;
	    const int		of = (O_CREAT|O_WRONLY|O_EXCL) ;
	    int			to = CYIMK_INTOPEN ;
	    while ((rs = cyimk_filesbegincreate(op,tbuf,of,om)) == nrs) {
#if	CF_DEBUGS
	        debugprintf("cyimk_filesbeginwait: loop ret rs=%d\n",rs) ;
#endif
	        c = 1 ;
	        sleep(1) ;
	        unlinkstale(tbuf,to_stale) ;
	        if (to-- == 0) break ;
	    } /* end while (db exists) */
	    if (rs == nrs) {
	        op->f.ofcreat = FALSE ;
	        c = 0 ;
	        rs = cyimk_filesbeginc(op) ;
	    }
	} /* end if (mknewfname) */
	} /* end if (mkpath) */
#if	CF_DEBUGS
	debugprintf("cyimk_filesbeginwait: ret ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (cyimk_filesbeginwait) */


static int cyimk_filesbegincreate(CYIMK *op,cchar *tfn,int of,mode_t om)
{
	int		rs ;
#if	CF_DEBUGS
	{
	    char	obuf[100+1] ;
	    snopenflags(obuf,100,of) ;
	    debugprintf("cyimk_filesbegincreate: ent of=%s\n",obuf) ;
	    debugprintf("cyimk_filesbegincreate: om=%05o\n",om) ;
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
	debugprintf("cyimk_filesbegincreate: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cyimk_filesbegincreate) */


static int cyimk_filesend(CYIMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("cyimk_filesend: ent\n") ;
#endif

	if (op->nfd >= 0) {
	    rs1 = u_close(op->nfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfd = -1 ;
	}

	if (op->nidxfname != NULL) {
	    if (op->f.created && (op->nidxfname[0] != '\0')) {
	        rs1 = u_unlink(op->nidxfname) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = uc_free(op->nidxfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nidxfname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("cyimk_filesend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cyimk_filesend) */


static int cyimk_listbegin(CYIMK *op,int n)
{
	int		rs ;
	int		size ;
	int		opts ;

	opts = 0 ;
	opts |= VECOBJ_OSTATIONARY ;
	opts |= VECOBJ_OORDERED ;
	opts |= VECOBJ_OCOMPACT ;
	size = sizeof(struct bventry) ;
	if ((rs = vecobj_start(&op->verses,size,n,opts)) >= 0) {
	    rs = vecobj_start(&op->lines,size,(n * 2),opts) ;
	    if (rs < 0)
	        vecobj_finish(&op->verses) ;
	}

	return rs ;
}
/* end subroutine (cyimk_listbegin) */


static int cyimk_listend(CYIMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecobj_finish(&op->lines) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->verses) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (cyimk_listend) */


static int cyimk_mkidx(CYIMK *op)
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUGS
	debugprintf("cyimk_mkidx: ent\n") ;
#endif

	if ((rs = cyimk_nidxopen(op)) >= 0) {
	    CYIHDR	hdr ;

	    memset(&hdr,0,sizeof(CYIHDR)) ;
	    hdr.vetu[0] = CYIHDR_VERSION ;
	    hdr.vetu[1] = ENDIAN ;
	    hdr.vetu[2] = 0 ;
	    hdr.vetu[3] = 0 ;
	    hdr.wtime = (uint) time(NULL) ;
	    hdr.nentries = op->nentries ;
	    hdr.nskip = CYIMK_NSKIP ;
	    hdr.year = op->year ;

	    if ((rs = cyimk_mkidxmain(op,&hdr)) >= 0) {
	        const int	hlen = HDRBUFLEN ;
	        char		hbuf[HDRBUFLEN+1] ;
	        hdr.fsize = rs ;
	        wlen = rs ;

	        if ((rs = cyihdr(&hdr,0,hbuf,hlen)) >= 0) {
	            const int	bl = rs ;
	            if ((rs = u_pwrite(op->nfd,hbuf,bl,0L)) >= 0) {
	                const mode_t	om = op->om ;
	                rs = uc_fminmod(op->nfd,om) ;
	            }
	        }

	    } /* end if (cyimk_mkidxmain) */

	    rs1 = cyimk_nidxclose(op) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cyimk_nidx) */

#if	CF_DEBUGS
	debugprintf("cyimk_mkidx: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cyimk_mkidx) */


static int cyimk_mkidxmain(CYIMK *op,CYIHDR *hdrp)
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
	    if ((rs = cyimk_mkidxhdr(op,hdrp,hfp)) >= 0) {
	        off += rs ;
	        if (rs >= 0) {
	            rs = cyimk_mkidxstrs(op,hdrp,hfp,off) ;
	            off += rs ;
	        }
	        if (rs >= 0) {
	            rs = cyimk_mkidxents(op,hdrp,hfp,off) ;
	            off += rs ;
	        }
	        if (rs >= 0) {
	            rs = cyimk_mkidxlines(op,hdrp,hfp,off) ;
	            off += rs ;
	        }
	    } /* end if (cyimk_mkidxhdr) */
	    rs1 = filebuf_finish(hfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */
	return (rs >= 0) ? off : rs ;
}
/* end subroutine (cyimk_mkidxmain) */


/* ARGSUSED */
static int cyimk_mkidxhdr(CYIMK *op,CYIHDR *hdrp,FILEBUF *hfp)
{
	const int	hlen = HDRBUFLEN ;
	int		rs ;
	int		wlen = 0 ;
	char		hbuf[HDRBUFLEN+1] ;
	if (op == NULL) return SR_FAULT ; /* LINT */
	if ((rs = cyihdr(hdrp,0,hbuf,hlen)) >= 0) {
	    rs = filebuf_writefill(hfp,hbuf,rs) ;
	    wlen += rs ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cyimk_mkidxhdr) */


static int cyimk_mkidxstrs(CYIMK *op,CYIHDR *hdrp,FILEBUF *hfp,int off)
{
	int		rs ;
	int		wlen = 0 ;
	char		tbuf[MAXPATHLEN + 1] ;

	hdrp->diroff = off ;
	if ((rs = pathclean(tbuf,op->idname,-1)) >= 0) {
	    int	tl = rs ;
	    if ((rs = filebuf_writefill(hfp,tbuf,(tl+1))) >= 0) {
	        tl = strlen(op->cname) ;
	        off += rs ;
	        wlen += rs ;
	        hdrp->caloff = off ;
	        rs = filebuf_writefill(hfp,op->cname,(tl+1)) ;
	        off += rs ;
	        wlen += rs ;
	    } /* end if (filebuf_writefill) */
	} /* end if (pathclean) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cyimk_mkidxstrs) */


static int cyimk_mkidxents(CYIMK *op,CYIHDR *hdrp,FILEBUF *hfp,int off)
{
	struct bventry	*bvep ;
	vecobj		*elp = &op->verses ;
	uint		a[5] ;
	const int	size = (5 * sizeof(uint)) ;
	int		rs = SR_OK ;
	int		i ;
	int		n = 0 ;
	int		wlen = 0 ;

	hdrp->vioff = off ;
	for (i = 0 ; vecobj_get(elp,i,&bvep) >= 0 ; i += 1) {
	    if (bvep != NULL) {
	        a[0] = bvep->voff ;
	        a[1] = bvep->vlen ;
	        a[2] = bvep->li ;
	        a[3] = bvep->citation ;
	        a[4] = bvep->hash ;
	        n += 1 ;
	        rs = filebuf_write(hfp,a,size) ;
	        off += rs ;
	        wlen += rs ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	hdrp->vilen = n ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cyimk_mkidxents) */


static int cyimk_mkidxlines(CYIMK *op,CYIHDR *hdrp,FILEBUF *hfp,int off)
{
	struct blentry	*blep ;
	vecobj		*llp = &op->lines ;
	uint		a[2] ;
	const int	size = (2 * sizeof(uint)) ;
	int		rs = SR_OK ;
	int		n = 0 ;
	int		i ;
	int		wlen = 0 ;

	hdrp->vloff = off ;
	for (i = 0 ; vecobj_get(llp,i,&blep) >= 0 ; i += 1) {
	    if (blep != NULL) {
	        a[0] = blep->loff ;
	        a[1] = blep->llen ;
	        n += 1 ;
	        rs = filebuf_write(hfp,a,size) ;
	        off += rs ;
	        wlen += rs ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	hdrp->vllen = n ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (cyimk_mkidxlines) */


static int cyimk_nidxopen(CYIMK *op)
{
	const mode_t	om = op->om ;
	int		rs ;
	int		fd = -1 ;
	int		of = (O_CREAT|O_WRONLY) ;
#if	CF_DEBUGS
	debugprintf("cyimk_nidxopen: ent\n") ;
	debugprintf("cyimk_nidxopen: nidxfname=%s\n",op->nidxfname) ;
#endif
	if (op->nidxfname == NULL) {
	    const int	type = (op->f.ofcreat && (! op->f.ofexcl)) ;
	    cchar	*dbn = op->cname ;
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
	debugprintf("cyimk_nidxopen: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (cyimk_nidxopen) */


static int cyimk_nidxclose(CYIMK *op)
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
/* end subroutine (cyimk_nidxclose) */


static int cyimk_renamefiles(CYIMK *op)
{
	int		rs ;
	cchar		*suf = FSUF_IDX ;
	cchar		*end = ENDIANSTR ;
	char		dbn[MAXPATHLEN+1] ;
#if	CF_DEBUGS
	debugprintf("cyimk_renamefiles: ent\n") ;
	debugprintf("cyimk_renamefiles: nidxfname=%s\n",op->nidxfname) ;
#endif
	if ((rs = mkpath2(dbn,op->idname,op->cname)) >= 0) {
	    char	idxfname[MAXPATHLEN + 1] ;
	if ((rs = mkfnamesuf2(idxfname,dbn,suf,end)) >= 0) {
#if	CF_DEBUGS
	debugprintf("cyimk_renamefiles: idxfname=%s\n",idxfname) ;
#endif
	    if ((rs = u_rename(op->nidxfname,idxfname)) >= 0) {
	        op->nidxfname[0] = '\0' ;
	    } else {
	        u_unlink(op->nidxfname) ;
	        op->nidxfname[0] = '\0' ;
	    }
	} /* end if (mkfnamesuf) */
	} /* end if (mkpath) */
#if	CF_DEBUGS
	debugprintf("cyimk_renamefiles: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cyimk_renamefiles) */


static int mkcitation(uint *cip,CYIMK_ENT *bvp)
{
	uint		ci = 0 ;
	uint		nlines ;

	nlines = (bvp->lines != NULL) ? bvp->nlines : 0 ;

	ci |= ((nlines & UCHAR_MAX) << 24) ;
	ci |= ((bvp->m & UCHAR_MAX) << 8) ;
	ci |= ((bvp->d & UCHAR_MAX) << 0) ;

	*cip = ci ;
	return SR_OK ;
}
/* end subroutine (mkcitation) */


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
	        uint	vc1 = (*e1pp)->citation & 0x0000FFFF ;
	        uint	vc2 = (*e2pp)->citation & 0x0000FFFF ;
	        rc = (vc1 - vc2) ;
	    } else
	        rc = -1 ;
	} else
	    rc = 1 ;

	return rc ;
}
/* end subroutine (vvecmp) */


