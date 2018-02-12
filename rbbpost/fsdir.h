/* fsdir */

/* UNIX® file system dirextory operations */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	FSDIR_INCLUDE
#define	FSDIR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<localmisc.h>


/* object defines */

#define	FSDIR_MAGIC	0x31415926
#define	FSDIR		struct fsdir_head
#define	FSDIR_SLOT	struct fsdir_e
#define	FSDIR_ENT	struct fsdir_e
#define	FSDIR_FL	struct fsdir_flags

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif


struct fsdir_e {
	uino_t		ino ;		/* UNIX® "inode number" of entry */
	offset_t	off ;		/* offset of disk directory entry */
	ushort		reclen ;	/* length of this record */
	char		name[MAXNAMELEN+1] ;	/* name of file */
} ;

struct fsdir_flags {
	uint		descname:1 ;	/* name was really a descriptor */
} ;

struct fsdir_head {
	uint		magic ;
	FSDIR_FL	f ;
	char		*bdata ;	/* buffer pointer (fixed) */
	int		bsize ;		/* buffer size (fixed) */
	int		blen ;		/* buffer length read */
	int		doff ;		/* we do not handle "huge" dirs! */
	int		eoff ;		/* entry offset with directory */
	int		dfd ;		/* file-descriptor */
	int		ei ;		/* entry index (into buffer) */
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


