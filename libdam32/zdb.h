/* zdb */

/* time-zone database management */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	ZDB_INCLUDE
#define	ZDB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* object defines */

#define	ZDB_MAGIC	0x26213711
#define	ZDB		struct zdb_e
#define	ZDB_E		struct zdb_e
#define	ZDB_ZNAMESIZE	8


struct zdb_e {
	const char	*name ;
	short		off ;		/* minutes west of GMT */
	short		isdst ;
} ;


#if	(! defined(ZDB_MASTER)) || (ZDB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	zdb_nameoff(ZDB *,const char *,int,int) ;
extern int	zdb_name(ZDB *,const char *,int) ;
extern int	zdb_off(ZDB *,int) ;
extern int	zdb_offisdst(ZDB *,int,int) ;
extern int	zdb_count(ZDB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* ZDB_MASTER */

#endif /* ZDB_INCLUDE */


