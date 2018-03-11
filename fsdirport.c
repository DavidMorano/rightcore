/* fsdir */

/* subroutines to read directory entries in the UNIX® file system */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_VOIDSEEKDIR	1		/* set if 'seekdir' returns 'void' */
#define	CF_SAFESLEEP	1		/* use 'uc_safesleep(3dam)' */


/* revision history:

	= 1998-06-16, David A­D­ Morano
        This subroutine was written so that we could use a single file-system
        directory interface due to all of the changes in the POSIX® standard and
        previous UNIX® standards.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This code module provides a platform independent implementation
	of UNIX® file system directory access subroutines.

	Oh!  Did I mention it?  What a f___ing pain the combination
	of different memory models and different OSes is!!!!!!!!!!!!

	Oh, and one final thing.  This whole API to read file-system
	directories sucks!  In fact, it sucks cock meat!  Deal with it!

	Oh!  One more thing.  Solaris UNIX® sucks!  It sucks cock meat.
	Solaris appears to have a bug in it where it writes a large
	amount of garbage data to the 'struct dirent' structure entry
	that is supposed to hold the directory file name.  It writes
	garbage to that entry longer than both MAXNAMELEN and MAXPATHLEN.
	It seems to require a character array of at least DIRBUF bytes
	in length (1048 in this case) -- defined in the stupid Solaris
	'/usr/include/dirent.h' include-file -- in order to prevent
	the OS 'readdir_r(3c)' implementation from overflowing that
	character array.  This is likely just a coincidence and not
	an actual intended behavior.  But an array size larger than 1024
	(or generally larger than MAXNAMELEN) is required nonetheless.

	The problem appears to be especially prevalent when reading
	directories that are actually indirect mount points (abstractions)
	maintained by the Solaris 'automountd(1m)' system server.
	It may be that this whole bug is just a bug of some sort with the
	AUTOMOUNT server, but I don't know for sure where it originates
	from.  In any event, the problem seems to be unique to Solaris
	(otherwise known as SlowLaris) -- the garbage operating system
	that it is.


*******************************************************************************/


#define	FSDIR_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<dirent.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"fsdir.h"


/* local defines */

#define	TO_AGAIN	10
#define	TO_NOLCK	20

#ifdef	DIRBUF
#define	DIRENTNAMELEN	MAX(FSDIR_MAXNAMELEN,DIRBUF)
#else
#define	DIRENTNAMELEN	MAX(FSDIR_MAXNAMELEN,1280)
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local typedefs */

#if	defined(_LARGEFILE64_SOURCE)
typedef struct dirent64		locdirent ;
#else
typedef struct dirent		locdirent ;
#endif


/* local structures */

#if	defined(_LARGEFILE64_SOURCE)
struct direntbuf {
	struct dirent64		de ;
	char			dummy[DIRENTNAMELEN + 1] ;
} ;
#else /* defined(_LARGEFILE64_SOURCE) */
struct direntbuf {
	struct dirent		de ;
	char			dummy[DIRENTNAMELEN + 1] ;
} ;
#endif /* defined(_LARGEFILE64_SOURCE) */


/* forward referecnces */


/* local variables */


/* exported subroutines */


int fsdir_open(op,dirpath)
fsdir		*op ;
const char	dirpath[] ;
{
	int	rs = SR_OK ;


	if (op == NULL) return SR_FAULT ;
	if (dirpath == NULL) return SR_FAULT ;

	memset(op,0,sizeof(FSDIR)) ;

	if ((op->dirp = opendir(dirpath)) == NULL)
	    rs = (- errno) ;

	if (rs >= 0)
	    op->magic = FSDIR_MAGIC ;

	return rs ;
}
/* end subroutine (fsdir_open) */


