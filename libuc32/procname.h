/* procsearch */


/* revision history:

	= 1998-02-23, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PROCSEARCH_INCLUDE
#define	PROCSEARCH_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<filebuf.h>


#define	PROCSEARCH	struct procsearch_head
#define	PROCSEARCH_INFO	struct procsearch_info
#define	PROCSEARCH_MAGIC	0x00019961


struct procsearch_head {
	uint		magic ;
	FILEBUF		b ;
	int		fd ;
} ;

struct procsearch_info {
	pid_t		pid ;
	uid_t		ruid ;
	uid_t		euid ;
	gid_t		rgid ;
	gid_t		egid ;
} ;


#if	(! defined(PROCSEARCH_MASTER)) || (PROCSEARCH_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	procsearch_open(PROCSEARCH *) ;
extern int	procsearch_read(PROCSEARCH *,PROCSEARCH_INFO,char *,int) ;
extern int	procsearch_close(PROCSEARCH *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PROCSEARCH_MASTER */

#endif /* PROCSEARCH_INCLUDE */


