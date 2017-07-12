/* fdb */

/* general-purpose file DB hashing */


#define	CF_DEBUG	0
#define	CF_ELFHASH	1


/* revision history:

	= 1998-04-01, David A­D­ Morano
        This hashing object is based in part on the "hdbm" hasing functions that
        are a part of Netnews software.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*************************************************************************

	This package provides a reasonably general purpose hashing
	object for use in cases calling for in-core hashing.

	Some of the features for these routines that made them necessary,
	as opposed to using everyone else's hashing routines, are that
	this package can store multiple key-data pairs with the same
	keys.

	The "walk" type method, which I never liked in other packages,
	is depreciated in favor of the "enumerate" ('hdbenum') type
	method instead.  There are two types of "delete" methods.  One
	deletes all entries with the same key (possibly useful for old
	timers who never knew what to do with muliple identical keys in
	the first place), and the other function deletes the entry
	under the current cursor.

	Oh, the "enumerate" and "fetch" functions require the concept
	of a CURSOR which is, roughly, a pointer type of a thing to the
	current entry.

	In terms of implementation, these routines are just as
	inelegant, or worse, than the DBM, HDBM, or other type of
	implementations ; namely, I implemented hashing with hash
	buckets (using a single table) with chained entries dangling.
	An additional dimension of chains may dangle from each of the
	previous described entries to facilitate muliple entries with
	the same key.

	Updated note :

	Before leaving, I want to bring to your attention to a new
	database package which looks to be really nice.  I am a great
	admirer of the newer Berkely DB functions and would recommend
	them for industrial strength hashing that requires file backing
	store.



***************************************************************************/


#define	FDB_MASTER	1


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<limits.h>

#include	<vsystem.h>

#include	"localmisc.h"
#include	"fdb.h"



/* local defines */

#define FDB_MAGIC		63141592

#define	NZ		2048

#define	TO_CHECKINIT	10		/* seconds */
#define	TO_WRITELOCK	10		/* seconds */

#define	OFF_FDB_MAGIC	0L
#define	OFF_CONTROL	4L
#define	OFF_HASHTABLE	(OFF_CONTROL + sizeof(struct control))

#define	CONSTRUCT	struct control



/* external subroutines */

extern int	lockfile(int,int,offset_t,offset_t,int) ;


/* local structures */

struct control {
	int	version ;
	int	nhash ;
	int	esize ;
	int	used ;			/* used bytes */
	int	free ;			/* free bytes */
	int	stamp ;			/* update stamp */
	time_t	gtime ;			/* last garbage collect time */
	offset_t	freelistp ;		/* pointer to free list */
} ;

struct vblock {
	offset_t	next ;
	int	size ;
} ;

struct fblock {
	offset_t	next ;
	int	size ;
} ;


/* forward references */

static int	readno(int,void *,int,offset_t) ;
static int	writeno(int,void *,int,offset_t) ;
static int	loadno(void *,void *,int) ;
static int	storeno(void *,void *,int) ;


/* exported subroutines */


