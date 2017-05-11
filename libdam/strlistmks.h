/* strlistmks */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	STRLISTMKS_INCLUDE
#define	STRLISTMKS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<strtab.h>
#include	<localmisc.h>


#define	STRLISTMKS		struct strlistmks_head
#define	STRLISTMKS_OBJ		struct strlistmks_obj
#define	STRLISTMKS_REC		struct strlistmks_rectab
#define	STRLISTMKS_FL		struct strlistmks_flags
#define	STRLISTMKS_MAGIC	0x88773423
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

struct strlistmks_rectab {
	uint		(*rectab)[1] ;
	int		i ;			/* highest index */
	int		n ;			/* extent */
} ;

struct strlistmks_head {
	uint		magic ;
	const char 	*dbname ;
	const char	*idname ;
	char		*nfname ;
	STRTAB		strs ;
	STRLISTMKS_REC	rectab ;
	STRLISTMKS_FL	f ;
	mode_t		om ;
	gid_t		gid ;
	int		nstrs ;
	int		nfd ;
} ;


#if	(! defined(STRLISTMKS_MASTER)) || (STRLISTMKS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	strlistmks_open(STRLISTMKS *,const char *,int,mode_t,int) ;
extern int	strlistmks_addvar(STRLISTMKS *,const char *,int) ;
extern int	strlistmks_abort(STRLISTMKS *) ;
extern int	strlistmks_chgrp(STRLISTMKS *,gid_t) ;
extern int	strlistmks_close(STRLISTMKS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* STRLISTMKS_MASTER */

#endif /* STRLISTMKS_INCLUDE */


