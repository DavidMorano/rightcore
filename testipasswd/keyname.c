/* keyname */

/* handle setting key parameter for names */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		1		/* safe mode */
#define	CF_SECONDARY	1		/* use secondary hashing */
#define	CF_RANDLC	1		/* use 'randlc()' for secondary hash */


/* revision history:

	= 02/05/01, David A­D­ Morano

	This object module was created for Levo research, to determine
	if a conditional branch at a given instruction address is
	a SS-Hamock or not.


	= 03/06/11, David A­D­ Morano

	I snarfed this file from the SS-Hammock crap since I thought
	it might be a bit similar.  We'll see how it works out ! :-)


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	


*******************************************************************************/


#define	KEYNAME_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<realname.h>
#include	<storeitem.h>
#include	<localmisc.h>

#include	"keyname.h"


/* local defines */


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */


/* local variables */


/* exported subroutines */


int keyname_initparse(knp,name,namelen)
KEYNAME		*knp ;
const char	name[] ;
int		namelen ;
{
	    STOREITEM	s ;

	REALNAME	rn ;

	int	rs ;


	    storeitem_start(&s,knp->store,IPASSWD_STORELEN) ;

/* last */

	    si = op->rectab[ri].last ;
	    sp = (char *) (op->stab + si) ;
	    storeitem_strw(&s,sp,-1,&cp) ;

	    np->last = sp ;

/* first */

	    si = op->rectab[ri].first ;
	    sp = (char *) (op->stab + si) ;
	    storeitem_strw(&s,sp,-1,&cp) ;

	    np->first = sp ;

/* middle-1 */

	    si = op->rectab[ri].m1 ;
	    sp = (char *) (op->stab + si) ;
	    storeitem_strw(&s,sp,-1,&cp) ;

	    np->m1 = sp ;

/* middle-2 */

	    si = op->rectab[ri].m2 ;
	    sp = (char *) (op->stab + si) ;
	    storeitem_strw(&s,sp,-1,&cp) ;

	    np->m2 = sp ;

/* done */

	    rs = storeitem_free(&s) ;

	} /* end if */

	if (up != NULL) {

	    ui = op->rectab[ri].username ;
	    strwcpy(up,(op->stab + ui),IPASSWD_USERNAMELEN) ;

	}

/* update the cursor */

	cup->i[0] = ri + 1 ;

	return (rs >= 0) ? ri : rs ;
}
/* end subroutine (ipasswd_enum) */