/* subroutine to initialize (create) a new hash DB file */
int fdb_open(dbp,filename,uflags,permission,n,esize)
FDB	*dbp ;
char	filename[] ;
int	uflags, permission ;
int	n, esize ;
{
	struct ustat	sb ;

	struct control	c ;

	int	i, rs ;
	int	zero[NZ] ;
	int	to_modecheck = 0 ;
	int	to_checkinit = 0 ;
	int	oflags ;
	int	mprot ;

	char	mbuf[8] ;


	if ((dbp == NULL) || (filename == NULL))
		return SR_FAULT ;

	if (filename[0] == '\0')
		return SR_INVALID ;

	dbp->stamp = 0 ;
	dbp->timeout = FDB_DEFTIMEOUT ;
	dbp->mapsize = -1 ;
	dbp->uflags = uflags ;

/* user flag handling */

	oflags = O_RDWR ;
	if (uflags & O_CREAT)
		oflags |= O_CREAT ;

/* open the file according to the proper modes */
top:
	if ((rs = u_open(filename,oflags,permission)) < 0) 
		goto failopen ;

opened:
	dbp->fd = rs ;
	(void) u_fcntl(dbp->fd,F_SETFD,FD_CLOEXEC) ;

/* are we the one who is supposed to be creating this thing ? */

/* check if the DB is initialized ! */
checkinit:
	if ((rs = u_fstat(dbp->fd,&sb)) < 0)
		goto badstat ;

	if (sb.st_size < OFF_HASHTABLE) {

		if (oflags & O_CREAT)
			goto create ;

		sleep(1) ;

		to_checkinit += 1 ;
		if (to_checkinit < TO_CHECKINIT)
			goto checkinit ;

		return SR_TIMEDOUT ;
	}

/* the DB file is supposed to be 'init'ed ! */

/* check if it has the proper FILEFDB_MAGIC in it ! */
checkmagic:
	mbuf[0] = '\0' ;
	if ((rs = u_read(dbp->fd,mbuf,4L)) < 0)
		goto badmagic ;

	if (strncmp(mbuf,FDB_FILEFDB_MAGIC,FDB_FILEFDB_MAGICLEN) != 0) {

		rs = SR_BADFMT ;
		goto badmagic ;
	}

	if ((rs = lockfile(dbp->fd,F_RLOCK,0L,0L,FDB_DEFTIMEOUT)) < 0)
		goto faillock1 ;

	if ((rs = readno(dbp->fd,&c,sizeof(CONSTRUCT),OFF_CONTROL)) < 0)
		goto badcorrupt ;

	if (rs < sizeof(CONSTRUCT)) {

		rs = SR_BADFMT ;
		goto badcorrupt ;
	}

	if (c.version != FDB_VERSION) {

		rs = SR_NOTSUP ;
		goto badversion ;
	}

#ifdef	COMMENT
	(void) lockfile(dbp->fd,F_ULOCK,0L,0L,FDB_DEFTIMEOUT) ;
#else
	(void) uc_lockf(dbp->fd,F_ULOCK,0L) ;
#endif /* COMMENT */


/* map the file (control area and hash table is what we're interested in) */
map:
	mprot = PROT_READ ;
	if ((oflags & O_CREAT) || (oflags & O_RDWR))
		mprot |= PROT_WRITE ;

	dbp->mapsize = OFF_HASHTABLE + (c.nhash * sizeof(int)) ;
	rs = u_mmap(NULL,dbp->mapsize,
		mprot,MAP_SHARED,dbp->fd,0L,&dbp->buf) ;

	if (rs < 0)
		goto badmap ;

	dbp->magic = FDB_MAGIC ;
	return dbp->fd ;

/* we got an 'open()' failure */
failopen:
	if (u_stat(filename,&sb) < 0)
		return rs ;

	if (sb.st_mode != 0) 
		return rs ;

/* it's a mode==0 type of creation in progress */

	sleep(1) ;

	to_modecheck += 1 ;
	if (to_modecheck < TO_CHECKINIT)
		goto top ;

		return SR_TIMEDOUT ;

/* we are to initialize the file ! */
create:
	if ((rs = lockfile(dbp->fd,F_WLOCK,0L,0L,TO_WRITELOCK)) < 0)
		goto checkinit ;

	if ((rs = u_fstat(dbp->fd,&sb)) < 0)
		return rs ;

	if (sb.st_size >= OFF_HASHTABLE) {

#ifdef	COMMENT
	lockfile(dbp->fd,F_ULOCK,0L,0L,FDB_DEFTIMEOUT) ;
#else
	uc_lockf(dbp->fd,F_ULOCK,0L) ;
#endif /* COMMENT */

		goto checkmagic ;
	}

/* we have the write lock on an empty file */

	u_pwrite(dbp->fd,FDB_FILEFDB_MAGIC,4,OFF_FDB_MAGIC) ;

	c.version = FDB_VERSION ;
	c.nhash = MAX(n,FDB_DEFNHASH) ;
	c.esize = MAX(esize,FDB_DEFESIZE) ;
	c.used = 0 ;
	c.free = 0 ;
	c.stamp = 0 ;
	c.gtime = 0 ;
	c.freelistp = 0L ;
	writeno(dbp->fd,&c,sizeof(CONSTRUCT),OFF_CONTROL) ;


/* write the hash table */

	for (i = 0 ; i < MIN(c.nhash,NZ) ; i += 1)
		zero[i] = 0 ;

	u_seek(dbp->fd,OFF_HASHTABLE,SEEK_SET) ;

	{
		int	nz = c.nhash ;


		while (nz > NZ) {

			u_write(dbp->fd,zero,(NZ * sizeof(int))) ;

			nz -= NZ ;

		} /* end while */

		if (nz > 0)
		u_write(dbp->fd,zero,(nz * sizeof(int))) ;

	} /* end block (writing the has table) */


/* get out! */

	uc_fsync(dbp->fd) ;

	lockfile(dbp->fd,F_ULOCK,0L,0L,FDB_DEFTIMEOUT) ;

	goto map ;

/* bad stuff comes here */
badmap:
badversion:
badcorrupt:
faillock1:
badmagic:
badstat:
	(void) u_close(dbp->fd) ;

	return rs ;
}
/* end subroutine (fdb_open) */


