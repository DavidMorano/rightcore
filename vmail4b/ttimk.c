/* ttimk */

/* make a Termianl-Translate-Index file management database */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CH_MINMOD	1		/* ensure minimum file mode */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module creates a TTI database file.

	Synopsis:

	int ttimk_open(op,dbname,...)
	TTIMK		*op ;
	const char	dbname[] ;

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


#define	TTIMK_MASTER	0


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
#include	<vecobj.h>
#include	<filebuf.h>
#include	<char.h>
#include	<endianstr.h>
#include	<localmisc.h>

#include	"ttimk.h"
#include	"bvifu.h"


/* local defines */

#define	TTIMK_NENTRIES	(19 * 1024)
#define	TTIMK_NSKIP	5

#define	BUFLEN		(sizeof(TTIFU) + 128)

#define	FSUF_IND	"tti"

#define	TO_OLDFILE	(5 * 60)


/* external subroutines */

extern uint	uceil(uint,int) ;

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
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	getpwd(char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	filebuf_writefill(FILEBUF *,const void *,int) ;
extern int	iceil(int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* exported variables */

TTIMK_OBJ	ttimk = {
	"ttimk",
	sizeof(TTIMK)
} ;


/* local structures */

struct bventry {
	uint		voff ;
	uint		vlen ;
	uint		li ;		/* index-number of first line-entry */
	uint		citation ;	/* (nlines, b, c, v) */
} ;

struct blentry {
	uint		loff ;
	uint		llen ;
} ;


/* forward references */

static int	ttimk_filesbegin(TTIMK *) ;
static int	ttimk_filesend(TTIMK *,int) ;
static int	ttimk_listbegin(TTIMK *,int) ;
static int	ttimk_listend(TTIMK *) ;
static int	ttimk_nfcreate(TTIMK *,const char *) ;
static int	ttimk_nfcreatecheck(TTIMK *,const char *,const char *) ;
static int	ttimk_nfdestroy(TTIMK *) ;
static int	ttimk_fexists(TTIMK *) ;
static int	ttimk_nfstore(TTIMK *,const char *) ;
static int	ttimk_mkbvi(TTIMK *) ;
static int	ttimk_renamefiles(TTIMK *) ;

static int	mkcitation(uint *,TTIMK_VERSE *) ;

static int	vvecmp(void *,void *) ;


/* local variables */

static const char	zerobuf[4] = {
	0, 0, 0, 0 
} ;


/* exported subroutines */


int ttimk_open(op,dbname,oflags,operms)
TTIMK		*op ;
const char	dbname[] ;
int		oflags ;
int		operms ;
{
	int		rs = SR_OK ;
	int		n = TTIMK_NENTRIES ;

	if (op == NULL) return SR_FAULT ;
	if (dbname == NULL) return SR_FAULT ;

	if (dbname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("ttimk_open: dbname=%s\n",dbname) ;
#endif /* CF_DEBUGS */

	memset(op,0,sizeof(TTIMK)) ;
	op->operms = operms ;
	op->nfd = -1 ;

	op->f.creat = (oflags & O_CREAT) ;
	op->f.excl = (oflags & O_EXCL) ;
	op->f.none = (! op->f.creat) && (! op->f.excl) ;

	rs = uc_mallocstrw(dbname,-1,&op->dbname) ;
	if (rs < 0)
	    goto bad0 ;

	rs = ttimk_filesbegin(op) ;
	if (rs < 0)
	    goto bad1 ;

/* initialize the data structures we need */

	rs = ttimk_listbegin(op,n) ;
	if (rs < 0)
	    goto bad2 ;

	op->magic = TTIMK_MAGIC ;

ret0:

#if	CF_DEBUGS
	debugprintf("ttimk_open: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad2:
	ttimk_filesend(op,FALSE) ;

bad1:
	if (op->dbname != NULL) {
	    uc_free(op->dbname) ;
	    op->dbname = NULL ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (ttimk_open) */


int ttimk_close(op)
TTIMK		*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		nverses = 0 ;
	int		f_remove = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TTIMK_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("ttimk_close: nverses=%u\n",op->nverses) ;
#endif

	if (op->f.notsorted)
	    vecobj_sort(&op->verses,vvecmp) ;

	nverses = op->nverses ;
	if (nverses > 0) {
	    rs1 = ttimk_mkbvi(op) ;
	    f_remove = (rs1 < 0) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("ttimk_close: ttimk_mkbvi() rs=%d\n",rs) ;
#endif

	if (op->nfd >= 0) {
	    u_close(op->nfd) ;
	    op->nfd = -1 ;
	}

	rs1 = ttimk_listend(op) ;
	if (! f_remove) f_remove = (rs1 < 0) ;
	if (rs >= 0) rs = rs1 ;

	if ((rs >= 0) && (nverses > 0))
	    rs = ttimk_renamefiles(op) ;

#if	CF_DEBUGS
	debugprintf("ttimk_close: ttimk_renamefiles() rs=%d\n",rs) ;
#endif

	rs1 = ttimk_filesend(op,f_remove) ;
	if (rs >= 0) rs = rs1 ;

	if (op->dbname != NULL) {
	    uc_free(op->dbname) ;
	    op->dbname = NULL ;
	}

	op->magic = 0 ;
	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (ttimk_close) */


int ttimk_add(op,bvp)
TTIMK		*op ;
TTIMK_VERSE	*bvp ;
{
	struct bventry	bve ;
	struct blentry	ble ;
	uint		li = UINT_MAX ;
	uint		citcmpval ;
	uint		v ;
	int		rs = SR_OK ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (bvp == NULL) return SR_FAULT ;

	if (op->magic != TTIMK_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("ttimk_add: q=%u:%u:%u\n",
		bvp->b,bvp->c,bvp->v) ;
#endif

	if ((bvp->lines != NULL) && (bvp->nlines > 0)) {
	    for (i = 0 ; i < bvp->nlines ; i += 1) {

		ble.loff = bvp->lines[i].loff ;
		ble.llen = bvp->lines[i].llen ;
		rs = vecobj_add(&op->lines,&ble) ;
		if (rs < 0) break ;

		if (i == 0) li = rs ;

	    } /* end for */
	} /* end if */

	if (rs < 0)
	    goto bad1 ;

#if	CF_DEBUGS
	debugprintf("ttimk_add: li=%u\n",li) ;
#endif

	bve.voff = bvp->voff ;
	bve.vlen = bvp->vlen ;
	bve.li = li ;
	mkcitation(&bve.citation,bvp) ;

	citcmpval = (bve.citation & 0x00FFFFFF) ;
	if (citcmpval < op->pcitation)
	    op->f.notsorted = TRUE ;

	op->pcitation = citcmpval ;

	rs = vecobj_add(&op->verses,&bve) ;
	if (rs < 0)
	    goto bad2 ;

	op->nverses += 1 ;
	if ((bvp->b > 0) && (bvp->c > 0) && (bvp->v > 0))
	    op->nzverses += 1 ;

	v = bvp->b ;
	if (v > op->maxbook) op->maxbook = v ;

	v = bvp->c ;
	if (v > op->maxchapter) op->maxchapter = v ;

	v = bvp->v ;
	if (v > op->maxverse) op->maxverse = v ;

ret0:

#if	CF_DEBUGS && 0
	debugprintf("ttimk_add: ret=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad2:
bad1:
bad0:
	goto ret0 ;
}
/* end subroutine (ttimk_add) */


int ttimk_info(op,bip)
TTIMK		*op ;
TTIMK_INFO	*bip ;
{
	int		rs = SR_OK ;
	int		nverses = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (bip == NULL) return SR_FAULT ;

	if (op->magic != TTIMK_MAGIC) return SR_NOTOPEN ;

	if (bip != NULL) {
		bip->maxbook = op->maxbook ;
		bip->maxchapter = op->maxchapter ;
		bip->maxverse = op->maxverse ;
		bip->nverses = op->nverses ;
		bip->nzverses = op->nzverses ;
	}

	return (rs >= 0) ? nverses : rs ;
}
/* end subroutine (ttimk_info) */


/* private subroutines */


static int ttimk_filesbegin(op)
TTIMK		*op ;
{
	int		rs = SR_INVALID ;
	int		dnl ;
	const char	*dnp ;
	char		tmpdname[MAXPATHLEN + 1] ;

/* check that the parent directory is writable to us */

	dnl = sfdirname(op->dbname,-1,&dnp) ;

#if	CF_DEBUGS
	debugprintf("ttimk_filesbegin: sfdirname() rs=%d\n",dnl) ;
#endif

	rs = uc_mallocstrw(dnp,dnl,&op->idname) ;
	if (rs < 0)
	    goto bad0 ;

	if (dnl == 0) {
	    rs = getpwd(tmpdname,MAXPATHLEN) ;
	    dnl = rs ;
	} else
	    rs = mkpath1w(tmpdname,dnp,dnl) ;

	if (rs >= 0) {
	    int	operm = (X_OK | W_OK | R_OK) ;
	    rs = perm(tmpdname,-1,-1,NULL,operm) ;
	}

	if (rs < 0)
	    goto bad1 ;

/* make the filename of the new hash file */

	rs = ttimk_nfcreate(op,FSUF_IND) ;
	if (rs < 0)
	    goto bad2 ;

	if (op->f.creat && op->f.excl)
	    rs = ttimk_fexists(op) ;

	if (rs < 0)
	    goto bad3 ;

ret0:

#if	CF_DEBUGS
	debugprintf("ttimk_filesbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad3:
	ttimk_nfdestroy(op) ;

bad2:
bad1:
	if (op->idname != NULL) {
	    uc_free(op->idname) ;
	    op->idname = NULL ;
	}
bad0:
	goto ret0 ;
}
/* end subroutine (ttimk_filesbegin) */


static int ttimk_filesend(op,f)
TTIMK		*op ;
{
	int		rs = SR_OK ;

	if (op->nfname != NULL) {
	    if (op->f.created && (op->nfname[0] != '\0')) {
	        u_unlink(op->nfname) ;
	    }
	    uc_free(op->nfname) ;
	    op->nfname = NULL ;
	}

	if (op->idname != NULL) {
	    uc_free(op->idname) ;
	    op->idname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("ttimk_filesend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ttimk_filesend) */


static int ttimk_listbegin(op,n)
TTIMK		*op ;
int		n ;
{
	int		rs ;
	int		size ;
	int		opts ;

	opts = 0 ;
	opts |= VECOBJ_OCOMPACT ;
	opts |= VECOBJ_OORDERED ;
	opts |= VECOBJ_OSTATIONARY ;
	size = sizeof(struct bventry) ;
	rs = vecobj_start(&op->verses,size,n,opts) ;
	if (rs < 0)
	    goto bad0 ;

	rs = vecobj_start(&op->lines,size,(n * 2),opts) ;
	if (rs < 0)
	    goto bad1 ;

ret0:
	return rs ;

/* bad stuff */
bad2:
bad1:
	vecobj_finish(&op->verses) ;

bad0:
	goto ret0 ;
}
/* end subroutine (ttimk_listbegin) */


static int ttimk_listend(op)
TTIMK		*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecobj_finish(&op->lines) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->verses) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (ttimk_listend) */


static int ttimk_mkbvi(op)
TTIMK		*op ;
{
	struct bventry	*bvep ;
	struct blentry	*blep ;
	TTIFU		hf ;
	FILEBUF		bvifile ;
	time_t		daytime = time(NULL) ;
	uint		fileoff = 0 ;
	uint		a[4] ;
	const int	pagesize = getpagesize() ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		size ;
	int		n, i ;
	int		bl ;
	char		buf[BUFLEN + 1] ;

/* open (create) the hash file */

	rs = ttimk_nfcreatecheck(op,"nb",FSUF_IND) ;
	if (rs < 0)
	    goto ret0 ;

	size = (pagesize * 4) ;
	rs = filebuf_start(&bvifile,op->nfd,0,size,0) ;
	if (rs < 0)
	    goto ret1 ;

/* prepare the file-header */

	memset(&hf,0,sizeof(TTIFU)) ;

	hf.vetu[0] = TTIFU_VERSION ;
	hf.vetu[1] = ENDIAN ;
	hf.vetu[2] = 0 ;
	hf.vetu[3] = 0 ;
	hf.wtime = (uint) daytime ;
	hf.nverses = op->nverses ;
	hf.nzverses = op->nzverses ;
	hf.maxbook = op->maxbook ;
	hf.maxchapter = op->maxchapter ;

/* create the file-header */

	rs = bvifu(&hf,0,buf,BUFLEN) ;
	bl = rs ;
	if (rs < 0)
	    goto ret2 ;

/* write header */

	if (rs >= 0) {
	    rs = filebuf_writefill(&bvifile,buf,bl) ;
	    fileoff += rs ;
	}

	if (rs < 0)
	    goto ret2 ;

/* write the "verses" table */

	hf.vioff = fileoff ;
	size = 4 * sizeof(uint) ;
	n = 0 ;
	for (i = 0 ; vecobj_get(&op->verses,i,&bvep) >= 0 ; i += 1) {
	    if (bvep == NULL) continue ;

	    a[0] = bvep->voff ;
	    a[1] = bvep->vlen ;
	    a[2] = bvep->li ;
	    a[3] = bvep->citation ;

#if	CF_DEBUGS
	debugprintf("ttimk_mkbvi: citation=%08X\n",a[3]) ;
#endif

	    n += 1 ;
	    rs = filebuf_write(&bvifile,a,size) ;
	    fileoff += rs ;
	    if (rs < 0)
		break ;

	} /* end for */

	hf.vilen = n ;
	if (rs < 0)
	    goto ret2 ;

/* write the "lines" table */

	hf.vloff = fileoff ;
	size = 2 * sizeof(uint) ;
	n = 0 ;
	for (i = 0 ; vecobj_get(&op->lines,i,&blep) >= 0 ; i += 1) {
	    if (blep == NULL) continue ;

	    a[0] = blep->loff ;
	    a[1] = blep->llen ;
	    n += 1 ;
	    rs = filebuf_write(&bvifile,a,size) ;
	    fileoff += rs ;
	    if (rs < 0)
		break ;

	} /* end for */

	hf.vllen = n ;

/* write out the header -- again! */
ret2:
	filebuf_finish(&bvifile) ;

	if (rs >= 0) {

	    hf.fsize = fileoff ;

	    rs = bvifu(&hf,0,buf,BUFLEN) ;
	    bl = rs ;
	    if (rs >= 0)
	        rs = u_pwrite(op->nfd,buf,bl,0L) ;

#if	CF_MINMOD
	    if (rs >= 0)
	        rs = uc_fminmod(op->nfd,op->operms) ;
#endif

	} /* end if */

/* we're out of here */
ret1:
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
/* end subroutine (ttimk_mkbvi) */


static int ttimk_renamefiles(op)
TTIMK		*op ;
{
	int		rs ;
	char		hashfname[MAXPATHLEN + 1] ;

	rs = mkfnamesuf2(hashfname,op->dbname,FSUF_IND,ENDIANSTR) ;
	if (rs < 0)
	    goto ret0 ;

	rs = u_rename(op->nfname,hashfname) ;

#if	CF_DEBUGS
	debugprintf("ttimk_renamefiles: u_rename() rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    op->nfname[0] = '\0' ;

	if (op->nfname[0] != '\0') {
	    u_unlink(op->nfname) ;
	    op->nfname[0] = '\0' ;
	}

ret0:
	return rs ;
}
/* end subroutine (ttimk_renamefiles) */


/* "exclusive create" of file */
static int ttimk_nfcreate(op,fsuf)
TTIMK		*op ;
const char	fsuf[] ;
{
	struct ustat	sb ;
	int		rs ;
	int		rs1 ;
	int		nfl ;
	int		oflags = (O_CREAT | O_EXCL | O_WRONLY) ;
	const char	*cp ;
	char		nfname[MAXPATHLEN + 1] ;

	rs = mkfnamesuf3(nfname,op->dbname,fsuf,ENDIANSTR,"n") ;
	nfl = rs ;
	if (rs >= 0)
	    rs = uc_mallocstrw(nfname,nfl,&cp) ;

	op->nfname = (char *) cp ;
	if (rs < 0)
	    goto bad0 ;

again:
	rs = u_open(op->nfname,oflags,op->operms) ;
	op->nfd = rs ;
	if (rs >= 0) {
	    u_close(op->nfd) ;
	    op->nfd = -1 ;
	}

	if (rs == SR_EXIST) {
	    time_t	daytime = time(NULL) ;
	    int		f_inprogress ;
	    rs1 = u_stat(op->nfname,&sb) ;
	    if ((rs1 >= 0) && ((daytime - sb.st_mtime) > TO_OLDFILE)) {
		u_unlink(op->nfname) ;
		goto again ;
	    }
	    op->f.inprogress = TRUE ;
	    f_inprogress = op->f.none ;
	    f_inprogress = f_inprogress || (op->f.creat && op->f.excl) ;
	    rs = (f_inprogress) ? SR_INPROGRESS : SR_OK ;
	} /* end if */

	if (rs < 0)
	    goto bad1 ;

	op->f.created = TRUE ;

ret0:
	return rs ;

/* bad stuff */
bad1:
	if (op->nfname != NULL) {
	    uc_free(op->nfname) ;
	    op->nfname = NULL ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (ttimk_nfcreate) */


static int ttimk_nfcreatecheck(op,fpre,fsuf)
TTIMK		*op ;
const char	fpre[] ;
const char	fsuf[] ;
{
	int		rs = SR_OK ;
	int		oflags ;

	if ((op->nfd < 0) || op->f.inprogress) {
	    if (op->nfd >= 0) {
		u_close(op->nfd) ;
		op->nfd = -1 ;
	    }
	    oflags = O_WRONLY | O_CREAT ;
	    if (op->f.inprogress) {
		char	cname[MAXNAMELEN + 1] ;
		char	infname[MAXPATHLEN + 1] ;
		char	outfname[MAXPATHLEN + 1] ;
		outfname[0] = '\0' ;
		rs = sncpy6(cname,MAXNAMELEN,
			fpre,"XXXXXXXX",".",fsuf,ENDIANSTR,"n") ;
		if (rs >= 0) {
		    if ((op->idname != NULL) && (op->idname[0] != '\0'))
		        rs = mkpath2(infname,op->idname,cname) ;
		    else
		        rs = mkpath1(infname,cname) ;
		}
		if (rs >= 0) {
		    rs = opentmpfile(infname,oflags,op->operms,outfname) ;
	            op->nfd = rs ;
		    op->f.created = (rs >= 0) ;
	 	}
		if (rs >= 0)
		    rs = ttimk_nfstore(op,outfname) ;
		if (rs < 0) {
		    if (op->nfd >= 0) {
			u_close(op->nfd) ;
			op->nfd = -1 ;
		    }
		    if (outfname[0] != '\0')
			u_unlink(outfname) ;
		}
	    } else {
	        rs = u_open(op->nfname,oflags,op->operms) ;
	        op->nfd = rs ;
		op->f.created = (rs >= 0) ;
	    }
	}

	return rs ;
}
/* end subroutine (ttimk_nfcreatecheck) */


static int ttimk_nfdestroy(op)
TTIMK		*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

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
	    uc_free(op->nfname) ;
	    op->nfname = NULL ;
	}

	return rs ;
}
/* end subroutine (ttimk_nfdestroy) */


static int ttimk_nfstore(op,outfname)
TTIMK		*op ;
const char	outfname[] ;
{
	int		rs ;
	const char	*cp ;

	if (op->nfname != NULL) {
	    uc_free(op->nfname) ;
	    op->nfname = NULL ;
	}

	rs = uc_mallocstrw(outfname,-1,&cp) ;
	if (rs >= 0) op->nfname = (char *) cp ;

	return rs ;
}
/* end subroutine (ttimk_nfstore) */


static int ttimk_fexists(op)
TTIMK		*op ;
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f ;
	char		bvifname[MAXPATHLEN + 1] ;

	f = (op->f.creat && op->f.excl) ;
	if ((! f) || (! op->f.inprogress))
	    goto ret0 ;

	rs = mkfnamesuf2(bvifname,op->dbname,FSUF_IND,ENDIANSTR) ;

	if (rs >= 0) {
	    rs1 = u_stat(bvifname,&sb) ;
	    if (rs1 >= 0) rs = SR_EXIST ;
	}

ret0:
	return rs ;
}
/* end subroutine (ttimk_fexists) */


static int mkcitation(cip,bvp)
uint		*cip ;
TTIMK_VERSE	*bvp ;
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


static int vvecmp(v1p,v2p)
void	*v1p, *v2p ;
{
	struct bventry	**e1pp = (struct bventry **) v1p ;
	struct bventry	**e2pp = (struct bventry **) v2p ;
	uint		vc1, vc2 ;
	int		rc ;

	if (*e1pp == NULL) {
	    rc = 1 ;
	    goto ret0 ;
	}

	if (*e2pp == NULL) {
	    rc = -1 ;
	    goto ret0 ;
	}

	vc1 = (*e1pp)->citation & 0x00FFFFFF ;
	vc2 = (*e2pp)->citation & 0x00FFFFFF ;

	rc = vc1 - vc2 ;

ret0:
	return rc ;
}
/* end subroutine (vvecmp) */


