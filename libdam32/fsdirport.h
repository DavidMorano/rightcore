/* fsdir */

/* UNIX® file system dirextory operations */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	FSDIR_INCLUDE
#define	FSDIR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<dirent.h>

#include	<localmisc.h>


/* object defines */

#define	FSDIR_MAGIC		0x31415926
#define	FSDIR			struct fsdir_head
#define	FSDIR_SLOT		struct fsdir_e
#define	FSDIR_ENT		struct fsdir_e

#ifdef	MAXNAMELEN
#ifdef	MAXNAMLEN
#define	FSDIR_MAXNAMELEN	MAX(MAXNAMELEN,MAXNAMLEN)
#else
#define	FSDIR_MAXNAMELEN	MAX(MAXNAMELEN,512)
#endif /* MAXNAMLEN */
#else
#ifdef	MAXNAMLEN
#define	FSDIR_MAXNAMELEN	MAX(MAXNAMLEN,512)
#else
#define	FSDIR_MAXNAMELEN	512
#endif /* MAXNAMLEN */
#endif /* MAXNAMELEN */


struct fsdir_e {
	uino_t		ino ;		/* "inode number" of entry */
	offset_t	off ;		/* offset of disk directory entry */
	ushort		reclen ;	/* length of this record */
	char		name[FSDIR_MAXNAMELEN + 1] ;	/* name of file */
} ;

struct fsdir_head {
	uint		magic ;
	DIR		*dirp ;
} ;


typedef struct fsdir_head	fsdir ;
typedef struct fsdir_e		fsdir_ent ;


#if	(! defined(FSDIR_MASTER)) || (FSDIR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	fsdir_open(fsdir *,const char *) ;
extern int	fsdir_read(fsdir *,fsdir_ent *) ;
extern int	fsdir_tell(fsdir *,long *) ;
extern int	fsdir_seek(fsdir *,long) ;
extern int	fsdir_rewind(fsdir *) ;
extern int	fsdir_audit(fsdir *) ;
extern int	fsdir_close(fsdir *) ;

#ifdef	__cplusplus
}
#endif

#endif /* FSDIR_MASTER */

#endif /* FSDIR_INCLUDE */