/* close the database */
int fdb_close(dbp)
FDB	*dbp ;
{


	if (dbp == NULL)
		return SR_FAULT ;

	if (dbp->magic != FDB_MAGIC)
		return SR_NOTOPEN ;

	if (dbp->mapsize > 0)
		u_munmap(dbp->buf,dbp->mapsize) ;

	dbp->magic = 0 ;
	return u_close(dbp->fd) ;
}
/* end subroutine (fdb_close) */



#ifdef	COMMENT

	(void) u_msync(dbp->fd,(OFF_HASHTABLE + (c.nhash * sizeof(int))),
		MS_SYNC) ;

#endif /* COMMENT */



#ifdef	CONTINUE

/* store an item into the DB */
int fdb_store(hhp,key,value)
FDB_HEAD	*hhp ;
FDB_DATUM	key, value ;
{
	FDB_ENT	*hp, *ohp ;
	FDB_ENT	**nextp ;


#if	CF_DEBUG
	debugprintf("hdbstore: entered\n") ;
#endif

	if (BADTBL(hhp))
	    return BAD ;

#if	CF_DEBUG
	debugprintf("hdbstore: good table\n") ;
#endif

#if	CF_DEBUG
	debugprintf("hdbstore: key=%s len=%d\n",
	    key.buf,key.len) ;
#endif

	if (key.buf == NULL) return BAD ;

	if (key.len < 0)
	    key.len = strlen(key.buf) ;

#if	CF_DEBUG
	debugprintf("hdbstore: key perused, key=%W len=%d\n",
	    key.buf,key.len,key.len) ;
#endif

#ifdef	COMMENT
	if (value.buf == NULL) return BAD ;
#endif

	if ((value.buf != NULL) && (value.len < 0))
	    value.len = strlen(value.buf) ;

#if	CF_DEBUG
	debugprintf("hdbstore: data perused\n") ;
#endif

	nextp = i_hdbfind(hhp,key) ;

#if	CF_DEBUG
	debugprintf("hdbstore: find done\n") ;
#endif

	if (nextp == NULL)
	    return BAD ;

#if	CF_DEBUG
	debugprintf("hdbstore: entering the entry\n") ;
#endif

	hp = (FDB_ENT *) malloc(sizeof(FDB_ENT)) ;

	if (hp == NULL)
	    return BAD ;

	hp->he_next = NULL ;
	hp->he_key = key ;
	hp->same = NULL ;
	hp->he_value = value ;

	ohp = *nextp ;
	if (ohp != NULL) {

	    while (ohp->same != NULL)
	        ohp = ohp->same ;

	    ohp->same = hp ;

	} else
	    *nextp = hp ;			/* append to hash chain */

	hhp->count += 1 ;
	return OK ;
}
/* end subroutine (hdbstore) */


/* delete all entries with a specified key */

/*
	This subroutine deletes all entries with the specified "key".
	We return the number of entries delete.
*/

int fdb_delkey(hhp,key)
register FDB_HEAD	*hhp ;
FDB_DATUM		key ;
{
	register FDB_ENT	*hp, *nhp ;
	FDB_ENT		**nextp ;

	int	initial ;


	if (BADTBL(hhp)) return BAD ;

	nextp = i_hdbfind(hhp, key) ;

	if (nextp == NULL)
	    return BAD ;

	hp = *nextp ;
	if (hp == NULL)				/* absent */
	    return 0 ;

	initial = hhp->count ;
	*nextp = hp->he_next ;			/* skip this entry */
	hp->he_next = NULL ;
	hp->he_value.buf = hp->he_key.buf = NULL ;	/* optional */

	while (hp->same != NULL) {

	    nhp = hp->same ;
	    free(hp) ;

	    hp = nhp ;
	    hhp->count -= 1 ;

	} /* end while */

	free(hp) ;

	hhp->count -= 1 ;
	return (initial - hhp->count) ;
}
/* end subroutine (hdbdelkey) */


