/* postfile */

/* indexed POST file */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		1		/* safe mode */
#define	CF_LOCKING	0		/* use file locking */
#define	CF_HOLDING	1		/* use file-map holding */
#define	CF_MEMSYNC	1		/* use memory synchronization ? */


/* revision history:

	= 2002-05-01, David A­D­ Morano

	This object module was created for Levo research, to determine
	if a conditional branch at a given instruction address is
	a SS-Hamock or not.


	= 2003-06-11, David A­D­ Morano

	I snarfed this file from the SS-Hammock crap since I thought
	it might be a bit similar.  We'll see how it works out ! :-)


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This object module provides an interface to a data base of
	information about the GECOS name in the system PASSWD database.

	The system PASSWD database (whether a file or whatever) was
	enumerated (separately) and an index file was made with several
	indices to lookup usernames based on a real name.  For starters
	only indices consisting of using the first character of the
	last name, the first 3 characters of the last name, and the
	first character of the first name have been used.  But future
	index files might provide more combinations, like using the
	first 3 characters of the last name combined with the first
	character of the first name !

	The various data and indices were written into a file.
	We are accessing that file within this object.


*****************************************************************************/


#define	POSTFILE_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>		/* Memory Management */
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storeitem.h>
#include	<dstr.h>
#include	<realname.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"postfile.h"


/* local defines */

#define	POSTFILE_MAGIC	0x23456787

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	TO_FILECOME	5
#define	TO_OPEN		(60 * 60)
#define	TO_ACCESS	(1 * 60)
#define	TO_LOCK		10		/* seconds */

#define	TI_MINUPDATE	4		/* minimum time between updates */

#ifndef	ENDIAN
#if	defined(SOLARIS) && defined(__sparc)
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

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	NSHIFT	6


/* external subroutines */

extern uint	nextpowtwo(uint) ;
extern uint	hashelf(const void *,int) ;

extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	randlc(int) ;
extern int	isfsremote(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* forward references */

static uint	ceiling(uint,uint) ;

static int	postfile_fileheader(POSTFILE *) ;
static int	postfile_holdget(POSTFILE *,time_t) ;
static int	postfile_holdrelease(POSTFILE *,time_t) ;

#if	CF_LOCKING
static int	postfile_lockget(POSTFILE *,int) ;
static int	postfile_lockrelease(POSTFILE *) ;
#endif

static int	postfile_fileopen(POSTFILE *,time_t) ;
static int	postfile_fileclose(POSTFILE *) ;
static int	postfile_keymatchlast(POSTFILE *,int,int,char *,int) ;
static int	postfile_keymatchall(POSTFILE *,int,int,REALNAME *) ;

static int	hashindex(uint,uint) ;


/* local variables */

enum headers {
	header_writetime,
	header_writecount,
	header_rectab,
	header_rectablen,
	header_strtab,
	header_strtabsize,
	header_indexl1,
	header_indexl3,
	header_indexf,
	header_indexlen,
	header_overlast
} ;

static const char	*localfs[] = {
	    "ufs",
	    "tmpfs",
	    "lofs",
	    NULL
} ;


/* exported subroutines */


int postfile_open(op,fname)
POSTFILE	*op ;
const char	fname[] ;
{
	struct ustat	sb ;

	uint	*table ;

	time_t	daytime ;

	int	rs ;
	int	i, cl ;
	int	prot, flags ;
	int	magiclen ;
	int	size ;

	const char	*cp ;


	if (op == NULL)
	    return SR_FAULT ;

	if (fname == NULL)
	    return SR_FAULT ;

	(void) memset(op,0,sizeof(POSTFILE)) ;

	op->fd = -1 ;
	op->oflags = O_RDONLY ;
	op->operm = 0666 ;

	rs = u_open(fname,op->oflags,op->operm) ;

#if	CF_DEBUGS
	debugprintf("postfile_open: u_open() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	op->fd = rs ;
	rs = u_fstat(op->fd,&sb) ;

	if (rs < 0)
	    goto bad1 ;

/* local or remote */

	rs = isfsremote(op->fd) ;
	op->f.remote = (i > 0) ;
	if (rs < 0)
	    goto bad1 ;

/* store filename away */

	op->fname = mallocstr(fname) ;
	if (op->fname == NULL) {
	    rs = SR_NOMEM ;
	    goto bad1 ;
	}

	size = 16 + 4 + (header_overlast * sizeof(int)) ;

#if	CF_DEBUGS
	debugprintf("postfile_open: filesize=%d headersize=%d\n",
	    sb.st_size,size) ;
#endif

/* wait for the file to come in if it is not yet available */

	for (i = 0 ; (i < TO_FILECOME) && (rs >= 0) && (sb.st_size < size) ;
	    i += 1) {

	    sleep(1) ;

	    rs = u_fstat(op->fd,&sb) ;

	} /* end while */

	if (rs < 0)
	    goto bad2 ;

	rs = SR_TIMEDOUT ;
	if (i >= TO_FILECOME)
	    goto bad2 ;

	if (sb.st_size < size)
	    goto bad2 ;

	op->mtime = sb.st_mtime ;
	op->filesize = sb.st_size ;
	op->pagesize = getpagesize() ;

/* OK, continue on */

	if (op->filesize >= size) {

/* map it */

#if	CF_DEBUGS
	    debugprintf("postfile_open: mapping file\n") ;
#endif

	    prot = PROT_READ ;
	    flags = MAP_SHARED ;
	    rs = u_mmap(NULL,(size_t) op->filesize,prot,flags,
	        op->fd,0L,&op->mapbuf) ;

#if	CF_DEBUGS
	    debugprintf("postfile_open: u_mmap() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto bad2 ;

/* OK, check it out */

	    rs = postfile_fileheader(op) ;

	    if (rs < 0)
	        goto bad3 ;

/* ok, we're good (?) */

	    u_time(&daytime) ;

	    op->opentime = daytime ;
	    op->accesstime = daytime ;

	    op->f.fileinit = TRUE ;

#if	CF_DEBUGS
	    debugprintf("postfile_open: header processing completed \n") ;
#endif

	} /* end if (file had some data) */

/* we should be good */

	op->magic = POSTFILE_MAGIC ;

ret1:
	u_close(op->fd) ;
	op->fd = -1 ;

ret0:
	return rs ;

/* we're out of here */
bad3:
	u_munmap(op->mapbuf,(size_t) op->filesize) ;

bad2:
	free(op->fname) ;

bad1:
	u_close(op->fd) ;
	op->fd = -1 ;

bad0:

#if	CF_DEBUGS
	debugprintf("postfile_open: bad ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (postfile_open) */


/* get the string count in the table */
int postfile_count(op)
POSTFILE	*op ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != POSTFILE_MAGIC)
	    return SR_NOTOPEN ;

	return (op->rtlen - 1) ;
}
/* end subroutine (postfile_count) */


/* calculate the index table length (number of entries) at this point */
int postfile_countindex(op)
POSTFILE	*op ;
{
	int	n ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != POSTFILE_MAGIC)
	    return SR_NOTOPEN ;

	return op->rilen ;
}
/* end subroutine (postfile_countindex) */


/* initialize a cursor */
int postfile_curbegin(op,cp)
POSTFILE	*op ;
POSTFILE_CUR	*cp ;
{
	int	i ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != POSTFILE_MAGIC)
	    return SR_NOTOPEN ;

	if (cp == NULL)
	    return SR_FAULT ;

	op->f.cursor = TRUE ;
	op->f.cursorlockbroken = FALSE ;
	op->f.cursoracc = FALSE ;

	for (i = 0 ; i < POSTFILE_NINDICES ; i += 1)
	    cp->i[i] = -1 ;

	cp->magic = POSTFILE_MAGIC ;
	return SR_OK ;
}
/* end subroutine (postfile_curbegin) */


/* free up a cursor */
int postfile_curend(op,cp)
POSTFILE	*op ;
POSTFILE_CUR	*cp ;
{
	time_t	daytime ;

	int	rs, i ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != POSTFILE_MAGIC)
	    return SR_NOTOPEN ;

	if (cp == NULL)
	    return SR_FAULT ;

	u_time(&daytime) ;

	if (op->f.cursoracc)
	    op->accesstime = daytime ;

	op->f.cursor = FALSE ;

#if	CF_LOCKING
	if (op->f.lockedread || op->f.lockedwrite)
	    postfile_lockrelease(op) ;
#endif

#if	CF_HOLDING
	if (op->f.held)
	    rs = postfile_holdrelease(op,daytime) ;
#endif

	for (i = 0 ; i < POSTFILE_NINDICES ; i += 1)
	    cp->i[i] = -1 ;

	cp->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (postfile_curend) */


/* enumerate */
int postfile_enum(op,cup,np,up)
POSTFILE	*op ;
POSTFILE_CUR	*cup ;
REALNAME	*np ;
char		*up ;
{
	time_t	daytime ;

	int	rs = SR_OK, ri, si ;
	int	ui ;

	char	*sp ;
	char	*cp ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != POSTFILE_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (cup == NULL)
	    return SR_FAULT ;

	if (! op->f.cursor)
	    return SR_INVALID ;

	ri = (cup->i[0] < 1) ? 1 : (cup->i[0] + 1) ;

#if	CF_DEBUGS
	debugprintf("postfile_enum: ri=%d\n",ri) ;
#endif

/* capture a hold on the file */

#if	CF_HOLDING
	if (! op->f.held) {

	    u_time(&daytime) ;

	    rs = postfile_holdget(op,daytime) ;

	    if (rs > 0)
	        rs = postfile_fileheader(op) ;

	    if (rs < 0)
	        return rs ;

	}
#endif /* CF_HOLDING */


/* ok, we're good to go */

	if (ri >= op->rtlen)
	    return SR_NOTFOUND ;

	if (np != NULL) {

	    DSTR	ss[5] ;


/* first */

	    si = op->rectab[ri].first ;
	    ss[0].s = (char *) (op->stab + si) ;
	    ss[0].slen = -1 ;

/* middle-1 */

	    si = op->rectab[ri].m1 ;
	    ss[1].s = (char *) (op->stab + si) ;
	    ss[1].slen = -1 ;

/* middle-2 */

	    si = op->rectab[ri].m2 ;
	    ss[2].s = (char *) (op->stab + si) ;
	    ss[2].slen = -1 ;

/* middle=3 */

	    ss[3].s = NULL ;
	    ss[3].slen = 0 ;

/* last */

	    si = op->rectab[ri].last ;
	    ss[4].s = (char *) (op->stab + si) ;
	    ss[4].slen = -1 ;

/* done */

	    rs = realname_startparts(np,ss) ;

	} /* end if */

	if (rs >= 0) {

	    ui = op->rectab[ri].username ;

#if	CF_DEBUGS
	    debugprintf("postfile_enum: ui=%d\n",ui) ;
#endif

	    if (up != NULL) {

	        cp = strwcpy(up,(op->stab + ui),POSTFILE_USERNAMELEN) ;

	        rs = cp - up ;

	    } else
	        rs = strlen(op->stab + ui) ;

/* update the cursor */

	    cup->i[0] = ri ;

	} /* end if */

	return rs ;
}
/* end subroutine (postfile_enum) */


/* fetch an entry by key lookup */
int postfile_fetch(op,np,cup,opts,up)
POSTFILE	*op ;
REALNAME	*np ;
POSTFILE_CUR	*cup ;
int		opts ;
char		*up ;
{
	POSTFILE_CUR	cur ;

	time_t		daytime = 0 ;

	uint		rhash ;

	const int	ns = NSHIFT ;

	int	rs = SR_OK, i, wi, si, ui, hi, ri ;
	int	hl, ul, c, n ;
	int	klen[POSTFILE_NINDICES] ;
	int	f_cur = FALSE ;

	char	*sp ;
	char	*cp ;

#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != POSTFILE_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (cup == NULL) {

	    cup = &cur ;
	    for (i = 0 ; i < POSTFILE_NINDICES ; i += 1)
	        cup->i[i] = -1 ;

	} else {

	    f_cur = TRUE ;
	    if (! op->f.cursor)
	        return SR_INVALID ;

	}

	if (np == NULL)
	    return SR_FAULT ;

#ifdef	COMMENT
	if (up != NULL)
	    *up = '\0' ;
#endif

/* do we have a hold on the file ? */

#if	CF_HOLDING
	if (! op->f.held) {

	    u_time(&daytime) ;

	    rs = postfile_holdget(op,daytime) ;

	    if (rs > 0) {

#if	CF_DEBUGS
	        debugprintf("postfile_fetch: file changed\n") ;
#endif

	        rs = postfile_fileheader(op) ;

	    }

	    if (rs < 0)
	        return rs ;

	}
#endif /* CF_HOLDING */

/* continue with regular fetch activities */

	op->f.cursoracc = TRUE ;	/* doesn't hurt if no cursor ! */
	n = op->rilen ;

/* which index do we want to use ? */

	wi = (np->len.last < 3) ? 0 : 1 ;

#if	CF_DEBUGS
	debugprintf("postfile_fetch: wi=%d\n",wi) ;
#endif

/* OK, we go from here */

	if (cup->i[wi] < 0) {

#if	CF_DEBUGS
	    debugprintf("postfile_fetch: new cursor\n") ;
#endif

	    sp = np->last ;
	    hl = MIN(np->len.last,((wi == 0) ? 1 : 3)) ;

	    rhash = hashelf(sp,hl) ;

	    hi = hashindex(rhash,op->rilen) ;

#if	CF_DEBUGS
	    debugprintf("postfile_fetch: rhash=%08x hi=%d\n",rhash,hi) ;
#endif

/* start searching ! */

	    if (op->ropts & POSTFILE_OSEC) {

#if	CF_DEBUGS
	        debugprintf("postfile_fetch: secondary initial ri=%d\n",
	            (op->recind[wi])[hi][0]) ;
#endif

	        c = 0 ;
	        while ((ri = (op->recind[wi])[hi][0]) != 0) {

#if	CF_DEBUGS
	            debugprintf("postfile_fetch: keymatchlast ri=%d\n",ri) ;
#endif

	            if (postfile_keymatchlast(op,opts,ri,sp,hl))
	                break ;

	            op->collisions += 1 ;
	            if (op->ropts & POSTFILE_ORANDLC)
	                rhash = randlc(rhash + c) ;

	            else
	                rhash = ((rhash << (32 - ns)) | (rhash >> ns)) + c ;

	            hi = hashindex(rhash,n) ;

	            c += 1 ;

	        } /* end while */

#if	CF_DEBUGS
	        debugprintf("postfile_fetch: index-key-match ri=%d\n",ri) ;
#endif

	        if (ri == 0)
	            rs = SR_NOTFOUND ;

	    } /* end if (secondard hasing) */

	} else {

#if	CF_DEBUGS
	    debugprintf("postfile_fetch: old cursor\n") ;
#endif

	    hi = cup->i[wi] ;
	    if (hi != 0) {

	        ri = (op->recind[wi])[hi][0] ;
	        if (ri != 0) {

	            hi = (op->recind[wi])[hi][1] ;
	            if (hi != 0)
	                ri = (op->recind[wi])[hi][0] ;

	            else
	                rs = SR_NOTFOUND ;

	        }

	    } else
	        rs = SR_NOTFOUND ;

	} /* end if (preparation) */

#if	CF_DEBUGS
	debugprintf("postfile_fetch: match search hi=%d\n",hi) ;
#endif

	if (rs >= 0) {

	    while ((ri = (op->recind[wi])[hi][0]) != 0) {

	        if (postfile_keymatchall(op,opts,ri,np))
	            break ;

	        hi = (op->recind[wi])[hi][1] ;
	        if (hi == 0)
	            break ;

	        op->collisions += 1 ;

	    } /* end while */

	    if ((ri == 0) || (hi == 0))
	        rs = SR_NOTFOUND ;

	} /* end if (following the existing chain) */

	if (rs >= 0) {

	    ui = op->rectab[ri].username ;
	    if (up != NULL) {

	        cp = strwcpy(up,(op->stab + ui),POSTFILE_USERNAMELEN) ;

#if	CF_DEBUGS
	        debugprintf("postfile_fetch: username=%s\n",up) ;
#endif

	        ul = (cp - up) ;

	    } else
	        ul = strlen(op->stab + ui) ;

/* update cursor */

	    if (f_cur)
	        cup->i[wi] = hi ;

	} /* end if (got one) */

#if	CF_HOLDING
	if (! op->f.cursor) {

	    if (daytime == 0)
	        u_time(&daytime) ;

	    op->accesstime = daytime ;
	    rs = postfile_holdrelease(op,daytime) ;

	}
#endif /* CF_HOLDING */

#if	CF_DEBUGS
	debugprintf("postfile_fetch: ret rs=%d ul=%d\n",rs,ul) ;
#endif

	return (rs >= 0) ? ul : rs ;
}
/* end subroutine (postfile_fetch) */