/* read a directory entry */
int fsdir_read(op,dirslotp)
fsdir		*op ;
fsdir_ent	*dirslotp ;
{
	locdirent	*rdep ;

	int	rs ;
	int	to_again = 0 ;
	int	to_nolck = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (dirslotp == NULL) return SR_FAULT ;

	if (op->magic != FSDIR_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("fsdir_read: dirslotp=%p\n",dirslotp) ;
	memset(dirslotp,0,sizeof(fsdir_ent)) ;
#endif

/* top of possible loop */
again:

#if	CF_DEBUGS
	debugprintf("fsdir_read: readdir()\n") ;
#endif

	rdep = NULL ;
	errno = 0 ;

#if	defined(SYSHAS_READDIRR) && (SYSHAS_READDIRR > 0)
	{
	    struct direntbuf	de ;

	    locdirent	*dep = (locdirent *) &de ;

#if	defined(_LARGEFILE64_SOURCE)

	    rs = - readdir64_r(op->dirp,dep,&rdep) ;

#else /* defined(_LARGEFILE64_SOURCE) */

	    rs = - readdir_r(op->dirp,dep,&rdep) ;

#endif /* defined(_LARGEFILE64_SOURCE) */

	    if ((rs >= 0) && (rdep == NULL))
	        rs = (- errno) ;

	}
#else /* defined(SYSHAS_READDIRR) && (SYSHAS_READDIRR > 0) */
	{

#if	CF_DEBUGS
	    debugprintf("fsdir_read: POSIX=0\n") ;
#endif

	    rs = SR_OK ;

#if	defined(_LARGEFILE64_SOURCE)

	    rdep = readdir64(op->dirp) ;

#else

	    rdep = readdir(op->dirp) ;

#endif /* defined(_LARGEFILE64_SOURCE) */

	    if (rdep == NULL)
	        rs = (- errno) ;

	}
#endif /* defined(SYSHAS_READDIRR) && (SYSHAS_READDIRR > 0) */

#if	CF_DEBUGS
	debugprintf("fsdir_read: readdir() rs=%d\n",rs) ;
#endif

	if (rs < 0) {
	    switch (rs) {

	    case SR_NOLCK:
	        if (to_nolck++ < TO_NOLCK) goto retry ;
	        break ;

	    case SR_AGAIN:
	        if (to_again++ < TO_AGAIN) goto retry ;
	        break ;

	    case SR_INTR:
	        goto again ;

	    } /* end switch */
	} else if ((rs == 0) && (rdep != NULL)) {

#if	CF_DEBUGS
	    debugprintf("fsdir_read: dirslotp=%p\n",dirslotp) ;
	    memset(dirslotp,0,sizeof(fsdir_ent)) ;
#endif

	    dirslotp->ino = (uino_t) rdep->d_ino ;
	    dirslotp->off = (offset_t) rdep->d_off ;
	    dirslotp->reclen = (ushort) rdep->d_reclen ;
	    rs = sncpy1(dirslotp->name,FSDIR_MAXNAMELEN,rdep->d_name) ;

	} else /* EOF condition? */ {

	    dirslotp->ino = 0 ;
	    dirslotp->off = 0 ;
	    dirslotp->reclen = 0 ;
	    dirslotp->name[0] = '\0' ;
	    rs = 0 ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("fsdir_read: ret rs=%d n=%s\n",rs,dirslotp->name) ;
#endif

	return rs ;

/* come here to retry */
retry:

#if	CF_SAFESLEEP
	uc_safesleep(1) ;
#else
	sleep(1) ;
#endif

	goto again ;
}
/* end subroutine (fsdir_read) */


int fsdir_tell(op,rp)
fsdir		*op ;
long		*rp ;
{
	long		lrs ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (op->magic != FSDIR_MAGIC) return SR_NOTOPEN ;

	errno = 0 ;
	lrs = telldir(op->dirp) ;

/* this is the assumed regular check for an error */

	rs = (lrs >= 0) ? ((int) lrs) : (- errno) ;

/* crazy error processing for stupid safety-check of crapola API */

	if ((rs >= 0) && (errno != 0))
	    rs = (- errno) ;

	*rp = lrs ;
	return rs ;
}
/* end subroutine (fsdir_tell) */


int fsdir_seek(op,o)
fsdir		*op ;
long		o ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != FSDIR_MAGIC) return SR_NOTOPEN ;

#if	CF_VOIDSEEKDIR
	(void) seekdir(op->dirp,o) ;
#else
	if (seekdir(op->dirp,o) < 0) rs = (- errno) ;
#endif

	return rs ;
}
/* end subroutine (fsdir_seek) */


/* rewind the directory */
int fsdir_rewind(op)
fsdir		*op ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != FSDIR_MAGIC) return SR_NOTOPEN ;

#if	CF_VOIDSEEKDIR
	(void) seekdir(op->dirp,0L) ;
#else
	if (seekdir(op->dirp,0L) < 0) rs = (- errno) ;
#endif

	return rs ;
}
/* end subroutine (fsdir_rewind) */


int fsdir_audit(op)
fsdir		*op ;
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != FSDIR_MAGIC) return SR_NOTOPEN ;

	if (op->dirp == NULL) rs = SR_BADFMT ;

	return rs ;
}
/* end subroutine (fsdir_audit) */


int fsdir_close(op)
fsdir		*op ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != FSDIR_MAGIC) return SR_NOTOPEN ;

	if ((rs = closedir(op->dirp)) < 0) rs = (- errno) ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (fsdir_close) */