/* delete an entry by its cursor */

/*
	This subroutine deletes one entry specified by the
	given cursor.
*/

int fdb_delcur(hhp,cp)
FDB_HEAD	*hhp ;
FDB_CUR	*cp ;
{
	FDB_ENT	*hp, *pihp, *pjhp ;
	FDB_ENT	**hepp ;

	int		hhpsize ;
	register int	i, j ;


	if (BADTBL(hhp)) return BAD ;

	if (cp == NULL) return BAD ;

#if	CF_DEBUG
	debugprintf("hdbdelcursor: entered, index=%d i=%d j=%d\n",
	    cp->index,cp->i,cp->j) ;
#endif

	hepp = hhp->ht_addr ;
	hhpsize = hhp->ht_size ;
	if ((cp->index < 0) || (cp->index > hhpsize)) return BAD ;

	if ((hp = hepp[cp->index]) == NULL) return BAD ;

	if (cp->i < 0) return BAD ;

#if	CF_DEBUG
	debugprintf("hdbdelcursor: ready to rock !\n") ;
#endif

	i = 0 ;
	while (i < cp->i) {

	    if (hp->he_next == NULL) return BAD ;

	    pihp = hp ;
	    hp = hp->he_next ;
	    i += 1 ;

	} /* end while */

#if	CF_DEBUG
	debugprintf("hdbdelcursor: past first loop\n") ;
#endif

	if (cp->j < 0) return BAD ;

#if	CF_DEBUG
	debugprintf("hdbdelcursor: continuing\n") ;
#endif

	j = 0 ;
	while (j < cp->j) {

	    if (hp->same == NULL) return BAD ;

	    pjhp = hp ;
	    hp = hp->same ;
	    j += 1 ;

	} /* end while */

/* do it */

#if	CF_DEBUG
	debugprintf("hdbdelcursor: the man says, do it !\n") ;
#endif

	if (j = 0) {

	    if (i == 0) {

	        if (hp->same != NULL) {

	            (hp->same)->he_next = hp->he_next ;
	            hepp[cp->index] = hp->same ;

	        } else {

	            hepp[cp->index] = hp->he_next ;

	        }

	    } else {

	        if (hp->same != NULL) {

	            (hp->same)->he_next = hp->he_next ;
	            pihp->he_next = hp->same ;

	        } else {

	            pihp->he_next = hp->he_next ;

	        }

	    }

	} else
	    pjhp->same = hp->same ;

#if	CF_DEBUG
	debugprintf("hdbdelcursor: freeing\n") ;
#endif

	free(hp) ;

#if	CF_DEBUG
	debugprintf("hdbdelcursor: out of here\n") ;
#endif

	hhp->count -= 1 ;
	return OK ;
}
/* end subroutine (hdbdelcursor) */


/* subroutine to fetch data corresponding to a key */

static FDB_DATUM	errdatum = {
	NULL, 0 
} ;

int fdb_fetch(hhp,cp,key,valuep)
FDB_HEAD	*hhp ;
FDB_CUR	*cp ;
FDB_DATUM	key ;
FDB_DATUM	*valuep ;
{
	FDB_ENT	*hp ;
	FDB_ENT	**nextp ;

	FDB_CUR	c ;

	register int	j ;


	if (cp == NULL) {

	    cp = &c ;
	    cp->j = -1 ;

	}

	if (key.buf == NULL) return BAD ;

#ifdef	COMMENT
	if (valuep == NULL) return BAD ;
#endif

	if (key.len < 0) key.len = strlen(key.buf) ;

	if (BADTBL(hhp)) goto bad ;

#if	CF_DEBUG
	debugprintf("hdbfetch: entered OK, key=%W j=%d\n",
	    key.buf,key.len,cp->j) ;
#endif

/* find it if we have it */

	if ((nextp = i_hdbfind(hhp,key)) == NULL) goto bad ;

#if	CF_DEBUG
	debugprintf("hdbfetch: got past find\n") ;
#endif

	hp = *nextp ;
	if (hp == NULL)				/* absent */
	    goto bad ;

#if	CF_DEBUG
	debugprintf("hdbfetch: got past absent\n") ;
#endif

	if (cp->j >= 0) {

#if	CF_DEBUG
	    debugprintf("hdbfetch: subsequent key request, j=%d\n",cp->j) ;
#endif

	    cp->j += 1 ;
	    j = 0 ;
	    while (j < cp->j) {

	        if (hp->same == NULL) goto bad ;

	        hp = hp->same ;
	        j += 1 ;

	    } /* end while */

	    if (valuep != NULL)
	        *valuep = hp->he_value ;

	} else {

#if	CF_DEBUG
	    debugprintf("hdbfetch: initial key request, j=%d\n",cp->j) ;
#endif

	    if (valuep != NULL)
	        *valuep = hp->he_value ;

	    cp->j = 0 ;

	} /* end if */

#if	CF_DEBUG
	debugprintf("hdbfetch: exiting OK, j=%d data=%W\n",
	    cp->j,((valuep->buf != NULL) ? valuep->buf : ""),valuep->len) ;
#endif

	return OK ;

/* bad stuff happened */
bad:

#if	CF_DEBUG
	debugprintf("hdbfetch: exiting BAD\n") ;
#endif

	cp->j = -1 ;
	if (valuep != NULL)
	    *valuep = errdatum ;

	return BAD ;
}
/* end subroutine (hdbfetch) */


