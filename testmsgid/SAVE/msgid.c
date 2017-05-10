/* msgid */

/* object to manipulate a MSGID file */


#define	F_DEBUGS	0		/* non-switchable debug print-outs */
#define	F_SAFE		1
#define	F_CREAT		0		/* always create the file ? */
#define	F_SOLARISBUG	1		/* work around Solaris MMAP bug */
#define	F_LOCKF		0		/* use 'lockf(3c)' */
#define	F_NIENUM	0		/* perform NI updates on ENUM */
#define	F_NISEARCH	0		/* perform NI updates on SEARCH */


/* revision history :

	= 91/06/01, Dave Morano

	This subroutine was originally written.


	= 03/06/26, Dave Morano

	Although this object works, it was only a micracle that it
	did.  There is a feature-bug in Solaris that doesn't allow
	a file to be both mapped and locked at the same time (in
	either order).  But there seems to be a crack in the stupid
	Solaris implementation because it doesn't enforce its stupid
	bug carefully enough and this object here fell through the
	cracks and continued working by accident.  We were locking
	the whole file beyond its end and that appears to get by
	the Solaris police-state bug-patrol and was accidentally
	being allowed.

	I reworked a good bit of this code to eliminate any file
	mapping (so that we can continue to use file-record locks).
	This whole Solaris crap (this is being done on Solaris 8 right
	now) is really a pain and Sun should face punitive charges for
	inhumanity to the programmer community.  Solaris has some nice
	things since it was derived from the older (and better) System
	V UNIX, but has really messed it up by not allowing what used
	to be allowed in the old days with things like the old RFS
	facility.  Oh, while we're on the subject: NFS sucks cock meat !


*/


/******************************************************************************

	This subroutine maintains a MSGID file.  This file is used to
	maintain machine status for nodes in the local machine
	cluster.


	Format of file records :

	- (see the 'entry' structure in the header)


	Design note: 

	In summary, Solaris sucks cock meat !  Solaris does not allow a
	file to be memory-mapped from an NFS remote server AND also be
	file-locked at the same time.  A lot of stupid Solaris
	documentation notes say something to the effect that the
	Solaris VM system cannot handle a remote file that is both
	mapped and subject to file-locking at the same time.  They use
	some sort of stupid circular reasoning that if any file is
	being file-locked, then obviously it cannot be memory-mapped
	since the file locking indicates that file-locking is taking
	place, and that obviously any file that is being file-locked
	cannot therefore also be memory mapped.  That is pretty much
	their reasoning -- I kid you not !

	Unfortunately, code, like this code here, that was first
	designed under System V UNIX that used file-locking AND memory
	mapping together really needs to be changed to eliminate either
	the file locking or the memory mapping.  Remote file were cross
	mounted in the late 80s and very early 90s using RFS (not
	stupid NFS).  The use of RFS provided many advantages not the
	least of them being full UFS file-system semantics, but it is
	not clear why Solaris took a step backward from simply allowing
	remote files to be both memory-mapped and file-locked at the
	same time.  Some bright light-bulb of a software developer must
	have gotten his underwear in a bunch at some point and decided
	to disallow both of these from ever occurring at the same time
	in Solaris.  We all have suffered from these dumb-butt Solaris
	developers since we have to take time out to rewrite old code
	(like this code here) to handle the case of stupid Solaris not
	allowing memory mapping for a file that is also file-locked.

	Implementation note:

	The code was actually running when files were being locked in
	their entirety and beyond their ends.  There was some sort of
	loop-hole in the stupid Solaris code that allowed a file to be
	both file-locked and memory mapped at the same time under
	certain circumstances.  However, there seemed to be problems
	with this code when other parties on other (remote) systems
	tried to do the same thing.  They sometimes failed with
	dead-lock types of errors (I forget the details).  As a result,
	I decided to change the code to fully comply with the stupid
	Solaris requirements that no remote file be both memory mapped
	and file locked at the same time.  Any code that is here now
	that has to be with mapping of files is really just code that
	now allocates local private memory.  This is done instead of
	using the process heap but was really done because it seemed to
	offer the minimal changes to the code to get a private memory
	access to the file while still retaining the ability to
	file-lock the file.  Finally, let me finish with the comment
	that Solaris sucks cock meat.

	Final note:

	Solaris sucks cock meat !  Give me back simultaneous memory
	mapping and file locking.  And while you're at it, give me back
	RFS also !  And to you stupid Solaris VM developers, get out of
	Solaris development.  Either get a new job somewhere else or
	think about committing suicide.  Either way, we can all be
	happier with one (or more) of those alternatives.

	Anecdotal note:

	Hey, you stupid Solaris developers: give me back the ability to
	push SOCKMOD on a TPI endpoint also !  Since you're so stupid,
	I know that you forgot that this was possible at one time.  You
	hosed that ability away when you botched up making Solaris
	2.6.


******************************************************************************/


#define	MSGID_MASTER		0


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
#include	<ctype.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<mapstrint.h>
#include	<mallocstuff.h>

#include	"misc.h"
#include	"msgid.h"
#include	"msgidentry.h"



/* local defines */

#define	MSGID_MAGIC		918245634

#define	MSGID_AFS		"msgida"
#define	MSGID_BFS		"msgidb"

#define	MSGID_IDLEN		(16 + 4)
#define	MSGID_HEADLEN		(3 * 4)
#define	MSGID_TOPLEN		(MSGID_IDLEN + MSGID_HEADLEN)

#define	MSGID_IDOFF		0
#define	MSGID_HEADOFF		(16 + 4)
#define	MSGID_TABOFF		(MSGID_HEADOFF + (3 * 4))

#define	MSGID_ENTRYSIZE		MSGIDENTRY_SIZE
#define	MSGID_EBS		((MSGIDNTRY_SIZE + 3) & (~ 3))
#define	MSGID_MAXFILESIZE	(4 * 1024 * 1024)

