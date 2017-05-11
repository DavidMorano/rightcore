/* fsdirtree */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	FSDIRTREE_INCLUDE
#define	FSDIRTREE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>

#include	<vsystem.h>
#include	<fsdir.h>
#include	<fifostr.h>
#include	<hdb.h>
#include	<localmisc.h>


#define	FSDIRTREE		struct fsdirtree_head
#define	FSDIRTREE_FL		struct fsdirtree_flags
#define	FSDIRTREE_STAT		struct ustat
#define	FSDIRTREE_MAGIC		0x98653217

#define	FSDIRTREESTAT		FSDIRTREE_STAT

/* options */
#define	FSDIRTREE_MFOLLOW	(1<<0)	/* follow symbolic links */
#define	FSDIRTREE_MLINK		(1<<1)
#define	FSDIRTREE_MREG		(1<<2)
#define	FSDIRTREE_MBLOCK	(1<<3)
#define	FSDIRTREE_MCHAR		(1<<4)
#define	FSDIRTREE_MPIPE		(1<<5)
#define	FSDIRTREE_MSOCK		(1<<6)
#define	FSDIRTREE_MDIR		(1<<7)
#define	FSDIRTREE_MNOENT	(1<<8)
#define	FSDIRTREE_MUNIQ		(1<<9)	/* unique directories */
#define	FSDIRTREE_MUNIQDIR	FSDIRTREE_MUNIQ


struct fsdirtree_flags {
	uint		eof:1 ;
	uint		dir:1 ;
	uint		dirids:1 ;		/* dir-id tracking */
} ;

struct fsdirtree_head {
	uint		magic ;
	FIFOSTR		dirq ;
	FSDIR		dir ;
	HDB		dirids ;
	cchar		**prune ;
	int		opts ;
	int		bdnlen ;
	int		cdnlen ;
	FSDIRTREE_FL	f ;
	char		basedname[MAXPATHLEN+1] ;
} ;


typedef struct fsdirtree_head	fsdirtree ;


#if	(! defined(FSDIRTREE_MASTER)) || (FSDIRTREE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	fsdirtree_open(FSDIRTREE *,cchar *,int) ;
extern int	fsdirtree_prune(FSDIRTREE *,cchar **) ;
extern int	fsdirtree_read(FSDIRTREE *,FSDIRTREE_STAT *,char *,int) ;
extern int	fsdirtree_close(FSDIRTREE *) ;

extern int	fsdirtreestat(const char *,int,FSDIRTREESTAT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* FSDIRTREE_MASTER */

#endif /* FSDIRTREE_INCLUDE */


