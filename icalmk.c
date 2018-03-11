/* icalmk */

/* make a iCalendar (ICS) database file */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_MINMOD	1		/* ensure minimum file mode */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module creates an iCalendar (ICS) database file.

	Synopsis:

	int icalmk_open(op,dirname,calname,oflags,operms,year,f_tmp)
	ICALMK		*op ;
	const char	dirname[] ;
	const char	calname[] ;
	int		oflags ;
	int		operms ;
	int		year ;
	int		f_tmp ;

	Arguments:

	op		object pointer
	dirname		directory path
	calname		name of calendar
	oflags		open-flags
	operms		open (create) file permissions 
	year		year
	f_tmp		whether resulting DB is temporary

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


#define	ICALMK_MASTER	0


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
#include	<vecobj.h>
#include	<filebuf.h>
#include	<char.h>
#include	<tmtime.h>
#include	<localmisc.h>

#include	"icalmk.h"


/* local defines */

#define	ICALMK_NENTRIES	(19 * 1024)
#define	ICALMK_NSKIP	5

#undef	RECTAB
#define	RECTAB		struct icalmk_rectab

#define	BUFLEN		(sizeof(ICALHDR) + 128)

#define	FSUF_IND	"ical"

#define	TO_OLDFILE	(5 * 60)

#define	MODP2(v,n)	((v) & ((n) - 1))


/* external subroutines */

extern uint	hashelf(const char *,int) ;

extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* exported variables */

