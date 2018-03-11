/* varmks */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VARMKS_INCLUDE
#define	VARMKS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<strtab.h>
#include	<localmisc.h>


#define	VARMKS		struct varmks_head
#define	VARMKS_OBJ	struct varmks_obj
#define	VARMKS_RECTAB	struct varmks_rectab
#define	VARMKS_FL	struct varmks_flags
#define	VARMKS_MAGIC	0x88773422
#define	VARMKS_NENTRIES	(2 * 1024)
#define	VARMKS_INTOPEN	(10*60)
#define	VARMKS_INTSTALE	(5*60)


/* this is the shared-object description */
struct varmks_obj {
	char		*name ;
	uint		objsize ;
} ;

struct varmks_flags {
	uint		viopen:1 ;
	uint		abort:1 ;
	uint		ofcreat:1 ;
	uint		ofexcl:1 ;
	uint		inprogress:1 ;
	uint		created:1 ;
} ;

struct varmks_rectab {
	uint		(*rectab)[2] ;
	int		i ;			/* highest index */
	int		n ;			/* extent */
} ;

struct varmks_head {
	uint		magic ;
	const char 	*dbname ;
	const char	*idname ;
	char		*nidxfname ;
	STRTAB		keys, vals ;
	VARMKS_RECTAB	rectab ;
	VARMKS_FL	f ;
	mode_t		om ;
	gid_t		gid ;
	int		nvars ;
	int		nfd ;
} ;


#if	(! defined(VARMKS_MASTER)) || (VARMKS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	varmks_open(VARMKS *,const char *,int,mode_t,int) ;
extern int	varmks_addvar(VARMKS *,const char *,const char *,int) ;
extern int	varmks_abort(VARMKS *) ;
extern int	varmks_chgrp(VARMKS *,gid_t) ;
extern int	varmks_close(VARMKS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VARMKS_MASTER */

#endif /* VARMKS_INCLUDE */


