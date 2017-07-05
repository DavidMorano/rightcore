/* srvreg */

/* manage reading or writing of a server registry file */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		1
#define	CF_CREAT	0		/* always create the file? */


/* revision history:

	= 1998-08-22, David A­D­ Morano
        This subroutine module was adopted for use from some previous code that
        performed the similar sorts of functions.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module manages the reading and writing of entries to or from a
        server registry file. A server registry file is where (system?) servers
        register themselves so that clients know how to communicate with them.


*******************************************************************************/


#define	SRVREG_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<inttypes.h>
#include	<limits.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"srvreg.h"
#include	"srvrege.h"


/* local defines */

#define	SRVREG_FLID		(16 + 4)
#define	SRVREG_FLHEAD		(3 * 4)
#define	SRVREG_FLTOP		(SRVREG_FLID + SRVREG_FLHEAD)

#define	SRVREG_FOID		0
#define	SRVREG_FOHEAD		(16 + 4)
#define	SRVREG_FOTAB		(SRVREG_FOHEAD + (3 * 4))

#define	SRVREG_ENTSIZE	SRVREGE_SIZE
#define	SRVREG_EBS		((SRVREGE_SIZE + 3) & (~ 3))
#define	SRVREG_MAXFILESIZE	(4 * 1024 * 1024)

#define	SRVREG_BUFSIZE		(64 * 1024)
#define	SRVREG_READSIZE		(16 * 1024)

#define	TO_OPEN		(60 * 60)	/* maximum file-open time */
#define	TO_ACCESS	(2 * 60)	/* maximum access idle time */
#define	TO_LOCK		10		/* seconds */

#define	SRVREG_OPENTIME	30		/* seconds */
#define	TI_MINUPDATE	4		/* minimum time between updates */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	NENTRIES	100
#define	FBUFLEN		(SRVREG_FLTOP + 9)

#ifndef	ENDIAN
#if	defined(OSNAME_SunOS) && defined(__sparc)
#define	ENDIAN		1
#else
#ifdef	_BIG_ENDIAN
#define	ENDIAN		1
#endif
#ifdef	_LITTLE_ENDIAN
#define	ENDIAN		0
#endif
#ifndef	ENDIAN
#error	"could not determine endianness of this machine"
#endif
#endif
#endif


/* external subroutines */

extern uint	uceil(uint,int) ;