/* subroutine to enumerate all entries */

/*
	This subroutine will return all entries in the DB using the
	given cursor to sequence through it all.
*/

int fdb_enum(hhp,cp,keyp,valuep)
FDB_HEAD	*hhp ;
FDB_CUR	*cp ;
FDB_DATUM	*keyp ;
FDB_DATUM	*valuep ;
{
	FDB_ENT	**nextp ;
	FDB_ENT	*hp, *ihp ;
	FDB_ENT	**hepp ;

	FDB_CUR	cur ;

	unsigned	idx ;

	int		hhpsize ;
	register int	i, j ;


	if (cp == NULL) {

	    cp = &cur ;
	    cp->j = -1 ;

	}

#if	CF_DEBUG
	debugprintf("hdbenum: entered, index=%d i=%d j=%d\n",
	    cp->index,cp->i,cp->j) ;
#endif

	if (keyp == NULL) return BAD ;

#ifdef	COMMENT
	if (valuep == NULL) return BAD ;
#endif

	if (BADTBL(hhp)) goto bad ;

#if	CF_DEBUG
	debugprintf("hdbenum: starting in\n") ;
#endif

	hepp = hhp->ht_addr ;
	hhpsize = hhp->ht_size ;
	while (cp->index < hhpsize) {

#if	CF_DEBUG
	    debugprintf("hdbenum: top of loop\n") ;
#endif

	    if ((hp = hepp[cp->index]) != NULL) {

#if	CF_DEBUG
	        debugprintf("hdbenum: have entry at index=%d\n",cp->index) ;
#endif

	        if (cp->i >= 0) {

#if	CF_DEBUG
	            debugprintf("hdbenum: i=%d\n",cp->i) ;
#endif

/* fast forward to the proper place with the I index */

	            for (i = 0 ; i < cp->i ; i += 1) {

	                if (hp->he_next == NULL) break ;

	                hp = hp->he_next ;

	            } /* end for */

	            if (i >= cp->i) {

	                ihp = hp ;
	                while (TRUE) {

	                    if (cp->j >= 0) {

#if	CF_DEBUG
	                        debugprintf("hdbenum: j=%d\n",cp->j) ;
#endif

/* get the NEXT J entry */

	                        cp->j += 1 ;
	                        for (j = 0 ; j < cp->j ; j += 1) {

	                            if (hp->same == NULL) break ;

	                            hp = hp->same ;

	                        } /* end for */

#if	CF_DEBUG
	                        debugprintf("hdbenum: fast forwarded on J, j=%d\n",
	                            j) ;
#endif

	                        if (j >= cp->j) {

	                            *keyp = hp->he_key ;
	                            if (valuep != NULL)
	                                *valuep = hp->he_value ;

#if	CF_DEBUG
	                            debugprintf("hdbenum: exiting 1 OK\n") ;
#endif

	                            return OK ;
	                        }

	                        cp->i += 1 ;
	                        cp->j = -1 ;
	                        hp = ihp->he_next ;

#if	CF_DEBUG
	                        debugprintf("hdbenum: backing out to I \n") ;
#endif

	                        if (hp == NULL) {

	                            cp->i = -1 ;
	                            break ;
	                        }

	                    } else {

	                        cp->j = 0 ;
	                        *keyp = hp->he_key ;
	                        if (valuep != NULL)
	                            *valuep = hp->he_value ;

#if	CF_DEBUG
	                        debugprintf("hdbenum: exiting 2 OK\n") ;
#endif

	                        return OK ;

	                    }

#if	CF_DEBUG
	                    debugprintf("hdbenum: bottom inner loop\n") ;
#endif

	                } /* end while (not got one) */

#if	CF_DEBUG
	                debugprintf("hdbenum: out of inner loop, i=%d j=%d\n",
	                    cp->i,cp->j) ;
#endif

	            } else
	                cp->i = -1 ;

	        } else {

	            cp->i = cp->j = 0 ;
	            *keyp = hp->he_key ;
	            if (valuep != NULL)
	                *valuep = hp->he_value ;

#if	CF_DEBUG
	            debugprintf("hdbenum: exiting 3 OK, index=%d i=%d j=%d\n",
	                cp->index,cp->i,cp->j) ;
#endif

	            return OK ;

	        } /* end if */

	    } else
	        cp->i = cp->j = -1 ;

	    cp->index += 1 ;

#if	CF_DEBUG
	    debugprintf("hdbenum: bottom outer loop\n") ;
#endif

	} /* end while */

#if	CF_DEBUG
	debugprintf("hdbenum: exiting BAD\n") ;
#endif

	return BAD ;

bad:
	if (valuep != NULL)
	    *valuep = errdatum ;

	return BAD ;
}
/* end subroutine (hdbenum) */