#ifdef	COMMENT

/* get the entries (serially) */
int postfile_get(op,ri,rpp)
POSTFILE	*op ;
int		ri ;
POSTFILE_ENT	**rpp ;
{


#if	CF_DEBUGS
	debugprintf("postfile_get: entered 0\n") ;
#endif

#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != POSTFILE_MAGIC)
	    return SR_NOTOPEN ;
#endif

#if	CF_DEBUGS
	debugprintf("postfile_get: entered 2\n") ;
#endif

	if (rpp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("postfile_get: entered ri=%d\n",ri) ;
#endif

	if ((ri < 0) || (ri >= op->rtlen))
	    return SR_NOTFOUND ;

	*rpp = NULL ;
	if (ri > 0)
	    *rpp = op->rectab + ri ;

#if	CF_DEBUGS
	debugprintf("postfile_get: OK\n") ;
#endif

	return ri ;
}
/* end subroutine (postfile_get) */

#endif /* COMMENT */


/* get information about all static branches */
int postfile_info(op,rp)
POSTFILE	*op ;
POSTFILE_INFO	*rp ;
{
	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != POSTFILE_MAGIC)
	    return SR_NOTOPEN ;

	if (rp == NULL)
	    return SR_FAULT ;

	if (rp != NULL) {

	    rp->collisions = op->collisions ;

	}

	return rs ;
}
/* end subroutine (postfile_info) */


