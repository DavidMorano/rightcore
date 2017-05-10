/* vars */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VARS_INCLUDE
#define	VARS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>

#include	"varhdr.h"		/* this is the hash-file-header */


#define	VARS_MAGIC	0x88773421
#define	VARS		struct vars_head
#define	VARS_INFO	struct vars_i

#define	VARS_OBJ	struct vars_obj
#define	VARS_CUR	struct vars_c
#define	VARS_FM		struct vars_fm
#define	VARS_MI		struct vars_mi


struct vars_i {
	time_t		wtime ;
	time_t		mtime ;
	uint		nvars ;
	uint		nskip ;
} ;

/* this is the shared-object description */
struct vars_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct vars_c {
	uint		chash ;		/* "check" hash for key */
	int		i ;
} ;

struct vars_fm {
	char		*mdata ;	/* file map */
	time_t		ti_mod ;
	time_t		ti_map ;
	size_t		msize ;
} ;

struct vars_mi {
	int		(*rt)[2] ;	/* mapped record table */
	int		(*it)[3] ;	/* mapped key-index table */
	char		*kst ;		/* mapped key-string table */
	char		*vst ;		/* mapped value-string table */
} ;

struct vars_head {
	uint		magic ;
	const char 	*dbname ;
	VARS_FM		vf ;
	VARS_MI		mi ;
	VARHDR		ifi ;		/* index-file (header) information */
	time_t		ti_lastcheck ;	/* time last check of file */
	int		ncursors ;
} ;


#if	(! defined(VARS_MASTER)) || (VARS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	vars_open(VARS *,const char *) ;
extern int	vars_count(VARS *) ;
extern int	vars_curbegin(VARS *,VARS_CUR *) ;
extern int	vars_fetch(VARS *,const char *,int,VARS_CUR *,char *,int) ;
extern int	vars_enum(VARS *,VARS_CUR *,char *,int,char *,int) ;
extern int	vars_curend(VARS *,VARS_CUR *) ;
extern int	vars_info(VARS *,VARS_INFO *) ;
extern int	vars_audit(VARS *) ;
extern int	vars_close(VARS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* VARS_MASTER */

#endif /* VARS_INCLUDE */