#define	TO_OPEN		(60 * 60)	/* maximum file-open time */
#define	TO_ACCESS	(2 * 60)	/* maximum access idle time */
#define	TO_LOCK		10		/* seconds */

#define	TI_MINUPDATE	4		/* minimum time between updates */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	NENTRIES	100

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



/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	strnlen(const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpyup(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

#define	A	(__STDC__ != 0) 
#define	B	defined(_POSIX_C_SOURCE) 
#define	C	defined(_XOPEN_SOURCE)

#if	(A != 0) || (B != 0) || (C != 0)
extern long	altzone ;
#endif


/* forward references */

static uint	ceiling(uint,uint) ;

static int	msgid_fileopen(MSGID *,time_t) ;
static int	msgid_fileclose(MSGID *) ;
static int	msgid_lockget(MSGID *,time_t,int) ;
static int	msgid_lockrelease(MSGID *) ;
static int	msgid_buf(MSGID *,uint,uint) ;
static int	msgid_fileinit(MSGID *,time_t) ;
static int	msgid_fileverify(MSGID *) ;
static int	msgid_fileheader(MSGID *,int) ;
static int	msgid_search(MSGID *,const char *,int,char **) ;

static int	namematch(const char *,const char *,int) ;


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







/* open a MSGID file */
int msgid_open(op,fname,oflags,operm)
MSGID		*op ;
const char	fname[] ;
int		oflags ;
int		operm ;
{
	struct stat	sb ;

	struct statvfs	fsb ;

	time_t	daytime ;

	int	rs, cl ;
	int	amode ;
	int	f_created = FALSE ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	*cp ;


#if	F_DEBUGS
	eprintf("msgid_open: entered\n") ;
#endif

#if	F_SAFE
	if (op == NULL)
	    return SR_FAULT ;
#endif /* F_SAFE */

	if ((fname == NULL) || (fname[0] == '\0'))
	    return SR_FAULT ;

#if	F_DEBUGS
	eprintf("msgid_open: oflags=%4o fname=%s\n",
	    oflags,fname) ;
	if (oflags & O_CREAT)
	    eprintf("msgid_open: creating as needed\n") ;
#endif

	(void) memset(op,0,sizeof(MSGID)) ;

#if	F_CREAT
	oflags |= O_CREAT ;
#endif

	oflags = (oflags &= (~ O_TRUNC)) ;

	op->oflags = oflags ;
	op->operm = operm ;

	mkfnamesuf(tmpfname,fname,MSGID_AFS) ;

	op->afname = mallocstr(tmpfname) ;

	mkfnamesuf(tmpfname,fname,MSGID_BFS) ;

	op->bfname = mallocstr(tmpfname) ;

	if ((op->afname == NULL) || (op->bfname == NULL)) {
		rs = SR_NOMEM ;
		goto bad1 ;
	}

/* try to open the file */

	oflags = (oflags & (~ O_CREAT)) ;
	rs = u_open(op->fname,oflags,operm) ;

#if	F_DEBUGS
	eprintf("msgid_open: u_open() rs=%d\n",rs) ;
#endif

	if ((rs < 0) && (op->oflags & O_CREAT)) {

	    f_created = TRUE ;
	    oflags = op->oflags ;
	    rs = u_open(op->fname,oflags,operm) ;

#if	F_DEBUGS
	    eprintf("msgid_open: u_open() rs=%d\n",rs) ;
#endif

	} /* end if (creating file) */

	if (rs < 0)
	    goto bad1 ;

	op->fd = rs ;
	uc_closeonexec(op->fd,TRUE) ;

	amode = (oflags & O_ACCMODE) ;
	op->f.writable = ((amode == O_WRONLY) || (amode == O_RDWR)) ;

#if	F_DEBUGS
	eprintf("msgid_open: f_writable=%d\n",op->f.writable) ;
#endif

	daytime = time(NULL) ;

	op->opentime = daytime ;
	op->accesstime = daytime ;
	rs = u_fstat(op->fd,&sb) ;

	if (rs < 0)
	    goto bad2 ;

	op->mtime = sb.st_mtime ;
	op->filesize = sb.st_size ;
	op->pagesize = getpagesize() ;

/* local or remote */

	rs = u_fstatvfs(op->fd,&fsb) ;

	if (rs < 0)
	    goto bad2 ;

	cp = fsb.f_basetype ;
	cl = strnlen(cp,FSTYPSZ) ;

	rs = matstr(localfs,cp,cl) ;

	op->f.remote = (rs < 0) ;	/* remote if not local ! */

/* header processing */

	rs = msgid_fileinit(op,daytime) ;

#if	F_DEBUGS
	eprintf("msgid_open: msgid_fileinit() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad2 ;

/* the index */

	rs = mapstrint_init(&op->ni,NENTRIES) ;

	if (rs < 0)
	    goto bad3 ;

/* out of here */

#if	F_DEBUGS
	eprintf("msgid_open: ret rs=%d\n",rs) ;
#endif

	op->magic = MSGID_MAGIC ;
	return (rs >= 0) ? f_created : rs ;

/* bad things */
bad3:

bad2:
	u_close(op->fd) ;

bad1:
	if (op->bfname != NULL)
	    free(op->afname) ;

	if (op->bfname != NULL)
	    free(op->bfname) ;

#if	F_DEBUGS
	eprintf("msgid_open: failed ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msgid_open) */


int msgid_close(op)
MSGID		*op ;
{


#if	F_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != MSGID_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	mapstrint_free(&op->ni) ;

	if (op->buffile != NULL)
	    u_munmap(op->buffile,(size_t) op->bufsize) ;

	if (op->fd >= 0)
	    u_close(op->fd) ;

	free(op->fname) ;

	op->magic = 0 ;
	return SR_OK ;
}
/* end subroutine (msgid_close) */


/* get a count of the number of entries */
int msgid_count(op)
MSGID		*op ;
{
	int	rs = SR_OK ;
	int	c ;


#if	F_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != MSGID_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	c = (op->filesize - MSGID_TABOFF) / MSGID_ENTRYSIZE ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (msgid_count) */


/* initialize a cursor */
int msgid_cursorinit(op,cp)
MSGID		*op ;
MSGID_CURSOR	*cp ;
{


#if	F_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != MSGID_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	if (cp == NULL)
	    return SR_FAULT ;

	op->cursors += 1 ;

	op->f.cursorlockbroken = FALSE ;
	op->f.cursoracc = FALSE ;
	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (msgid_cursorinit) */


/* free up a cursor */
int msgid_cursorfree(op,cp)
MSGID		*op ;
MSGID_CURSOR	*cp ;
{
	time_t	daytime ;


#if	F_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != MSGID_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	if (cp == NULL)
	    return SR_FAULT ;

	if (op->f.cursoracc) {

	    daytime = time(NULL) ;

	    op->accesstime = daytime ;

	}

	if (op->cursors > 0)
	    op->cursors -= 1 ;

	if ((op->cursors == 0) && (op->f.readlocked || op->f.writelocked))
	    msgid_lockrelease(op) ;

	cp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (msgid_cursorfree) */


/* enumerate the entries */
int msgid_enum(op,cup,ep)
MSGID		*op ;
MSGID_CURSOR	*cup ;
MSGID_ENTRY	*ep ;
{
	time_t	daytime = 0 ;

	uint	eoff, ebs = ceiling(MSGID_ENTRYSIZE,4) ;

	int	rs, rs1, size, nl ;
	int	ei, ei2 ;

	char	*bp ;
	char	*np ;


#if	F_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != MSGID_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	if (cup == NULL)
	    return SR_FAULT ;

/* is the file initialized ? */

	if (! op->f.fileinit)
	    return SR_NOTFOUND ;

/* has our lock been broken */

	if (op->f.cursorlockbroken)
	    return SR_LOCKLOST ;

/* is the file open */

	if (op->fd < 0) {

	    daytime = time(NULL) ;

	    rs = msgid_fileopen(op,daytime) ;

	    if (rs < 0)
	        goto bad0 ;

	}

/* capture the lock if we do not already have it */

	if ((! op->f.readlocked) && (! op->f.writelocked)) {

	    if (daytime == 0)
	        daytime = time(NULL) ;

	    rs = msgid_lockget(op,daytime,1) ;

	    if (rs < 0)
	        goto bad0 ;

	}

#ifdef	COMMENT

/* is the file initialized ? */

	if (! op->f.fileinit) {

	    if (daytime == 0)
	        daytime = time(NULL) ;

	    rs = msgid_fileinit(op,daytime) ;

	    if (rs < 0)
	        goto bad0 ;

	}

#endif /* COMMENT */

/* OK, give an entry back to caller */

	ei = (cup->i < 0) ? 0 : cup->i + 1 ;
	eoff = MSGID_TABOFF + (ei * ebs) ;

/* form result to caller */

	rs = ((eoff + ebs) <= op->filesize) ? SR_OK : SR_NOTFOUND ;

	if (rs < 0)
	    goto bad0 ;

/* verify sufficient file buffering */

	rs = msgid_buf(op,0,(eoff + ebs)) ;

	if (rs < 0)
	    goto bad0 ;

	bp = op->buftable + (ei * ebs) ;

/* enter into the index */

#if	F_NIENUM

	np = bp + MSGIDENTRY_ONODENAME ;
	nl = strnlen(np,MSGIDENTRY_LNODENAME) ;

	rs1 = mapstrint_fetch(&op->ni,np,nl,NULL,&ei2) ;

	if ((rs1 >= 0) && (ei != ei2)) {

	    rs1 = SR_NOTFOUND ;
	    mapstrint_delkey(&op->ni,np,nl) ;

	}

	if (rs1 == SR_NOTFOUND) {

	    rs = mapstrint_add(&op->ni,np,nl,ei) ;

	    if (rs < 0)
	        goto bad0 ;

	}

#endif /* F_NIENUM */

/* copy entry to caller buffer */

	if (ep != NULL) {

	    msgidentry_all(bp,MSGIDENTRY_SIZE,1,ep) ;

	} /* end if */

/* commit the cursor movement ? */

	if (rs >= 0)
	    cup->i = ei ;

	op->f.cursoracc = TRUE ;
	return (rs >= 0) ? ei : rs ;

/* bad stuff */
bad1:

bad0:
	return rs ;
}
/* end subroutine (msgid_enum) */


/* match on a nodename */
int msgid_match(op,daytime,nodename,nodenamelen,ep)
MSGID		*op ;
time_t		daytime ;
const char	nodename[] ;
int		nodenamelen ;
MSGID_ENTRY	*ep ;
{
	MSGID_ENTRY	e ;

	uint	eoff, ebs = ceiling(MSGID_ENTRYSIZE,4) ;

	int	rs, rs1, i, ne ;
	int	ei, ei2 ;
	int	nl ;

	char	ebuf[MSGID_ENTRYSIZE + 2] ;
	char	*bp, *np ;


#if	F_DEBUGS
	eprintf("msgid_match: entered nnl=%d nodename=%w\n",
	    nodenamelen,nodename,strnlen(nodename,nodenamelen)) ;
	eprintf("msgid_match: ebs=%u\n",ebs) ;
#endif

#if	F_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != MSGID_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	if (nodename == NULL)
	    return SR_FAULT ;

	i = MSGID_NODENAMELEN ;
	if (nodenamelen >= 0)
	    i = MIN(nodenamelen,MSGID_NODENAMELEN) ;

	nl = strnlen(nodename,i) ;

#if	F_DEBUGS
	eprintf("msgid_match: nodename=%w\n",nodename,nl) ;
#endif

/* is the file even initialized ? */

	if (! op->f.fileinit)
	    return SR_NOTFOUND ;

/* is the file open */

	if (op->fd < 0) {

	    if (daytime == 0)
	        daytime = time(NULL) ;

	    rs = msgid_fileopen(op,daytime) ;

	    if (rs < 0)
	        goto bad0 ;

	}

/* capture the lock if we do not already have it */

	if ((! op->f.readlocked) && (! op->f.writelocked)) {

#if	F_DEBUGS
	    eprintf("msgid_match: need a file lock\n") ;
#endif

	    if (daytime == 0)
	        daytime = time(NULL) ;

	    rs = msgid_lockget(op,daytime,(! op->f.writable)) ;

	    if (rs < 0)
	        goto bad0 ;

	}

/* is it in the index ? */

	rs = mapstrint_fetch(&op->ni,nodename,nl,NULL,&ei) ;

	if (rs >= 0) {

#if	F_DEBUGS
	    eprintf("msgid_match: cache hit ei=%d\n",ei) ;
#endif

	    eoff = MSGID_TABOFF + (ei * ebs) ;
	    rs = msgid_buf(op,0,(eoff + ebs)) ;

	    if (rs >= (eoff + ebs)) {

	        bp = op->buftable + (ei * ebs) ;
	        np = bp + MSGIDENTRY_ONODENAME ;
	        if ((rs >= 0) && (! namematch(np,nodename,nl))) {

#if	F_DEBUGS
	            eprintf("msgid_match: but it was bad !\n") ;
#endif

	            rs = SR_NOTFOUND ;
	            mapstrint_delkey(&op->ni,np,nl) ;

	        }

	    } else
	        rs = SR_NOTFOUND ;

	} /* end if (was in the index) */

	if (rs == SR_NOTFOUND) {

#if	F_DEBUGS
	    eprintf("msgid_match: need full search\n") ;
#endif

/* do the search */

	    rs = msgid_search(op,nodename,nl,&bp) ;

#if	F_DEBUGS
	    eprintf("msgid_match: msgid_search() rs=%d bp=%p\n",rs,bp) ;
	    if (rs >= 0)
	        eprintf("msgid_match: found nodename=%w\n",
	            (bp + MSGIDENTRY_ONODENAME),
	            strnlen((bp + MSGIDENTRY_ONODENAME),
		MSGIDENTRY_LNODENAME)) ;
#endif

	    ei = rs ;
	    if (rs >= 0)
	        mapstrint_add(&op->ni,nodename,nl,ei) ;

	}

	if ((rs >= 0) && (ep != NULL)) {

#if	F_DEBUGS
	    eprintf("msgid_match: found it rs=%d\n",rs) ;
#endif

	    rs1 = msgidentry_all(bp,MSGIDENTRY_SIZE,1,ep) ;

#if	F_DEBUGS
	    eprintf("msgid_match: rs1=%d ep->nodename=%s\n",
	        rs1,ep->nodename) ;
#endif

	}

/* if we are a writer, update the access time also */

	if ((rs >= 0) && op->f.writable) {

	    MSGIDENTRY_ATIME	a ;


	    if (daytime == 0)
	        daytime = time(NULL) ;

	    a.atime = daytime ;
	    msgidentry_atime(bp,MSGIDENTRY_SIZE,0,&a) ;

	    eoff = MSGID_TABOFF + (ei * ebs) ;
	    rs = u_pwrite(op->fd,bp,ebs,(off_t) eoff) ;

	}

/* optionally release our lock if we didn't have a cursor outstanding */

	if (op->cursors == 0)
	    msgid_lockrelease(op) ;

/* update access time as appropriate */

	if (op->cursors == 0) {

	    if (daytime == 0)
	        daytime = time(NULL) ;

	    op->accesstime = daytime ;

	} else
	    op->f.cursoracc = TRUE ;

/* we're out of here */

#if	F_DEBUGS
	eprintf("msgid_match: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad1:

bad0:

#if	F_DEBUGS
	eprintf("msgid_match: bad ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msgid_match) */


/* update an entry */
int msgid_update(op,daytime,ep)
MSGID		*op ;
time_t		daytime ;
MSGID_ENTRY	*ep ;
{
	MSGID_ENTRY	e ;

	int	rs, rs1, i, ne, nl ;
	int	ei, eoff, ebs = ceiling(MSGID_ENTRYSIZE,4) ;
	int	f_changed = FALSE ;
	int	f_newentry = FALSE ;

	char	ebuf[MSGID_ENTRYSIZE + 2] ;
	char	*bp, *np ;


#if	F_DEBUGS
	eprintf("msgid_update: entered\n") ;
#endif

#if	F_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != MSGID_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */

	if (ep == NULL)
	    return SR_FAULT ;

#if	F_DEBUGS
	eprintf("msgid_update: writable ?\n") ;
#endif

	if (! op->f.writable)
	    return SR_RDONLY ;

#if	F_DEBUGS
	eprintf("msgid_update: writable ? => YES\n") ;
#endif

/* is the file open */

	if (op->fd < 0) {

#if	F_DEBUGS
	    eprintf("msgid_update: need open file\n") ;
#endif

	    if (daytime == 0)
	        daytime = time(NULL) ;

	    rs = msgid_fileopen(op,daytime) ;

#if	F_DEBUGS
	    eprintf("msgid_update: msgid_fileopen() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto bad0 ;

	}

/* capture the lock if we do not already have it */

/* lose any READ lock (we need a WRITE lock) ... */

	if (op->f.readlocked) {

	    if (op->cursors > 0)
	        op->f.cursorlockbroken = TRUE ;

#ifdef	OPTIONAL
	    msgid_lockrelease(op) ;
#else
	    op->f.readlocked = FALSE ;
#endif /* OPTIONAL */

	}

/* ... so that we can get a WRITE lock */

	if (! op->f.writelocked) {

	    if (daytime == 0)
	        daytime = time(NULL) ;

	    rs = msgid_lockget(op,daytime,0) ;

#if	F_DEBUGS
	    eprintf("msgid_update: msgid_lockget() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto bad0 ;

	    f_changed = (rs > 1) ;

#if	F_DEBUGS
	    eprintf("msgid_update: file changed !\n") ;
#endif

	} /* end if (acquired lock) */

/* is the file initialized ? */

	if (! op->f.fileinit) {

	    if (daytime == 0)
	        daytime = time(NULL) ;

	    rs = msgid_fileinit(op,daytime) ;

	    if (rs < 0)
	        goto bad0 ;

	}

/* do the search */

	nl = strnlen(ep->nodename,MSGID_NODENAMELEN) ;

/* is it in the index ? */

	rs = mapstrint_fetch(&op->ni,ep->nodename,nl,NULL,&ei) ;

	if (rs >= 0) {

	    eoff = MSGID_TABOFF + (ei * ebs) ;
	    rs = msgid_buf(op,0,(eoff + ebs)) ;

	    if (rs >= (eoff + ebs)) {

	        bp = op->buftable + (ei * ebs) ;
	        np = bp + MSGIDENTRY_ONODENAME ;
	        if ((rs >= 0) && (! namematch(np,ep->nodename,nl))) {

	            rs = SR_NOTFOUND ;
	            mapstrint_delkey(&op->ni,np,nl) ;

	        }

	    } else
	        rs = SR_NOTFOUND ;

	} /* end if (was in the index) */

	if (rs == SR_NOTFOUND) {

	    rs = msgid_search(op,ep->nodename,nl,&bp) ;

	    ei = rs ;
	    if (rs >= 0)
	        mapstrint_add(&op->ni,ep->nodename,nl,ei) ;

	}

/* update the entry that we found and write it back */

	if (rs >= 0) {

/* found existing entry */

#if	F_DEBUGS
	    eprintf("msgid_update: existing entry\n") ;
#endif

	    msgidentry_all(bp,ebs,0,ep) ;

	    eoff = MSGID_TABOFF + (ei * ebs) ;
	    rs = u_pwrite(op->fd,bp,ebs,(off_t) eoff) ;

	} else if (rs == SR_NOTFOUND) {

	    MSGIDENTRY_ATIME	a ;


/* need a new entry (write it to the file) */

	    f_newentry = TRUE ;
	    eoff = op->filesize ;

#if	F_DEBUGS
	    eprintf("msgid_update: need new entry, eoff=%d\n",eoff) ;
	    eprintf("msgid_update: nodename=%s\n",ep->nodename) ;
	    for (i = 0 ; i < 3 ; i += 1)
	        eprintf("msgid_update: ls[%d]=%08x\n",i,ep->la[i]) ;
#endif

	    if (ep->atime == 0) {

	        a.atime = ep->mtime ;
	        msgidentry_atime(ebuf,ebs,0,&a) ;

	    }

	    msgidentry_all(ebuf,ebs,0,ep) ;

#if	F_DEBUGS
	    {
	        char	*tp = (ebuf + MSGIDENTRY_ONODENAME) ;
	        eprintf("msgid_update: buf nodename=%w\n",
	            tp,strnlen(tp,MSGIDENTRY_LNODENAME)) ;
	    }
#endif

	    rs = u_pwrite(op->fd,ebuf,ebs,(off_t) eoff) ;

#if	F_DEBUGS
	    eprintf("msgid_update: u_pwrite() rs=%d\n",rs) ;
#endif

	    if (rs >= 0)
	        op->filesize += ebs ;

	} /* end if (entry update) */

/* update the file header */

	if (rs >= 0) {

#if	COMMENT
	    rs = u_pread(op->fd,op->bufheader,
	        MSGID_HEADLEN,(off_t) MSGID_HEADOFF) ;
#endif

	    rs = msgid_fileheader(op,1) ;

	    if (rs >= 0) {

	        if (f_newentry)
	            op->h.nentries += 1 ;

	        op->h.wcount += 1 ;
	        op->h.wtime = daytime ;
	        msgid_fileheader(op,0) ;

	        rs = u_pwrite(op->fd,op->bufheader,
	            MSGID_HEADLEN,(off_t) MSGID_HEADOFF) ;

	    }
	}

/* optionally release our lock if we didn't have a cursor outstanding */

	if (op->cursors == 0) {

	    rs1 = msgid_lockrelease(op) ;

#if	F_DEBUGS
	    eprintf("msgid_update: msgid_lockrelease() rs=%d\n",rs1) ;
#endif

	}

/* update access time as appropriate */

	if (op->cursors == 0) {

	    if (daytime == 0)
	        daytime = time(NULL) ;

	    op->accesstime = daytime ;

	} else
	    op->f.cursoracc = TRUE ;

/* we're out of here */

	return rs ;

/* bad stuff */
bad1:
	if (op->cursors == 0)
	    msgid_lockrelease(op) ;

bad0:
	return rs ;
}
/* end subroutine (msgid_update) */


/* do some checking */
int msgid_check(op,daytime)
MSGID		*op ;
time_t		daytime ;
{
	int	rs = SR_OK ;

#if	F_DEBUGS
	char	timebuf[TIMEBUFLEN + 1] ;
#endif


#if	F_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != MSGID_MAGIC)
	    return SR_NOTOPEN ;
#endif /* F_SAFE */


	if (op->fd < 0)
	    return SR_OK ;

#if	F_DEBUGS
	eprintf("msgid_check: %s\n",
	    timestr_log(daytime,timebuf)) ;
#endif

	if (op->f.readlocked || op->f.writelocked)
	    return SR_OK ;

	if ((daytime - op->accesstime) > TO_ACCESS)
	    goto closeit ;

	if ((daytime - op->opentime) > TO_OPEN)
	    goto closeit ;

	return rs ;

/* handle a close out */
closeit:
	rs = msgid_fileclose(op) ;

	return rs ;
}
/* end subroutine (msgid_check) */



/* PRIVATE SUBROUTINES */



/* search for an entry */
static int msgid_search(op,nodename,nl,rpp)
MSGID		*op ;
const char	nodename[] ;
int		nl ;
char		**rpp ;
{
	uint	off, eoff, ebs = ceiling(MSGID_ENTRYSIZE,4) ;

	int	rs, rs1, i ;
	int	ne, ei, ei2 ;
	int	f_found ;

	char	*bp, *np ;


#if	F_DEBUGS
	eprintf("msgid_search: entered nodename=%w\n",
	    nodename,strnlen(nodename,nl)) ;
#endif

	if (nl < 0)
	    nl = strlen(nodename) ;

	ne = MAX((op->pagesize / ebs),1) ;

	ei = 0 ;
	off = 0 ;
	eoff = MSGID_TABOFF ;
	f_found = FALSE ;
	while ((! f_found) && (off < op->filesize)) {

	    if (ne > ((op->filesize - eoff) / ebs))
	        ne = (op->filesize - eoff) / ebs ;

	    off = MSGID_TABOFF + ((ei + ne) * ebs) ;
	    rs = msgid_buf(op,0,off) ;

#if	F_DEBUGS
	    eprintf("msgid_search: msgid_buf() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        break ;

	    off = rs ;

	    for (i = 0 ; (i < ne) && (eoff < off) ; i += 1) {

	        bp = op->buftable + (ei * ebs) ;
	        np = bp + MSGIDENTRY_ONODENAME ;

#if	F_DEBUGS
	        hexblock("msgid_search: bp=",&bp,1) ;
	        hexblock("msgid_search: np=",&np,1) ;
	        eprintf("msgid_search: bp=%p np=%p\n",bp,np) ;
	        eprintf("msgid_search: entry ei=%d eoff=%u nodename=%w\n",
	            ei,eoff,np,strnlen(np,MSGIDENTRY_LNODENAME)) ;
#endif

#if	F_NISEARCH

/* add to the name-index if necessary */

	        rs1 = mapstrint_fetch(&op->ni,np,nl,NULL,&ei2) ;

	        if ((rs1 >= 0) && (ei != ei2)) {

	            rs1 = SR_NOTFOUND ;
	            mapstrint_delkey(&op->ni,np,nl) ;

	        }

	        if (rs1 == SR_NOTFOUND) {

	            int	nl2 ;


	            nl2 = strnlen(np,MSGIDENTRY_LNODENAME) ;

	            mapstrint_add(&op->ni,np,nl2,ei) ;

	        }

#endif /* F_NISEARCH */

/* is this a match for what we want ? */

	        if (namematch(np,nodename,nl))
	            break ;

	        ei += 1 ;
	        eoff += ebs ;

	    } /* end for (looping through entries) */

	    f_found = (i < ne) ;
	    if (f_found)
	        break ;

	} /* end while */

	if (rs >= 0) {

	    if (f_found) {

	        if (rpp != NULL) {

	            bp = op->buftable + (ei * ebs) ;
	            *rpp = bp ;

	        }

	    } else
	        rs = SR_NOTFOUND ;

	}

	return (rs >= 0) ? ei : rs ;
}
/* end subroutine (msgid_search) */


/* initialize the file header (either read it only or write it) */
static int msgid_fileinit(op,daytime)
MSGID		*op ;
time_t		daytime ;
{
	int	rs = SR_OK, len, bl ;
	int	f_locked = FALSE ;

	char	buf[16 + 4 + sizeof(struct msgid_h) + 9], *bp ;
	char	*cp ;


#if	F_DEBUGS
	eprintf("msgid_fileinit: entered filesize=%u\n",op->filesize) ;
#endif

	if (op->filesize == 0) {

	    u_seek(op->fd,0L,SEEK_SET) ;

	    op->f.fileinit = FALSE ;
	    if (op->f.writable) {

#if	F_DEBUGS
	        eprintf("msgid_fileinit: writing header\n") ;
#endif

	        if (! op->f.writelocked) {

	            rs = msgid_lockget(op,daytime,0) ;

	            if (rs < 0)
	                goto ret0 ;

	            f_locked = TRUE ;
	        }

/* write the file header stuff */

	        bp = buf ;
	        bp = strwcpy(bp,MSGID_FILEMAGIC,15) ;

	        *bp++ = '\n' ;
	        bl = (buf + 16) - bp ;
	        (void) memset(bp,0,bl) ;

	        bp += bl ;
	        *bp++ = MSGID_FILEVERSION ;
	        *bp++ = MSGID_ENDIAN ;
	        *bp++ = 0 ;		/* file type */
	        *bp++ = 0 ;		/* unused */

/* next is the header (we just write zeros here) */

	        memset(bp,0,MSGID_HEADLEN) ;

	        bp += MSGID_HEADLEN ;
	        bl = bp - buf ;

#if	F_DEBUGS
	        eprintf("msgid_fileinit: u_write() wlen=%d\n",bl) ;
#endif

	        rs = u_write(op->fd,buf,bl) ;

#if	F_DEBUGS
	        eprintf("msgid_fileinit: u_write() rs=%d\n",rs) ;
#endif

	        if (rs > 0)
	            op->filesize = rs ;

	        op->fileversion = MSGID_FILEVERSION ;
	        op->filetype = 0 ;

/* file header */

	        memset(&op->h,0,MSGID_HEADLEN) ;

	        op->f.fileinit = (rs >= 0) ;

	    } /* end if (writing) */

	} else if (op->filesize >= MSGID_TABOFF) {

/* read the file header */

	    if (! op->f.readlocked) {

	        rs = msgid_lockget(op,daytime,1) ;

	        if (rs < 0)
	            goto ret0 ;

	        f_locked = TRUE ;
	    }

#if	F_DEBUGS
	    eprintf("msgid_fileinit: msgid_fileverify() \n") ;
#endif

	    rs = msgid_fileverify(op) ;

#if	F_DEBUGS
	    eprintf("msgid_fileinit: msgid_fileverify() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

	        rs = msgid_fileheader(op,1) ;

	        op->f.fileinit = (rs >= 0) ;

	    }

	} /* end if */

/* if we locked, we unlock it, otherwise leave it ! */

	if (f_locked)
	    msgid_lockrelease(op) ;

/* we're out of here */
ret0:

#if	F_DEBUGS
	eprintf("msgid_fileinit: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msgid_fileinit) */


/* verify the file */
static int msgid_fileverify(op)
MSGID		*op ;
{
	int	rs = SR_OK ;
	int	size ;
	int	f ;

	char	*cp ;


#if	F_DEBUGS
	eprintf("msgid_fileverify: entered\n") ;
#endif

	size = MSGID_TOPLEN ;
	rs = msgid_buf(op,0,size) ;

	if (rs < 0)
	    goto bad0 ;

	if (rs < MSGID_TOPLEN) {

	    rs = SR_INVALID ;
	    goto bad0 ;
	}

	cp = (char *) op->buffile ;
	f = (strncmp(cp,MSGID_FILEMAGIC,MSGID_FILEMAGICLEN) == 0) ;
	f = f && (*(cp + MSGID_FILEMAGICLEN) == '\n') ;

	if (! f) {

#if	F_DEBUGS
	    eprintf("msgid_fileverify: bad magic=>%w<\n",
	        cp,strnlen(cp,14)) ;
#endif

	    rs = SR_BADFMT ;
	    goto bad3 ;
	}

	cp += 16 ;
	if (cp[0] > MSGID_FILEVERSION) {

	    rs = SR_NOTSUP ;
	    goto bad3 ;
	}

	op->fileversion = cp[0] ;

	if (cp[1] != MSGID_ENDIAN) {

	    rs = SR_NOTSUP ;
	    goto bad3 ;
	}

	op->filetype = cp[2] ;

ret0:

#if	F_DEBUGS
	eprintf("msgid_fileverify: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff comes here */
bad3:

#if	F_DEBUGS
	eprintf("msgid_fileverify: bad ret rs=%d\n",rs) ;
#endif

bad0:
	return rs ;
}
/* end subroutine (msgid_fileverify) */


/* read the file header and check it out */
static int msgid_fileheader(op,f_read)
MSGID		*op ;
int		f_read ;
{
	uint	*table ;


	if (op->buffile == NULL)
	    return SR_BADFMT ;

	table = (uint *) op->bufheader ;

	if (f_read) {

	    op->h.nentries = ntohl(table[0]) ;

	    op->h.wtime = ntohl(table[1]) ;

	    op->h.wcount = ntohl(table[2]) ;

	} else {

	    table[0] = htonl(op->h.nentries) ;

	    table[1] = htonl(op->h.wtime) ;

	    table[2] = htonl(op->h.wcount) ;

	} /* end if */

	return MSGID_HEADLEN ;
}
/* end subroutine (msgid_fileheader) */


/* acquire access to the file */
static int msgid_lockget(op,daytime,f_read)
MSGID		*op ;
int		f_read ;
time_t		daytime ;
{
	struct stat	sb ;

	int	rs = SR_OK ;
	int	lockcmd ;
	int	f_already = FALSE ;
	int	f_changed = FALSE ;


#if	F_DEBUGS
	eprintf("msgid_lockget: entered f_read=%d\n",f_read) ;
#endif

	if (op->fda < 0) {

	    rs = msgid_fileopen(op,daytime) ;

#if	F_DEBUGS
	    eprintf("msgid_lockget: msgid_fileopen() rs=%d fd=%d\n",
	        rs,op->fd) ;
#endif

	    if (rs < 0)
	        goto bad0 ;

	} /* end if (needed to open the file) */

/* acquire a file record lock */

	if (f_read || (! op->f.writable)) {

#if	F_DEBUGS
	    eprintf("msgid_lockget: need READ lock\n") ;
#endif
	    f_already = op->f.readlocked ;
	    op->f.readlocked = TRUE ;
	    op->f.writelocked = FALSE ;
	    lockcmd = F_RLOCK ;

	} else {

#if	F_DEBUGS
	    eprintf("msgid_lockget: need WRITE lock\n") ;
#endif

	    f_already = op->f.writelocked ;
	    op->f.readlocked = FALSE ;
	    op->f.writelocked = TRUE ;
	    lockcmd = F_WLOCK ;

	}

/* get out if we have the lock that we want already */

	if (f_already)
	    return SR_OK ;

/* we need to actually do the lock */

	rs = lockfile(op->fd,lockcmd,0L,0,TO_LOCK) ;

	if (rs < 0)
	    goto bad2 ;

/* has the file changed at all ? */

	rs = u_fstat(op->fdb,&sb) ;

#ifdef	COMMENT
	if (rs == SR_NOENT)
	    rs = SR_OK ;
#endif /* COMMENT */

	if (rs < 0)
	    goto bad2 ;

	f_changed = 
	    (sb.st_size != op->filesize) ||
	    (sb.st_mtime != op->mtime) ;

	if (f_changed) {

	    if (op->f.bufvalid)
	        op->f.bufvalid = FALSE ;

	    op->filesize = sb.st_size ;
	    op->mtime = sb.st_mtime ;

	}

	if (op->filesize < MSGID_TABOFF)
	    op->f.fileinit = FALSE ;

#if	F_DEBUGS
	eprintf("msgid_lockget: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_changed : rs ;

/* bad stuff */
bad2:
	op->f.fileinit = FALSE ;

	lockfile(op->fda,F_ULOCK,0L,0,TO_LOCK) ;

bad1:
	op->f.readlocked = FALSE ;
	op->f.writelocked = FALSE ;

bad0:
	return rs ;
}
/* end subroutine (msgid_lockget) */


static int msgid_lockrelease(op)
MSGID		*op ;
{
	int	rs = SR_OK ;


#if	F_DEBUGS
	eprintf("msgid_lockrelease: entered\n") ;
#endif

	if ((op->f.readlocked || op->f.writelocked)) {

	    if (op->fd >= 0) {

	        rs = lockfile(op->fda,F_ULOCK,0L,0,TO_LOCK) ;

	    } /* end if (file was open) */

	    op->f.readlocked = FALSE ;
	    op->f.writelocked = FALSE ;

	} /* end if (there was a possible lock set) */

#if	F_DEBUGS
	eprintf("msgid_lockrelease: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (msgid_lockrelease) */


static int msgid_fileopen(op,daytime)
MSGID		*op ;
time_t		daytime ;
{
	struct stat	sb ;

	int	rs ;


#if	F_DEBUGS
	eprintf("msgid_fileopen: fname=%s\n",op->afname) ;
#endif

	if (op->fda >= 0)
	    return op->fda ;

#if	F_DEBUGS
	eprintf("msgid_fileopen: need open\n") ;
#endif

	op->f.fileinit = FALSE ;
	rs = u_open(op->afname,op->oflags,op->operm) ;

#if	F_DEBUGS
	eprintf("msgid_fileopen: u_open() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	op->fda = rs ;
	uc_closeonexec(op->fda,TRUE) ;

	op->opentime = daytime ;
	return (rs >= 0) ? op->fda : rs ;

/* bad things */
bad0:
	return rs ;
}
/* end subroutine (msgid_fileopen) */


int msgid_fileclose(op)
MSGID		*op ;
{
	int	rs = SR_OK ;


	if (op->fda >= 0) {

	    u_close(op->fda) ;

	    op->fda = -1 ;

	}

	return rs ;
}
/* end subroutine (msgid_fileclose) */


/* map a region (specified) of the file into memory */
static int msgid_mapbuf(op,roff,rlen,rpp)
MSGID		*op ;
off_t		roff ;
int		rlen ;
MSGID_ENTRY	**rpp ;
{
	    uint	ext ;

	int	rs = SR_OK, len = 0 ;
	int	prot, flags ;
	int	f ;


#if	F_DEBUGS
	eprintf("msgid_mapbuf: entered roff=%u rlen=%u fsize=%u\n",
	    roff,rlen,op->fsize) ;
#endif

	if (roff >= op->fsize)
	    return 0 ;

	if ((rlen == UINT_MAX) || ((roff + rlen) > op->fsize))
	    rlen = op->fsize - roff ;

	if (op->mapbuf != NULL) {

	    f = (roff < op->mapoff) || 
	        ((roff + rlen) > (op->mapoff + op->maplen)) ;

	    if (f) {

	        u_munmap(op->mapbuf,(size_t) op->maplen) ;

	        op->mapbuf = NULL ;
	        op->maplen = 0 ;
	    }
	}

	if (op->mapbuf == NULL) {

	    uint	desired, round ;


#if	F_DEBUGS
	    eprintf("msgid_mapbuf: pagesize=%u roff=%u \n",
	        op->pagesize,roff) ;
#endif

	    op->mapoff = floor(roff,op->pagesize) ;

	    desired = op->fsize + op->pagesize - op->mapoff ;
	    round = MIN(desired, MSGID_MAPSIZE) ;

		if (round < rlen)
			round = rlen ;

	    ext = ceiling((roff + round),op->pagesize) ;

	    op->maplen = ext - op->mapoff ;

#if	F_DEBUGS
	    eprintf("msgid_mapbuf: u_mapfile() mapoff=%u maplen=%u\n",
	        op->mapoff,op->maplen) ;
#endif

	    prot = PROT_READ ;
	    flags = MAP_SHARED ;
	    rs = u_mapfile(NULL,(size_t) op->maplen,prot,flags,
	        op->fd,(off_t) op->mapoff,&op->mapbuf) ;

#if	F_DEBUGS
	    eprintf("msgid_mapbuf: u_mapfile() rs=%d fsize=%u maplen=%u\n",
	        rs,op->fsize,op->maplen) ;
#endif

	} /* end if (mapping file) */

	if ((op->mapbuf != NULL) && (op->maplen > 0)) {

#if	F_DEBUGS
	    eprintf("msgid_mapbuf: non-zero file map\n") ;
#endif

	    if (rpp != NULL) {

	        len = roff - op->mapoff ;
	        *rpp = (MSGID_ENTRY *) (op->mapbuf + len) ;

	    }

	    ext = MIN((op->mapoff + op->maplen),op->fsize) ;
	    len = MIN(rlen,(ext - roff)) ;

#if	F_DEBUGS
	    eprintf("msgid_mapbuf: ext=%u len=%u\n",ext,len) ;
#endif

	}

#if	F_DEBUGS
	eprintf("msgid_mapbuf: ret rs=%d len=%u\n",
	    rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (msgid_mapbuf) */


static int namematch(np,nodename,nl)
const char	np[] ;
const char	nodename[] ;
int		nl ;
{


	if (nl > MSGIDENTRY_LNODENAME)
	    return FALSE ;

	if (strncmp(np,nodename,nl) != 0)
	    return FALSE ;

	if (nl == MSGIDENTRY_LNODENAME)
	    return TRUE ;

	return (np[nl] == '\0') ;
}
/* end subroutine (namematch) */


static uint ceiling(v,a)
uint	v, a ;
{


	return (v + (a - 1)) & (~ (a - 1)) ;
}
/* end subroutine (ceiling) */



