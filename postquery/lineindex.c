/* lineindex */

/* line indexing object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		1		/* safe mode */
#define	CF_MEMSYNC	1		/* use memory synchronization? */
#define	CF_FILEMAP	1		/* use FILEMAP object */
#define	CF_TESTMODE	0		/* for testing */


/* revision history:

	= 2003-06-11, David A­D­ Morano
        I snarfed this file from the SS-Hammock crap since I thought it might be
        a bit similar. We'll see how it works out!

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object module manages a line-index database. It can also create
        such a database if it is opened with the O_CREAT option.

        Note that line indexing is so fast that trying to super-optimize
        anything here is not really work it.


*******************************************************************************/


#define	LINEINDEX_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>		/* Memory Management */
#include	<netinet/in.h>
#include	<inttypes.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<mallocstuff.h>
#include	<filemap.h>
#include	<localmisc.h>

#include	"lineindex.h"


/* local defines */

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	TO_FILECOME	2
#define	TO_OPEN		(60 * 60)
#define	TO_MAP		(1 * 60)
#define	TO_CHECK	4
#define	TO_ACCESS	(1 * 60)

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	FILEMAPSIZE	(500 * 1024 * 1024)
#define	NRECS		(2 * 1024)


/* external subroutines */

extern uint	uceil(uint,int) ;
extern uint	nextpowtwo(uint) ;
extern uint	hashelf(const void *,int) ;

extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	randlc(int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	getpwd(char *,int) ;
extern int	isfsremote(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* forward references */

static int	lineindex_mkindex(LINEINDEX *) ;
static int	lineindex_fileheader(LINEINDEX *) ;
static int	lineindex_filemap(LINEINDEX *,time_t) ;
static int	lineindex_fileunmap(LINEINDEX *) ;

static int	lineindex_fileopen(LINEINDEX *,time_t) ;
static int	lineindex_fileclose(LINEINDEX *) ;


/* local variables */

enum headers {
	header_rectab,
	header_wtime,
	header_lines,
	header_overlast
} ;


/* exported subroutines */


int lineindex_open(op,idxfname,oflags,operm,txtfname)
LINEINDEX	*op ;
const char	idxfname[] ;
const char	txtfname[] ;
int		oflags ;
mode_t		operm ;
{
	struct ustat	sb ;

	time_t	dt = time(NULL) ;

	int	rs ;
	int	i ;
	int	amode ;
	int	prot, flags ;
	int	size ;


	if (op == NULL)
	    return SR_FAULT ;

	if (idxfname == NULL)
	    return SR_FAULT ;

	if (idxfname[0] == '\0')
	    return SR_INVALID ;

	memset(op,0,sizeof(LINEINDEX)) ;

	op->pagesize = getpagesize() ;
	op->fd = -1 ;
	op->oflags = oflags ;
	op->operm = operm ;

	amode = (oflags & O_ACCMODE) ;
	op->f.wantwrite = ((amode == O_WRONLY) || (amode == O_RDWR)) ;

/* store filename away */

	op->idxfname = mallocstr(idxfname) ;

	if (op->idxfname == NULL) {
	    rs = SR_NOMEM ;
	    goto bad0 ;
	}

	if ((txtfname != NULL) && (txtfname[0] != '\0')) {
	    if ((op->txtfname = mallocstr(txtfname)) == NULL) {
	        rs = SR_NOMEM ;
	        goto bad1 ;
	    }

	}

/* try to open the file */

#if	CF_TESTMODE
	dt = 0 ;
#endif

	rs = lineindex_fileopen(op,dt) ;
	if (rs < 0)
	    goto bad2 ;

/* wait for the file to come in if it is not yet available */

	size = 16 + 4 + (header_overlast * sizeof(int)) ;

	rs = u_fstat(op->fd,&sb) ;

#if	CF_DEBUGS
	debugprintf("lineindex_open: filesize=%d headersize=%d\n",
	    sb.st_size,size) ;
#endif

	for (i = 0 ; (i < TO_FILECOME) && (rs >= 0) && (sb.st_size < size) ;
	    i += 1) {

#if	CF_DEBUGS
	    debugprintf("lineindex_open: wating for file i=%u\n",i) ;
#endif

	    sleep(1) ;

	    rs = u_fstat(op->fd,&sb) ;

	} /* end while */

	if (rs < 0)
	    goto bad2 ;

	if ((i >= TO_FILECOME) || (sb.st_size < size)) {
	    rs = SR_TIMEDOUT ;
	    goto bad2 ;
	}

	op->mtime = sb.st_mtime ;
	op->filesize = (uint) sb.st_size ;

/* OK, continue on */

	if (op->filesize >= size) {

/* map it */

#if	CF_DEBUGS
	    debugprintf("lineindex_open: mapping file\n") ;
#endif

	    prot = PROT_READ ;
	    flags = MAP_SHARED ;
	    op->mapsize = op->filesize ;
	    rs = u_mmap(NULL,(size_t) op->mapsize,prot,flags,
	        op->fd,0L,&op->mapbuf) ;

#if	CF_DEBUGS
	    debugprintf("lineindex_open: u_mmap() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto bad3 ;

	    op->ti_map = dt ;

/* OK, check it out */

	    rs = lineindex_fileheader(op) ;
	    if (rs < 0)
	        goto bad4 ;

/* ok, we're good (?) */

	    op->ti_access = dt ;
	    op->f.fileinit = TRUE ;

#if	CF_DEBUGS
	    debugprintf("lineindex_open: header processing completed \n") ;
#endif

	} /* end if (file had some data) */

/* we should be good */

	op->magic = LINEINDEX_MAGIC ;

ret1:
	lineindex_fileclose(op) ;

ret0:

#if	CF_DEBUGS
	debugprintf("lineindex_open: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* we're out of here */
bad4:
	u_munmap(op->mapbuf,(size_t) op->filesize) ;

bad3:
	lineindex_fileclose(op) ;

bad2:
	if (op->txtfname != NULL)
	    uc_free(op->txtfname) ;

bad1:
	uc_free(op->idxfname) ;

bad0:
	goto ret0 ;

}
/* end subroutine (lineindex_open) */


/* get the string count in the table */
int lineindex_count(op)
LINEINDEX	*op ;
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LINEINDEX_MAGIC) return SR_NOTOPEN ;

	return op->lines ;
}
/* end subroutine (lineindex_count) */


/* initialize a cursor */
int lineindex_curbegin(op,cp)
LINEINDEX	*op ;
LINEINDEX_CUR	*cp ;
{

	if (op == NULL) return SR_FAULT ;
	if (cp == NULL) return SR_FAULT ;

	if (op->magic != LINEINDEX_MAGIC) return SR_NOTOPEN ;

	op->cursors += 1 ;
	op->f.cursorlockbroken = FALSE ;
	op->f.cursoracc = FALSE ;
	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (lineindex_curbegin) */


/* free up a cursor */
int lineindex_curend(op,cp)
LINEINDEX	*op ;
LINEINDEX_CUR	*cp ;
{
	const time_t	dt = time(NULL) ;

	if (op == NULL) return SR_FAULT ;
	if (cp == NULL) return SR_FAULT ;

	if (op->magic != LINEINDEX_MAGIC) return SR_NOTOPEN ;

#if	CF_TESTMODE
	dt = 0 ;
#endif

	if (op->f.cursoracc)
	    op->ti_access = dt ;

	if (op->cursors > 0)
	    op->cursors -= 1 ;

	cp->i = -1 ;

	return SR_OK ;
}
/* end subroutine (lineindex_curend) */


/* enumerate */
int lineindex_enum(op,cup,rp)
LINEINDEX	*op ;
LINEINDEX_CUR	*cup ;
offset_t	*rp ;
{
	int		rs = SR_OK ;
	int		ri ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != LINEINDEX_MAGIC) return SR_NOTOPEN ;
#endif

	if (cup == NULL) return SR_FAULT ;

	if (op->cursors == 0) return SR_INVALID ;

	ri = (cup->i < 0) ? 0 : (cup->i + 1) ;

#if	CF_DEBUGS
	debugprintf("lineindex_enum: ri=%d\n",ri) ;
#endif

/* is the file mapped? */

	if (op->mapbuf == NULL) {
	    time_t	dt = time(NULL) ;

#if	CF_TESTMODE
	    dt = 0 ;
#endif

	    rs = lineindex_filemap(op,dt) ;

	    if (rs > 0)
	        rs = lineindex_fileheader(op) ;

	} /* end if (checking if file is mapped) */

	if (rs < 0)
	   goto ret0 ;

/* ok, we're good to go */

	if (ri >= op->lines)
	    return SR_NOTFOUND ;

	if (rp != NULL) {

	    *rp = ntohl(op->rectab[ri]) ;

#if	CF_DEBUGS
	    debugprintf("lineindex_enum: off=%lu\n",*rp) ;
#endif

	}

/* update the cursor */

	cup->i = ri ;

ret0:
	return (rs >= 0) ? ri : rs ;
}
/* end subroutine (lineindex_enum) */


/* get the entries (serially) */
int lineindex_lookup(op,ri,rp)
LINEINDEX	*op ;
uint		ri ;
offset_t	*rp ;
{
	int		rs = SR_OK ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != LINEINDEX_MAGIC) return SR_NOTOPEN ;
#endif

#if	CF_DEBUGS
	debugprintf("lineindex_lookup: ent 2\n") ;
#endif

	if (rp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("lineindex_lookup: ent ri=%d\n",ri) ;
#endif

	if (ri >= op->lines) return SR_NOTFOUND ;

/* is the file mapped? */

	if (op->mapbuf == NULL) {
	    time_t	dt = time(NULL) ;

#if	CF_TESTMODE
	    dt = 0 ;
#endif

	    rs = lineindex_filemap(op,dt) ;

	    if (rs > 0)
	        rs = lineindex_fileheader(op) ;

	} /* end if (checking if file is mapped) */

	if (rs < 0)
	    goto ret0 ;

/* do the real work */

	if (ri > 0)
	    *rp = ntohl(op->rectab[ri]) ;

#if	CF_DEBUGS
	debugprintf("lineindex_lookup: OK\n") ;
#endif

ret0:
	return (rs >= 0) ? ri : rs ;
}
/* end subroutine (lineindex_lookup) */


/* do some checking */
int lineindex_check(op,dt)
LINEINDEX	*op ;
time_t		dt ;
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		f_changed = FALSE ;
#if	CF_DEBUGS
	char		timebuf[TIMEBUFLEN + 1] ;
#endif
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LINEINDEX_MAGIC) return SR_NOTOPEN ;

/* check things */

	if (op->cursors > 0)
	    goto ret0 ;

	if (op->mapbuf == NULL)
	    goto ret0 ;

/* check for "unused" */

	cp = (const char *) (op->mapbuf + 16 + 3) ;
	if (*cp)
	    goto closeit ;

/* check for time limits */

	if ((dt - op->ti_map) > TO_MAP)
	    goto closeit ;

	if ((dt - op->ti_check) > TO_CHECK) {

	    op->ti_check = dt ;
	    if ((rs = u_stat(op->idxfname,&sb)) >= 0) {

	        if ((sb.st_mtime > op->mtime) ||
	            (sb.st_size > op->filesize))
	            goto changed ;

	    }

	    if (op->txtfname != NULL) {

	        if ((rs = u_stat(op->txtfname,&sb)) >= 0) {

	            if (sb.st_mtime > op->mtime)
	                goto changed ;

	        }
	    }

	} /* end if (check) */

ret0:
	return (rs >= 0) ? f_changed : rs ;

/* handle a close out */
changed:
	f_changed = TRUE ;

closeit:
	op->ti_check = dt ;
	rs = lineindex_fileunmap(op) ;

	goto ret0 ;
}
/* end subroutine (lineindex_check) */


