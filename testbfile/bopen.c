/* bopen */

/* "Basic I/O" package */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0	/* compile-time debug print-outs */
#define	CF_UNIXAPPEND	1	/* BRAINDAMAGED_IN_UNIX (on NFS mounted FSs) */
#define	CF_MAPABLE	0	/* allow mapped files */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


	= 1999-01-10, David A­D­ Morano

	I added the little extra code to allow for memory mapped I/O.
	It is all a waste because it is way slower than without it!
	This should teach me to leave old programs alone!!!


*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This code piece provides for the basic "open" and "close"
	functions for the BFILE I/O library.

	The following global functions are made available from
	this code piece:

	- bopen
	- bopenprog
	- bclose


*******************************************************************************/


#define	BFILE_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/resource.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#define	BDESC		BFILE_BD

#define	BO_READ		(1<<0)
#define	BO_WRITE	(1<<1)
#define	BO_APPEND	(1<<2)
#define	BO_FILEDESC	(1<<3)

#undef	FLBUFLEN
#define	FLBUFLEN	100

#ifndef	MKCHAR
#define	MKCHAR(c)	((c) & 0xff)
#endif


/* external subroutines */

extern LONG	llceil(LONG,int) ;

extern int	snopenflags(char *,int,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	findfilepath(const char *,char *,const char *,int) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */

static int	bfile_bufbegin(bfile *,int) ;
static int	bfile_bufend(bfile *) ;

static int	bfile_opts(bfile *,int,mode_t) ;

#if	CF_MAPABLE
static int	bfile_mapbegin(bfile *) ;
static int	bfile_mapend(bfile *) ;
#endif /* CF_MAPABLE */

static int	mkoflags(const char *,int *) ;
static int	extfd(const char *) ;


/* local variables */


/* exported subroutines */


int bopene(fp,name,os,perm,timeout)
bfile		*fp ;
const char	name[] ;
const char	os[] ;
mode_t		perm ;
int		timeout ;
{
	BFILE_STAT	sb ;
	offset_t	soff ;
	const int	maxopenfile = getdtablesize() ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		fd_existing = -1 ;
	int		boflags = 0 ;
	int		oflags = 0 ;
	int		bsize = 0 ;
	int		f_readonly = FALSE ;
	int		f_lessthan ;
	int		f_mappable ;
	int		f_notseek ;
	int		f_created = FALSE ;
	int		f ;

#if	CF_DEBUGS
	{
	    int	f_lf = FALSE ;
#if	BFILE_LARGEFILE
	    f_lf = TRUE ;
#endif
	    debugprintf("bopen: entered lf=%u\n",f_lf) ;
	}
#endif /* CF_DEBUGS */

	if (fp == NULL) return SR_FAULT ;
	if (os == NULL) return SR_FAULT ;

	if (os[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("bopene: stat-size=%u\n",sizeof(struct ustat)) ;
	debugprintf("bopene: ustat-size=%u\n",sizeof(struct ustat)) ;
	debugprintf("bopene: offset-size=%u\n",sizeof(offset_t)) ;
	debugprintf("bopene: ino-size=%u\n",sizeof(ino_t)) ;
	debugprintf("bopene: dev-size=%u\n",sizeof(dev_t)) ;
	debugprintf("bopene: mode-size=%u\n",sizeof(mode_t)) ;
	debugprintf("bopene: LONG-size=%u\n",sizeof(LONG)) ;
	debugprintf("bopene: BFILEOFF-size=%u\n",sizeof(BFILE_OFF)) ;
	debugprintf("bopene: BFILESTAT-size=%u\n",sizeof(BFILE_STAT)) ;
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	debugprintf("bopen: BFILE-size=%u\n",sizeof(bfile)) ;
#endif

	memset(fp,0,sizeof(bfile)) ;
	fp->fd = -1 ;

#if	CF_DEBUGS
	debugprintf("bopen: oflags=>%t<\n",os,strlinelen(os,-1,40)) ;
#endif

	oflags = mkoflags(os,&boflags) ;

#if	CF_DEBUGS
	debugprintf("bopen: mkoflags() ret\n") ;
 	    debugprintf("bopen: file oflags=%08x\n",oflags) ;
 	    debugprintf("bopen: file boflags=%08x\n",boflags) ;
	{
	    char	flbuf[FLBUFLEN+1] ;
	    snopenflags(flbuf,FLBUFLEN,oflags) ;
	    debugprintf("bopen: oflags=%s\n",flbuf) ;
	}
#endif /* CF_DEBUGS */

/* we make "exclusive" imply "create" also! */

	if (oflags & O_EXCL)
	    oflags |= O_CREAT ;

	if ((oflags & O_ACCMODE) == O_RDONLY) f_readonly = TRUE ;

#ifdef	COMMENT
	if ((boflags & BO_READ) && (boflags & BO_WRITE)) {
	    oflags |= O_RDWR ;
	} else if (boflags & BO_WRITE) {
	    oflags |= O_WRONLY ;
	} else
	    oflags |= O_RDONLY ;
#endif /* COMMENT */

#ifdef	COMMENT
	if ((oflags & O_WRONLY) && (! (oflags & O_APPEND)))
	    oflags |= (O_CREAT | O_TRUNC) ;
#endif

	fp->pagesize = getpagesize() ;
	bsize = (fp->pagesize * BFILE_BUFPAGES) ;

	f_lessthan = (((ulong) name) < maxopenfile) ;
	if (f_lessthan)
	    fd_existing = ((int) name) ;

#if	CF_DEBUGS
	debugprintf("bopen: fd-trick=%u fd=%u\n",f_lessthan,fd_existing) ;
#endif

	if (f_lessthan && (! (boflags & BO_FILEDESC))) {
	    rs = SR_FAULT ;
	    goto bad0 ;
	}

	if (! f_lessthan) {
	    int i ;
#if	CF_DEBUGS
	    debugprintf("bopen: fname=>%s<\n",name) ;
#endif
	    if ((i = matstr(bfile_fnames,name,-1)) >= 0) {
		if (i == BFILE_IDXSTDNULL) {
		    fp->f.nullfile = TRUE ;
		    goto ret1 ;
		}
		boflags |= BO_FILEDESC ;
		fd_existing = i ;
	    } else if (name[0] == BFILE_FDCH) {
		if ((i = extfd(name)) >= 0) {
		    boflags |= BO_FILEDESC ;
		    fd_existing = i ;
	 	}
	    }
	} /* end if */

	if (boflags & BO_READ) {
	    boflags &= (~ BO_APPEND) ;
	    oflags &= (~ O_APPEND) ;
	}

/* OK, go! */

	if (boflags & BO_FILEDESC) {
	    int	ooflags ;
	    int	f_setflags = FALSE ;
	    int	f_addappend = FALSE ;

#if	CF_DEBUGS
	    debugprintf("bopen: special file specified fd=%u\n",fd_existing) ;
#endif

	    rs = u_dup(fd_existing) ;
#if	CF_DEBUGS
	    debugprintf("bopen: u_dup() rs=%d\n",rs) ;
#endif
	    fp->fd = rs ;
	    if (rs < 0) goto bad0 ;

#if	CF_DEBUGS
	    debugprintf("bopen: special file nfd=%d\n",fp->fd) ;
#endif

	    rs = u_fcntl(fp->fd,F_GETFL,0) ;
	    ooflags = rs ;

	    if (rs >= 0) {
		if ((oflags & O_NDELAY) && (! (ooflags & O_NDELAY))) {
		    ooflags |= O_NDELAY ;
		    f_setflags = TRUE ;
		}
		if ((oflags & O_NONBLOCK) && (! (ooflags & O_NONBLOCK))) {
		    ooflags |= O_NONBLOCK ;
		    f_setflags = TRUE ;
		}
	    }

#if	CF_DEBUGS
 	    debugprintf("bopen: file oflags=%08x\n",ooflags) ;
 	    debugprintf("bopen: file boflags=%08x\n",boflags) ;
	{
	    char	flbuf[FLBUFLEN+1] ;
	    snopenflags(flbuf,FLBUFLEN,ooflags) ;
	    debugprintf("bopen: ooflags=%s\n",flbuf) ;
	}
#endif /* CF_DEBUGS */

/* if caller asked for "append", give it to her -- if even artificially */

	    f_addappend = (boflags & BO_APPEND) && (! (boflags & BO_READ)) ;

#if	CF_UNIXAPPEND
	    if ((rs >= 0) && (f_addappend && (! (oflags & O_APPEND)))) {

#if	CF_DEBUGS
	        debugprintf("bopen: setting APPEND flag\n") ;
#endif

	        ooflags |= O_APPEND ;
		f_setflags = TRUE ;
	    }

#else /* CF_UNIXAPPEND */

	    if ((rs >= 0) && f_addappend) {
	        ooflags |= O_APPEND ;
		f_setflags = TRUE ;
	    }

#endif /* CF_UNIXAPPEND */

	    if ((rs >= 0) && f_setflags)
		rs = u_fcntl(fp->fd,F_SETFL,ooflags) ;

	    oflags = ooflags ;

	} else {

#if	CF_DEBUGS
	    debugprintf("bopen: opening normally fname=%s\n",name) ;
#endif

	    if (oflags & O_CREAT) {
	        rs = bfilestat(name,0,&sb) ;
	        if (rs == SR_NOENT) {
		    rs = SR_OK ;
		    fp->f.created = TRUE ;
		}
	    }

#if	CF_DEBUGS
	    if (oflags & O_APPEND)
	        debugprintf("bopen: 1 in append mode\n") ;
#endif

#if	CF_DEBUGS
	    debugprintf("bopen: uc_opene fname=%s\n",name) ;
#endif

	    oflags |= O_LARGEFILE ;

	    if ((rs = uc_opene(name,oflags,perm,timeout)) >= 0) {
	        fp->fd = rs ;
		rs = bfile_opts(fp,oflags,perm) ;
	        if (rs < 0) {
		    u_close(fp->fd) ;
		    fp->fd = -1 ;
	        }
	    }

#if	CF_DEBUGS
	    debugprintf("bopen: uc_openx() rs=%d\n",rs) ;
#endif

	    if (rs < 0) goto bad0 ;

#if	CF_DEBUGS
	    {
	        int	tmpflags ;
	        tmpflags = u_fcntl(fp->fd,F_GETFL,0) ;
	        if (tmpflags & O_APPEND)
	            debugprintf("bopen: 2 in append mode\n") ;
	    }
#endif /* CF_DEBUGS */

	} /* end if (opening the new FD) */

	if (rs < 0) goto bad1 ;

#if	CF_DEBUGS
	debugprintf("bopen: file opened FD=%d\n",fp->fd) ;
#endif

/* OK, we had our fun, now set all of the proper other modes for this file */

	fp->oflags = oflags ;
	rs = bfilefstat(fp->fd,&sb) ;
	if (rs < 0) goto bad1 ;

#if	CF_DEBUGS
	{
	    struct ustat	lsb ;
	    LONG	lv = -1 ;
	     debugprintf("bopen: lv=%lld\n",lv) ;
#if	BFILE_LARGEFILE
	     debugprintf("bopen: st_size=%llu\n",sb.st_size) ;
		rs1 = u_fstat(fp->fd,&lsb) ;
	     debugprintf("bopen: u_fstat() rs=%d st_size=%lld\n",
		rs1,lsb.st_size) ;
		rs1 = fstat(fp->fd,&lsb) ;
	     debugprintf("bopen: fstat() rs=%d st_size=%lld\n",
		rs1,lsb.st_size) ;
#else
	    debugprintf("bopen: st_size=%ld\n",sb.st_size) ;
#endif
	}
#endif /* CF_DEBUGS */

#ifdef	COMMENT
	fp->fsize = sb.st_size ;
#else
	fp->fsize = 0 ;
#endif

	fp->mode = sb.st_mode ;
	fp->ino = sb.st_ino ;
	fp->dev = sb.st_dev ;
	f_notseek = TRUE ;
	f = FALSE ;
	f = f || S_ISREG(sb.st_mode) ;
	f = f || S_ISDIR(sb.st_mode) ;
	f = f || S_ISBLK(sb.st_mode) ;
	if (f) {
	    if (f_readonly) {
	        LONG	ps = fp->pagesize ;
	        LONG	fs = sb.st_size ;
	        LONG	cs ;
		if (fs == 0) fs = 1 ;
	        cs = llceil(fs,512) ;
		bsize = MIN(fs,ps) & INT_MAX ;
#if	CF_DEBUGS
	    debugprintf("bopen: RDONLY fs=%lld ps=%llu bsize=%u\n",
		fs,ps,bsize) ;
#endif
	    }
	    f_notseek = FALSE ;
	} else if (S_ISFIFO(sb.st_mode)) {
	    bsize = MIN(LINEBUFLEN,2048) ;
	    fp->bm = bfile_bmline ;
	    fp->f.slow = TRUE ;
	} else if (S_ISCHR(sb.st_mode)) {
	    if (isatty(fp->fd)) {
	        bsize = MIN(LINEBUFLEN,2048) ;
	        fp->f.terminal = TRUE ;
	        fp->bm = bfile_bmline ;
	    } /* end if (is a terminal) */
	    fp->f.slow = TRUE ;
	} else if (S_ISSOCK(sb.st_mode)) {
	    fp->f.network = TRUE ;
	    bsize = (64*1024) ;
	    fp->bm = bfile_bmline ;
	    fp->f.slow = TRUE ;
	} else {
	    bsize = MIN(LINEBUFLEN,2048) ;
	    fp->bm = bfile_bmline ;
	    fp->f.slow = TRUE ;
	}

	if (! f_notseek) {
	    rs1 = u_seeko(fp->fd,0L,SEEK_CUR,&soff) ;
	    fp->offset = (BFILE_OFF) soff ;
	    f_notseek = (rs1 < 0) ;
	}

	if (f_notseek) {
	    fp->f.notseek = TRUE ;
	    fp->offset = 0 ;
	}

	fp->len = 0 ;
	if ((oflags & O_APPEND) && (! fp->f.notseek)) {
	    rs = u_seeko(fp->fd,0L,SEEK_END,&soff) ;
	    fp->offset = (BFILE_OFF) soff ;
	} /* end if (append mode) */

	fp->maps = NULL ;

#if	CF_DEBUGS
	debugprintf("bopen: mapping?\n") ;
#endif

#if	CF_MAPABLE
	f_mappable = (! fp->f.notseek) && (! fp->f.terminal) ;
	if (f_mappable) {
	    fp->f.mappable = TRUE ;
	}
	if (f_mappable && (! (oflags & O_WRONLY))) {
	    rs = bfile_mapbegin(fp) ;
	    if (rs < 0) goto bad1 ;
	} /* end if (file object is mappable) */
#else /* CF_MAPABLE */
#if	CF_DEBUGS
	debugprintf("bopen: buffable bsize=%u\n",bsize) ;
#endif
	rs = bfile_bufbegin(fp,bsize) ;
#endif /* CF_MAPABLE */

	if (rs < 0) goto bad1 ;

	if ((oflags & O_CREAT) && fp->f.created) f_created = TRUE ;

ret1:
	fp->magic = BFILE_MAGIC ;		/* set magic number */

ret0:

#if	CF_DEBUGS
	debugprintf("bopen: ret rs=%d f_c=%u fd=%u\n",
		rs,f_created,fp->fd) ;
#endif

	return (rs >= 0) ? f_created : rs ;

/* bad things come here */
bad1:
	if (fp->fd >= 0) {
	    u_close(fp->fd) ;
	    fp->fd = -1 ;
	}
	if (fp->maps != NULL) {
	    uc_free(fp->maps) ;
	    fp->maps = NULL ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (bopene) */


int bopen(fp,fname,os,perm)
bfile		*fp ;
const char	fname[] ;
const char	os[] ;
mode_t		perm ;
{


	return bopene(fp,fname,os,perm,-1) ;
}
/* end subroutine (bopen) */


int bopenprog(fp,progname,os,argv,envv)
bfile		*fp ;
const char	*progname ;
const char	*os ;
const char	**argv ;
const char	**envv ;
{
	int	rs = SR_OK ;
	int	oflags = 0 ;
	int	boflags = 0 ;

	const char	*args[2] ;

	char	progfname[MAXPATHLEN+1] ;


	if (fp == NULL) return SR_FAULT ;
	if (progname == NULL) return SR_FAULT ;
	if (os == NULL) return SR_FAULT ;

	if (progname[0] == '\0') return SR_INVALID ;
	if (os[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("bopenprog: progname=%s\n",progname) ;
#endif

	memset(fp,0,sizeof(bfile)) ;
	fp->pagesize = getpagesize() ;
	fp->fd = -1 ;
	fp->f.notseek = TRUE ;

	oflags = mkoflags(os,&boflags) ;
	oflags |= O_CLOEXEC ;

	if (argv == NULL) {
	    int	i = 0 ;
	    argv = args ;
	    args[i++] = progname ;
	    args[i] = NULL ;
	}

	if (strchr(progname,'/') == NULL) {
	    rs = findfilepath(NULL,progfname,progname,X_OK) ;
	    if (rs > 0) progname = progfname ;
	}

	if (rs < 0) goto ret0 ;

#if	CF_DEBUGS
	{
		char	obuf[FLBUFLEN+1] ;
		snopenflags(obuf,FLBUFLEN,oflags) ;
		debugprintf("bopenprog: progname=%s\n",progname) ;
		debugprintf("bopenprog: of=%s\n",obuf) ;
	}
#endif /* CF_DEBUGS */

	if ((rs = uc_openprog(progname,oflags,argv,envv)) >= 0) {
	    fp->fd = rs ;
	    if ((rs = bfile_opts(fp,oflags,0)) >= 0) {
		const int	bsize = MIN(LINEBUFLEN,2048) ;
		if ((rs = bfile_bufbegin(fp,bsize)) >= 0) {
		    fp->oflags = oflags ;
		    fp->bm = bfile_bmline ;
		    fp->magic = BFILE_MAGIC ;
		} /* end if (buffer-allocation) */
	    }
	    if (rs < 0) {
		u_close(fp->fd) ;
		fp->fd = -1 ;
	    }
	} /* end if (opened) */

ret0:

#if	CF_DEBUGS
	debugprintf("bopenprog: ret rs=%d\n") ;
#endif

	return rs ;
}
/* end subroutine (bopenprog) */


int bclose(fp)
bfile	*fp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (fp == NULL)
	    return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC)
	    return SR_NOTOPEN ;

	if (fp->f.nullfile) goto ret1 ;

#if	CF_DEBUGS
	debugprintf("bclose: f_w=%u blen=%u\n",fp->f.write,fp->len) ;
#endif

	if (fp->f.write && (fp->len > 0)) {
	    rs1 = bfile_flush(fp) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("bclose: bfile_flush() rs=%d\n",rs) ;
#endif

	if (fp->bdata != NULL) {
	    rs1 = bfile_bufend(fp) ;
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGS
	debugprintf("bclose: bfile_bufend() rs=%d\n",rs) ;
#endif

#if	CF_MAPABLE
	if (fp->maps != NULL) {
	    rs1 = bfile_mapend(fp) ;
	    if (rs >= 0) rs = rs1 ;
	}
#endif /* CF_MAPABLE */

#if	CF_DEBUGS
	debugprintf("bclose: bfile_mapend() rs=%d\n",rs) ;
#endif

	rs1 = u_close(fp->fd) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("bclose: u_close() rs=%d\n",rs) ;
#endif

ret1:
	fp->magic = 0 ;

ret0:

#if	CF_DEBUGS
	debugprintf("bclose: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bclose) */


/* private subroutines */


static int bfile_bufbegin(bfile *fp,int bsize)
{
	int		rs ;
	char		*p ;

	if (fp->pagesize == 0) fp->pagesize = getpagesize() ;

	if (bsize == 0) bsize = fp->pagesize ;

	if ((rs = uc_malloc(bsize,&p)) >= 0) {
	    fp->bdata = p ;
	    fp->bsize = bsize ;
	    fp->bbp = p ;
	    fp->bp = p ;
	}

	return rs ;
}
/* end subroutine (bfile_bufbegin) */


static int bfilebd_start(BFILE_bfile *fp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (fp->bdata != NULL) {
	    rs1 = uc_free(fp->bdata) ;
	    if (rs >= 0) rs = rs1 ;
	    fp->bdata = NULL ;
	    fp->bbp = NULL ;
	    fp->bp = NULL ;
	    fp->bsize = 0 ;
	}

	return rs ;
}
/* end subroutine (bfile_bdend) */


static int bfile_opts(bfile *fp,int oflags,mode_t om)
{
	int		rs ;

	rs = uc_closeonexec(fp->fd,TRUE) ;

	if ((rs >= 0) && (oflags & O_MINMODE) && (om > 0)) {
	    rs = uc_fminmod(fp->fd,om) ;
	}

	if (oflags & O_NETWORK) fp->f.network = TRUE ;

	return rs ;
}
/* end subroutine (bfile_opts) */


#if	CF_MAPABLE

static int bfile_mapbegin(fp)
bfile	*fp ;
{
	int		rs ;
	int		i ;
	int		size ;

/* allocate the map structures */

	size = BFILE_NMAPS * sizeof(struct bfile_map) ;
	rs = uc_malloc(size,&fp->maps) ;
	if (rs < 0)
	    goto ret0 ;

#ifdef	MALLOCLOG
	malloclog_alloc(fp->maps,size,"bopen/bfile_mapbegin:maps") ;
#endif

/* initialize the maps, let them get paged in as needed! */

	for (i = 0 ; i < BFILE_NMAPS ; i += 1) {

	    fp->maps[i].f.valid = FALSE ;
	    fp->maps[i].buf = NULL ;

	} /* end for */

	fp->bp = NULL ;
	fp->f.mapped = TRUE ;

ret0:
	return rs ;
}
/* end subroutine (bfile_mapbegin) */


static int bfile_mapend(fp)
bfile	*fp ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; i < BFILE_NMAPS ; i += 1) {
	    if ((fp->maps[i].f.valid) && (fp->maps[i].buf != NULL)) {
	        rs1 = u_munmap(fp->maps[i].buf,(size_t) fp->pagesize) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	rs1 = uc_free(fp->maps) ;
	if (rs >= 0) rs = rs1 ;
	fp->maps = NULL ;

	return rs ;
}
/* end subroutine (bfile_mapend) */

#endif /* CF_MAPABLE */


static int mkoflags(const char *os,int *bfp)
{
	int		oflags = 0 ;
	int		bflags = 0 ;

#if	CF_DEBUGS
	    debugprintf("bopen/mkoflags: os=>%s<\n",os) ;
#endif

	while (*os) {
	    int	sc = (*os++ & 0xff) ;

#if	CF_DEBUGS
	    debugprintf("bopen/mkoflags: fl=>%c<\n",sc) ;
#endif

	    switch (sc) {

	    case 'd':
	        bflags |= BO_FILEDESC ;
	        break ;

	    case 'r':
	        bflags |= BO_READ ;
	        break ;

	    case 'w':
		bflags |= BO_WRITE ;
	        break ;

	    case 'm':
	    case '+':
	        bflags |= (BO_READ | BO_WRITE) ;
	        break ;

	    case 'a':
	        oflags |= O_APPEND ;
	        bflags |= BO_APPEND ;
	        break ;

	    case 'c':
	        oflags |= O_CREAT ;
	        break ;

	    case 'e':
	        oflags |= (O_CREAT | O_EXCL) ;
	        break ;

	    case 't':
	        oflags |= (O_CREAT | O_TRUNC) ;
	        break ;

	    case 'n':
	        oflags |= O_NDELAY ;
	        break ;

/* POSIX "binary" mode -- does nothing on real UNIXes® */
	    case 'b':
	        break ;

	    case 'x':
	        oflags |= O_EXCL ;
		break ;

	    case 'N':
	        oflags |= O_NETWORK ;
		break ;

	    case 'M':
	        oflags |= O_MINMODE ;
		break ;

	    } /* end switch */

	} /* end while (open flags) */

#if	CF_DEBUGS
	    debugprintf("bopen/mkoflags: bflags=%04x\n",bflags) ;
#endif

	if (bflags & BO_WRITE)
	   oflags |= ((bflags & BO_READ) ? O_RDWR : O_WRONLY) ;

	if (bfp != NULL) *bfp = bflags ;

#if	CF_DEBUGS
	{
	    char	flbuf[FLBUFLEN+1] ;
	    snopenflags(flbuf,FLBUFLEN,oflags) ;
	    debugprintf("bopen/mkoflags: ret oflags=%s\n",flbuf) ;
	}
#endif

	return oflags ;
}
/* end subroutine (mkoflags) */


#if	CF_DEBUGS

int bfile_amodes(fp,name)
bfile	*fp ;
char	name[] ;
{
	int		oflags ;
	char		amodes[100] ;

	amodes[0] = '\0' ;
	oflags = u_fcntl(fp->fd,F_GETFL,0) ;

	if ((oflags & O_CREAT) == O_CREAT)
	    strcat(amodes," APPEND") ;

	if ((oflags & O_TRUNC) == O_TRUNC)
	    strcat(amodes," TRUNC") ;

	if ((oflags & O_APPEND) == O_APPEND)
	    strcat(amodes," APPEND") ;

	if (amodes[0] != '\0')
	    debugprintf("%s: FD=%d offset=%08lx amodes=%s\n",
	        name,fp->fd,fp->offset,amodes) ;

	return oflags ;
}
/* end subroutine (bfile_amodes) */

#endif /* CF_DEBUGS */


static int extfd(const char *s) 
{
	int		rs = SR_INVALID ;
	int		fd = -1 ;
	if (*s++ == BFILE_FDCH) {
	   const int	ch = MKCHAR(s[0]) ;
	   if (isdigit(ch)) rs = cfdeci(s,-1,&fd) ;
	}
	return (rs >= 0) ? fd : rs ;
}
/* end if (extfd) */


