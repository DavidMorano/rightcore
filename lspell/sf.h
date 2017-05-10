/* strlistmks */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	STRLISTMKS_INCLUDE
#define	STRLISTMKS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#define	STRLISTMKS		struct strlistmks_head
#define	STRLISTMKS_OBJ		struct strlistmks_obj
#define	STRLISTMKS_FL		struct strlistmks_flags
#define	STRLISTMKS_IMAP		struct strlistmks_imap
#define	STRLISTMKS_MAGIC	0x88773423
#define	STEFILEMKS_MAXFILESIZE	(10*1024*1024)
#define	STRLISTMKS_NENTRIES	(2 * 1024)


/* this is the shared-object description */
struct strlistmks_obj {
	char		*name ;
	uint		objsize ;
} ;

struct strlistmks_flags {
	uint		viopen:1 ;
	uint		abort:1 ;
	uint		creat:1 ;
	uint		excl:1 ;
	uint		none:1 ;
	uint		inprogress:1 ;
	uint		created:1 ;
} ;

struct strlistmks_imap {
	void		*mdata ;
	size_t		msize ;
} ;

struct strlistmks_head {
	uint		magic ;
	const char 	*dbname ;
	const char	*idname ;
	char		*nfname ;
	void		*recorder ;
	void		*idx ;
	STRLISTMKS_IMAP	imap ;
	STRLISTMKS_FL	f ;
	mode_t		om ;
	gid_t		gid ;
	int		pagesize ;
	int		nstrs ;
	int		nfd ;
} ;


#if	(! defined(STRLISTMKS_MASTER)) || (STRLISTMKS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	strlistmks_open(STRLISTMKS *,const char *,int,mode_t,int) ;
extern int	strlistmks_addfile(STRLISTMKS *,const char *,int) ;
extern int	strlistmks_abort(STRLISTMKS *) ;
extern int	strlistmks_chgrp(STRLISTMKS *,gid_t) ;
extern int	strlistmks_close(STRLISTMKS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* STRLISTMKS_MASTER */

#endif /* STRLISTMKS_INCLUDE */