/* free up this lineindex object */
int lineindex_close(op)
LINEINDEX	*op ;
{
	int		rs = SR_BADFMT ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != LINEINDEX_MAGIC) return SR_NOTOPEN ;

	if (op->mapbuf != NULL) {
	    rs = u_munmap(op->mapbuf,(size_t) op->mapsize) ;
	}

	if (op->fd >= 0) {
	    u_close(op->fd) ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (lineindex_close) */


/* private subroutines */


/* read the file header and check it out */
static int lineindex_fileheader(op)
LINEINDEX	*op ;
{
	uint		*table ;
	uint		recoff ;
	int		rs = SR_OK ;
	int		f ;
	const char	*cp = (cchar *) op->mapbuf ;

	f = (strncmp(cp,LINEINDEX_FILEMAGIC,LINEINDEX_FILEMAGICLEN) == 0) ;

	f = f && (*(cp + LINEINDEX_FILEMAGICLEN) == '\n') ;

	if (! f) {

#if	CF_DEBUGS
	    debugprintf("lineindex_fileheader: bad magic=>%t<\n",
	        cp,strnlen(cp,14)) ;
#endif

	    rs = SR_BADFMT ;
	    goto bad3 ;
	}

	cp += 16 ;
	if (cp[0] > LINEINDEX_FILEVERSION) {

	    rs = SR_NOTSUP ;
	    goto bad3 ;
	}

#ifdef	COMMENT
	if (cp[1] != ENDIAN) {

	    rs = SR_NOTSUP ;
	    goto bad3 ;
	}
#else
	if (cp[1] != LINEINDEX_ENDIAN) {

	    rs = SR_NOTSUP ;
	    goto bad3 ;
	}
#endif /* COMMENT */

/* the recorder options */

	op->ropts = cp[2] ;

#if	CF_DEBUGS
	debugprintf("lineindex_fileheader: ropts=%02x\n",op->ropts) ;
#endif

/* if looks good, read the header stuff */

	table = (uint *) (op->mapbuf + 16 + 4) ;

	recoff = ntohl(table[header_rectab]) ;
	op->lines = ntohl(table[header_lines]) ;
	op->wtime = ntohl(table[header_wtime]) ;

#if	CF_DEBUGS
	{
		char	timebuf[TIMEBUFLEN + 1] ;
	debugprintf("lineindex_fileheader: wtime=%s\n",
		timestr_logz(op->wtime,timebuf)) ;
	}
#endif

	op->rectab = (uint *) (op->mapbuf + recoff) ;

#if	CF_DEBUGS
	debugprintf("lineindex_fileheader: recoff=%u lines=%u\n",
	    recoff,op->lines) ;
#endif

ret0:
	return rs ;

/* bad stuff comes here */
bad3:
	goto ret0 ;
}
/* end subroutine (lineindex_fileheader) */


/* acquire access to the file (mapped memory) */
static int lineindex_filemap(op,dt)
LINEINDEX	*op ;
time_t		dt ;
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("lineindex_filemap: ent\n") ;
#endif

/* do a map or a re-map */

	if (op->mapbuf == NULL) {

	    if ((rs = lineindex_fileopen(op,dt)) >= 0) {
		caddr_t	p ;
		size_t	ms ;
		int	mp, mf ;

	        op->mapsize = uceil(op->filesize,op->pagesize) ;

	        if (op->mapsize == 0)
	            op->mapsize = op->pagesize ;

		ms = (size_t) op->mapsize ;
	        mp = PROT_READ ;
	        mf = MAP_SHARED ;
	        if ((rs = u_mmap(NULL,ms,mp,mf,op->fd,0L,&p)) >= 0) {
		    op->mapbuf = p ;
	            op->ti_map = dt ;
		}

	        lineindex_fileclose(op) ;
	    } /* end if (opened the file) */

	} /* end if (mapping file) */

	return rs ;
}
/* end subroutine (lineindex_filemap) */


