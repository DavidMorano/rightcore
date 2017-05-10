/* deffile */

/* object to handle parameter files */


/* revision history:

	= 1998-02-15, David A­D­ Morano
	This code was started for Levo related work.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DEFFILE_INCLUDE
#define	DEFFILE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* object defines */

#define	DEFFILE_MAGIC	0x12349872
#define	DEFFILE		struct deffile_head
#define	DEFFILE_CUR	struct deffile_c


struct deffile_head {
	uint		magic ;
	const char	*fname ;
	VECSTR		vars ;
	time_t		ti_check ;
	time_t		ti_filemod ;
	int		intcheck ;
} ;

struct deffile_c {
	int		i ;
} ;


#if	(! defined(DEFFILE_MASTER)) || (DEFFILE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int deffile_open(DEFFILE *,const char *) ;
extern int deffile_curbegin(DEFFILE *,DEFFILE_CUR *) ;
extern int deffile_curend(DEFFILE *,DEFFILE_CUR *) ;
extern int deffile_enum(DEFFILE *,DEFFILE_CUR *,char *,int,const char **) ;
extern int deffile_fetch(DEFFILE *,const char *,const char **) ;
extern int deffile_close(DEFFILE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DEFFILE_MASTER */

#endif /* DEFFILE_INCLUDE */


