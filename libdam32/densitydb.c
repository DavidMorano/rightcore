/* densitydb */

/* manage reading or writing of a density file */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		1
#define	CF_CREAT	0		/* always create the file? */
#define	CF_HASH		1		/* use hash for faster lookup */
#define	CF_CHEATEXT	0		/* cheat on message field extraction */


/* revision history:

	= 2004-02-17, David A­D­ Morano
        This code module was inspired from the mail-message-id database (which
	is used to eliminate repeated mail messages).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module manages the reading and writing of entries in a density
        file.


*******************************************************************************/


#define	DENSITYDB_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/statvfs.h>
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

#include	"densitydb.h"
#include	"densitydbe.h"


/* local defines */

#define	DENSITYDB_FILEMAGICA	"DENSITYDBA"
#define	DENSITYDB_FILEMAGICB	"DENSITYDBB"

#define	DENSITYDB_FS		"densitydb"
#define	DENSITYDB_FSA		"densitydba"
#define	DENSITYDB_FSB		"densitydbb"

#define	DENSITYDB_FLID		(16 + 4)
#define	DENSITYDB_FLHEAD	(3 * 4)
#define	DENSITYDB_FLTOP		(DENSITYDB_FLID + DENSITYDB_FLHEAD)

#define	DENSITYDB_FOID		0
#define	DENSITYDB_FOHEAD	(DENSITYDB_FOID + DENSITYDB_FLID)
#define	DENSITYDB_FOTAB		(DENSITYDB_FOHEAD + DENSITYDB_FLHEAD)

#define	DENSITYDB_ENTSIZE	sizeof(uint)
#define	DENSITYDB_EBS		((DENSITYDBE_SIZE + 3) & (~ 3))

#define	DENSITYDB_BUFSIZE	(64 * 1024)
#define	DENSITYDB_READSIZE	(16 * 1024)

#define	DENSITYDB_FBUFLEN	(DENSITYDB_FLTOP + 9)

#define	TO_OPEN		(60 * 60)	/* maximum file-open time */
#define	TO_ACCESS	(2 * 60)	/* maximum access idle time */
#define	TO_LOCK		10		/* seconds */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern uint	ufloor(uint,int) ;
extern uint	uceil(uint,int) ;

extern int	matstr(cchar **,cchar *,int) ;
extern int	mkfnamesuf1(char *,cchar *,cchar *) ;
extern int	isfsremote(int) ;
extern int	isNotPresent(int) */

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */

struct oldentry {
	time_t		utime ;
	int		ei ;
} ;


/* forward references */

int		densitydb_close(DENSITYDB *) ;

static int	densitydb_opening(DENSITYDB *,cchar *) ;
static int	densitydb_opener(DENSITYDB *,char *,cchar *,int) ;

static int	densitydb_fileopen(DENSITYDB *,time_t) ;
static int	densitydb_fileclose(DENSITYDB *) ;
static int	densitydb_lockget(DENSITYDB *,time_t,int) ;
static int	densitydb_lockrelease(DENSITYDB *) ;
static int	densitydb_fileinit(DENSITYDB *,time_t) ;
static int	densitydb_filechanged(DENSITYDB *) ;
static int	densitydb_filecheck(DENSITYDB *,time_t,int) ;
static int	densitydb_searchid(DENSITYDB *,const char *,int,char **) ;
static int	densitydb_search(DENSITYDB *,DENSITYDB_KEY *,uint,char **) ;
static int	densitydb_searchempty(DENSITYDB *,struct oldentry *,char **) ;
static int	densitydb_searchemptyrange(DENSITYDB *,int,int,
			struct oldentry *,char **) ;
static int	densitydb_buf(DENSITYDB *,uint,int,char **) ;
static int	densitydb_bufupdate(DENSITYDB *,uint,int,cchar *) ;
static int	densitydb_bufbegin(DENSITYDB *) ;
static int	densitydb_bufend(DENSITYDB *) ;
static int	densitydb_writehead(DENSITYDB *) ;

static int	filemagic(char *,int,DENSITYDB_FM *) ;
static int	filehead(char *,int,DENSITYDB_FH *) ;

static int	matfield(const char *,int,const char *,int) ;


/* local variables */

static const char	*localfs[] = {
	"ufs",
	"tmpfs",
	"lofs",
	"pcfs",
	"hfs",
	"vxfs",
	NULL
} ;


/* exported subroutines */


