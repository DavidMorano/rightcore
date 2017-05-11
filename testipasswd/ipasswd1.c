/* ipasswd */

/* indexed PASSWD GECOS DB */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		1		/* safe mode */
#define	CF_LOCKING	0		/* use file locking */
#define	CF_HOLDING	1		/* use file-map holding */
#define	CF_MEMSYNC	1		/* use memory synchronization? */
#define	CF_USEFL3	1		/* use first-last3 index (fast) */
#define	CF_FILEWAIT	0		/* wait for file to "come in" */
#define	CF_ENUM		1		/* compile in '_enum()' */


/* revision history:

	= 2003-06-11, David A�D� Morano

	This was rewritten (as messy as it currently is) from a
	previous pierce of code that was even worse. 


*/

/* Copyright � 2003 David A�D� Morano.  All rights reserved. */

/******************************************************************************

	This object module provides an interface to a data base of
	information about the GECOS name in the system PASSWD database.

	The system PASSWD database (whether a file or whatever) was
	enumerated (separately) and an index file was made with several
	indices to lookup usernames based on a real name.  For starters
	only indices consisting of using the first character of the last
	name, the first 3 characters of the last name, and the first
	character of the first name have been used.  But future index
	files might provide more combinations, like using the first 3
	characters of the last name combined with the first character
	of the first name!

	The various data and indices were written into a file.	We are
	accessing that file within this object.

	Extra note: this code needs another rewrite!


*****************************************************************************/


#define	IPASSWD_MASTER	0


#undef	LOCAL_SUNOS
#define	LOCAL_SUNOS	(defined(SUNOS) || \
	(defined(OSTYPE_SUNOS) && (OSTYPE_SUNOS > 0)))

#include	<envstandards.h>	/* MUST be first to configure */

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
#include	<ctype.h>

#include	<vsystem.h>
#include	<mkfnamesuf.h>
#include	<dstr.h>
#include	<storeitem.h>
#include	<realname.h>
#include	<localmisc.h>

#include	"ipasswd.h"


/* local defines */

#define	IPASSWD_MAGIC	0x23456787

#define	IPASSWD_IDLEN	(16 + 4)
#define	IPASSWD_HEADLEN	(12 * 4)
#define	IPASSWD_TOPLEN	(IPASSWD_IDLEN + IPASSWD_HEADLEN)

#define	IPASSWD_IDOFF	0
#define	IPASSWD_HEADOFF	IPASSWD_IDLEN
#define	IPASSWD_BUFOFF	(IPASSWD_HEADOFF + IPASSWD_HEADLEN)

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	TO_FILECOME	5
#define	TO_OPEN		(60 * 60)
#define	TO_ACCESS	(1 * 60)
#define	TO_LOCK		10		/* seconds */
#define	TO_CHECK	5

#define	TI_MINUPDATE	4		/* minimum time between updates */

#ifndef	ENDIAN
#if	LOCAL_SUNOS && defined(__sparc)
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
#endif /* ENDIAN */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	NSHIFT		6		/* shift bits for secondary hash */

#if	CF_DEBUGS
#define	PRSTRLEN	80
#endif


/* external subroutines */