/* do some checking */
int postfile_check(op,daytime)
POSTFILE	*op ;
time_t		daytime ;
{
	int	rs = SR_OK ;

#if	CF_DEBUGS
	char	timebuf[TIMEBUFLEN + 1] ;
#endif


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != POSTFILE_MAGIC)
	    return SR_NOTOPEN ;

	if (op->fd < 0)
	    return SR_OK ;

#if	CF_DEBUGS
	debugprintf("postfile_check: %s\n",
	    timestr_log(daytime,timebuf)) ;
#endif

	if ((daytime - op->accesstime) > TO_ACCESS)
	    goto closeit ;

	if ((daytime - op->opentime) > TO_OPEN)
	    goto closeit ;

	return rs ;

/* handle a close out */
closeit:
	rs = postfile_fileclose(op) ;

	return rs ;
}
/* end subroutine (postfile_check) */


/* free up this postfile object */
int postfile_close(op)
POSTFILE	*op ;
{
	int	rs = SR_BADFMT ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != POSTFILE_MAGIC)
	    return SR_NOTOPEN ;

	if (op->mapbuf != NULL)
	    rs = u_munmap(op->mapbuf,(size_t) op->filesize) ;

	if (op->fd >= 0)
	    u_close(op->fd) ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (postfile_close) */