int densitydb_open(op,fname,oflags,om,maxentry)
DENSITYDB	*op ;
const char	fname[] ;
int		oflags ;
mode_t		om ;
int		maxentry ;
{
	time_t		dt = time(NULL) ;
	int		rs ;
	int		f_create = FALSE ;

#if	CF_DEBUGS
	debugprintf("densitydb_open: ent fname=%s\n",fname) ;
#endif

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
#endif /* CF_SAFE */

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("densitydb_open: oflags=%4o fname=%s\n",
	    oflags,fname) ;
	if (oflags & O_CREAT)
	    debugprintf("densitydb_open: creating as needed\n") ;
#endif

#if	CF_CREAT
	oflags |= O_CREAT ;
#endif
	oflags = (oflags &= (~ O_TRUNC)) ;

	memset(op,0,sizeof(DENSITYDB)) ;
	op->pagesize = getpagesize() ;
	op->oflags = oflags ;
	op->om = om ;
	op->maxentry = maxentry ;
	op->ebs = uceil(DENSITYDB_ENTSIZE,4) ;

#if	CF_DEBUGS
	debugprintf("densitydb_open: densitydb_bufbegin()\n") ;
#endif

	if ((rs = densitydb_bufbegin(op)) >= 0) {
	    if ((rs = densitydb_opening(op,fname)) >= 0) {
	        cchar	*cp ;
	        if ((rs = uc_malloc(tmpfname,-1,&cp)) >= 0) {
		    USTAT	sb ;
	            int		am = (oflags & O_ACCMODE) ;
	            op->fname = cp ;
	            op->f.writable = ((am == O_WRONLY) || (am == O_RDWR)) ;
	            op->opentime = dt ;
	            op->accesstime = dt ;
	            if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	                op->mtime = sb.st_mtime ;
	                op->filesize = sb.st_size ;
	                if ((rs = isfsremote(op->fd)) >= 0) {
	                    op->f.remote = (rs > 0) ;
	                    if ((rs = densitydb_fileinit(op,dt)) >= 0) {
		                op->magic = DENSITYDB_MAGIC ;
		            }
	                }
	            }
	            if (rs < 0) {
	                uc_free(op->fname) ;
		        op->fname = NULL ;
	            }
	        } /* end if (m-a) */
		if (rs < 0) {
		    u_close(op->fd) ;
		    op->fd = -1 ;
		}
	    } /* end if (densitydb_opening) */
	    if (rs < 0) {
		densitydb_bufend(op) ;
	    }
	} /* end if (densitydb_bufbegin) */

	return (rs >= 0) ? f_create : rs ;
}
/* end subroutine (densitydb_open) */