/* walk the database similar to 'ftw(3)' one FS directories */

/*
	Visit each entry by calling 'nodefunc()' with 'key', 'data' and
	'argp' as arguments.
*/

int fdb_walk(hhp,nodefunc,argp)
FDB_HEAD	*hhp ;
int		(*nodefunc)() ;
void		*argp ;
{
	FDB_ENT	*hp, *nhp ;
	FDB_ENT	**hepp ;

	unsigned	idx ;
	unsigned	hhpsize ;

	int		rs ;


	if (BADTBL(hhp))
	    return BAD ;

	hepp = hhp->ht_addr ;
	hhpsize = hhp->ht_size ;
	for (idx = 0 ; idx < hhpsize ; idx += 1) {

	    for (hp = hepp[idx] ; hp != NULL; hp = hp->he_next) {

	        rs = (*nodefunc)(hp->he_key,hp->he_value,argp) ;

	        if (rs < 0) return rs ;

	        nhp = hp ;
	        while (nhp->same != NULL) {

	            nhp = nhp->same ;
	            rs = (*nodefunc)(nhp->he_key,nhp->he_value,argp) ;

	            if (rs < 0) return rs ;

	        } /* end while */

	    } /* end for (inner) */

	} /* end for (outer) */

	return OK ;
}
/* end subroutine (hdbwalk) */


/* count 'em up */
int fdb_count(hdbp)
FDB_HEAD	*hdbp ;
{


	if (hdbp == NULL) return BAD ;

	if (hdbp->magic != FDB_FDB_MAGIC) return BAD ;

	return hdbp->count ;
}
/* end subroutine (hdbcount) */




/* subroutine to NULL out a cursor */
int fdb_nullcursor(dbp,cp)
FDB		*dbp ;
FDB_CUR	*cp ;
{


	if (cp == NULL) return BAD ;

	cp->i = cp->j = -1 ;
	cp->index = 0 ;
	return OK ;
}
/* end subroutine (hdbnullcursor) */

#endif /* CONTINUE */



/* INTERNAL SUBROUTINES */



/* read a number at offet */
static int readno(fd,bp,size,off)
int	fd ;
void	*bp ;
int	size ;
offset_t	off ;
{
	int	*ip = (int *) bp ;
	int	buf[20] ;
	int	rs, i ;


	if ((rs = u_pread(fd,buf,size,off)) >= 0) {

		for (i = 0 ; i < (size/sizeof(int)) ; i += 1)
			*ip++ = ntohl(buf[i]) ;

	}

	return rs ;
}
/* end subroutine (readno) */


/* write a number at offset */
static int writeno(fd,bp,size,off)
int	fd ;
void	*bp ;
int	size ;
offset_t	off ;
{
	int	*ip = (int *) bp ;
	int	buf[20] ;
	int	i ;


		for (i = 0 ; i < (size/sizeof(int)) ; i += 1)
			buf[i] = htonl(ip[i]) ;

	return u_pwrite(fd,buf,size,off) ;
}
/* end subroutine (writeno) */