extern int	matstr(const char **,const char *,int) ;
extern int	lockfile(int,int,offset_t,offset_t,int) ;
extern int	isfsremote(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

int		srvreg_close(SRVREG *) ;

static int	srvreg_fileopen(SRVREG *,time_t) ;
static int	srvreg_fileclose(SRVREG *) ;
static int	srvreg_lockget(SRVREG *,time_t,int) ;
static int	srvreg_lockrelease(SRVREG *) ;
static int	srvreg_fileinit(SRVREG *,time_t) ;
static int	srvreg_filechanged(SRVREG *) ;
static int	srvreg_filecheck(SRVREG *,time_t,int) ;
static int	srvreg_buf(SRVREG *,uint,uint,char **) ;
static int	srvreg_bufupdate(SRVREG *,uint,int,const char *) ;
static int	srvreg_bufinit(SRVREG *) ;
static int	srvreg_buffree(SRVREG *) ;
static int	srvreg_writehead(SRVREG *) ;

static int	filemagic(char *,int,struct srvreg_filemagic *) ;
static int	filehead(char *,int,struct srvreg_filehead *) ;


/* local variables */

#ifdef	COMMENT

static const char	*aitypes[] = {
	"empty",
	"disabled",
	"fifo",
	"pipe",
	"fmq",
	"tlistreamu",
	"tlistreamo",
	"tlidgram",
	"sockstream",
	"sockdgram",
	"pmq",
	NULL
} ;

#endif /* COMMENT */


/* exported subroutines */


int srvreg_open(op,fname,oflags,operm)
SRVREG		*op ;
const char	fname[] ;
int		oflags ;
int		operm ;
{
	struct ustat	sb ;

	time_t	daytime = time(NULL) ;

	int	rs ;
	int	amode ;
	int	f_create = FALSE ;


#if	CF_DEBUGS
	debugprintf("srvreg_open: ent fname=%s\n",fname) ;
#endif

#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;
#endif /* CF_SAFE */

	if ((fname == NULL) || (fname[0] == '\0'))
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("srvreg_open: oflags=%4o fname=%s\n",
	    oflags,fname) ;
	if (oflags & O_CREAT)
	    debugprintf("srvreg_open: creating as needed\n") ;
#endif

	memset(op,0,sizeof(SRVREG)) ;

	op->magic = 0 ;
	op->fname = NULL ;

#if	CF_CREAT
	oflags |= O_CREAT ;
#endif

	oflags = (oflags &= (~ O_TRUNC)) ;

	op->oflags = oflags ;
	op->operm = operm ;

	rs = uc_mallocstrw(fname,-1,&op->fname) ;
	if (rs < 0)
	    goto bad0 ;

	op->mtime = 0 ;
	memset(&op->f,0,sizeof(struct srvreg_flags)) ;

/* initialize the buffer structure */

	rs = srvreg_bufinit(op) ;
	if (rs < 0)
	    goto bad1 ;

/* try to open the file */

	oflags = (oflags & (~ O_CREAT)) ;
	rs = u_open(op->fname,oflags,operm) ;
	op->fd = rs ;

#if	CF_DEBUGS
	debugprintf("srvreg_open: u_open() rs=%d\n",rs) ;
#endif

	if ((rs < 0) && (op->oflags & O_CREAT)) {

	    f_create = TRUE ;
	    oflags = op->oflags ;
	    rs = u_open(op->fname,oflags,operm) ;
	    op->fd = rs ;

#if	CF_DEBUGS
	    debugprintf("srvreg_open: u_open() rs=%d\n",rs) ;
#endif

	} /* end if (creating file) */

	if (rs < 0)
	    goto bad2 ;

	amode = (oflags & O_ACCMODE) ;
	op->f.writable = ((amode == O_WRONLY) || (amode == O_RDWR)) ;

#if	CF_DEBUGS
	debugprintf("srvreg_open: f_writable=%d\n",op->f.writable) ;
#endif

	op->opentime = daytime ;
	op->accesstime = daytime ;
	rs = u_fstat(op->fd,&sb) ;

#if	CF_DEBUGS
	debugprintf("srvreg_open: u_fstat() rs=%d\n",rs) ;
	debugprintf("srvreg_open: f_size=%08x\n",sb.st_size) ;
	debugprintf("srvreg_open: f_mtime=%08x\n",sb.st_mtime) ;
#endif

	if (rs < 0)
	    goto bad3 ;

	op->mtime = sb.st_mtime ;
	op->filesize = sb.st_size ;
	op->pagesize = getpagesize() ;

/* local or remote */

	rs = isfsremote(op->fd) ;
	op->f.remote = (rs > 0) ;
	if (rs < 0)
	    goto bad3 ;

/* header processing */

	rs = srvreg_fileinit(op,daytime) ;

#if	CF_DEBUGS
	debugprintf("srvreg_open: srvreg_fileinit() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad3 ;

/* out of here */

#if	CF_DEBUGS
	debugprintf("srvreg_open: ret rs=%d\n",rs) ;
#endif

	op->magic = SRVREG_MAGIC ;

ret0:
	return (rs >= 0) ? f_create : rs ;

/* bad things */
bad4:
bad3:
	if (op->fd >= 0) {
	    u_close(op->fd) ;
	    op->fd = -1 ;
	}

bad2:
	srvreg_buffree(op) ;

bad1:
	if (op->fname != NULL) {
	    uc_free(op->fname) ;
	    op->fname = NULL ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (srvreg_open) */


int srvreg_close(op)
SRVREG		*op ;
{


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SRVREG_MAGIC)
	    return SR_NOTOPEN ;
#endif /* CF_SAFE */

	srvreg_buffree(op) ;

	if (op->fd >= 0) {
	    u_close(op->fd) ;
	    op->fd = -1 ;
	}

	if (op->fname != NULL) {
	    uc_free(op->fname) ;
	    op->fname = NULL ;
	}

	op->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (srvreg_close) */


/* get a count of the number of entries */
int srvreg_count(op)
SRVREG		*op ;
{
	int	rs = SR_OK ;
	int	c ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SRVREG_MAGIC)
	    return SR_NOTOPEN ;
#endif /* CF_SAFE */

	c = (op->filesize - SRVREG_FOTAB) / SRVREG_ENTSIZE ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (srvreg_count) */


/* initialize a cursor */
int srvreg_curbegin(op,cp)
SRVREG		*op ;
SRVREG_CUR	*cp ;
{


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SRVREG_MAGIC)
	    return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (cp == NULL)
	    return SR_FAULT ;

	op->cursors += 1 ;

	op->f.cursorlockbroken = FALSE ;
	op->f.cursoracc = FALSE ;
	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (srvreg_curbegin) */


/* free up a cursor */
int srvreg_curend(op,cp)
SRVREG		*op ;
SRVREG_CUR	*cp ;
{
	time_t	daytime ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SRVREG_MAGIC)
	    return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (cp == NULL)
	    return SR_FAULT ;

	if (op->f.cursoracc) {

	    daytime = time(NULL) ;

	    op->accesstime = daytime ;

	} /* end if */

	if (op->cursors > 0)
	    op->cursors -= 1 ;

	if ((op->cursors == 0) && (op->f.readlocked || op->f.writelocked))
	    srvreg_lockrelease(op) ;

	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (srvreg_curend) */


/* enumerate the entries */
int srvreg_enum(op,cup,ep)
SRVREG		*op ;
SRVREG_CUR	*cup ;
SRVREG_ENT	*ep ;
{
	time_t	daytime = 0 ;

	uint	ebs = uceil(SRVREG_ENTSIZE,4) ;
	uint	eoff ;

	int	rs ;
	int	ei ;

	char	*bp ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SRVREG_MAGIC)
	    return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_DEBUGS
	debugprintf("srvreg_enum: ent fileinit=%u\n",
		op->f.fileinit) ;
	debugprintf("srvreg_enum: cursorlockbroken=%u\n",
		op->f.cursorlockbroken) ;
#endif

	if (cup == NULL)
	    return SR_FAULT ;

/* is the file even initialized? */

	if (! op->f.fileinit)
	    return SR_NOTFOUND ;

/* has our lock been broken */

	if (op->f.cursorlockbroken)
	    return SR_LOCKLOST ;

/* do we have proper file access? */

	if (daytime == 0)
	    daytime = time(NULL) ;

	rs = srvreg_filecheck(op,daytime,1) ;

#if	CF_DEBUGS
	debugprintf("srvreg_enum: srvreg_filecheck() rs=%d\n",rs) ;
#endif

	if ((rs < 0) || (! op->f.fileinit))
	    goto bad1 ;

/* OK, give an entry back to caller */

	ei = (cup->i < 0) ? 0 : cup->i + 1 ;
	eoff = SRVREG_FOTAB + (ei * ebs) ;

#if	CF_DEBUGS
	debugprintf("srvreg_enum: ei=%d eoff=%u\n",ei,eoff) ;
#endif

/* form result to caller */

	rs = ((eoff + ebs) <= op->filesize) ? SR_OK : SR_NOTFOUND ;
	if (rs < 0)
	    goto ret0 ;

/* verify sufficient file buffering */

	rs = srvreg_buf(op,eoff,ebs,&bp) ;

#if	CF_DEBUGS
	debugprintf("srvreg_enum: srvreg_buf() rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (rs < ebs))
	    rs = SR_EOF ;

	if (rs < 0)
	    goto ret0 ;

/* copy entry to caller buffer */

	if (ep != NULL) {

	    srvrege_all(bp,SRVREGE_SIZE,1,ep) ;

	} /* end if */

/* commit the cursor movement? */

	if (rs >= 0)
	    cup->i = ei ;

	op->f.cursoracc = TRUE ;

ret0:

#if	CF_DEBUGS
	debugprintf("srvreg_enum: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;

/* bad stuff */
bad1:

#if	CF_DEBUGS
	debugprintf("srvreg_enum: BAD1 rs=%d fileinit=%u\n",
		rs,op->f.fileinit) ;
#endif

	if ((rs >= 0) && (! op->f.fileinit))
	    rs = SR_EOF ;

bad0:
	goto ret0 ;
}
/* end subroutine (srvreg_enum) */


/* fetch by service name */
int srvreg_fetchsvc(op,svc,cp,ep)
SRVREG		*op ;
const char	svc[] ;
SRVREG_CUR	*cp ;
SRVREG_ENT	*ep ;
{
	time_t	daytime = 0 ;

	uint	ebs = uceil(SRVREG_ENTSIZE,4) ;
	uint	eoff ;
	uint	len ;

	int	rs = SR_OK ;
	int	i, n, ei ;
	int	ne ;
	int	f ;

	char	*bp, *bep, *svcp ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SRVREG_MAGIC)
	    return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (svc == NULL)
	    return SR_FAULT ;

#ifdef	OPTIONAL
	if (ep == NULL)
	    return SR_FAULT ;
#endif

/* is the file even initialized? */

	if (! op->f.fileinit)
	    return SR_NOTFOUND ;

/* do we have proper file access? */

	if (daytime == 0)
	    daytime = time(NULL) ;

	rs = srvreg_filecheck(op,daytime,1) ;
	if ((rs < 0) || (! op->f.fileinit))
	    goto bad1 ;

/* deal with the cursor */

	if (cp == NULL) {
	    ei = 0 ;
	} else
	    ei = (cp->i < 0) ? 0 : (cp->i + 1) ;

#if	CF_DEBUGS
	debugprintf("srvreg_fetchsvc: svc=%s ei=%d\n",svc,ei) ;
#endif

/* continue with the search */

	ne = 20 ;

	f = FALSE ;
	while (! f) {

	    eoff = SRVREG_FOTAB + (ei * ebs) ;

	    len = ne * ebs ;
	    rs = srvreg_buf(op,eoff,len,&bp) ;

	    if (rs < ebs)
	        break ;

	    n = rs / ebs ;
	    for (i = 0 ; i < n ; i += 1) {

	        bep = bp + (i * ebs) ;
	        svcp = bep + SRVREGE_OSVC ;
	        f = (strncmp(svc,svcp,MAXNAMELEN) == 0) ;

	        if (f)
	            break ;

	        ei += 1 ;

	    } /* end for */

	} /* end while */

	if ((rs >= 0) && f && (ep != NULL)) {

	    srvrege_all(bep,SRVREGE_SIZE,1,ep) ;

	} /* end if */

	if ((rs == 0) || (! f))
	    rs = SR_NOTFOUND ;

	if ((rs >= 0) && (cp != NULL))
	    cp->i = ei ;

/* optionally release our lock if we didn't have a cursor outstanding */

	if (op->cursors == 0)
	    srvreg_lockrelease(op) ;

/* update access time as appropriate */

	if (op->cursors == 0) {

	    if (daytime == 0)
	        daytime = time(NULL) ;

	    op->accesstime = daytime ;

	} else
	    op->f.cursoracc = TRUE ;

/* done */
ret0:

#if	CF_DEBUGS
	debugprintf("srvreg_fetchsvc: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;

/* bad stuff */
bad1:
	if (op->cursors == 0)
	    srvreg_lockrelease(op) ;

bad0:
	goto ret0 ;
}
/* end subroutine (srvreg_fetchsvc) */


/* write an entry */
int srvreg_write(op,ei,ep)
SRVREG		*op ;
int		ei ;
SRVREG_ENT	*ep ;
{
	time_t	daytime = 0 ;

	offset_t	uoff ;

	uint	eoff, ebs = SRVREG_EBS ;
	uint	len ;

	int	rs ;
	int	i, n ;
	int	ne, nr ;
	int	f ;

	char	ebuf[SRVREG_EBS + 1] ;
	char	zbuf[5] ;
	char	*bp, *bep, *fp ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SRVREG_MAGIC)
	    return SR_NOTOPEN ;

	if (ep == NULL)
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("srvreg_write: ent ei=%d fileinit=%u\n",
		ei,op->f.fileinit) ;
#endif

	if (! op->f.writable)
	    return SR_RDONLY ;

/* do we have proper file access? */

	if (daytime == 0)
	    daytime = time(NULL) ;

	rs = srvreg_filecheck(op,daytime,0) ;
	if (rs < 0)
	    goto ret0 ;

/* is the file initialized? */

#if	CF_DEBUGS
	debugprintf("srvreg_write: fileinit=%u\n",op->f.fileinit) ;
#endif

	if (! op->f.fileinit) {

	    if (daytime == 0)
	        daytime = time(NULL) ;

	    rs = srvreg_fileinit(op,daytime) ;
	    if (rs < 0)
	        goto ret0 ;

	}

/* figure where we need to write this */

	ne = (op->filesize - SRVREG_FOTAB) / ebs ;
	if (ei > ne) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

/* encode into a buffer */

	srvrege_all(ebuf,SRVREGE_SIZE,0,ep) ;

/* do we need to find an empty slot? */

#if	CF_DEBUGS
	debugprintf("srvreg_write: find empty ne=%d \n",ne) ;
#endif

	f = FALSE ;
	if (ei < 0) {

	    memset(zbuf,0,4) ;

	    nr = 20 ;
	    ei = 0 ;
	    eoff = SRVREG_FOTAB ;
	    while (! f) {

#ifdef	OPTIONAL
	        if (nr > ((op->filesize - eoff) / ebs))
	            nr = (op->filesize - eoff) / ebs ;
#endif

	        len = nr * ebs ;

#if	CF_DEBUGS
	        debugprintf("srvreg_write: srvreg_buf() eoff=%u len=%d\n",
			eoff,len) ;
#endif

	        rs = srvreg_buf(op,eoff,len,&bp) ;

#if	CF_DEBUGS
	        debugprintf("srvreg_write: srvreg_buf() rs=%d\n",rs) ;
#endif

	        if (rs < ebs)
	            break ;

	        n = rs / ebs ;
	        for (i = 0 ; i < n ; i += 1) {

	            bep = bp + (i * ebs) ;
	            fp = bep + SRVREGE_OITYPE ;
	            f = (memcmp(zbuf,fp,sizeof(uint)) == 0) ;

	            if (f)
	                break ;

	            ei += 1 ;

	        } /* end for */

	        eoff += (i * ebs) ;

	    } /* end while */

	    if ((rs >= 0) && (! f))
	        ei = ne ;

	} /* end if (needed an empty slot) */

#if	CF_DEBUGS
	debugprintf("srvreg_write: writing at slot ei=%u\n",ei) ;
#endif

	eoff = SRVREG_FOTAB + (ei * ebs) ;
	uoff = eoff ;
	rs = u_pwrite(op->fd,ebuf,ebs,uoff) ;

#if	CF_DEBUGS
	debugprintf("srvreg_write: u_pwrite() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

#if	CF_DEBUGS
	debugprintf("srvreg_write: f_found=%u\n",f) ;
#endif

	    if (daytime == 0)
	        daytime = time(NULL) ;

		op->h.wcount += 1 ;
		op->h.wtime = daytime ;
	    srvreg_bufupdate(op,eoff,ebs,ebuf) ;

		if (! f) {

	    op->h.nentries += 1 ;
	    op->filesize += ebs ;

#if	CF_DEBUGS
	debugprintf("srvreg_write: filesize=%u\n",op->filesize) ;
#endif

		}

		rs = srvreg_writehead(op) ;

#if	CF_DEBUGS
	debugprintf("srvreg_write: srvreg_writehead() rs=%d\n",rs) ;
#endif

		if ((rs >= 0) && op->f.remote)
			u_fsync(op->fd) ;

	} /* end if (data write was successful) */

ret0:

#if	CF_DEBUGS
	debugprintf("srvreg_write: ret rs=%u filesize=%u\n",rs,op->filesize) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (srvreg_write) */


/* do some checking */
int srvreg_check(op,daytime)
SRVREG		*op ;
time_t		daytime ;
{
	int	rs = SR_OK ;

#if	CF_DEBUGS
	char	timebuf[TIMEBUFLEN + 1] ;
#endif


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SRVREG_MAGIC)
	    return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (op->fd < 0)
	    return SR_OK ;

#if	CF_DEBUGS
	debugprintf("srvreg_check: %s\n",
	    timestr_log(daytime,timebuf)) ;
#endif

	if (op->f.readlocked || op->f.writelocked)
		return SR_OK ;

	if ((daytime - op->accesstime) > TO_ACCESS)
	    goto closeit ;

	if ((daytime - op->opentime) > TO_OPEN)
	    goto closeit ;

ret0:
	return rs ;

/* handle a close out */
closeit:
	rs = srvreg_fileclose(op) ;

	goto ret0 ;
}
/* end subroutine (srvreg_check) */


/* private subroutines */


/* check the file for coherency */
static int srvreg_filecheck(op,daytime,f_read)
SRVREG		*op ;
time_t		daytime ;
int		f_read ;
{
	int	rs = SR_OK ;
	int	f_changed = FALSE ;


/* is the file open */

	if (op->fd < 0) {

	    if (daytime == 0)
	        daytime = time(NULL) ;

	    rs = srvreg_fileopen(op,daytime) ;
	    if (rs < 0)
	        goto ret0 ;

	} /* end if */

/* capture the lock if we do not already have it */

	if ((! op->f.readlocked) && (! op->f.writelocked)) {

	    if (daytime == 0)
	        daytime = time(NULL) ;

	    rs = srvreg_lockget(op,daytime,f_read) ;
	    if (rs < 0)
	        goto ret0 ;

	    rs = srvreg_filechanged(op) ;
	    if (rs < 0)
	        goto bad1 ;

	    f_changed = (rs > 0) ;

	} /* end if (capture lock) */

ret0:
	return (rs >= 0) ? f_changed : rs ;

/* bad stuff */
bad1:
	srvreg_lockrelease(op) ;

bad0:
	goto ret0 ;
}
/* end subroutine (srvreg_filecheck) */


/* initialize the file header (either read it only or write it) */
static int srvreg_fileinit(op,daytime)
SRVREG		*op ;
time_t		daytime ;
{
	struct srvreg_filemagic	fm ;

	int	rs = SR_OK ;
	int	bl ;
	int	f_locked = FALSE ;

	char	fbuf[FBUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("srvreg_fileinit: ent filesize=%u\n",op->filesize) ;
#endif

	if (op->filesize == 0) {

	    u_seek(op->fd,0L,SEEK_SET) ;

	    op->f.fileinit = FALSE ;
	    if (op->f.writable) {

#if	CF_DEBUGS
	        debugprintf("srvreg_fileinit: writing header\n") ;
#endif

	        if (! op->f.writelocked) {

	            rs = srvreg_lockget(op,daytime,0) ;
	            if (rs < 0)
	                goto ret0 ;

	            f_locked = TRUE ;
	        }

/* write the file header stuff */

	        strwcpy(fm.magic,SRVREG_FILEMAGIC,14) ;

	        fm.vetu[0] = SRVREG_FILEVERSION ;
	        fm.vetu[1] = SRVREG_ENDIAN ;
	        fm.vetu[2] = 0 ;
	        fm.vetu[3] = 0 ;

	        bl = 0 ;
	        bl += filemagic((fbuf + bl),0,&fm) ;

	        memset(&op->h,0,sizeof(struct srvreg_filehead)) ;

	        bl += filehead((fbuf + bl),0,&op->h) ;

#if	CF_DEBUGS
	        debugprintf("srvreg_fileinit: u_pwrite() wlen=%d\n",bl) ;
#endif

	        rs = u_pwrite(op->fd,fbuf,bl,0L) ;

#if	CF_DEBUGS
	        debugprintf("srvreg_fileinit: u_pwrite() rs=%d\n",rs) ;
#endif

	        if (rs > 0) {

	            op->filesize = rs ;
		    op->mtime = daytime ;
		    if (op->f.remote)
			u_fsync(op->fd) ;

		}

	        op->f.fileinit = (rs >= 0) ;

	    } /* end if (writing) */

	} else if (op->filesize >= SRVREG_FOTAB) {

	    int	f ;


/* read the file header */

	    if (! op->f.readlocked) {

	        rs = srvreg_lockget(op,daytime,1) ;
	        if (rs < 0)
	            goto ret0 ;

	        f_locked = TRUE ;
	    }

	    rs = u_pread(op->fd,fbuf,FBUFLEN,0L) ;

#if	CF_DEBUGS
	        debugprintf("srvreg_fileinit: u_pread() rs=%d\n",rs) ;
#endif

	    if (rs >= SRVREG_FLTOP) {

	        bl = 0 ;
	        bl += filemagic((fbuf + bl),1,&fm) ;

	        filehead((fbuf + bl),1,&op->h) ;

#if	CF_DEBUGS
	debugprintf("srvreg_fileinit: f_wtime=%08x\n",
		op->h.wtime) ;
	debugprintf("srvreg_fileinit: f_wcount=%08x\n",
		op->h.wcount) ;
	debugprintf("srvreg_fileinit: f_nentries=%08x\n",
		op->h.nentries) ;
#endif

	        f = (strcmp(fm.magic,SRVREG_FILEMAGIC) == 0) ;

#if	CF_DEBUGS
	        debugprintf("srvreg_fileinit: fm.magic=%s\n",fm.magic) ;
	        debugprintf("srvreg_fileinit: magic cmp f=%d\n",f) ;
#endif

#if	(SRVREG_FILEVERSION > 0)
	        f = f && (fm.vetu[0] <= SRVREG_FILEVERSION) ;
#endif

#if	CF_DEBUGS
	        debugprintf("srvreg_fileinit: version cmp f=%d\n",f) ;
#endif

	        f = f && (fm.vetu[1] == SRVREG_ENDIAN) ;

#if	CF_DEBUGS
	        debugprintf("srvreg_fileinit: endian cmp f=%d\n",f) ;
#endif

	        if (! f)
	            rs = SR_BADFMT ;

	        op->f.fileinit = f ;

	    } /* end if */

	} /* end if */

/* if we locked, we unlock it, otherwise leave it ! */

	if (f_locked)
	    srvreg_lockrelease(op) ;

/* we're out of here */
ret0:

#if	CF_DEBUGS
	debugprintf("srvreg_fileinit: ret rs=%d fileinit=%u\n",
		rs,op->f.fileinit) ;
#endif

	return rs ;
}
/* end subroutine (srvreg_fileinit) */


/* has the file changed at all? */
static int srvreg_filechanged(op)
SRVREG		*op ;
{
	struct ustat	sb ;

	int	rs ;
	int	f_changed = FALSE ;


/* has the file changed at all? */

	rs = u_fstat(op->fd,&sb) ;

#if	CF_DEBUGS
	debugprintf("srvreg_filechanged: u_fstat() rs=%d filesize=%u\n",
		rs,sb.st_size) ;
#endif

#ifdef	COMMENT
	if (rs == SR_NOENT)
	    rs = SR_OK ;
#endif /* COMMENT */

	if (rs < 0)
	    goto bad2 ;

	if (sb.st_size < SRVREG_FOTAB)
	    op->f.fileinit = FALSE ;

	f_changed = (! op->f.fileinit) ||
	    (sb.st_size != op->filesize) ||
	    (sb.st_mtime != op->mtime) ;

#if	CF_DEBUGS
	debugprintf("srvreg_filechanged: fileinit=%u\n",op->f.fileinit) ;
	debugprintf("srvreg_filechanged: f_size=%08x o_size=%08x\n",
		sb.st_size,op->filesize) ;
	debugprintf("srvreg_filechanged: f_mtime=%08x o_mtime=%08x\n",
		sb.st_mtime,op->mtime) ;
	debugprintf("srvreg_filechanged: fstat f_changed=%u\n",f_changed) ;
#endif

/* if it has NOT changed, read the file header for write indications */

	if ((! f_changed) && op->f.fileinit) {

	    struct srvreg_filehead	h ;

	    char	hbuf[SRVREG_FLTOP + 1] ;


	    rs = u_pread(op->fd,hbuf,SRVREG_FLTOP,0) ;

#if	CF_DEBUGS
	debugprintf("srvreg_filechanged: u_pread() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto bad2 ;

	    if (rs < SRVREG_FLTOP)
	        op->f.fileinit = FALSE ;

#if	CF_DEBUGS
	debugprintf("srvreg_filechanged: fileinit=%u\n",op->f.fileinit) ;
#endif

	    if (rs > 0) {

	        filehead((hbuf + SRVREG_FOHEAD),1,&h) ;

	        f_changed = (op->h.wtime != h.wtime) ||
	            (op->h.wcount != h.wcount) ||
	            (op->h.nentries != h.nentries) ;

#if	CF_DEBUGS
	debugprintf("srvreg_filechanged: o_wtime=%08x f_wtime=%08x\n",
		op->h.wtime,h.wtime) ;
	debugprintf("srvreg_filechanged: o_wcount=%08x f_wcount=%08x\n",
		op->h.wcount,h.wcount) ;
	debugprintf("srvreg_filechanged: o_nentries=%08x f_nentries=%08x\n",
		op->h.nentries,h.nentries) ;
	debugprintf("srvreg_filechanged: header f_changed=%u\n",f_changed) ;
#endif

	        if (f_changed)
	            op->h = h ;

	    } /* end if */

	} /* end if (reading file header) */

/* OK, we're done */

	if (f_changed) {
	    op->b.len = 0 ;
	    op->filesize = sb.st_size ;
	    op->mtime = sb.st_mtime ;
	}


ret0:

#if	CF_DEBUGS
	debugprintf("srvreg_filechanged: ret rs=%d f_changed=%u\n",
		rs,f_changed) ;
#endif

	return (rs >= 0) ? f_changed : rs ;

/* bad stuff */
bad2:
bad1:
bad0:
	goto ret0 ;
}
/* end subroutine (srvreg_filechanged) */


/* acquire access to the file */
static int srvreg_lockget(op,daytime,f_read)
SRVREG		*op ;
int		f_read ;
time_t		daytime ;
{
	int	rs = SR_OK ;
	int	lockcmd ;
	int	f_already = FALSE ;


#if	CF_DEBUGS
	debugprintf("srvreg_lockget: ent f_read=%d\n",f_read) ;
#endif

	if (op->fd < 0) {

	    rs = srvreg_fileopen(op,daytime) ;

#if	CF_DEBUGS
	    debugprintf("srvreg_lockget: srvreg_fileopen() rs=%d fd=%d\n",
	        rs,op->fd) ;
#endif

	    if (rs < 0)
	        goto bad0 ;

	} /* end if (needed to open the file) */

/* acquire a file record lock */

	if (f_read || (! op->f.writable)) {

#if	CF_DEBUGS
	    debugprintf("srvreg_lockget: need READ lock\n") ;
#endif
	    f_already = op->f.readlocked ;
	    op->f.readlocked = TRUE ;
	    op->f.writelocked = FALSE ;
	    lockcmd = F_RLOCK ;

	} else {

#if	CF_DEBUGS
	    debugprintf("srvreg_lockget: need WRITE lock\n") ;
#endif

	    f_already = op->f.writelocked ;
	    op->f.readlocked = FALSE ;
	    op->f.writelocked = TRUE ;
	    lockcmd = F_WLOCK ;

	} /* end if */

/* get out if we have the lock that we want already */

	if (f_already)
	    goto ret0 ;

/* we need to actually do the lock */

	rs = lockfile(op->fd,lockcmd,0L,0L,TO_LOCK) ;

#if	CF_DEBUGS
	debugprintf("srvreg_lockget: lockfile() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad2 ;

ret0:

#if	CF_DEBUGS
	debugprintf("srvreg_lockget: ret rs=%d fileinit=%u\n",
		rs,op->f.fileinit) ;
#endif

	return rs ;

/* bad stuff */
bad2:

#if	CF_DEBUGS
	debugprintf("srvreg_lockget: BAD rs=%d\n",rs) ;
#endif

	op->f.fileinit = FALSE ;

	lockfile(op->fd,F_ULOCK,0L,0L,TO_LOCK) ;

bad1:
	op->f.readlocked = FALSE ;
	op->f.writelocked = FALSE ;

bad0:
	goto ret0 ;
}
/* end subroutine (srvreg_lockget) */


static int srvreg_lockrelease(op)
SRVREG		*op ;
{
	int	rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("srvreg_lockrelease: ent\n") ;
#endif

	if ((op->f.readlocked || op->f.writelocked)) {
	    if (op->fd >= 0) {
	        rs = lockfile(op->fd,F_ULOCK,0L,0L,TO_LOCK) ;
	    }
	    op->f.readlocked = FALSE ;
	    op->f.writelocked = FALSE ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("srvreg_lockrelease: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (srvreg_lockrelease) */


static int srvreg_fileopen(op,daytime)
SRVREG		*op ;
time_t		daytime ;
{
	int	rs ;


#if	CF_DEBUGS
	debugprintf("srvreg_fileopen: fname=%s\n",op->fname) ;
#endif

	if (op->fd >= 0)
	    return op->fd ;

#if	CF_DEBUGS
	debugprintf("srvreg_fileopen: need open\n") ;
#endif

	if ((rs = u_open(op->fname,op->oflags,op->operm)) >= 0) {
	    op->fd = rs ;
	    uc_closeonexec(op->fd,TRUE) ;
	    op->opentime = daytime ;
	}

	return (rs >= 0) ? op->fd : rs ;
}
/* end subroutine (srvreg_fileopen) */


int srvreg_fileclose(op)
SRVREG		*op ;
{
	int	rs = SR_OK ;

	if (op->fd >= 0) {
	    rs = u_close(op->fd) ;
	    op->fd = -1 ;
	}

	return rs ;
}
/* end subroutine (srvreg_fileclose) */


static int srvreg_bufinit(op)
SRVREG		*op ;
{
	int	rs ;
	int	size =  SRVREG_BUFSIZE ;

	char	*bp ;

	op->b.off = 0 ;
	op->b.len = 0 ;
	op->b.size = 0 ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    op->b.buf = bp ;
	    op->b.size = size ;
	}

	return rs ;
}
/* end subroutine (srvreg_bufinit) */


static int srvreg_buffree(op)
SRVREG		*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (op->b.buf != NULL) {
	    rs1 = uc_free(op->b.buf) ;
	    if (rs >= 0) rs = rs1 ;
	    op->b.buf = NULL ;
	}

	op->b.size = 0 ;
	op->b.len = 0 ;
	return rs ;
}
/* end subroutine (srvreg_buffree) */


/* try to buffer up some of the file */
static int srvreg_buf(op,roff,rlen,rpp)
SRVREG		*op ;
uint		roff, rlen ;
char		**rpp ;
{
	offset_t	foff ;

	uint	bend, fext ;
	uint	rext = (roff + rlen), ext ;

	int	rs = SR_OK ;
	int	len ;

	char	*rbuf ;


#if	CF_DEBUGS
	debugprintf("srvreg_buf: ent roff=%u rlen=%u\n",roff,rlen) ;
	debugprintf("srvreg_buf: b.size=%u b.off=%u b.len=%u \n",
	    op->b.size,op->b.off,op->b.len) ;
#endif

/* do we need to read in more data? */

	len = rlen ;
	fext = op->b.off + op->b.len ;
	if ((roff < op->b.off) || (rext > fext)) {

#if	CF_DEBUGS
	    debugprintf("srvreg_buf: need more data\n") ;
#endif

/* can we do an "add-on" type read operation? */

	    bend = op->b.off + op->b.size ;
	    if ((roff >= op->b.off) &&
	        (rext <= bend)) {

#if	CF_DEBUGS
	        debugprintf("srvreg_buf: add-on read\n") ;
#endif

	        foff = op->b.off + op->b.len ;
	        rbuf = op->b.buf + op->b.len ;

	        ext = roff + MAX(rlen,SRVREG_READSIZE) ;
	        fext = uceil(ext,op->pagesize) ;

	        if (fext > bend)
	            fext = bend ;

	        len = fext - foff ;

#if	CF_DEBUGS
	        debugprintf("srvreg_buf: u_pread() foff=%llu len=%u\n",
	            foff,len) ;
#endif

	        rs = u_pread(op->fd,rbuf,len, foff) ;

#if	CF_DEBUGS
	        debugprintf("srvreg_buf: u_pread() rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {

	            op->b.len += rs ;
	            len = MIN(((op->b.off + op->b.len) - roff),rlen) ;

#if	CF_DEBUGS
	            debugprintf("srvreg_buf: len=%d\n",len) ;
#endif

	        }

	    } else {

#if	CF_DEBUGS
	        debugprintf("srvreg_buf: fresh read\n") ;
#endif

	        op->b.off = roff ;
	        op->b.len = 0 ;

	        bend = roff + op->b.size ;

	        foff = roff ;
	        rbuf = op->b.buf ;

	        ext = roff + MAX(rlen,SRVREG_READSIZE) ;
	        fext = uceil(ext,op->pagesize) ;

	        if (fext > bend)
	            fext = bend ;

	        len = fext - foff ;
	        if ((rs = u_pread(op->fd,op->b.buf,len, foff)) >= 0) {
	            op->b.len = rs ;
	            len = MIN(rs,rlen) ;
	        }

	    } /* end if */

	} /* end if (needed to read more data) */

	if (rpp != NULL)
	    *rpp = op->b.buf + (roff - op->b.off) ;

#if	CF_DEBUGS
	debugprintf("srvreg_buf: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (srvreg_buf) */


/* update the file buffer from a user supplied source */
static int srvreg_bufupdate(op,roff,rbuflen,rbuf)
SRVREG		*op ;
uint		roff ;
int		rbuflen ;
const char	rbuf[] ;
{
	uint	boff, bext ;
	uint	rext = roff + rbuflen ;

	int	buflen, bdiff ;


	buflen = op->b.len ;
	boff = op->b.off ;
	bext = boff + buflen ;

	if (roff < boff) {

	    if (rext <= boff)
	        return 0 ;

	    rbuf += (boff - roff) ;
	    rbuflen -= (boff - roff) ;
	    roff = boff ;

	} /* end if */

	if (rext > bext) {

	    if (roff >= bext)
	        return 0 ;

	    rbuflen -= (rext - bext) ;
	    rext = bext ;

	} /* end if */

	if (rbuflen > 0) {
	    bdiff = roff - boff ;
	    memcpy((op->b.buf + bdiff),rbuf,rbuflen) ;
	}

	return rbuflen ;
}
/* end subroutine (srvreg_bufupdate) */


/* write out the file header */
static int srvreg_writehead(op)
SRVREG		*op ;
{
	offset_t	uoff ;

	int	rs ;
	int	bl ;

	char	fbuf[FBUFLEN + 1] ;


	bl = filehead(fbuf,0,&op->h) ;

	uoff = SRVREG_FOHEAD ;
	rs = u_pwrite(op->fd,fbuf,bl,uoff) ;

	return rs ;
}
/* end subroutine (srvreg_writehead) */


static int filemagic(buf,f_read,mp)
char			*buf ;
int			f_read ;
struct srvreg_filemagic	*mp ;
{
	int	rs = 20 ;

	char	*bp = buf ;
	char	*cp ;


	if (buf == NULL)
	    return SR_BADFMT ;

	if (f_read) {

	    bp[15] = '\0' ;
	    strncpy(mp->magic,bp,15) ;

	    if ((cp = strchr(mp->magic,'\n')) != NULL)
	        *cp = '\0' ;

	    bp += 16 ;
	    memcpy(mp->vetu,bp,4) ;

	} else {

	    bp = strwcpy(bp,mp->magic,14) ;

	    *bp++ = '\n' ;
	    memset(bp,0,(16 - (bp - buf))) ;

	    bp = buf + 16 ;
	    memcpy(bp,mp->vetu,4) ;

	} /* end if */

	return rs ;
}
/* end subroutine (filemagic) */


/* encode or decode the file header */
static int filehead(buf,f_read,hp)
char			*buf ;
int			f_read ;
struct srvreg_filehead	*hp ;
{
	SERIALBUF	msgbuf ;
	const int	buflen = sizeof(struct srvreg_filehead) ;
	int		rs ;
	int		rs1 ;

	if (buf == NULL)
	    return SR_BADFMT ;

	if ((rs = serialbuf_start(&msgbuf,buf,buflen)) >= 0) {

	if (f_read) { /* read */

	    serialbuf_ruint(&msgbuf,&hp->wcount) ;

	    serialbuf_ruint(&msgbuf,&hp->wtime) ;

	    serialbuf_ruint(&msgbuf,&hp->nentries) ;

	} else { /* write */

	    serialbuf_wuint(&msgbuf,hp->wcount) ;

	    serialbuf_wuint(&msgbuf,hp->wtime) ;

	    serialbuf_wuint(&msgbuf,hp->nentries) ;

	} /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (filehead) */