int densitydb_close(DENSITYDB *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != DENSITYDB_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	rs1 = densitydb_bufend(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	}

	if (op->fname != NULL) {
	    uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (densitydb_close) */


/* get a count of the number of entries */
int densitydb_count(DENSITYDB *op)
{
	int		rs = SR_OK ;
	int		c ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != DENSITYDB_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	c = (op->filesize - DENSITYDB_FOTAB) / DENSITYDB_ENTSIZE ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (densitydb_count) */


/* enumerate the entries */
int densitydb_enum(DENSITYDB *op,DENSITYDB_CUR *cup,DENSITYDB_ENT *ep)
{
	time_t		dt = 0 ;
	uint		eoff ;
	int		rs ;
	int		ei ;
	char		*bp ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != DENSITYDB_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_DEBUGS
	debugprintf("densitydb_enum: ent fileinit=%u\n",op->f.fileinit) ;
	debugprintf("densitydb_enum: cursorlockbroken=%u\n",
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

	if (dt == 0)
	    dt = time(NULL) ;

	rs = densitydb_filecheck(op,dt,1) ;

#if	CF_DEBUGS
	debugprintf("densitydb_enum: densitydb_filecheck() rs=%d\n",rs) ;
#endif

	if ((rs < 0) || (! op->f.fileinit))
	    goto bad1 ;

/* OK, give an entry back to caller */

	ei = (cup->i < 0) ? 0 : cup->i + 1 ;
	eoff = DENSITYDB_FOTAB + (ei * op->ebs) ;

#if	CF_DEBUGS
	debugprintf("densitydb_enum: ei=%d eoff=%u\n",ei,eoff) ;
#endif

/* form result to caller */

	rs = ((eoff + op->ebs) <= op->filesize) ? SR_OK : SR_NOTFOUND ;

	if (rs < 0)
	    goto ret0 ;

/* verify sufficient file buffering */

	rs = densitydb_buf(op,eoff,op->ebs,&bp) ;

#if	CF_DEBUGS
	debugprintf("densitydb_enum: densitydb_buf() rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (rs < op->ebs))
	    rs = SR_EOF ;

	if (rs < 0)
	    goto ret0 ;

/* copy entry to caller buffer */

	if (ep != NULL) {

	    densitydbe_all(bp,DENSITYDBE_SIZE,1,ep) ;

#if	CF_DEBUGS
	    debugprintf("densitydb_enum: densitydbe_all() c=%u\n",ep->count) ;
#endif

	} /* end if */

/* commit the cursor movement? */

	if (rs >= 0)
	    cup->i = ei ;

	op->f.cursoracc = TRUE ;

ret0:

#if	CF_DEBUGS
	debugprintf("densitydb_enum: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;

/* bad stuff */
bad1:

#if	CF_DEBUGS
	debugprintf("densitydb_enum: BAD1 rs=%d fileinit=%u\n",
	    rs,op->f.fileinit) ;
#endif

	if ((rs >= 0) && (! op->f.fileinit))
	    rs = SR_EOF ;

bad0:

#if	CF_DEBUGS
	debugprintf("densitydb_enum: BAD ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (densitydb_enum) */


/* update a recipient <=> message-id entry match */
int densitydb_update(DENSITYDB *op,time_t dt,int index,DENSITYDB_ENT *ep)
{
	DENSITYDBE_ALL	m0 ;
	offset_t	uoff ;
	uint		eoff ;
	int		rs ;
	int		ei ;
	int		wlen ;
	int		f_addition = FALSE ;
	int		f_bufupdate = FALSE ;
	char		ebuf[DENSITYDBE_SIZE + 4] ;
	char		*bep ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != DENSITYDB_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#ifdef	OPTIONAL
	if (ep == NULL)
	    return SR_FAULT ;
#endif

	if (index < 0)
		return SR_INVALID ;

/* is the file even initialized? */

	if (! op->f.fileinit)
	    return SR_NOTFOUND ;

/* do we have proper file access? */

	if (dt == 0)
	    dt = time(NULL) ;

#if	CF_DEBUGS
	debugprintf("densitydb_update: densitydb_filecheck()\n") ;
#endif

	rs = densitydb_filecheck(op,dt,1) ;

	if ((rs < 0) || (! op->f.fileinit))
	    goto ret0 ;

/* continue with the search */

#if	CF_DEBUGS
	debugprintf("densitydb_update: densitydb_search() recip=%s\n",
	    kp->recip) ;
	debugprintf("densitydb_update: mid=%s\n",
	    kp->mid) ;
#endif

	ei = index ;
	if (ei > (op->h.nentries - 1))
	    ei = (op->h.nentries - 1) ;

	if (ep != NULL) {
	    densitydbe_all(bep,DENSITYDBE_SIZE,1,ep) ;
	    m0 = *ep ;
	} else {
	    densitydbe_all(bep,DENSITYDBE_SIZE,1,&m0) ;
	}

	    m0.count += 1 ;
	    m0.utime = dt ;
	    wlen = densitydbe_all(bep,DENSITYDBE_SIZE,0,&m0) ;

#if	CF_DEBUGS
	    if (ep != NULL) {
	        debugprintf("densitydb_update: mid=%s\n",ep->messageid) ;
	        debugprintf("densitydb_update: count=%u\n",ep->count) ;
	    }
#endif

/* update the in-core file buffer as needed or as appropriate */

	    if ((rs >= 0) && f_bufupdate && op->f.writable) {

#if	CF_DEBUGS
	        debugprintf("densitydb_update: need update ebuf(%p) wlen=%u\n",
	            ebuf,wlen) ;
	        debugprintf("densitydb_update: bep(%p)\n",bep) ;
#endif

	        eoff = DENSITYDB_FOTAB + (ei * op->ebs) ;
	        densitydb_bufupdate(op,eoff,wlen,ebuf) ;

	    }

#if	CF_DEBUGS
	    {
	        DENSITYDBE_ALL	m0 ;
	        char		timebuf[TIMEBUFLEN + 1] ;
	        densitydbe_all(bep,op->ebs,1,&m0) ;
	        debugprintf("densitydb_update: writing count=%u utime=%s\n",
	            m0.count,timestr_log(m0.utime,timebuf)) ;
	    }
#endif /* CF_DEBUGS */

	if ((rs >= 0) && op->f.writable) {

/* write back this entry */

	    eoff = DENSITYDB_FOTAB + (ei * op->ebs) ;

#if	CF_DEBUGS
	    {
	        DENSITYDBE_ALL	m0 ;
	        char		timebuf[TIMEBUFLEN + 1] ;
	        densitydbe_all(bep,op->ebs,1,&m0) ;
	        debugprintf("densitydb_update: writing ei=%u foff=%u wlen=%u\n",
	            ei,eoff,wlen) ;
	        debugprintf("densitydb_update: writing count=%u utime=%s\n",
	            m0.count,timestr_log(m0.utime,timebuf)) ;
	    }
#endif /* CF_DEBUGS */

	    uoff = eoff ;
	    rs = u_pwrite(op->fd,bep,wlen,uoff) ;

#if	CF_DEBUGS
	    debugprintf("densitydb_update: u_pwrite() rs=%d\n",rs) ;
#endif

	    if (rs >= wlen) {

#if	CF_DEBUGS
	        debugprintf("densitydb_update: writing back header\n") ;
#endif

	        if (dt == 0)
	            dt = time(NULL) ;

	        op->h.wcount += 1 ;
	        op->h.wtime = dt ;
	        if (f_addition) {

	            op->h.nentries += 1 ;
	            op->filesize += wlen ;

#if	CF_DEBUGS
	            debugprintf("densitydb_update: filesize=%u\n",
			op->filesize) ;
#endif

	        }

	        rs = densitydb_writehead(op) ;

#if	CF_DEBUGS
	        debugprintf("densitydb_update: densitydb_writehead() rs=%d\n",
			rs) ;
#endif

	        if ((rs >= 0) && op->f.remote)
	            u_fsync(op->fd) ;

	    } /* end if (data write was successful) */

	} /* end if (writing updated entry to file) */

/* update access time as appropriate */

	if (dt == 0)
	    dt = time(NULL) ;

	op->accesstime = dt ;

/* done */
ret0:

#if	CF_DEBUGS
	debugprintf("densitydb_update: ret rs=%d ei=%u\n",rs,ei) ;
#endif

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (densitydb_update) */


/* do some checking */
int densitydb_check(DENSITYDB *op,time_t dt)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	char		timebuf[TIMEBUFLEN + 1] ;
#endif


#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != DENSITYDB_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */


	if (op->fd < 0)
	    return SR_OK ;

#if	CF_DEBUGS
	debugprintf("densitydb_check: %s\n",
	    timestr_log(dt,timebuf)) ;
#endif

	if (op->f.readlocked || op->f.writelocked)
	    return SR_OK ;

	if ((dt - op->accesstime) > TO_ACCESS)
	    goto closeit ;

	if ((dt - op->opentime) > TO_OPEN)
	    goto closeit ;

	return rs ;

/* handle a close out */
closeit:
	rs = densitydb_fileclose(op) ;

	return rs ;
}
/* end subroutine (densitydb_check) */


/* private subroutines */


static int densitydb_opening(DENSITYDB *op,cchar *fname)
{
	const int	of = (op->oflags & (~ O_CREAT)) ;
	int		rs ;
	int		f_create = FALSE ;
	char		tbuf[MAXPATHLEN+1] ;

	rs = densitydb_opener(op,tbuf,fname,of) ;
	if ((rs < 0) && isNotPresent(rs)) {
	    of = op->oflags ;
	    if (of & O_CREAT) {
		f_create = TRUE ;
	        rs = densitydb_opener(op,tbuf,fname,of) ;
	    }
	}

	return (rs >= 0) ? f_create : rs ;
}
/* end subroutine (densitydb_opening) */


static int densitydb_opener(DENSITYDB *op,char *tbuf,cchar *fname,int of)
{
	int		rs ;
	cchar		*suf ;

	suf = DENSITYDB_FS ;
	if ((rs = mkfnamesuf1(tbuf,fname,suf)) >= 0) {
	    rs = u_open(tbuf,of,om) ;
	    op->fd = rs ;
	    if ((rs < 0) && isNotPresent(rs)) {
		suf = DENSITYDB_FSB ;
	        if ((rs = mkfnamesuf1(tbuf,fname,suf)) >= 0) {
	    	    rs = u_open(tbuf,of,om) ;
	            op->fd = rs ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (densitydb_opener) */


/* check the file for coherency */
static int densitydb_filecheck(DENSITYDB *op,time_t dt,int f_read)
{
	int		rs = SR_OK ;
	int		f_changed = FALSE ;

	if (op->fd < 0) {
	    if (dt == 0) dt = time(NULL) ;
	    rs = densitydb_fileopen(op,dt) ;
	    if (rs < 0) goto ret0 ;
	}

/* capture the lock if we do not already have it */

	if ((! op->f.readlocked) && (! op->f.writelocked)) {
	    if (dt == 0) dt = time(NULL) ;

	    rs = densitydb_lockget(op,dt,f_read) ;

	    if (rs < 0)
	        goto ret0 ;

	    rs = densitydb_filechanged(op) ;

	    if (rs < 0)
	        goto bad1 ;

	    f_changed = (rs > 0) ;

	} /* end if (capture lock) */

ret0:
	return (rs >= 0) ? f_changed : rs ;

/* bad stuff */
bad1:
	densitydb_lockrelease(op) ;

bad0:
	return rs ;
}
/* end subroutine (densitydb_filecheck) */


/* initialize the file header (either read it only or write it) */
static int densitydb_fileinit(DENSITYDB *op,time_t dt)
{
	DENSITYDB_FM	fm ;
	int		rs ;
	int		bl ;
	int		f_locked = FALSE ;
	char		fbuf[DENSITYDB_FBUFLEN + 1], *bp ;
	char		*cp ;

#if	CF_DEBUGS
	debugprintf("densitydb_fileinit: ent filesize=%u\n",op->filesize) ;
	debugprintf("densitydb_fileinit: fbuf(%p)\n",fbuf) ;
#endif

	if (op->filesize == 0) {

	    u_seek(op->fd,0L,SEEK_SET) ;

	    op->f.fileinit = FALSE ;
	    if (op->f.writable) {

#if	CF_DEBUGS
	        debugprintf("densitydb_fileinit: writing header\n") ;
#endif

	        if (! op->f.writelocked) {

	            rs = densitydb_lockget(op,dt,0) ;

	            if (rs < 0)
	                goto ret0 ;

	            f_locked = TRUE ;
	        }

/* write the file magic and header stuff */

/* file magic */

	        strwcpy(fm.magic,DENSITYDB_FILEMAGICB,14) ;

	        fm.vetu[0] = DENSITYDB_FILEVERSION ;
	        fm.vetu[1] = DENSITYDB_ENDIAN ;
	        fm.vetu[2] = 0 ;
	        fm.vetu[3] = 0 ;

	        bl = 0 ;
	        bl += filemagic((fbuf + bl),0,&fm) ;

/* file header */

	        memset(&op->h,0,sizeof(DENSITYDB_FH)) ;

#if	CF_DEBUGS
	        debugprintf("densitydb_fileinit: filehead() bl=%d\n",bl) ;
#endif

	        bl += filehead((fbuf + bl),0,&op->h) ;

/* write them to the file */

#if	CF_DEBUGS
	        debugprintf("densitydb_fileinit: u_pwrite() wlen=%d\n",bl) ;
#endif

	        rs = u_pwrite(op->fd,fbuf,bl,0L) ;

#if	CF_DEBUGS
	        debugprintf("densitydb_fileinit: u_pwrite() rs=%d\n",rs) ;
#endif

	        if (rs > 0) {

	            op->filesize = rs ;
	            op->mtime = dt ;
	            if (op->f.remote)
	                u_fsync(op->fd) ;

	            densitydb_bufupdate(op,0,bl,fbuf) ;

	        }

	        op->f.fileinit = (rs >= 0) ;

	    } /* end if (writing) */

	} else if (op->filesize >= DENSITYDB_FOTAB) {
	    int	f ;

/* read the file header */

	    if (! op->f.readlocked) {

	        rs = densitydb_lockget(op,dt,1) ;

	        if (rs < 0)
	            goto ret0 ;

	        f_locked = TRUE ;
	    }

	    rs = u_pread(op->fd,fbuf,DENSITYDB_FBUFLEN,0L) ;

#if	CF_DEBUGS
	    debugprintf("densitydb_fileinit: u_pread() rs=%d\n",rs) ;
#endif

	    if (rs >= DENSITYDB_FLTOP) {

	        bl = 0 ;
	        bl += filemagic((fbuf + bl),1,&fm) ;

	        filehead((fbuf + bl),1,&op->h) ;

#if	CF_DEBUGS
	        debugprintf("densitydb_fileinit: f_wtime=%08x\n",
	            op->h.wtime) ;
	        debugprintf("densitydb_fileinit: f_wcount=%08x\n",
	            op->h.wcount) ;
	        debugprintf("densitydb_fileinit: f_nentries=%08x\n",
	            op->h.nentries) ;
#endif

	        f = (strcmp(fm.magic,DENSITYDB_FILEMAGICB) == 0) ;

#if	CF_DEBUGS
	        debugprintf("densitydb_fileinit: fm.magic=%s\n",fm.magic) ;
	        debugprintf("densitydb_fileinit: magic cmp f=%d\n",f) ;
#endif

	        f = f && (fm.vetu[0] <= DENSITYDB_FILEVERSION) ;

#if	CF_DEBUGS
	        debugprintf("densitydb_fileinit: version cmp f=%d\n",f) ;
#endif

	        f = f && (fm.vetu[1] == DENSITYDB_ENDIAN) ;

#if	CF_DEBUGS
	        debugprintf("densitydb_fileinit: endian cmp f=%d\n",f) ;
#endif

	        if (! f)
	            rs = SR_BADFMT ;

	        op->f.fileinit = f ;

	    }

	} /* end if */

/* if we locked, we unlock it, otherwise leave it ! */

	if (f_locked)
	    densitydb_lockrelease(op) ;

/* we're out of here */
ret0:

#if	CF_DEBUGS
	debugprintf("densitydb_fileinit: ret rs=%d fileinit=%u\n",rs,op->f.fileinit) ;
#endif

	return rs ;
}
/* end subroutine (densitydb_fileinit) */


/* has the file changed at all? */
static int densitydb_filechanged(DENSITYDB *op)
{
	struct ustat	sb ;
	int		rs ;
	int		f_changed = FALSE ;

	rs = u_fstat(op->fd,&sb) ;

#if	CF_DEBUGS
	debugprintf("densitydb_filechanged: u_fstat() rs=%d filesize=%u\n",
	    rs,sb.st_size) ;
#endif

#ifdef	COMMENT
	if (rs == SR_NOENT)
	    rs = SR_OK ;
#endif /* COMMENT */

	if (rs < 0)
	    goto bad2 ;

	if (sb.st_size < DENSITYDB_FOTAB)
	    op->f.fileinit = FALSE ;

	f_changed = (! op->f.fileinit) ||
	    (sb.st_size != op->filesize) ||
	    (sb.st_mtime != op->mtime) ;

#if	CF_DEBUGS
	debugprintf("densitydb_filechanged: fileinit=%u\n",op->f.fileinit) ;
	debugprintf("densitydb_filechanged: sb_size=%08x o_size=%08x\n",
	    sb.st_size,op->filesize) ;
	debugprintf("densitydb_filechanged: sb_mtime=%08x o_mtime=%08x\n",
	    sb.st_mtime,op->mtime) ;
	debugprintf("densitydb_filechanged: fstat f_changed=%u\n",f_changed) ;
#endif

/* if it has NOT changed, read the file header for write indications */

	if ((! f_changed) && op->f.fileinit) {
	    DENSITYDB_FH	h ;
	    char	hbuf[DENSITYDB_FLTOP + 1] ;

	    rs = u_pread(op->fd,hbuf,DENSITYDB_FLTOP,0L) ;

#if	CF_DEBUGS
	    debugprintf("densitydb_filechanged: u_pread() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto bad2 ;

	    if (rs < DENSITYDB_FLTOP)
	        op->f.fileinit = FALSE ;

#if	CF_DEBUGS
	    debugprintf("densitydb_filechanged: fileinit=%u\n",op->f.fileinit) ;
#endif

	    if (rs > 0) {

	        filehead((hbuf + DENSITYDB_FOHEAD),1,&h) ;

	        f_changed = (op->h.wtime != h.wtime) ||
	            (op->h.wcount != h.wcount) ||
	            (op->h.nentries != h.nentries) ;

#if	CF_DEBUGS
	        debugprintf("densitydb_filechanged: "
			"o_wtime=%08x fh_wtime=%08x\n",
	            op->h.wtime,h.wtime) ;
	        debugprintf("densitydb_filechanged: "
			"o_wcount=%08x fh_wcount=%08x\n",
	            op->h.wcount,h.wcount) ;
	        debugprintf("densitydb_filechanged: o_nentries=%08x "
			"fh_nentries=%08x\n",
	            op->h.nentries,h.nentries) ;
	        debugprintf("densitydb_filechanged: header f_changed=%u\n",
			f_changed) ;
#endif

	        if (f_changed)
	            op->h = h ;

	    }

	} /* end if (reading file header) */

/* OK, we're done */

	if (f_changed) {
	    op->b.len = 0 ;
	    op->filesize = sb.st_size ;
	    op->mtime = sb.st_mtime ;
	}

#if	CF_DEBUGS
	debugprintf("densitydb_filechanged: ret rs=%d f_changed=%u\n",
	    rs,f_changed) ;
#endif

	return (rs >= 0) ? f_changed : rs ;

/* bad stuff */
bad2:
bad1:
bad0:
	return rs ;
}
/* end subroutine (densitydb_filechanged) */


/* acquire access to the file */
static int densitydb_lockget(DENSITYDB *op,int f_read,time_t dt)
{
	int		rs = SR_OK ;
	int		f_already = FALSE ;

#if	CF_DEBUGS
	debugprintf("densitydb_lockget: ent f_read=%d\n",f_read) ;
#endif

	if (op->fd < 0) {
	    rs = densitydb_fileopen(op,dt) ;
	} /* end if (needed to open the file) */

/* acquire a file record lock */

	if (rs >= 0) {
	    int		lockcmd ;

	if (f_read || (! op->f.writable)) {
	    f_already = op->f.readlocked ;
	    op->f.readlocked = TRUE ;
	    op->f.writelocked = FALSE ;
	    lockcmd = F_RLOCK ;
	} else {
	    f_already = op->f.writelocked ;
	    op->f.readlocked = FALSE ;
	    op->f.writelocked = TRUE ;
	    lockcmd = F_WLOCK ;
	}

/* get out if we have the lock that we want already */

	if (! f_already) {
	rs = lockfile(op->fd,lockcmd,0L,0,TO_LOCK) ;
	}

	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("densitydb_lockget: ret rs=%d fileinit=%u\n",
	    rs,op->f.fileinit) ;
#endif

	return rs ;
}
/* end subroutine (densitydb_lockget) */


static int densitydb_lockrelease(DENSITYDB *op)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("densitydb_lockrelease: ent\n") ;
#endif

	if ((op->f.readlocked || op->f.writelocked)) {
	    if (op->fd >= 0) {
	        rs = lockfile(op->fd,F_ULOCK,0L,0,TO_LOCK) ;
	    }
	    op->f.readlocked = FALSE ;
	    op->f.writelocked = FALSE ;
	}

#if	CF_DEBUGS
	debugprintf("densitydb_lockrelease: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (densitydb_lockrelease) */


static int densitydb_fileopen(DENSITYDB *op,time_t dt)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("densitydb_fileopen: fname=%s\n",op->fname) ;
#endif

	if (op->fd < 0) {
	    if ((rs = u_open(op->fname,op->oflags,op->om)) >= 0) {
	        op->fd = rs ;
	        uc_closeonexec(op->fd,TRUE) ;
	        op->opentime = dt ;
	    }
	}

	return (rs >= 0) ? op->fd : rs ;
}
/* end subroutine (densitydb_fileopen) */


int densitydb_fileclose(DENSITYDB *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	}

	return rs ;
}
/* end subroutine (densitydb_fileclose) */


/* buffer mangement stuff */
static int densitydb_bufbegin(DENSITYDB *op)
{
	const int	size = DENSITYDB_BUFSIZE ;
	int		rs ;

	op->b.off = 0 ;
	op->b.len = 0 ;
	op->b.size = 0 ;
	op->b.buf = NULL ;

#if	CF_DEBUGS
	debugprintf("densitydb_bufbegin: size=%u\n",size) ;
#endif

	if ((rs = uc_malloc(size,&op->b.buf)) >= 0) {
	    op->b.size = size ;
	}

	return rs ;
}
/* end subroutine (densitydb_bufbegin) */


static int densitydb_bufend(DENSITYDB *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->b.buf != NULL) {
	    rs1 = uc_free(op->b.buf) ;
	    if (rs >= 0) rs = rs1 ;
	    op->b.buf = NULL ;
	}

	op->b.size = 0 ;
	op->b.len = 0 ;
	return rs ;
}
/* end subroutine (densitydb_bufend) */


/* try to buffer up some of the file */
static int densitydb_buf(DENSITYDB *op,uint roff,int rlen,char **rpp)
{
	offset_t	foff ;
	uint		bext, bend, fext, fend ;
	uint		rext = (roff + rlen), ext ;
	int		rs = SR_OK ;
	int		len = rlen ;
	char		*rbuf ;

#if	CF_DEBUGS
	debugprintf("densitydb_buf: ent roff=%u rlen=%u\n",roff,rlen) ;
	debugprintf("densitydb_buf: b.size=%u b.off=%u b.len=%u \n",
	    op->b.size,op->b.off,op->b.len) ;
#endif

/* do we need to read in more data? */

	fext = op->b.off + op->b.len ;
	if ((roff < op->b.off) || (rext > fext)) {

#if	CF_DEBUGS
	    debugprintf("densitydb_buf: need more data\n") ;
#endif

/* can we do an "add-on" type read operation? */

	    bend = op->b.off + op->b.size ;
	    if ((roff >= op->b.off) && (rext <= bend)) {

#if	CF_DEBUGS
	        debugprintf("densitydb_buf: add-on read\n") ;
#endif

	        foff = op->b.off + op->b.len ;
	        rbuf = op->b.buf + op->b.len ;

	        ext = roff + MAX(rlen,DENSITYDB_READSIZE) ;
	        fext = uceil(ext,op->pagesize) ;

	        if (fext > bend)
	            fext = bend ;

	        len = (fext - foff) ;

#if	CF_DEBUGS
	        debugprintf("densitydb_buf: u_pread() foff=%llu len=%u\n",
	            foff,len) ;
#endif

	        if ((rs = u_pread(op->fd,rbuf,len,foff)) >= 0) {
	            op->b.len += rs ;
	            len = MIN(((op->b.off + op->b.len) - roff),rlen) ;
	        }

	    } else {

#if	CF_DEBUGS
	        debugprintf("densitydb_buf: fresh read\n") ;
#endif

	        op->b.off = roff ;
	        op->b.len = 0 ;

	        bend = roff + op->b.size ;

	        foff = roff ;
	        rbuf = op->b.buf ;

	        ext = roff + MAX(rlen,DENSITYDB_READSIZE) ;
	        fext = uceil(ext,op->pagesize) ;

	        if (fext > bend)
	            fext = bend ;

	        len = fext - foff ;
	        if ((rs = u_pread(op->fd,op->b.buf,len,foff)) >= 0) {
	            op->b.len = rs ;
	            len = MIN(rs,rlen) ;
	        }

	    } /* end if */

	} /* end if (needed to read more data) */

	if (rpp != NULL) {
	    *rpp = op->b.buf + (roff - op->b.off) ;
	}

#if	CF_DEBUGS
	debugprintf("densitydb_buf: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (densitydb_buf) */


/* update the file buffer from a user supplied source */
static int densitydb_bufupdate(DENSITYDB *op,uint roff,int rbuflen,cchar *rbuf)
{
	uint		boff, bext ;
	uint		rext = roff + rbuflen ;
	int		buflen, bdiff ;

	buflen = op->b.len ;
	boff = op->b.off ;
	bext = boff + buflen ;

	if (roff < boff) {

	    if (rext <= boff)
	        return 0 ;

	    rbuf += (boff - roff) ;
	    rbuflen -= (boff - roff) ;
	    roff = boff ;

	}

	if (rext > bext) {

	    if (roff >= bext)
	        return 0 ;

	    rbuflen -= (rext - bext) ;
	    rext = bext ;

	}

	if (rbuflen > 0) {
	    bdiff = roff - boff ;
	    memcpy((op->b.buf + bdiff),rbuf,rbuflen) ;
	}

	return rbuflen ;
}
/* end subroutine (densitydb_bufupdate) */


/* write out the file header */
static int densitydb_writehead(DENSITYDB *op)
{
	int		rs ;
	char		fbuf[DENSITYDB_FBUFLEN + 1] ;

	if ((rs = filehead(fbuf,0,&op->h)) >= 0) {
	    const offset_t	uoff = DENSITYDB_FOHEAD ;
	    const int		bl = rs ;
	    rs = u_pwrite(op->fd,fbuf,bl,uoff) ;
	}

	return rs ;
}
/* end subroutine (densitydb_writehead) */


static int filemagic(char *buf,int f_read,DENSITYDB_FM *mp)
{
	int		rs = 20 ;
	char		*bp = buf ;
	char		*cp ;

	if (buf == NULL) return SR_BADFMT ;

	if (f_read) {

	    bp[15] = '\0' ;
	    strncpy(mp->magic,bp,15) ;

	    if ((cp = strchr(mp->magic,'\n')) != NULL) {
	        *cp = '\0' ;
	    }

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
static int filehead(buf,f,hp)
char		buf[] ;
int		f ;
DENSITYDB_FH	*hp ;
{
	SERIALBUF	msgbuf ;
	const int	bsize = sizeof(DENSITYDB_FH) ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&msgbuf,buf,bsize)) >= 0) {

	    if (f) {
	        serialbuf_ruint(&msgbuf,&hp->wcount) ;
	        serialbuf_ruint(&msgbuf,&hp->wtime) ;
	        serialbuf_ruint(&msgbuf,&hp->nentries) ;
	    } else {
	        serialbuf_wuint(&msgbuf,hp->wcount) ;
	        serialbuf_wuint(&msgbuf,hp->wtime) ;
	        serialbuf_wuint(&msgbuf,hp->nentries) ;
	    }

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (filehead) */


static int extutime(char *ep)
{
	DENSITYDBE_UPD	m1 ;
	uint		*ip ;

#if	CF_CHEATEXT
	ip = (uint *) (ep + DENSITYDBE_OMTIME) ;
	m1.utime = ntohl(*ip) ;
#else
	densitydbe_update(ep,DENSITYDBE_SIZE,1,&m1) ;
#endif

	return m1.utime ;
}
/* end subroutine (extutime) */