/* release our hold on the filemap */
static int lineindex_fileunmap(op)
LINEINDEX	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->mapbuf != NULL) {
	    rs1 = u_munmap(op->mapbuf,(size_t) op->mapsize) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mapbuf = NULL ;
	    op->mapsize = 0 ;
	} /* end if (checking existing map) */

	return rs ;
}
/* end subroutine (lineindex_fileunmap) */


static int lineindex_fileopen(op,dt)
LINEINDEX	*op ;
time_t		dt ;
{
	struct ustat	sb ;

	int	rs ;
	int	oflags ;
	int	f ;
	int	f_create ;
	int	f_changed = FALSE ;

#if	CF_DEBUGS
	debugprintf("lineindex_fileopen: fname=%s\n",op->idxfname) ;
#endif

	if (op->fd < 0) {

#if	CF_DEBUGS
	debugprintf("lineindex_fileopen: need open\n") ;
#endif

	oflags = op->oflags ;
	oflags &= (~ (O_TRUNC | O_CREAT)) ;
	rs = u_open(op->idxfname,oflags,op->operm) ;

#if	CF_DEBUGS
	debugprintf("lineindex_fileopen: u_open() rs=%d\n",rs) ;
#endif

	f_create = (op->oflags & O_CREAT) ;

#if	CF_DEBUGS
	debugprintf("lineindex_fileopen: f_create=%u lfname=%s\n",
	    f_create,op->txtfname) ;
#endif

	if ((rs == SR_NOENT) && f_create && (op->txtfname != NULL)) {

	    if ((rs = lineindex_mkindex(op)) >= 0)
	        rs = u_open(op->idxfname,oflags,op->operm) ;

	}

	if (rs < 0)
	    goto bad0 ;

	op->fd = rs ;
	op->ti_open = dt ;

/* local or remote */

	rs = isfsremote(op->fd) ;
	f = (rs > 0) ;
	if (rs < 0)
	    goto bad1 ;

	f_changed = (! LEQUIV(f,op->f.remote)) ;
	if (f_changed)
	    op->f.remote = f ;

/* had the file itself changed? */

	if (rs >= 0) {

	    if ((rs = u_fstat(op->fd,&sb)) >= 0) {

	        if (! f_changed)
	            f_changed = (op->filesize != sb.st_size) ||
	                (op->mtime != sb.st_mtime) ;

	        op->filesize = (uint) sb.st_size ;
	        op->mtime = sb.st_mtime ;
	    }
	}

	} 

ret0:
	return (rs >= 0) ? f_changed : rs ;

/* bad things */
bad1:
	u_close(op->fd) ;
	op->fd = -1 ;

bad0:
	goto ret0 ;
}
/* end subroutine (lineindex_fileopen) */