ICALMK_OBJ	icalmk = {
	"icalmk",
	sizeof(ICALMK)
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

static int	icalmk_filesbegin(ICALMK *) ;
static int	icalmk_filesend(ICALMK *,int) ;
static int	icalmk_listbegin(ICALMK *,int) ;
static int	icalmk_listend(ICALMK *) ;
static int	icalmk_nfcreate(ICALMK *,const char *) ;
static int	icalmk_nfcreatecheck(ICALMK *,cchar *,char *) ;
static int	icalmk_nfstore(ICALMK *,const char *) ;
static int	icalmk_nfdestroy(ICALMK *) ;
static int	icalmk_nfexists(ICALMK *) ;
static int	icalmk_mkcyi(ICALMK *) ;
static int	icalmk_renamefiles(ICALMK *) ;

static int	filebuf_writefill(FILEBUF *,const char *,int) ;

static int	mkcitation(uint *,ICALMK_ENT *) ;

static int	vvecmp(void *,void *) ;


/* local variables */

static const char	zerobuf[4] = {
	0, 0, 0, 0 
} ;


/* exported subroutines */


int icalmk_open(op,dirname,calname,oflags,operms,year,f_tmp)
ICALMK		*op ;
const char	dirname[] ;
const char	calname[] ;
int		oflags ;
int		operms ;
int		year ;
int		f_tmp ;
{
	int	rs = SR_OK ;
	int	omode ;
	int	n = ICALMK_NENTRIES ;


	if (op == NULL)
	    return SR_FAULT ;

	if (dirname == NULL)
	    return SR_FAULT ;

	if (calname == NULL)
	    return SR_FAULT ;

	if (dirname[0] == '\0')
	    return SR_INVALID ;

	if (calname[0] == '\0')
	    return SR_INVALID ;

	if (year < 0) {
	    TMTIME	tm ;
	    time_t	daytime = time(NULL) ;
	    rs = tmtime_localtime(&tm,daytime) ;
	    year = (tm.year + TM_YEAR_BASE) ;
	    if (rs < 0)
		goto ret0 ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("icalmk_open: dirname=%s\n",dirname) ;
	debugprintf("icalmk_open: calname=%s\n",calname) ;
	debugprintf("icalmk_open: f_tmp=%u\n",f_tmp) ;
#endif /* CF_DEBUGS */

	memset(op,0,sizeof(ICALMK)) ;

	op->operms = operms ;
	op->nfd = -1 ;
	op->year = year ;

	op->f.tmpfile = f_tmp ;
	op->f.creat = (oflags & O_CREAT) ;
	op->f.excl = (oflags & O_EXCL) ;
	op->f.none = (! op->f.creat) && (! op->f.excl) ;

	omode = (W_OK | X_OK | R_OK) ;
	rs = perm(dirname,-1,-1,NULL,omode) ;
	if (rs < 0)
	    goto bad0 ;

	rs = uc_mallocstrw(dirname,-1,&op->idname) ;
	if (rs < 0)
	    goto bad0 ;

	rs = uc_mallocstrw(calname,-1,&op->calname) ;
	if (rs < 0)
	    goto bad1 ;

	rs = icalmk_filesbegin(op) ;
	if (rs < 0)
	    goto bad2 ;

/* initialize the data structures we need */

	rs = icalmk_listbegin(op,n) ;
	if (rs < 0)
	    goto bad3 ;

	op->magic = ICALMK_MAGIC ;

ret0:

#if	CF_DEBUGS
	debugprintf("icalmk_open: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad3:
	icalmk_filesend(op,TRUE) ;

bad2:
	if (op->calname != NULL) {
	    uc_free(op->calname) ;
	    op->calname = NULL ;
	}

bad1:
	if (op->idname != NULL) {
	    uc_free(op->idname) ;
	    op->idname = NULL ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (icalmk_open) */


int icalmk_close(op)
ICALMK		*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	nentries = 0 ;
	int	f_remove = FALSE ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != ICALMK_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("icalmk_close: nentries=%u\n",op->nentries) ;
#endif

	nentries = op->nentries ;
	if (nentries > 0) {
	    if (op->f.notsorted)
	        vecobj_sort(&op->verses,vvecmp) ;
	    rs = icalmk_mkcyi(op) ;
	    f_remove = (rs < 0) ;
	}

#if	CF_DEBUGS
	debugprintf("icalmk_close: icalmk_mkcyi() rs=%d\n",rs) ;
#endif

	icalmk_listend(op) ;

	if ((rs >= 0) && (nentries > 0)) {
	    rs = icalmk_renamefiles(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("icalmk_close: icalmk_renamefiles() rs=%d\n",rs) ;
#endif

	rs1 = icalmk_filesend(op,f_remove) ;
	if (rs >= 0) rs = rs1 ;

	if (op->calname != NULL) {
	    rs1 = uc_free(op->calname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->calname = NULL ;
	}

	if (op->idname != NULL) {
	    rs1 = uc_free(op->idname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->idname = NULL ;
	}

	op->magic = 0 ;
	return (rs >= 0) ? nentries : rs ;
}
/* end subroutine (icalmk_close) */


int icalmk_add(op,bvp)
ICALMK		*op ;
ICALMK_ENT	*bvp ;
{
	struct bventry	bve ;

	struct blentry	ble ;

	uint	li = UINT_MAX ;
	uint	citcmpval ;

	const int	imax = UCHAR_MAX ;

	int	rs = SR_OK ;
	int	i ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != ICALMK_MAGIC)
	    return SR_NOTOPEN ;

	if (bvp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("icalmk_add: q=%u:%u\n",
		bvp->m,bvp->d) ;
#endif

	if ((bvp->lines != NULL) && (bvp->nlines > 0)) {
	    for (i = 0 ; (i < bvp->nlines) && (i < imax) ; i += 1) {

		ble.loff = bvp->lines[i].loff ;
		ble.llen = bvp->lines[i].llen ;
		rs = vecobj_add(&op->lines,&ble) ;
		if (rs < 0)
		    break ;

		if (i == 0)
		    li = rs ;

	    } /* end for */
	} /* end if */

	if (rs < 0)
	    goto bad1 ;

#if	CF_DEBUGS
	debugprintf("icalmk_add: li=%u\n",li) ;
#endif

	bve.voff = bvp->voff ;
	bve.vlen = bvp->vlen ;
	bve.li = li ;
	bve.hash = bvp->hash ;
	mkcitation(&bve.citation,bvp) ;

	citcmpval = (bve.citation & 0x0000FFFF) ;
	if (citcmpval < op->pcitation)
	    op->f.notsorted = TRUE ;

	op->pcitation = citcmpval ;

	rs = vecobj_add(&op->verses,&bve) ;
	if (rs < 0)
	    goto bad2 ;

	op->nentries += 1 ;

ret0:

#if	CF_DEBUGS && 0
	debugprintf("icalmk_add: ret=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad2:
bad1:
bad0:
	goto ret0 ;
}
/* end subroutine (icalmk_add) */


/* private subroutines */


static int icalmk_filesbegin(op)
ICALMK		*op ;
{
	int	rs = SR_OK ;
	int	dl = 0 ;
	int	fl = 0 ;

	const char	*fsuf = FSUF_IND ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	dbname[MAXPATHLEN + 1] ;


	tmpfname[0] = '\0' ;
	if (op->f.tmpfile) {

	    char	template[MAXPATHLEN + 1] ;

	    if (rs >= 0)
	        rs = mkpath2(tmpfname,op->idname,"XXXXXXXXX") ;

	    if (rs >= 0)
	        rs = mkfnamesuf2(template,tmpfname,fsuf,ENDIANSTR) ;

	    if (rs >= 0) {
		rs = mktmpfile(tmpfname,op->operms,template) ;
		fl = rs ;
		if (rs < 0) tmpfname[0] = '\0' ;
	    }

	    if (rs >= 0) {
		const char	*tp ;
		tp = strrchr(tmpfname,'.') ; /* cannot fail! */
		rs = mkpath1w(dbname,tmpfname,(tp-tmpfname)) ;
		dl = rs ;
	    } /* end if */

	} else {

	    rs = mkpath2(dbname,op->idname,op->calname) ;
	    dl = rs ;
	    if (rs >= 0) {
		rs = mkfnamesuf2(tmpfname,dbname,fsuf,ENDIANSTR) ;
		fl = rs ;
	    }

	} /* end if */

	if (rs < 0)
	    goto bad1 ;

	rs = uc_mallocstrw(dbname,dl,&op->dbname) ;
	if (rs < 0)
	    goto bad2 ;

/* store index file-name */

	if (rs >= 0)
	    rs = uc_mallocstrw(tmpfname,fl,&op->cyifname) ;

	if (rs < 0)
	    goto bad3 ;

/* create it */

	rs = icalmk_nfcreate(op,FSUF_IND) ;
	if (rs < 0)
	    goto bad4 ;

	if (op->f.creat && op->f.excl)
	    rs = icalmk_nfexists(op) ;

	if (rs < 0)
	    goto bad5 ;

ret0:

#if	CF_DEBUGS
	debugprintf("icalmk_filesbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad5:
	icalmk_nfdestroy(op) ;

bad4:
	if (op->cyifname != NULL) {
	    uc_free(op->cyifname) ;
	    op->cyifname = NULL ;
	}

bad3:
	if (op->dbname != NULL) {
	    uc_free(op->dbname) ;
	    op->dbname = NULL ;
	}

bad2:
bad1:
	if (op->f.tmpfile && (tmpfname[0] != '\0')) {

#if	CF_DEBUGS
	debugprintf("icalmk_filesbegin: unlink tmpfname=%s\n",tmpfname) ;
#endif

	    u_unlink(tmpfname) ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (icalmk_filesbegin) */


static int icalmk_filesend(op,f)
ICALMK		*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


#if	CF_DEBUGS
	debugprintf("icalmk_filesend: f_remote=%u\n",f) ;
#endif

	if (op->nfd >= 0) {
	    rs1 = u_close(op->nfd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfd = -1 ;
	}

	if (op->nfname != NULL) {
	    if ((op->f.created || op->f.tmpfile) && (op->nfname[0] != '\0')) {

#if	CF_DEBUGS
	debugprintf("icalmk_filesend: unlink nfname=%s\n",op->nfname) ;
#endif

	        rs1 = u_unlink(op->nfname) ;
		if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = uc_free(op->nfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfname = NULL ;
	}

	if (op->cyifname != NULL) {
	    if (f && (op->cyifname[0] != '\0')) {

#if	CF_DEBUGS
	debugprintf("icalmk_filesend: unlink fname=%s\n",op->cyifname) ;
#endif

	        rs1 = u_unlink(op->cyifname) ;
		if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = uc_free(op->cyifname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->cyifname = NULL ;
	}

	if (op->dbname != NULL) {
	    rs1 = uc_free(op->dbname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("icalmk_filesend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (icalmk_filesend) */


static int icalmk_listbegin(op,n)
ICALMK		*op ;
int		n ;
{
	int	rs ;
	int	size ;
	int	opts ;


	opts = 0 ;
	opts |= VECOBJ_OSTATIONARY ;
	opts |= VECOBJ_OORDERED ;
	opts |= VECOBJ_OCOMPACT ;
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
/* end subroutine (icalmk_listbegin) */


static int icalmk_listend(op)
ICALMK		*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	rs1 = vecobj_finish(&op->lines) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->verses) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (icalmk_listend) */


static int icalmk_mkcyi(op)
ICALMK	*op ;
{
	struct bventry	*bvep ;

	struct blentry	*blep ;

	ICALHDR	hf ;

	FILEBUF	cyifile ;

	time_t	daytime = time(NULL) ;

	uint	fileoff = 0 ;
	uint	a[5] ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	pagesize = getpagesize() ;
	int	size ;
	int	n, i ;
	int	bl ;

	char	buf[BUFLEN + 1] ;


/* open (create) the VAR file */

	rs = icalmk_nfcreatecheck(op,"nc",FSUF_IND) ;
	if (rs < 0)
	    goto ret0 ;

	op->f.viopen = TRUE ;
	size = (pagesize * 4) ;
	rs = filebuf_start(&cyifile,op->nfd,0,size,0) ;
	if (rs < 0)
	    goto ret1 ;

/* prepare the file-header */

	memset(&hf,0,sizeof(ICALHDR)) ;

	hf.vetu[0] = ICALHDR_VERSION ;
	hf.vetu[1] = ENDIAN ;
	hf.vetu[2] = 0 ;
	hf.vetu[3] = 0 ;
	hf.wtime = (uint) daytime ;
	hf.nentries = op->nentries ;
	hf.nskip = ICALMK_NSKIP ;
	hf.year = op->year ;

/* create the file-header */

	rs = icalhdr(&hf,0,buf,BUFLEN) ;
	bl = rs ;
	if (rs < 0)
	    goto ret2 ;

/* write header */

	if (rs >= 0) {
	    rs = filebuf_writefill(&cyifile,buf,bl) ;
	    fileoff += rs ;
	}

/* write the directory-name and the calendar-name */

	if (rs >= 0) {
	    int		tl ;
	    char	tmpdname[MAXPATHLEN + 1] ;
	    hf.diroff = fileoff ;
	    rs = pathclean(tmpdname,op->idname,-1) ;
	    tl = rs ;
	    if (rs >= 0) {
	        rs = filebuf_writefill(&cyifile,tmpdname,(tl+1)) ;
	        fileoff += rs ;
	    }
	}

	if (rs >= 0) {
	    int		tl = strlen(op->calname) ;
	    hf.caloff = fileoff ;
	    rs = filebuf_writefill(&cyifile,op->calname,(tl+1)) ;
	    fileoff += rs ;
	}

	if (rs < 0)
	    goto ret2 ;

/* write the "verses" table */

	hf.vioff = fileoff ;
	size = 5 * sizeof(uint) ;
	n = 0 ;
	for (i = 0 ; vecobj_get(&op->verses,i,&bvep) >= 0 ; i += 1) {
	    if (bvep == NULL) continue ;

	    a[0] = bvep->voff ;
	    a[1] = bvep->vlen ;
	    a[2] = bvep->li ;
	    a[3] = bvep->citation ;
	    a[4] = bvep->hash ;

#if	CF_DEBUGS
	debugprintf("icalmk_mkcyi: citation=%08X\n",a[3]) ;
#endif

	    n += 1 ;
	    rs = filebuf_write(&cyifile,a,size) ;
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
	    rs = filebuf_write(&cyifile,a,size) ;
	    fileoff += rs ;
	    if (rs < 0)
		break ;

	} /* end for */

	hf.vllen = n ;

/* write out the header -- again! */
ret2:
	filebuf_finish(&cyifile) ;

	if (rs >= 0) {

	    hf.fsize = fileoff ;

	    rs = icalhdr(&hf,0,buf,BUFLEN) ;
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
	op->f.viopen = FALSE ;
	rs1 = u_close(op->nfd) ;
	if (rs >= 0) rs = rs1 ;
	op->nfd = -1 ;

	if ((rs < 0) && (op->nfname[0] != '\0')) {

#if	CF_DEBUGS
	debugprintf("icalmk_mkcyi: unlink nfname=%s\n",op->nfname) ;
#endif

	    u_unlink(op->nfname) ;
	    op->nfname[0] = '\0' ;
	}

ret0:
	return rs ;
}
/* end subroutine (icalmk_mkcyi) */


static int icalmk_renamefiles(op)
ICALMK	*op ;
{
	int	rs ;


#if	CF_DEBUGS
	debugprintf("icalmk_renamefiles: ncyifname=%s\n",op->nfname) ;
	debugprintf("icalmk_renamefiles: cyifname=%s\n",op->cyifname) ;
#endif

	rs = u_rename(op->nfname,op->cyifname) ;

#if	CF_DEBUGS
	debugprintf("icalmk_renamefiles: u_rename() rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    op->nfname[0] = '\0' ;

	if (op->nfname[0] != '\0') {

#if	CF_DEBUGS
	debugprintf("icalmk_renamefiles: unlink nfname=%s\n",op->nfname) ;
#endif

	    u_unlink(op->nfname) ;
	    op->nfname[0] = '\0' ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("icalmk_renamefiles: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (icalmk_renamefiles) */


static int filebuf_writefill(bp,fbuf,flen)
FILEBUF		*bp ;
const char	fbuf[] ;
int		flen ;
{
	int	rs ;
	int	r, nzero ;
	int	asize = sizeof(uint) ;
	int	wlen = 0 ;


	if (flen < 0)
	    flen = strlen(fbuf) ;

	rs = filebuf_write(bp,fbuf,flen) ;
	wlen = rs ;

	r = (flen & (asize - 1)) ;
	if ((rs >= 0) && (r > 0)) {
	    nzero = (asize - r) ;
	    if (nzero > 0) {
	        rs = filebuf_write(bp,zerobuf,nzero) ;
	        wlen += rs ;
	    }
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filebuf_writefill) */


static int mkcitation(cip,bvp)
uint		*cip ;
ICALMK_ENT	*bvp ;
{
	uint	ci = 0 ;
	uint	nlines ;


	nlines = (bvp->lines != NULL) ? bvp->nlines : 0 ;

	ci |= ((nlines & UCHAR_MAX) << 24) ;

	ci |= ((bvp->m & UCHAR_MAX) << 8) ;

	ci |= ((bvp->d & UCHAR_MAX) << 0) ;

	*cip = ci ;
	return SR_OK ;
}
/* end subroutine (mkcitation) */


/* "exclusive create" of file */
static int icalmk_nfcreate(op,fsuf)
ICALMK		*op ;
const char	fsuf[] ;
{
	struct ustat	sb ;

	int	rs ;
	int	rs1 ;
	int	nfl ;
	int	oflags = (O_CREAT | O_EXCL | O_WRONLY) ;

	const char	*cp ;

	char	nfname[MAXPATHLEN + 1] ;


	rs = mkfnamesuf3(nfname,op->dbname,fsuf,ENDIANSTR,"n") ;
	nfl = rs ;
	if (rs >= 0)
	    rs = uc_mallocstrw(nfname,nfl,&cp);

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

#if	CF_DEBUGS
	debugprintf("icalmk_nfcreate: unlink nfname=%s\n",op->nfname) ;
#endif

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

bad1:
	if (op->nfname != NULL) {
	    uc_free(op->nfname) ;
	    op->nfname = NULL ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (icalmk_nfcreate) */


static int icalmk_nfcreatecheck(op,fpre,fsuf)
ICALMK		*op ;
const char	fpre[] ;
const char	fsuf[] ;
{
	int	rs = SR_OK ;
	int	oflags ;


	if ((op->nfd < 0) || op->f.inprogress) {
	    if (op->nfd >= 0) {
		u_close(op->nfd) ;
		op->nfd = -1 ;
	    }
	    oflags = O_WRONLY ;
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
		    rs = icalmk_nfstore(op,outfname) ;
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
	} /* end if */

	return rs ;
}
/* end subroutine (icalmk_nfcreatecheck) */


static int icalmk_nfdestroy(op)
ICALMK		*op ;
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

#if	CF_DEBUGS
	debugprintf("icalmk_nfdestroy: unlink nfname=%s\n",op->nfname) ;
#endif

		rs1 = u_unlink(op->nfname) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    uc_free(op->nfname) ;
	    op->nfname = NULL ;
	}

	return rs ;
}
/* end subroutine (icalmk_nfdestroy) */


static int icalmk_nfstore(op,outfname)
ICALMK		*op ;
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
/* end subroutine (icalmk_nfstore) */


static int icalmk_nfexists(op)
ICALMK		*op ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	f ;


	f = (op->f.creat && op->f.excl) ;
	if ((! f) || (! op->f.inprogress))
	    goto ret0 ;

	if (rs >= 0) {
	    rs1 = u_stat(op->cyifname,&sb) ;
	    if (rs1 >= 0) rs = SR_EXIST ;
	}

ret0:
	return rs ;
}
/* end subroutine (icalmk_exits) */


static int vvecmp(v1p,v2p)
void	*v1p, *v2p ;
{
	struct bventry	**e1pp = (struct bventry **) v1p ;
	struct bventry	**e2pp = (struct bventry **) v2p ;

	uint	vc1, vc2 ;

	int	rc ;


	if (*e1pp == NULL) {
	    rc = 1 ;
	    goto ret0 ;
	}

	if (*e2pp == NULL) {
	    rc = -1 ;
	    goto ret0 ;
	}

	vc1 = (*e1pp)->citation & 0x0000FFFF ;
	vc2 = (*e2pp)->citation & 0x0000FFFF ;

	rc = (vc1 - vc2) ;

ret0:
	return rc ;
}
/* end subroutine (vvecmp) */