int ipasswd_fetch(op,np,cup,up)
IPASSWD		*op ;
IPASSWD_NAME	*np ;
IPASSWD_CURSOR	*cup ;
char		*up ;
{
	IPASSWD_CURSOR	cur ;

	uint	rhash ;

	int	rs, i, wi, si, ui, hi, ri ;
	int	hl, c, n ;
	int	klen[IPASSWD_NINDICES] ;
	int	nlen[4] ;
	int	f_cur = FALSE ;

	char	*sp ;
	char	*cp ;

#if	CF_SAFE
	if (op == NULL)
	    return SR_FAULT ;
#endif

/* this is needed for when no file was found */

	if (op->magic != IPASSWD_MAGIC)
	    return SR_NOTOPEN ;

	if (cup == NULL) {

	    cup = &cur ;
		for (i = 0 ; i < IPASSWD_NINDICES ; i += 1)
	    	cup->i[i] = -1 ;

	} else {

	    f_cur = TRUE ;
	    if (! op->f.cursor)
	        return SR_INVALID ;

	}

	n = op->rilen ;

/* calculate length of our given keys */

	for (i = 0 ; i < 4 ; i += 1) {

		switch (i) {

		case 0:
	sp = (char *) np->first ;
		break ;

		case 1:
	sp = (char *) np->m1 ;
		break ;

		case 2:
	sp = (char *) np->m2 ;
		break ;

		case 3:
	sp = (char *) np->last ;
		break ;

	} /* end switch */

	nlen[i] = strlen(sp) ;

	} /* end for */

/* which index do we want to use ? */

	wi = (nlen[3] < 3) ? 0 : 1 ;

/* OK, we go from here */

	hl = MIN(nlen[3],((wi == 0) ? 1 : 3)) ;

	if (cup->i[wi] < 0) {

	    rhash = hashelf(sp,hl) ;

#if	CF_DEBUGS
	    debugprintf("ipasswd_fetch: rhash=%08x \n",rhash) ;
#endif

	    hi = hashindex(rhash,op->rilen) ;

/* start searching ! */

#if	CF_SECONDARY
	    c = 0 ;
	    while ((ri = (op->recind[wi])[hi][0]) != 0) {

	        if (keymatchlast(op,ri,sp,hl))
	            break ;

#if	CF_RANDLC
	        rhash = randlc(rhash + c) ;
#else
	        rhash = ((rhash << (32 - ns)) | (rhash >> ns)) + c ;
#endif
	        hi = hashindex(rhash,n) ;

	        c += 1 ;

	    } /* end while */

	    if (ri == 0)
	        return SR_NOTFOUND ;

#endif /* CF_SECONDARY */

	} else {

	    hi = cup->i[wi] ;
	    if (hi == 0)
		return SR_NOTFOUND ;

	    ri = (op->recind[wi])[hi][0] ;
	    if (ri != 0) {

		hi = (op->recind[wi])[hi][1] ;
		if (hi == 0)
			return SR_NOTFOUND ;

	        ri = (op->recind[wi])[hi][0] ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("ipasswd_fetch: hi=%d\n",hi) ;
#endif

	while ((ri = (op->recind[wi])[hi][0]) != 0) {

	    if (keymatchall(op,ri,np,nlen))
	        break ;

	    hi = (op->recind[wi])[hi][1] ;
	    if (hi == 0)
	        break ;

	} /* end while */

	if ((ri == 0) || (hi == 0))
	    rs = SR_NOTFOUND ;

	if (up != NULL) {

	    ui = op->rectab[ri].username ;
	    cp = strwcpy(up,(op->stab + ui),IPASSWD_USERNAMELEN) ;

	    rs = (cp - up) ;
	}

/* update cursor */

	if (f_cur) {

	    cup->i[wi] = (op->recind[wi])[hi][1] ;

	}

	return rs ;
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
	debugprintf("ipasswd_get: entered ri=%d\n",ri) ;
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


/* get information about all static branches */
int ipasswd_info(op,rp)
IPASSWD		*op ;
IPASSWD_INFO	*rp ;
{
	int	rs = SR_NOTSUP ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != IPASSWD_MAGIC)
	    return SR_NOTOPEN ;

	if (rp == NULL)
	    return SR_FAULT ;


	return rs ;
}
/* end subroutine (ipasswd_info) */


/* do some checking */
int ipasswd_check(lzp,daytime)
IPASSWD		*lzp ;
time_t		daytime ;
{
	int	rs = SR_OK ;


	if (lzp == NULL)
	    return SR_FAULT ;

	if (lzp->magic != IPASSWD_MAGIC)
	    return SR_NOTOPEN ;

	if (lzp->fd < 0)
	    return SR_OK ;

#if	CF_DEBUGS
	debugprintf("ipasswd_check: %s\n",
	    timestr_log(daytime,timebuf)) ;
#endif

	if ((daytime - lzp->accesstime) > TO_ACCESS)
	    goto closeit ;

	if ((daytime - lzp->opentime) > TO_OPEN)
	    goto closeit ;

	return rs ;

/* handle a close out */
closeit:
	rs = ipasswd_fileclose(lzp) ;

	return rs ;
}
/* end subroutine (ipasswd_check) */


/* free up this ipasswd object */
int ipasswd_close(op)
IPASSWD		*op ;
{
	int	rs = SR_BADFMT ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != IPASSWD_MAGIC)
	    return SR_NOTOPEN ;

	if (op->mapbuf != NULL)
	    rs = u_munmap(op->mapbuf,(size_t) op->filesize) ;

	if (op->fd >= 0)
	    u_close(op->fd) ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (ipasswd_close) */



/* INTERNAL SUBROUTINES */



static int ipasswd_fileopen(lzp)
IPASSWD	*lzp ;
{
	time_t	daytime ;

	int	rs ;


#if	CF_DEBUGS
	debugprintf("ipasswd_fileopen: fname=%s\n",lzp->fname) ;
#endif

	if (lzp->fd >= 0)
	    return lzp->fd ;

#if	CF_DEBUGS
	debugprintf("ipasswd_fileopen: need open\n") ;
#endif

	rs = u_open(lzp->fname,lzp->oflags,lzp->operm) ;

#if	CF_DEBUGS
	debugprintf("ipasswd_fileopen: u_open() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	lzp->fd = rs ;

	u_time(&daytime) ;

	lzp->opentime = daytime ;
	return lzp->fd ;

/* bad things */
bad0:
	return rs ;
}
/* end subroutine (ipasswd_fileopen) */


ipasswd_fileclose(lzp)
IPASSWD	*lzp ;
{
	int	rs = SR_OK ;


	if (lzp->fd >= 0) {

	    u_close(lzp->fd) ;

	    lzp->fd = -1 ;

	}

	return rs ;
}
/* end subroutine (ipasswd_fileclose) */


/* acquire access to the file (mapped memory) */
static int ipasswd_lockget(lzp,f_read)
IPASSWD	*lzp ;
int		f_read ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	mpages, fpages ;
	int	prot, flags ;


#if	CF_DEBUGS
	debugprintf("ipasswd_lockget: entered\n") ;
#endif

	if (lzp->fd < 0)
	    rs = ipasswd_fileopen(lzp) ;

#if	CF_DEBUGS
	debugprintf("ipasswd_lockget: ipasswd_fileopen() rs=%d fd=%d\n",
	    rs,lzp->fd) ;
#endif

	if (rs < 0)
	    goto bad0 ;

	if (f_read) {

#if	CF_DEBUGS
	    debugprintf("ipasswd_lockget: need READ lock\n") ;
#endif

	    lzp->f.lockedread = TRUE ;
	    rs = lockfile(lzp->fd,F_RLOCK,0L,lzp->filesize,TO_LOCK) ;

	} else {

#if	CF_DEBUGS
	    debugprintf("ipasswd_lockget: need WRITE lock\n") ;
#endif

	    lzp->f.lockedwrite = TRUE ;
	    rs = lockfile(lzp->fd,F_WLOCK,0L,lzp->filesize,TO_LOCK) ;

	}

#if	CF_DEBUGS
	debugprintf("ipasswd_lockget: lockfile() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad1 ;

/* has the size of the file changed ? */

	rs = u_fstat(lzp->fd,&sb) ;

	if (rs < 0)
	    goto bad2 ;

	if (lzp->mapbuf != NULL) {

	    mpages = ceiling(lzp->mapsize,lzp->pagesize) ;

	    fpages = ceiling(lzp->filesize,lzp->pagesize) ;

	    if (fpages > mpages) {

	        u_munmap(lzp->mapbuf,(size_t) lzp->mapsize) ;

	        lzp->mapbuf = NULL ;

	    }

#if	F_MEMSYNC
	    if ((lzp->mapbuf != NULL) &&
	        lzp->f.remote && (lzp->mtime != sb.st_mtime)) {

	        flags = MS_SYNC | MS_INVALIDATE ;
	        rs = uc_msync(lzp->mapbuf,lzp->mapsize,flags) ;

	    }
#endif /* F_MEMSYNC */

	} /* end if (checking existing map) */

	if (lzp->mapbuf == NULL) {

	    lzp->mapsize = ceiling(lzp->filesize,lzp->pagesize) ;

	    if (lzp->mapsize == 0)
	        lzp->mapsize = lzp->pagesize ;

	    prot = PROT_READ | PROT_WRITE ;
	    flags = MAP_SHARED ;
	    rs = u_mmap(((caddr_t) 0),(size_t) lzp->mapsize,prot,flags,
	        lzp->fd,0L,&lzp->mapbuf) ;

#if	CF_DEBUGS
	    debugprintf("ipasswd_lockget: u_mmap() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto bad2 ;

	    lzp->mtime = sb.st_mtime ;

	} /* end if (mapping file) */

#if	CF_DEBUGS
	debugprintf("ipasswd_lockget: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad2:
	lockfile(lzp->fd,F_ULOCK,0L,lzp->filesize,TO_LOCK) ;

bad1:
	lzp->f.lockedread = FALSE ;
	lzp->f.lockedwrite = FALSE ;

bad0:
	return rs ;
}
/* end subroutine (ipasswd_lockget) */


static int ipasswd_lockrelease(lzp)
IPASSWD	*lzp ;
{
	int	rs = SR_OK ;


	if (! (lzp->f.lockedread || lzp->f.lockedwrite))
	    return SR_OK ;

	if (lzp->fd >= 0)
	    rs = lockfile(lzp->fd,F_ULOCK,0L,lzp->filesize,TO_LOCK) ;

	lzp->f.lockedread = FALSE ;
	lzp->f.lockedwrite = FALSE ;

	return rs ;
}
/* end subroutine (ipasswd_lockrelease) */


/* calculate the next hash from a given one */
static int hashindex(i,n)
uint	i, n ;
{
	int	hi ;


	hi = MODP2(i,n) ;

	if (hi == 0)
	    hi += 1 ;

	return hi ;
}
/* end if (hashindex) */


static uint ceiling(v,a)
uint	v, a ;
{


	return (v + (a - 1)) & (~ (a - 1)) ;
}
/* end subroutine (ceiling) */


static int keymatchlast(op,ri,sp,hl)
IPASSWD		*op ;
int		ri ;
char		*sp ;
int		hl ;
{
	int	si ;


	si = op->rectab[ri].last ;
	return (strncmp(sp,(op->stab + si),hl) == 0) ;
}


static int keymatchall(op,ri,np,nlen)
IPASSWD		*op ;
int		ri ;
IPASSWD_NAME	*np ;
int		nlen[] ;
{
	int	si ;

	const char	*s = op->stab ;
	const char	*sp ;


/* last */

	si = op->rectab[ri].last ;
	if (strncmp(np->last,(s + si),nlen[3]) != 0)
	    return FALSE ;

/* first */

	si = op->rectab[ri].first ;
	sp = np->first ;
	if ((sp != NULL) && (sp[0] != '\0')) {

	    si = op->rectab[ri].first ;

	    if (strncmp(sp,(s + si),nlen[0]) != 0)
	        return FALSE ;

	}

/* middle-1 */

	si = op->rectab[ri].m1 ;
	sp = np->m1 ;
	if ((sp != NULL) && (sp[0] != '\0')) {

	    si = op->rectab[ri].m1 ;

	    if (strncmp(sp,(s + si),nlen[1]) != 0)
	        return FALSE ;

	}

/* middle-2 */

	si = op->rectab[ri].m1 ;
	sp = np->m2 ;
	if ((sp != NULL) && (sp[0] != '\0')) {

	    si = op->rectab[ri].m2 ;

	    if (strncmp(sp,(s + si),nlen[2]) != 0)
	        return FALSE ;

	}

	return TRUE ;
}
/* end subroutine (keymatchall) */