/* INTERNAL SUBROUTINES */



/* read the file header and check it out */
static int postfile_fileheader(op)
POSTFILE	*op ;
{
	uint	*table ;

	int	rs = SR_OK, i ;
	int	f ;

	char	*cp ;


	cp = (char *) op->mapbuf ;
	f = (strncmp(cp,POSTFILE_FILEMAGIC,POSTFILE_FILEMAGICLEN) == 0) ;
	f = f && (*(cp + POSTFILE_FILEMAGICLEN) == '\n') ;

	if (! f) {

#if	CF_DEBUGS
	    debugprintf("postfile_fileheader: bad magic=>%t<\n",
	        cp,strnlen(cp,14)) ;
#endif

	    rs = SR_BADFMT ;
	    goto bad3 ;
	}

	cp += 16 ;
	if (cp[0] > POSTFILE_FILEVERSION) {

	    rs = SR_NOTSUP ;
	    goto bad3 ;
	}

	if (cp[1] != ENDIAN) {

	    rs = SR_NOTSUP ;
	    goto bad3 ;
	}

/* the recorder options */

	op->ropts = cp[2] ;

#if	CF_DEBUGS
	debugprintf("postfile_fileheader: ropts=%02x\n",op->ropts) ;
#endif

/* if looks good, read the header stuff */

	table = (uint *) (op->mapbuf + 16 + 4) ;

#if	CF_DEBUGS
	{
	    int	i ;


	    for (i = 0 ; i < header_overlast ; i += 1)
	        debugprintf("postfile_fileheader: header[%d]=%08x\n",
	            i,table[i]) ;

	}
#endif /* CF_DEBUGS */

	op->rectab = (POSTFILE_ENT *) (op->mapbuf + table[header_rectab]) ;
	op->rtlen = table[header_rectablen] ;

	op->stab = (const char *) (op->mapbuf + table[header_strtab]) ;
	op->stlen = table[header_strtabsize] ;

	op->recind[0] = (uint (*)[2]) (op->mapbuf + table[header_indexl1]) ;
	op->recind[1] = (uint (*)[2]) (op->mapbuf + table[header_indexl3]) ;
	op->recind[3] = (uint (*)[2]) (op->mapbuf + table[header_indexf]) ;

	op->rilen = table[header_indexlen] ;

#if	CF_DEBUGS
	debugprintf("postfile_fileheader: rtlen=%d rilen=%d\n",op->rtlen,op->rilen) ;
	debugprintf("postfile_fileheader: stlen=%d \n",op->stlen) ;
#endif

#if	CF_DEBUGS
	{
	    int	i ;


	    for (i = 1 ; i < op->rtlen ; i += 1) {

	        debugprintf("postfile_fileheader: rec[%d] username=%d last=%d\n",
	            i,op->rectab[i].username,op->rectab[i].last) ;

	    }

	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	{
	    int	i ;

	    const char	*ol = op->stab + op->stlen ;


	    cp = (char *) (op->stab + 1) ;
	    for (i = 0 ; cp < ol ; i += 1) {

	        debugprintf("postfile_fileheader: s[%d]=%s\n",
	            i,cp) ;

	        cp += (strlen(cp) + 1) ;
	    }

	}
#endif /* CF_DEBUGS */

ret0:
	return rs ;

/* bad stuff comes here */
bad3:
	return rs ;
}
/* end subroutine (ipsswd_fileheader) */


/* acquire access to the file (mapped memory) */
static int postfile_holdget(op,daytime)
POSTFILE	*op ;
time_t		daytime ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	mpages, fpages ;
	int	prot, flags ;
	int	f_changed = FALSE ;
	int	f_opened = FALSE ;


#if	CF_DEBUGS
	debugprintf("postfile_holdget: entered\n") ;
#endif

	if ((daytime - op->accesstime) < TO_ACCESS)
	    return SR_OK ;

/* has the file changed at all (try checking size and mtime) ? */

	rs = u_stat(op->fname,&sb) ;

	if (rs == SR_NOENT)
	    return SR_OK ;

	if (rs < 0)
	    goto bad2 ;

	if (op->mapbuf != NULL) {

/* has the file size changed ? */

	    if (sb.st_size != op->filesize) {

	        f_changed = TRUE ;
	        mpages = ceiling(op->mapsize,op->pagesize) ;

	        fpages = ceiling(sb.st_size,op->pagesize) ;

	        if (fpages > mpages) {

	            f_changed = TRUE ;
	            u_munmap(op->mapbuf,(size_t) op->mapsize) ;

	            op->mapbuf = NULL ;	/* signal a re-map */

	        }

	    } /* end if (file size changed) */

#if	CF_MEMSYNC
	    if ((op->mapbuf != NULL) &&
	        op->f.remote && (sb.st_mtime != op->mtime)) {

	        f_changed = TRUE ;
	        flags = MS_SYNC | MS_INVALIDATE ;
	        rs = uc_msync(op->mapbuf,op->mapsize,flags) ;

	    }
#endif /* CF_MEMSYNC */

	} /* end if (checking existing map) */

/* do a map or a re-map */

	if ((rs >= 0) && (op->mapbuf == NULL)) {

	    f_changed = TRUE ;
	    rs = postfile_fileopen(op,daytime) ;

	    if (rs >= 0) {

	        op->mapsize = ceiling(sb.st_size,op->pagesize) ;

	        if (op->mapsize == 0)
	            op->mapsize = op->pagesize ;

	        prot = PROT_READ ;
	        flags = MAP_SHARED ;
	        rs = u_mmap(((caddr_t) 0),(size_t) op->mapsize,prot,flags,
	            op->fd,0L,&op->mapbuf) ;

#if	CF_DEBUGS
	        debugprintf("postfile_holdget: u_mmap() rs=%d\n",rs) ;
#endif

	        postfile_fileclose(op) ;

	    } /* end if (opened the file) */

	    if (rs < 0)
	        goto bad2 ;

	} /* end if (mapping file) */

	if (f_changed) {

	    op->filesize = sb.st_size ;
	    op->mtime = sb.st_mtime ;

	}

#if	CF_DEBUGS
	debugprintf("postfile_holdget: ret rs=%d f_changed=%d\n",
	    rs,f_changed) ;
#endif

	if (rs >= 0)
	    op->f.held = TRUE ;

	return (rs >= 0) ? f_changed : rs ;

/* bad stuff */
bad2:

bad1:
	op->f.held = FALSE ;

bad0:
	return rs ;
}
/* end subroutine (postfile_holdget) */


