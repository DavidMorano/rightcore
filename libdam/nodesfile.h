/* nodesfile */

/* UNIX "nodes" file support */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This little object supports "nodes" file management.


*******************************************************************************/


#ifndef	NODESFILE_INCLUDE
#define	NODESFILE_INCLUDE		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/mman.h>
#include	<limits.h>

#include	<vsystem.h>		/* for 'uino_t' */
#include	<hdb.h>
#include	<localmisc.h>


/* local defines */

#define	NODESFILE	struct nodesfile_head
#define	NODESFILE_CUR	struct nodesfile_c


struct nodesfile_c {
	HDB_CUR		cur ;
} ;

struct nodesfile_finfo {
	const char	*fname ;
	time_t		mtime ;
	uino_t		ino ;
	dev_t		dev ;
	int		oflags ;
} ;

struct nodesfile_head {
	char		*mapbuf ;
	struct nodesfile_finfo	fi ;
	HDB		nodes ;
	time_t		ti_check ;
	time_t		ti_load ;
	uint		pagesize ;
	uint		mapsize ;
	uint		filesize ;
	uint		maxsize ;
	uint		len ;
} ;


#if	(! defined(NODESFILE_MASTER)) || (NODESFILE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	nodesfile_open(NODESFILE *,const char *,int,int) ;
extern int	nodesfile_search(NODESFILE *,const char *,int) ;
extern int	nodesfile_curbegin(NODESFILE *,NODESFILE_CUR *) ;
extern int	nodesfile_curend(NODESFILE *,NODESFILE_CUR *) ;
extern int	nodesfile_enum(NODESFILE *,NODESFILE_CUR *,char *,int) ;
extern int	nodesfile_check(NODESFILE *,time_t) ;
extern int	nodesfile_close(NODESFILE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* NODESFILE_MASTER */

#endif /* NODESFILE_INCLUDE */