/* load a number from memory */
static int loadno(dst,src,size)
void	*dst, *src ;
int	size ;
{
	int	i ;
	int	*s = (int *) src ;
	int	*d = (int *) dst ;


		for (i = 0 ; i < (size/sizeof(int)) ; i += 1)
			*d++ = ntohl(s[i]) ;

	return size ;
}
/* end subroutine (loadno) */


/* store a number in memory */
static int storeno(dst,src,size)
void	*dst, *src ;
int	size ;
{
	int	i ;
	int	*s = (int *) src ;
	int	*d = (int *) dst ;


		for (i = 0 ; i < (size/sizeof(int)) ; i += 1)
			d[i] = htonl(s[i]) ;

	return size ;
}
/* end subroutine (storeno) */




#ifdef	ICONTINUE

/* see if an entry is in the database */

/*
	The returned value is the address of the pointer that refers to
	the found object.  The pointer may be NULL if the object was
	not found ; if so, this pointer should be updated with the
	address of the object to be inserted, if insertion is desired.

*/

static FDB_ENT **i_hdbfind(hhp, key)
FDB_HEAD	*hhp ;
FDB_DATUM	key ;
{
	register FDB_ENT	*hp ;
	FDB_ENT		*prevhp = NULL ;
	FDB_ENT		**hepp ;

	unsigned	size ;

	int		i, keylen = key.len ;

	char		*hpkeydat, *keydat = key.buf ;


#if	CF_DEBUG
	debugprintf("i_hdbfind: entered, key=%W\n",key.buf,key.len) ;
#endif

	if (BADTBL(hhp))
	    return NULL ;

#if	CF_DEBUG
	debugprintf("i_hdbfind: table OK\n") ;
#endif

	size = hhp->ht_size ;
	if (size == 0)			/* paranoia: avoid division by zero */
	    size = 1 ;

#if	CF_DEBUG
	debugprintf("i_hdbfind: size check OK\n") ;
#endif

	i = abs((*hhp->ht_hash)(key) % size) ;

#if	CF_DEBUG
	debugprintf("i_hdbfind: hash function hash_value=%d\n",i) ;
#endif

	hepp = hhp->ht_addr + i ;
	for (hp = *hepp ; hp != NULL ; (prevhp = hp, hp = hp->he_next)) {

#if	CF_DEBUG
	    debugprintf("i_hdbfind: top of loop\n") ;
#endif

	    if ((hpkeydat = hp->he_key.buf) == NULL) continue ;

	    if ((hp->he_key.len == keylen) && 
	        (hpkeydat[0] == keydat[0]) &&
	        (memcmp(hpkeydat, keydat, keylen) == 0))
	        break ;

#if	CF_DEBUG
	    debugprintf("i_hdbfind: bottom of loop\n") ;
#endif

	} /* end for */

/* assert: *(returned value) == hp */

#if	CF_DEBUG
	    debugprintf("i_hdbfind: exiting\n") ;
#endif

	return ((prevhp == NULL) ? hepp : &prevhp->he_next) ;
}
/* end subroutine (i_hdbfind) */


/* not yet taken modulus table size */
static unsigned	i_hdbdefhash(key)
FDB_DATUM	key ;
{
	register unsigned	len = key.len ;
	register unsigned	h = 0, g ;

	register char		*s = key.buf ;


#if	CF_DEBUG
	debugprintf("i_hdbdefhash: entered key=%W len=%d\n",
	    key.buf,key.len,key.len) ;
#endif

#if	CF_ELFHASH
	while (len-- > 0) {

	    h <<= 4 ;
	    h += *s++ ;
	    if ((g = (h & 0xF0000000)) != 0) {

	        h ^= (g >> 24) ;
	        h ^= g ;

	    }

	} /* end while */
#else
	while (len-- > 0) {

	    h = h << 1 ;
	    h = (h + *s++) ;

	} /* end while */
#endif /* CF_ELFHASH */

#if	CF_DEBUG
	debugprintf("i_hdbdefhash: exiting hash_value=%d\n",h) ;
#endif

	return h ;
}
/* end subroutine (i_hdbdefhash) */


#endif /* ICONTINUE */