/* release our hold on the filemap */
static int postfile_holdrelease(op,daytime)
POSTFILE	*op ;
time_t		daytime ;
{
	int	rs = SR_OK ;


	if (! op->f.held)
	    return SR_OK ;


	op->f.held = FALSE ;

	return rs ;
}
/* end subroutine (postfile_holdrelease) */


#if	CF_LOCKING

/* acquire access to the file (mapped memory) */
static int postfile_lockget(op,daytime,f_read)
POSTFILE	*op ;
time_t		daytime ;
int		f_read ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	mpages, fpages ;
	int	prot, flags ;
	int	f_changed = FALSE ;


#if	CF_DEBUGS
	debugprintf("postfile_lockget: entered\n") ;
#endif

	if (op->fd < 0)
	    rs = postfile_fileopen(op,daytime) ;

#if	CF_DEBUGS
	debugprintf("postfile_lockget: postfile_fileopen() rs=%d fd=%d\n",
	    rs,op->fd) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	if (f_read) {

#if	CF_DEBUGS
	    debugprintf("postfile_lockget: need READ lock\n") ;
#endif

	    op->f.lockedread = TRUE ;
	    rs = lockfile(op->fd,F_RLOCK,0L,op->filesize,TO_LOCK) ;

	} else {

#if	CF_DEBUGS
	    debugprintf("postfile_lockget: need WRITE lock\n") ;
#endif

	    op->f.lockedwrite = TRUE ;
	    rs = lockfile(op->fd,F_WLOCK,0L,op->filesize,TO_LOCK) ;

	}