extern uint	uceil(uint,int) ;
extern uint	hashelf(const void *,int) ;

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	randlc(int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	isprintlatin(int) ;
extern int	getfstype(char *,int,int) ;
extern int	islocalfs(const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_log(time_t,char *) ;


/* forward references */

static int	ipasswd_fileheader(IPASSWD *) ;
static int	ipasswd_holdget(IPASSWD *,time_t) ;
static int	ipasswd_holdrelease(IPASSWD *,time_t) ;

#if	CF_LOCKING
static int	ipasswd_lockget(IPASSWD *,int) ;
static int	ipasswd_lockrelease(IPASSWD *) ;
#endif

static int	ipasswd_fileopen(IPASSWD *,time_t) ;
static int	ipasswd_fileclose(IPASSWD *) ;
static int	ipasswd_mapbegin(IPASSWD *,time_t) ;
static int	ipasswd_mapend(IPASSWD *) ;
static int	ipasswd_keymatchfl3(IPASSWD *,int,int,REALNAME *) ;
static int	ipasswd_keymatchl3(IPASSWD *,int,int,REALNAME *) ;
static int	ipasswd_keymatchl1(IPASSWD *,int,int,REALNAME *) ;
static int	ipasswd_keymatchf(IPASSWD *,int,int,REALNAME *) ;
static int	ipasswd_keymatchall(IPASSWD *,int,int,REALNAME *) ;

#ifdef	COMMENT
static int	ipasswd_keymatchlast(IPASSWD *,int,int,char *,int) ;
#endif

static int	hashindex(uint,uint) ;

#if	CF_DEBUGS
static int	mkprstr(char *,int,const char *,int) ;
#endif


/* local variables */

enum headers {
	header_writetime,
	header_writecount,
	header_rectab,
	header_rectablen,
	header_strtab,
	header_strtabsize,
	header_indexlen,
	header_indexl1,
	header_indexl3,
	header_indexf,
	header_indexfl3,
	header_indexun,
	header_overlast
} ;

enum indices {
	index_l1,
	index_l3,
	index_f,
	index_fl3,
	index_un,
	index_overlast
} ;


/* exported subroutines */


int ipasswd_open(op,dbname)
IPASSWD		*op ;
const char	dbname[] ;
{
	time_t	daytime = time(NULL) ;

	size_t	msize ;

	int	rs ;
	int	mprot, mflags ;

	const char	*endstr = ((ENDIAN != 0) ? "1" : "0") ;

	char	*p ;


	if (op == NULL)
	    return SR_FAULT ;

	if (dbname == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("ipasswd_open: dbname=%s\n",dbname) ;
#endif

	memset(op,0,sizeof(IPASSWD)) ;
	op->fd = -1 ;
	op->oflags = O_RDONLY ;
	op->operm = 0666 ;
	op->pagesize = getpagesize() ;

/* store filename away */

	{
	    const char	*suf = IPASSWD_SUF ;
	    char	dbfname[MAXPATHLEN + 1] ;
	    if ((rs = mkfnamesuf2(dbfname,dbname,suf,endstr)) >= 0) {
	            const char	*cp ;
	            if ((rs = uc_mallocstrw(dbfname,-1,&cp)) >= 0) {
		        op->fname = cp ;
	            }
	    }
	}
	if (rs < 0) goto bad0 ;

/* open the file */

	rs = ipasswd_fileopen(op,daytime) ;

#if	CF_DEBUGS
	debugprintf("ipasswd_open: ipasswd_fileopen() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

/* map it */

#if	CF_DEBUGS
	debugprintf("ipasswd_open: mapping file\n") ;
#endif

	msize = (size_t) op->filesize ;
	mprot = PROT_READ ;
	mflags = MAP_SHARED ;
	if ((rs = u_mmap(NULL,msize,mprot,mflags,op->fd,0L,&p)) >= 0) {
		op->mapdata = p ;
		op->mapsize = msize ;
	}

#if	CF_DEBUGS
	debugprintf("ipasswd_open: u_mmap() rs=%d\n",rs) ;
#endif

	if (rs < 0) goto bad2 ;

/* OK, check it out */

	rs = ipasswd_fileheader(op) ;

#if	CF_DEBUGS
	debugprintf("ipasswd_open: ipasswd_fileheader() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad3 ;

/* ok, we're good (?) */

	daytime = time(NULL) ;

	op->ti_open = daytime ;
	op->ti_access = daytime ;
	op->ti_map = daytime ;

	op->f.fileinit = TRUE ;

#if	CF_DEBUGS
	debugprintf("ipasswd_open: header processing completed \n") ;
#endif

	op->magic = IPASSWD_MAGIC ;

ret1:
	ipasswd_fileclose(op) ;

/* done (returning) */
ret0:

#if	CF_DEBUGS
	debugprintf("ipasswd_open: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad3:
	if (op->mapdata != NULL) {
	    u_munmap(op->mapdata,op->mapsize) ;
	    op->mapdata = NULL ;
	}

bad2:
	ipasswd_fileclose(op) ;

bad1:
	if (op->fname != NULL)
	    uc_free(op->fname) ;

bad0:
	goto ret0 ;
}
/* end subroutine (ipasswd_open) */


/* get the string count in the table */
int ipasswd_count(op)
IPASSWD		*op ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != IPASSWD_MAGIC)
	    return SR_NOTOPEN ;

	return (op->rtlen - 1) ;
}
/* end subroutine (ipasswd_count) */


/* calculate the index table length (number of entries) at this point */
int ipasswd_countindex(op)
IPASSWD		*op ;
{


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != IPASSWD_MAGIC)
	    return SR_NOTOPEN ;

	return op->rilen ;
}
/* end subroutine (ipasswd_countindex) */


/* initialize a cursor */
int ipasswd_curbegin(op,cp)
IPASSWD		*op ;
IPASSWD_CUR	*cp ;
{
	int	i ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != IPASSWD_MAGIC)
	    return SR_NOTOPEN ;

	if (cp == NULL)
	    return SR_FAULT ;

	op->f.cursor = TRUE ;
	op->f.cursorlockbroken = FALSE ;
	op->f.cursoracc = FALSE ;

	for (i = 0 ; i < IPASSWD_NINDICES ; i += 1)
	    cp->i[i] = -1 ;

	cp->magic = IPASSWD_MAGIC ;
	return SR_OK ;
}
/* end subroutine (ipasswd_curbegin) */


/* free up a cursor */
int ipasswd_curend(op,cp)
IPASSWD		*op ;
IPASSWD_CUR	*cp ;
{
	time_t	daytime ;

	int	rs = SR_OK ;
	int	i ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != IPASSWD_MAGIC)
	    return SR_NOTOPEN ;

	if (cp == NULL)
	    return SR_FAULT ;

	daytime = time(NULL) ;

	if (op->f.cursoracc)
	    op->ti_access = daytime ;

	op->f.cursor = FALSE ;

#if	CF_LOCKING
	if (op->f.lockedread || op->f.lockedwrite)
	    ipasswd_lockrelease(op) ;
#endif

#if	CF_HOLDING
	if (op->f.held)
	    rs = ipasswd_holdrelease(op,daytime) ;
#endif

	for (i = 0 ; i < IPASSWD_NINDICES ; i += 1)
	    cp->i[i] = -1 ;

	cp->magic = 0 ;
	return rs ;
}
/* end subroutine (ipasswd_curend) */


/* enumerate */
#if	CF_ENUM
int ipasswd_enum(op,curp,ubuf,sa,rbuf,rlen)
IPASSWD		*op ;
IPASSWD_CUR	*curp ;
char		ubuf[] ;
const char	*sa[] ;
char		rbuf[] ;
int		rlen ;
{
	time_t	daytime ;

	int	rs = SR_OK ;
	int	ri, si ;
	int	ui ;

	const char	*cp ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != IPASSWD_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;
	if (sa == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (! op->f.cursor)
	    return SR_INVALID ;

	ri = (curp->i[0] < 1) ? 1 : (curp->i[0] + 1) ;

#if	CF_DEBUGS
	debugprintf("ipasswd_enum: ri=%u\n",ri) ;
#endif

/* capture a hold on the file */

#if	CF_HOLDING
	if (! op->f.held) {
	    daytime = time(NULL) ;
	    rs = ipasswd_holdget(op,daytime) ;
	    if (rs > 0)
	        rs = ipasswd_fileheader(op) ;
	    if (rs < 0) return rs ;
	}
#endif /* CF_HOLDING */

/* ok, we're good to go */

	if (ri >= op->rtlen)
	    return SR_NOTFOUND ;

	if (sa != NULL) {
	    STOREITEM	sio ;
	    DSTR	ss[5] ;

#if	CF_DEBUGS
	    debugprintf("ipasswd_enum: sa-ing\n") ;
#endif

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

	    if ((rs = storeitem_start(&sio,rbuf,rlen)) >= 0) {
		int		j ;
		int		sl ;
		int		c = 0 ;
		const char	*sp ;
		const char	**spp ;
		for (j = 0 ; (rs >= 0) && (j < 5) ; j += 1) {
		    sp = ss[j].s ;
		    sl = ss[j].slen ;
#if	CF_DEBUGS
	    debugprintf("ipasswd_enum: j=%u sl=%d sp=%s\n",j,sl,sp) ;
#endif
		    if ((sp != NULL) && (sl != 0) && (*sp != '\0')) {
			spp = (sa+c++) ;
			rs = storeitem_strw(&sio,ss[j].s,ss[j].slen,spp) ;
		    }
		} /* end for */
		sa[c] = NULL ;
		storeitem_finish(&sio) ;
	    } /* end if (storeitem) */
	} /* end if */

	if (rs >= 0) {

	    ui = op->rectab[ri].username ;

#if	CF_DEBUGS
	    debugprintf("ipasswd_enum: ui=%d\n",ui) ;
#endif

	    if (ubuf != NULL) {
	        cp = strwcpy(ubuf,(op->stab + ui),IPASSWD_USERNAMELEN) ;
	        rs = (cp - ubuf) ;
	    } else
	        rs = strlen(op->stab + ui) ;

/* update the cursor */

	    curp->i[0] = ri ;

	} /* end if */

	return rs ;
}
/* end subroutine (ipasswd_enum) */
#endif /* CF_ENUM */


/* fetch an entry by key lookup */
int ipasswd_fetch(op,np,curp,opts,up)
IPASSWD		*op ;
REALNAME	*np ;
IPASSWD_CUR	*curp ;
int		opts ;
char		*up ;
{
	IPASSWD_CUR	cur ;

	time_t		daytime = 0 ;

	uint		hv, hi, ri, ui ;

	const int	ns = NSHIFT ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i, wi ;
	int	hl, c ;
	int	ul = 0 ;
	int	f_cur = FALSE ;

	const char	*hp ;
	const char	*cp ;

	char	hbuf[4 + 1] ;


#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != IPASSWD_MAGIC)
	    return SR_NOTOPEN ;
#endif

	if (curp == NULL) {

	    curp = &cur ;
	    for (i = 0 ; i < IPASSWD_NINDICES ; i += 1)
	        curp->i[i] = -1 ;

	} else {

	    f_cur = TRUE ;
	    if (! op->f.cursor)
	        return SR_INVALID ;

	}

	if (up != NULL)
	    *up = '\0' ;

	if (np == NULL)
	    return SR_FAULT ;

#ifdef	COMMENT
	if (up != NULL)
	    *up = '\0' ;
#endif

/* do we have a hold on the file? */

#if	CF_HOLDING
	if (! op->f.held) {

	    daytime = time(NULL) ;

	    rs = ipasswd_holdget(op,daytime) ;

	    if (rs > 0) {

#if	CF_DEBUGS
	        debugprintf("ipasswd_fetch: file changed\n") ;
#endif

	        rs = ipasswd_fileheader(op) ;

	    }

	    if (rs < 0)
	        goto ret0 ;

	}
#endif /* CF_HOLDING */

/* continue with regular fetch activities */

	op->f.cursoracc = TRUE ;	/* doesn't hurt if no cursor! */

/* which index do we want to use? */

	wi = -1 ;
	if ((np->len.first >= 1) && (np->len.last >= 3)) {

#if	CF_USEFL3
	    wi = index_fl3 ;
	    hp = hbuf ;
	    hl = 4 ;
	    hbuf[0] = np->first[0] ;
	    strncpy((hbuf + 1),np->last,3) ;
#else
	    wi = index_l3 ;
	    hp = np->last ;
	    hl = 3 ;
#endif /* CF_USEFL3 */

	} else if (np->len.last >= 3) {

	    wi = index_l3 ;
	    hp = np->last ;
	    hl = 3 ;

	} else if (np->len.last >= 1) {

	    wi = index_l1 ;
	    hp = np->last ;
	    hl = 1 ;

	} else if (np->len.first >= 1) {

	    wi = index_f ;
	    hp = np->first ;
	    hl = 1 ;

	} else
	    wi = -1 ;

	if (wi < 0) {
	    rs = SR_INVALID ;
	    goto ret0 ;
	}

#if	CF_DEBUGS
	debugprintf("ipasswd_fetch: wi=%d key=%t\n",wi,hp,hl) ;
	debugprintf("ipasswd_fetch: rilen=%u (\\x%08x)\n",
	    op->rilen, op->rilen) ;
#endif

/* OK, we go from here */

	if (curp->i[wi] < 0) {

#if	CF_DEBUGS
	    debugprintf("ipasswd_fetch: new cursor\n") ;
#endif

	    hv = hashelf(hp,hl) ;

	    hi = hashindex(hv,op->rilen) ;

#if	CF_DEBUGS
	    debugprintf("ipasswd_fetch: hv=%08x hi=%u\n",hv,hi) ;
#endif

/* start searching! */

	    if (op->ropts & IPASSWD_OSEC) {
	        int	f ;

#if	CF_DEBUGS
	        if (hi < op->rilen)
	            debugprintf("ipasswd_fetch: secondary initial ri=%u\n",
	                (op->recind[wi])[hi][0]) ;
#endif

	        c = 0 ;
	        while ((ri = (op->recind[wi])[hi][0]) != 0) {

#if	CF_DEBUGS
	            debugprintf("ipasswd_fetch: trymatch ri=%u\n",ri) ;
#endif

	            if (ri >= op->rtlen) {
	                rs = 0 ;
	                break ;
	            }

	            switch (wi) {

	            case index_fl3:
	                f = ipasswd_keymatchfl3(op,opts,ri,np) ;
	                break ;

	            case index_l3:
	                f = ipasswd_keymatchl3(op,opts,ri,np) ;
	                break ;

	            case index_l1:
	                f = ipasswd_keymatchl1(op,opts,ri,np) ;
	                break ;

	            case index_f:
	                f = ipasswd_keymatchf(op,opts,ri,np) ;
	                break ;

	            } /* end switch */

	            if (f)
	                break ;

	            op->collisions += 1 ;
	            if (op->ropts & IPASSWD_ORANDLC) {
	                hv = randlc(hv + c) ;
	            } else
	                hv = ((hv << (32 - ns)) | (hv >> ns)) + c ;

	            hi = hashindex(hv,op->rilen) ;

#if	CF_DEBUGS
	            debugprintf("ipasswd_fetch: new hv=%08x hi=%u\n",hv,hi) ;
#endif

	            c += 1 ;

	        } /* end while */

#if	CF_DEBUGS
	        debugprintf("ipasswd_fetch: index-key-match ri=%u\n",ri) ;
#endif

	        if (ri == 0)
	            rs = SR_NOTFOUND ;

	    } /* end if (secondary hashing) */

	} else {

/* get the next record index (if there is one) */

#if	CF_DEBUGS
	    debugprintf("ipasswd_fetch: old cursor\n") ;
#endif

	    hi = curp->i[wi] ;
	    if ((hi != 0) && (hi < op->rilen)) {

	        ri = (op->recind[wi])[hi][0] ;
	        if ((ri != 0) && (ri < op->rtlen)) {

	            hi = (op->recind[wi])[hi][1] ;
	            if ((hi != 0) && (hi < op->rilen)) {
	                ri = (op->recind[wi])[hi][0] ;
	            } else
	                rs = SR_NOTFOUND ;

	        } else
	            rs = SR_NOTFOUND ;

	    } else
	        rs = SR_NOTFOUND ;

	} /* end if (preparation) */

#if	CF_DEBUGS
	debugprintf("ipasswd_fetch: match search rs=%d hi=%u\n",rs,hi) ;
#endif

	if (rs >= 0) {

	    while ((ri = (op->recind[wi])[hi][0]) != 0) {

	        if (ri >= op->rtlen) {
	            ri = 0 ;
	            break ;
	        }

	        if (ipasswd_keymatchall(op,opts,ri,np))
	            break ;

	        hi = (op->recind[wi])[hi][1] ;
	        if (hi == 0)
	            break ;

	        op->collisions += 1 ;

	    } /* end while */

	    if ((ri == 0) || (hi == 0))
	        rs = SR_NOTFOUND ;

	} /* end if (following the existing chain) */

#if	CF_DEBUGS
	debugprintf("ipasswd_fetch: done w/ search rs=%d\n",rs) ;
#endif

/* if successful, retrieve username */

	if (rs >= 0) {

	    ui = op->rectab[ri].username ;
	    if (up != NULL) {

	        cp = strwcpy(up,(op->stab + ui),IPASSWD_USERNAMELEN) ;

#if	CF_DEBUGS
	        debugprintf("ipasswd_fetch: username=%s\n",up) ;
#endif

	        ul = (cp - up) ;

	    } else
	        ul = strlen(op->stab + ui) ;

/* update cursor */

	    if (f_cur)
	        curp->i[wi] = hi ;

	} /* end if (got one) */

#if	CF_HOLDING
	if (! op->f.cursor) {
	    if (daytime == 0) daytime = time(NULL) ;
	    op->ti_access = daytime ;
	    rs1 = ipasswd_holdrelease(op,daytime) ;
#if	CF_DEBUGS
	    debugprintf("ipasswd_fetch: ipasswd_holdrelease() rs=%d\n",rs) ;
#endif
	    if (rs >= 0) rs = rs1 ;
	}
#endif /* CF_HOLDING */

ret0:

#if	CF_DEBUGS
	debugprintf("ipasswd_fetch: ret rs=%d ul=%u\n",rs,ul) ;
#endif

	return (rs >= 0) ? ul : rs ;
}
/* end subroutine (ipasswd_fetch) */


#ifdef	COMMENT

/* get the entries (serially) */
int ipasswd_get(op,ri,rpp)
IPASSWD		*op ;
int		ri ;
IPASSWD_ENT	**rpp ;
{


#if	CF_DEBUGS
	debugprintf("ipasswd_get: entered 0\n") ;
#endif

#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != IPASSWD_MAGIC)
	    return SR_NOTOPEN ;
#endif

#if	CF_DEBUGS
	debugprintf("ipasswd_get: entered 2\n") ;
#endif

	if (rpp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("ipasswd_get: entered ri=%u\n",ri) ;
#endif

	if ((ri < 0) || (ri >= op->rtlen))
	    return SR_NOTFOUND ;

	*rpp = NULL ;
	if (ri > 0)
	    *rpp = op->rectab + ri ;

#if	CF_DEBUGS
	debugprintf("ipasswd_get: OK\n") ;
#endif

	return ri ;
}
/* end subroutine (ipasswd_get) */

#endif /* COMMENT */


/* get information */
int ipasswd_info(op,rp)
IPASSWD		*op ;
IPASSWD_INFO	*rp ;
{
	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != IPASSWD_MAGIC)
	    return SR_NOTOPEN ;

	if (rp == NULL)
	    return SR_FAULT ;

	if (rp != NULL) {

	    rp->collisions = op->collisions ;

	}

	return rs ;
}
/* end subroutine (ipasswd_info) */


/* do some checking */
int ipasswd_check(op,daytime)
IPASSWD		*op ;
time_t		daytime ;
{
	int	rs = SR_OK ;
	int	f_changed = FALSE ;

#if	CF_DEBUGS
	char	timebuf[TIMEBUFLEN + 1] ;
#endif


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != IPASSWD_MAGIC)
	    return SR_NOTOPEN ;

	if (op->f.held)
	    return SR_OK ;

/* OK, now we can check some stuff */

	if (op->fd < 0)
	    return SR_OK ;

#if	CF_DEBUGS
	debugprintf("ipasswd_check: %s\n",
	    timestr_log(daytime,timebuf)) ;
#endif

	if ((daytime - op->ti_access) > TO_ACCESS)
	    goto closeit ;

	if ((daytime - op->ti_open) > TO_OPEN)
	    goto closeit ;

ret0:
	return (rs >= 0) ? f_changed : rs ;

/* handle a close out */
changed:
	f_changed = TRUE ;

closeit:
	rs = ipasswd_fileclose(op) ;

	goto ret0 ;
}
/* end subroutine (ipasswd_check) */


/* free up this IPASSWD object */
int ipasswd_close(op)
IPASSWD		*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != IPASSWD_MAGIC)
	    return SR_NOTOPEN ;

	if (op->mapdata != NULL) {
	    rs1 = ipasswd_mapend(op) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->fd >= 0) {
	    u_close(op->fd) ;
	    op->fd = -1 ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (ipasswd_close) */


/* private subroutines */


/* read the file header and check it out */
static int ipasswd_fileheader(op)
IPASSWD		*op ;
{
	uint	*table = NULL ;

	int	rs = SR_OK ;
	int	i ;
	int	f ;

	const uchar	*vetu ;

	const char	*cp ;


	cp = (char *) op->mapdata ;
	f = (strncmp(cp,IPASSWD_FILEMAGIC,IPASSWD_FILEMAGICLEN) == 0) ;
	f = f && (*(cp + IPASSWD_FILEMAGICLEN) == '\n') ;

	if (! f) {

#if	CF_DEBUGS
	    debugprintf("ipasswd_fileheader: bad magic=>%t<\n",
	        cp,strnlen(cp,14)) ;
#endif

	    rs = SR_BADFMT ;
	    goto bad3 ;
	}

	vetu = (const uchar *) (cp + 16) ;

	if (vetu[0] > IPASSWD_FILEVERSION) {
	    rs = SR_NOTSUP ;
	    goto bad3 ;
	}

	if (vetu[1] != ENDIAN) {
	    rs = SR_NOTSUP ;
	    goto bad3 ;
	}

/* the recorder options (type) */

	op->ropts = vetu[2] ;

#if	CF_DEBUGS
	debugprintf("ipasswd_fileheader: ropts=%02x\n",op->ropts) ;
#endif

/* if looks good, read the header stuff */

	table = (uint *) (op->mapdata + IPASSWD_IDLEN) ;

#if	CF_DEBUGS
	{
	    for (i = 0 ; i < header_overlast ; i += 1)
	        debugprintf("ipasswd_fileheader: header[%02d]=%08x\n",
	            i,table[i]) ;
	}
#endif /* CF_DEBUGS */

/* try to validate the header table */

	for (i = header_rectab ; i < IPASSWD_IDLEN ; i += 1) {

	    if (table[i] >= op->filesize)
	        break ;

	} /* end for */

	if (i < IPASSWD_IDLEN) {
	    rs = SR_BADFMT ;
	    goto bad0 ;
	}

/* extract the header table values */

	op->rectab = (IPASSWD_ENT *) (op->mapdata + table[header_rectab]) ;
	op->rtlen = table[header_rectablen] ;

	op->stab = (const char *) (op->mapdata + table[header_strtab]) ;
	op->stlen = table[header_strtabsize] ;

	op->rilen = table[header_indexlen] ;

	op->recind[0] = (uint (*)[2]) (op->mapdata + table[header_indexl1]) ;
	op->recind[1] = (uint (*)[2]) (op->mapdata + table[header_indexl3]) ;
	op->recind[2] = (uint (*)[2]) (op->mapdata + table[header_indexf]) ;
	op->recind[3] = (uint (*)[2]) (op->mapdata + table[header_indexfl3]) ;
	op->recind[4] = (uint (*)[2]) (op->mapdata + table[header_indexun]) ;

#if	CF_DEBUGS
	debugprintf("ipasswd_fileheader: rtlen=%u\n",op->rtlen) ;
	debugprintf("ipasswd_fileheader: rilen=%u\n",op->rilen) ;
	debugprintf("ipasswd_fileheader: stlen=%u\n",op->stlen) ;
#endif

#if	CF_DEBUGS
	{
	    for (i = 1 ; i < op->rtlen ; i += 1) {
	        debugprintf("ipasswd_fileheader: rec[%02d] ui=%03u li=%03u\n",
	            i,op->rectab[i].username,op->rectab[i].last) ;
	    }
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	{
	    const char	*ol = op->stab + op->stlen ;
	    char	prstr[PRSTRLEN + 1] ;
	    cp = (char *) (op->stab + 1) ;
	    for (i = 0 ; cp < ol ; i += 1) {
	        mkprstr(prstr,PRSTRLEN,cp,-1) ;
	        debugprintf("ipasswd_fileheader: s[%02d]=>%s<\n", i,prstr) ;
	        cp += (strlen(cp) + 1) ;
	    }
	}
#endif /* CF_DEBUGS */

ret0:
	return rs ;

/* bad stuff comes here */
bad3:
bad0:
	goto ret0 ;
}
/* end subroutine (ipasswd_fileheader) */


/* acquire access to the file (mapped memory) */
static int ipasswd_holdget(op,daytime)
IPASSWD		*op ;
time_t		daytime ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	mpages, fpages ;
	int	f_changed = FALSE ;


#if	CF_DEBUGS
	debugprintf("ipasswd_holdget: entered\n") ;
#endif

	if (op->f.held)
	    return SR_OK ;

	if (op->mapdata != NULL) {

	    if ((daytime - op->ti_access) < TO_CHECK)
	        return SR_OK ;

/* has the file changed at all (try checking size and mtime)? */

	    rs = u_stat(op->fname,&sb) ;

	    if (rs == SR_NOENT)
	        return SR_OK ;

	    if (rs < 0)
	        goto bad2 ;

/* has the file size changed? */

	    if (sb.st_size != op->filesize) {

	        f_changed = TRUE ;
	        mpages = uceil(op->mapsize,op->pagesize) ;

	        fpages = uceil(sb.st_size,op->pagesize) ;

	        if (fpages > mpages) {

	            f_changed = TRUE ;
	            u_munmap(op->mapdata,op->mapsize) ;

	            op->mapdata = NULL ;	/* signal a re-map */

	        }

	    } /* end if (file size changed) */

#if	CF_MEMSYNC
	    if ((op->mapdata != NULL) &&
	        op->f.remote && (sb.st_mtime != op->mtime)) {
	        const int	mflags = (MS_SYNC | MS_INVALIDATE) ;

	        f_changed = TRUE ;
	        rs = uc_msync(op->mapdata,op->mapsize,mflags) ;

	    }
#endif /* CF_MEMSYNC */

	} /* end if (checking existing map) */

/* do a map or a re-map */

	if ((rs >= 0) && (op->mapdata == NULL)) {

	    f_changed = TRUE ;
	    if ((rs = ipasswd_fileopen(op,daytime)) >= 0) {

	        if ((rs = u_fstat(op->fd,&sb)) >= 0) {
		    op->filesize = sb.st_size ;
		    rs = ipasswd_mapbegin(op,daytime) ;
	        } /* end if (fstat) */

	        ipasswd_fileclose(op) ;
	    } /* end if (opened the file) */

	    if (rs < 0)
	        goto bad2 ;

	} /* end if (mapping file) */

	if (f_changed) {

	    op->filesize = sb.st_size ;
	    op->mtime = sb.st_mtime ;

	}

#if	CF_DEBUGS
	debugprintf("ipasswd_holdget: ret rs=%d f_changed=%d\n",
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
/* end subroutine (ipasswd_holdget) */


/* release our hold on the filemap */
/* ARGSUSED */
static int ipasswd_holdrelease(op,daytime)
IPASSWD		*op ;
time_t		daytime ;
{
	int	rs = SR_OK ;

	if (op->f.held) {
	    op->f.held = FALSE ;
	}

	return rs ;
}
/* end subroutine (ipasswd_holdrelease) */


#if	CF_LOCKING

/* acquire access to the file (mapped memory) */
static int ipasswd_lockget(op,daytime,f_read)
IPASSWD		*op ;
time_t		daytime ;
int		f_read ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	mpages, fpages ;
	int	f_changed = FALSE ;


#if	CF_DEBUGS
	debugprintf("ipasswd_lockget: entered\n") ;
#endif

	if (op->fd < 0)
	    rs = ipasswd_fileopen(op,daytime) ;

#if	CF_DEBUGS
	debugprintf("ipasswd_lockget: ipasswd_fileopen() rs=%d fd=%d\n",
	    rs,op->fd) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	if (f_read) {

#if	CF_DEBUGS
	    debugprintf("ipasswd_lockget: need READ lock\n") ;
#endif

	    op->f.lockedread = TRUE ;
	    rs = lockfile(op->fd,F_RLOCK,0L,op->filesize,TO_LOCK) ;

	} else {

#if	CF_DEBUGS
	    debugprintf("ipasswd_lockget: need WRITE lock\n") ;
#endif

	    op->f.lockedwrite = TRUE ;
	    rs = lockfile(op->fd,F_WLOCK,0L,op->filesize,TO_LOCK) ;

	}

#if	CF_DEBUGS
	debugprintf("ipasswd_lockget: lockfile() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

/* has the size of the file changed? */

	rs = u_fstat(op->fd,&sb) ;

	if (rs < 0)
	    goto bad2 ;

	op->filesize = sb.st_size ;
	if (op->mapdata != NULL) {

	    mpages = uceil(op->mapsize,op->pagesize) ;

	    fpages = uceil(op->filesize,op->pagesize) ;

	    if (fpages > mpages) {

	        f_changed = TRUE ;
	        u_munmap(op->mapdata,(size_t) op->mapsize) ;
	        op->mapdata = NULL ;

	    }

#if	CF_MEMSYNC
	    if ((op->mapdata != NULL) &&
	        op->f.remote && (op->mtime != sb.st_mtime)) {

	        f_changed = TRUE ;
	        op->mtime = sb.st_mtime ;
	        flags = MS_SYNC | MS_INVALIDATE ;
	        rs = uc_msync(op->mapdata,op->mapsize,flags) ;

	    }
#endif /* CF_MEMSYNC */

	} /* end if (checking existing map) */

	if ((rs >= 0) && (op->mapdata == NULL)) {
	    size_t	msize ;
	    int		mprot ;
	    int		mflags ;
	    void	*p ;

	    f_changed = TRUE ;
	    msize = uceil(op->filesize,op->pagesize) ;

	    if (msize == 0) msize = op->pagesize ;

	    mprot = (PROT_READ | PROT_WRITE) ;
	    mflags = MAP_SHARED ;
	    if ((rs = u_mmap(NULL,msize,mprot,mflags,op->fd,0L,&p)) >= 0) {
		op->mapdata = p ;
		op->mapsize = msize ;
	        op->mtime = sb.st_mtime ;
	    }

#if	CF_DEBUGS
	    debugprintf("ipasswd_lockget: u_mmap() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto bad2 ;

	} /* end if (mapping file) */

#if	CF_DEBUGS
	debugprintf("ipasswd_lockget: ret rs=%d\n",rs) ;
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
/* end subroutine (ipasswd_lockget) */


static int ipasswd_lockrelease(op)
IPASSWD		*op ;
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
/* end subroutine (ipasswd_lockrelease) */

#endif /* CF_LOCKING */


static int ipasswd_fileopen(op,daytime)
IPASSWD		*op ;
time_t		daytime ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	size ;
	int	f ;


	if (op->fd >= 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("ipasswd_fileopen: fname=%s\n",op->fname) ;
#endif

	rs = uc_open(op->fname,op->oflags,op->operm) ;

#if	CF_DEBUGS
	debugprintf("ipasswd_fileopen: u_open() rs=%d\n",rs) ;
#endif

	op->fd = rs ;
	if (rs < 0)
	    goto bad0 ;

	if (daytime < 0)
	    daytime = time(NULL) ;

	op->ti_open = daytime ;
	uc_closeonexec(op->fd,TRUE) ;

	rs = u_fstat(op->fd,&sb) ;

#if	CF_DEBUGS
	debugprintf("ipasswd_fileopen: u_fstat() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

/* check file size */

	size = IPASSWD_IDLEN + (header_overlast * sizeof(int)) ;

/* wait for the file to come in if it is not yet available */

#if	CF_FILEWAIT

	for (i = 0 ; 
	    (i < TO_FILECOME) && (rs >= 0) && (sb.st_size < size) ;
	    i += 1) {

	    sleep(1) ;

	    rs = u_fstat(op->fd,&sb) ;

	} /* end while */

	if (rs < 0)
	    goto bad2 ;

	if (i >= TO_FILECOME) {

#if	CF_DEBUGS
	    debugprintf("ipasswd_fileopen: file-open timed out i=%d\n",i) ;
#endif

	    rs = SR_TIMEDOUT ;
	    goto bad2 ;
	}

#else /* CF_FILEWAIT */

	if (sb.st_size < size) {
	    rs = SR_TIMEDOUT ;
	    goto bad2 ;
	}

#endif /* CF_FILEWAIT */

/* is the file on a local or remote filesystem? */

	{
	    char	fstype[USERNAMELEN + 1] ;
	    int		fslen ;

	    if ((rs = getfstype(fstype,USERNAMELEN,op->fd)) >= 0) {
	        fslen = rs ;
		f = islocalfs(fstype,fslen) ;
	        op->f.remote = (! f) ; /* remote if not local! */
	    }

	} /* end block */

	if (rs < 0)
	    goto bad1 ;

/* store some file information */

	op->mtime = sb.st_mtime ;
	op->filesize = sb.st_size ;

/* done */
ret0:
	return (rs >= 0) ? op->fd : rs ;

/* bad things */
bad2:
bad1:
	u_close(op->fd) ;

	op->fd = -1 ;

bad0:
	goto ret0 ;
}
/* end subroutine (ipasswd_fileopen) */


static int ipasswd_fileclose(op)
IPASSWD		*op ;
{
	int	rs = SR_OK ;

	if (op->fd >= 0) {
	    rs = u_close(op->fd) ;
	    op->fd = -1 ;
	}

	return rs ;
}
/* end subroutine (ipasswd_fileclose) */


static int ipasswd_mapbegin(IPASSWD *op,time_t daytime)
{
	const int	mp = PROT_READ ;
	const int	mf = MAP_SHARED ;
	const int	fd = op->fd ;
	size_t		ms ;
	int		rs ;
	void		*md ;

	ms = uceil(op->filesize,op->pagesize) ;
	if (ms == 0) ms = op->pagesize ;

	if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	    op->mapdata = md ;
	    op->mapsize = ms ;
	    op->ti_map = daytime ;
	}

	return rs ;
}
/* end subroutine (ipasswd_mapbegin) */


static int ipasswd_mapend(IPASSWD *op)
{
	int	rs = SR_OK ;
	if (op->mapdata != NULL) {
	    rs = u_munmap(op->mapdata,op->mapsize) ;
	    op->mapdata = NULL ;
	    op->mapsize = 0 ;
	}
	return rs ;
}
/* end subroutine (ipasswd_mapend) */


static int ipasswd_keymatchfl3(op,opts,ri,np)
IPASSWD		*op ;
int		opts ;
int		ri ;
REALNAME	*np ;
{
	int	si ;
	int	f ;


	si = op->rectab[ri].last ;
	f = (strncmp((op->stab + si),np->last,3) == 0) ;

#if	CF_DEBUGS
	debugprintf("ipasswd_keymatchfl3: sp=%s last=%s f=%u\n",
	    (op->stab + si),np->last,f) ;
#endif

	if (f) {

	    si = op->rectab[ri].first ;
	    f = ((op->stab + si)[0] == np->first[0]) ;

#if	CF_DEBUGS
	    debugprintf("ipasswd_keymatchfl3: sp=%s firrst=%s f=%u\n",
	        (op->stab + si),np->first,f) ;
#endif

	}

	return f ;
}
/* end subroutine (ipasswd_keymatchfl3) */


static int ipasswd_keymatchl3(op,opts,ri,np)
IPASSWD		*op ;
int		opts ;
int		ri ;
REALNAME	*np ;
{
	int	si ;
	int	f ;


	si = op->rectab[ri].last ;
	f = (strncmp((op->stab + si),np->last,3) == 0) ;

	return f ;
}
/* end subroutine (ipasswd_keymatchl3) */


static int ipasswd_keymatchl1(op,opts,ri,np)
IPASSWD		*op ;
int		opts ;
int		ri ;
REALNAME	*np ;
{
	int	si ;
	int	f ;


	si = op->rectab[ri].last ;
	f = ((op->stab + si)[0] == np->last[0]) ;

	return f ;
}
/* end subroutine (ipasswd_keymatchl1) */


static int ipasswd_keymatchf(op,opts,ri,np)
IPASSWD		*op ;
int		opts ;
int		ri ;
REALNAME	*np ;
{
	int	si ;
	int	f ;


	si = op->rectab[ri].first ;
	f = ((op->stab + si)[0] == np->first[0]) ;

	return f ;
}
/* end subroutine (ipasswd_keymatchf) */


#ifdef	COMMENT

static int ipasswd_keymatchlast(op,opts,ri,sp,hl)
IPASSWD		*op ;
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

	if (opts & IPASSWD_FLASTFULL)
	    f = (strncmp((op->stab + si),sp,3) == 0) ;

	else
	    f = (strncmp((op->stab + si),sp,hl) == 0) ;

	return f ;
}
/* end subroutine (ipasswd_keymatchlast) */

#endif /* COMMENT */


static int ipasswd_keymatchall(op,opts,ri,np)
IPASSWD		*op ;
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
	debugprintf("keymatchall: ri=%u last=%s first=%s\n",ri,
	    (s + op->rectab[ri].last),
	    (s + op->rectab[ri].first)) ;
#endif

/* last */

	if (opts & IPASSWD_FLASTFULL) {

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
/* end subroutine (ipasswd_keymatchall) */


/* calculate the next hash table index from a given one */
static int hashindex(i,n)
uint	i, n ;
{
	uint	hi ;


	hi = MODP2(i,n) ;

	if (hi == 0)
	    hi = 1 ;

	return hi ;
}
/* end subroutine (hashindex) */


#if	CF_DEBUGS

static int mkprstr(buf,buflen,s,slen)
char		buf[] ;
int		buflen ;
const char	s[] ;
int		slen ;
{
	int	n, i ;


	if (slen < 0)
	    slen = strlen(s) ;

	if (buflen < 0)
	    buflen = INT_MAX ;

	n = 0 ;
	for (i = 0 ; (i < slen) && s[i] ; i += 1) {

	    if (n >= buflen)
	        break ;

	    buf[n] = (char) ('?' + 128) ;
	    if (isprintlatin(s[i]))
	        buf[n] = s[i] ;

	    n += 1 ;

	} /* end for */

	buf[n] = '\0' ;
	return n ;
}
/* end subroutine (mkprstr) */

#endif /* CF_DEBUGS */