static int lineindex_fileclose(op)
LINEINDEX	*op ;
{
	int		rs = SR_OK ;

	if (op->fd >= 0) {
	    rs = u_close(op->fd) ;
	    op->fd = -1 ;
	}

	return rs ;
}
/* end subroutine (lineindex_fileclose) */


static int lineindex_mkindex(op)
LINEINDEX	*op ;
{
	FILEMAP		lmap ;
	bfile		ifile ;
	offset_t	headoff ;
	uint		recoff, lineoff ;
	uint		table[header_overlast + 1] ;
	uint		recs[NRECS + 1] ;
	int		rs = SR_OK ;
	int		i, cl, len ;
	int		headsize, lines ;
	int		size ;
	const char	*cp ;
	char		dfname[MAXPATHLEN + 1] ;
	char		template[MAXPATHLEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("lineindex_mkindex: lfname=%s\n",op->txtfname) ;
#endif

/* determine if the directory is writable */

	if ((cl = sfdirname(op->idxfname,-1,&cp)) > 0) {
	    mkpath1w(dfname,cp,cl) ;
	} else {
	    mkpath1(dfname,".") ;
	}

#if	CF_DEBUGS
	debugprintf("lineindex_mkindex: dfname=%s\n",dfname) ;
#endif

	rs = perm(dfname,-1,-1,NULL,W_OK) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_FILEMAP
	rs = filemap_open(&lmap,op->txtfname,O_RDONLY,FILEMAPSIZE) ;
#else
	rs = bopen(&lfile,op->txtfname,"r",0666) ;

#if	CF_DEBUGS
	debugprintf("lineindex_mkindex: bopen() rs=%d\n",rs) ;
#endif

#endif /* CF_FILEMAP */

	if (rs < 0)
	    goto ret0 ;

/* make a temporary file for the index file */

	mkpath2(template,dfname,"liXXXXXXXXXXXX") ;

#if	CF_DEBUGS
	debugprintf("lineindex_mkindex: template=%s\n",template) ;
#endif

	rs = mktmpfile(tmpfname,op->operm,template) ;

#if	CF_DEBUGS
	debugprintf("lineindex_mkindex: mktmpfile() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret1 ;

#if	CF_DEBUGS
	debugprintf("lineindex_mkindex: tmpfname=%s\n",tmpfname) ;
#endif

	rs = bopen(&ifile,tmpfname,"wct",op->operm) ;

#if	CF_DEBUGS
	debugprintf("lineindex_mkindex: bopen() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret2 ;

/* write the header on the index file */

	{
	    int		fml = MIN(LINEINDEX_FILEMAGICLEN,15) ;
	    char	magicbuf[16 + 1], vetu[4] ;

	    strncpy(magicbuf,LINEINDEX_FILEMAGIC,16) ;
	    magicbuf[fml] = '\n' ;

	    vetu[0] = LINEINDEX_FILEVERSION ;
	    vetu[1] = LINEINDEX_ENDIAN ;
	    vetu[2] = LINEINDEX_FILETYPE ;
	    vetu[3] = 0 ;

	    rs = bwrite(&ifile,magicbuf,16) ;

	    if (rs >= 0)
	    rs = bwrite(&ifile,vetu,4) ;

#if	CF_DEBUGS
	    debugprintf("lineindex_mkindex: bwrite() rs=%d\n",rs) ;
#endif

	    headoff = 20 ;
	    headsize = header_overlast * sizeof(uint) ;
	    recoff = 20 + headsize ;

/* seek past the header */

	    bseek(&ifile,headsize,SEEK_CUR) ;

	} /* end block */

/* OK, start reading file and writing index */

	lines = 0 ;
	lineoff = 0 ;
	i = 0 ;
	while (rs >= 0) {
		const char	*lp ;

#if	CF_FILEMAP
		rs = filemap_getline(&lmap,&lp) ;
	    len = rs ;
		if (rs <= 0)
			break ;

#else /* CF_FILEMAP */
	{
	    char	linebuf[LINEBUFLEN+1] ;

		rs = breadline(&lfile,linebuf,LINEBUFLEN) ;
	    len = rs ;
		if (rs <= 0)
			break ;

	    if (linebuf[len - 1] != '\n') {
	        rs = bwasteline(&lfile,linebuf,LINEBUFLEN) ;
	        if (rs > 0) len += rs ;
	    }
	    }
#endif /* CF_FILEMAP */

#if	CF_DEBUGS && (! CF_FILEMAP)
	    {
	        int	ll = len ;
	        if (linebuf[ll - 1] == '\n')
	            ll -= 1 ;
	        debugprintf("lineindex_mkindex: line=%u off=%lu >%t<\n",
	            lines,lineoff,linebuf,MIN(ll,10)) ;
	    }
#endif /* CF_DEBUGS */

	    recs[i++] = htonl(lineoff) ;
	    if (i >= NRECS) {

	        size = i * sizeof(uint) ;
	        bwrite(&ifile,recs,size) ;

	        i = 0 ;
	    }

	    lineoff += len ;
	    lines += 1 ;

	} /* end while */

#if	CF_DEBUGS
	debugprintf("lineindex_mkindex: out-of-loop rs=%d i=%u\n",rs,i) ;
#endif

	if ((rs >= 0) && (i > 0)) {

	    size = i * sizeof(uint) ;
	    rs = bwrite(&ifile,recs,size) ;

#if	CF_DEBUGS
	    {
	        struct ustat	sb ;
	        debugprintf("lineindex_mkindex: final "
			"wsize=%u bwrite() rs=%d\n",
	            size,rs) ;
	        bflush(&ifile) ;
	        u_stat(tmpfname,&sb) ;
	        debugprintf("lineindex_mkindex: filesize=%lu\n",
			sb.st_size) ;
	    }
#endif /* CF_DEBUGS */

	}

/* seek back and write the header */

	if (rs >= 0) {
	    time_t	dt = time(NULL) ;


#if	CF_TESTMODE
	    dt = 0 ;
#endif

	    memset(table,0,headsize) ;

	    table[header_rectab] = htonl(recoff) ;
	    table[header_lines] = htonl(lines) ;

#if	CF_TESTMODE
	    table[header_wtime] = 0 ;
#else
	    table[header_wtime] = htonl(dt) ;
#endif

#if	CF_DEBUGS
	    debugprintf("lineindex_mkindex: headoff=%lu\n",headoff) ;
#endif

	    if ((rs = bseek(&ifile,headoff,SEEK_SET)) >= 0)
	        rs = bwrite(&ifile,table,headsize) ;

	} /* end block */

#if	CF_DEBUGS
	{
	    offset_t	off ;
	    bseek(&ifile,0L,SEEK_END) ;
	    btell(&ifile,&off) ;
	    debugprintf("lineindex_mkindex: index filesize=%lu\n",off) ;
	}
#endif /* CF_DEBUGS */

ret3:
	bclose(&ifile) ;

	if (rs >= 0) {

#if	CF_DEBUGS
	    debugprintf("lineindex_mkindex: rename tmpfname=%s\n",tmpfname) ;
	    debugprintf("lineindex_mkindex: rename fname=%s\n",op->idxfname) ;
#endif

	    rs = u_rename(tmpfname,op->idxfname) ;

#if	CF_DEBUGS
	    debugprintf("lineindex_mkindex: u_rename() rs=%d\n",rs) ;
#endif

	    if (rs >= 0)
		tmpfname[0] = '\0' ;

	} /* end if */

ret2:
	if (tmpfname[0] != '\0')
	    u_unlink(tmpfname) ;

ret1:

#if	CF_FILEMAP
	filemap_close(&lmap) ;
#else
	bclose(&lfile) ;
#endif /* CF_FILEMAP */

ret0:
	return rs ;
}
/* end subroutine (lineindex_mkindex) */