#if	CF_DEBUGS
	debugprintf("postfile_lockget: lockfile() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

/* has the size of the file changed ? */

	rs = u_fstat(op->fd,&sb) ;

	if (rs < 0)
	    goto bad2 ;

	op->filesize = sb.st_size ;
	if (op->mapbuf != NULL) {

	    mpages = ceiling(op->mapsize,op->pagesize) ;

	    fpages = ceiling(op->filesize,op->pagesize) ;

	    if (fpages > mpages) {

	        f_changed = TRUE ;
	        u_munmap(op->mapbuf,(size_t) op->mapsize) ;

	        op->mapbuf = NULL ;

	    }

#if	CF_MEMSYNC
	    if ((op->mapbuf != NULL) &&
	        op->f.remote && (op->mtime != sb.st_mtime)) {

	        f_changed = TRUE ;
	        op->mtime = sb.st_mtime ;
	        flags = MS_SYNC | MS_INVALIDATE ;
	        rs = uc_msync(op->mapbuf,op->mapsize,flags) ;

	    }
#endif /* CF_MEMSYNC */

	} /* end if (checking existing map) */

	if ((rs >= 0) && (op->mapbuf == NULL)) {

	    f_changed = TRUE ;
	    op->mapsize = ceiling(op->filesize,op->pagesize) ;

	    if (op->mapsize == 0)
	        op->mapsize = op->pagesize ;

	    prot = PROT_READ | PROT_WRITE ;
	    flags = MAP_SHARED ;
	    rs = u_mmap(((caddr_t) 0),(size_t) op->mapsize,prot,flags,
	        op->fd,0L,&op->mapbuf) ;

#if	CF_DEBUGS
	    debugprintf("postfile_lockget: u_mmap() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto bad2 ;

	    op->mtime = sb.st_mtime ;

	} /* end if (mapping file) */

#if	CF_DEBUGS
	debugprintf("postfile_lockget: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_changed : rs ;

/* bad stuff */
bad2:
	lockfile(op->fd,F_ULOCK,0L,op->filesize,TO_LOCK) ;

bad1:
	op->f.lockedread = FALSE ;
	op->f.lockedwrite = FALSE ;

bad0:
	return rs ;
}
/* end subroutine (postfile_lockget) */


static int postfile_lockrelease(op)
POSTFILE	*op ;
{
	int	rs = SR_OK ;


	if (! (op->f.lockedread || op->f.lockedwrite))
	    return SR_OK ;

	if (op->fd >= 0)
	    rs = lockfile(op->fd,F_ULOCK,0L,op->filesize,TO_LOCK) ;

	op->f.lockedread = FALSE ;
	op->f.lockedwrite = FALSE ;

	return rs ;
}
/* end subroutine (postfile_lockrelease) */

#endif /* CF_LOCKING */


static int postfile_fileopen(op,daytime)
POSTFILE	*op ;
time_t		daytime ;
{
	int	rs ;


#if	CF_DEBUGS
	debugprintf("postfile_fileopen: fname=%s\n",op->fname) ;
#endif

	if (op->fd >= 0)
	    return op->fd ;

#if	CF_DEBUGS
	debugprintf("postfile_fileopen: need open\n") ;
#endif

	rs = u_open(op->fname,op->oflags,op->operm) ;

#if	CF_DEBUGS
	debugprintf("postfile_fileopen: u_open() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	op->fd = rs ;

	op->opentime = daytime ;
	return (rs >= 0) ? op->fd : rs ;

/* bad things */
bad0:
	return rs ;
}
/* end subroutine (postfile_fileopen) */


static int postfile_fileclose(op)
POSTFILE	*op ;
{
	int	rs = SR_OK ;


	if (op->fd >= 0) {

	    u_close(op->fd) ;

	    op->fd = -1 ;

	}

	return rs ;
}
/* end subroutine (postfile_fileclose) */


static int postfile_keymatchlast(op,opts,ri,sp,hl)
POSTFILE	*op ;
int		opts ;
int		ri ;
char		*sp ;
int		hl ;
{
	int	si ;
	int	f ;


	si = op->rectab[ri].last ;

#if	CF_DEBUGS
	debugprintf("keymatchlast: hl=%d sp=%t stab=%t\n",
	    hl,
	    sp,hl,
	    (op->stab + si),hl) ;
#endif

	if (opts & POSTFILE_FLASTFULL)
	    f = (strncmp((op->stab + si),sp,3) == 0) ;

	else
	    f = (strncmp((op->stab + si),sp,hl) == 0) ;

	return f ;
}
/* end subroutine (postfile_keymatchlast) */


static int postfile_keymatchall(op,opts,ri,np)
POSTFILE	*op ;
int		opts ;
int		ri ;
REALNAME	*np ;
{
	int	si ;

	const char	*s = op->stab ;
	const char	*sp ;


#if	CF_DEBUGS
	debugprintf("keymatchall: keyname last=%s first=%s\n",
	    np->last,np->first) ;
	debugprintf("keymatchall: ri=%d last=%s first=%s\n",ri,
	    (s + op->rectab[ri].last),
	    (s + op->rectab[ri].first)) ;
#endif

/* last */

	if (opts & POSTFILE_FLASTFULL) {

	    si = op->rectab[ri].last ;
	    if (strcmp((s + si),np->last) != 0)
	        return FALSE ;

	} else {

	    si = op->rectab[ri].last ;
	    if (strncmp((s + si),np->last,np->len.last) != 0)
	        return FALSE ;

	}

/* first */

	si = op->rectab[ri].first ;
	sp = np->first ;
	if ((sp != NULL) && (sp[0] != '\0')) {

	    si = op->rectab[ri].first ;
	    if (strncmp((s + si),sp,np->len.first) != 0)
	        return FALSE ;

	}

/* middle-1 */

	si = op->rectab[ri].m1 ;
	sp = np->m1 ;
	if ((sp != NULL) && (sp[0] != '\0')) {

	    si = op->rectab[ri].m1 ;
	    if (strncmp((s + si),sp,np->len.m1) != 0)
	        return FALSE ;

	}

/* middle-2 */

	si = op->rectab[ri].m2 ;
	sp = np->m2 ;
	if ((sp != NULL) && (sp[0] != '\0')) {

	    si = op->rectab[ri].m2 ;
	    if (strncmp((s + si),sp,np->len.m2) != 0)
	        return FALSE ;

	}

#if	CF_DEBUGS
	debugprintf("keymatchall: got one\n") ;
#endif

	return TRUE ;
}
/* end subroutine (postfile_keymatchall) */


/* calculate the next hash table index from a given one */
static int hashindex(i,n)
uint	i, n ;
{
	int	hi ;


	hi = MODP2(i,n) ;

	if (hi == 0)
	    hi = 1 ;

	return hi ;
}
/* end if (hashindex) */


static uint ceiling(v,a)
uint	v, a ;
{


	return (v + (a - 1)) & (~ (a - 1)) ;
}
/* end subroutine (ceiling) */



