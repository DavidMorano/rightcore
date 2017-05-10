/* commandments */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	COMMANDMENTS_INCLUDE
#define	COMMANDMENTS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecobj.h>
#include	<localmisc.h>


#define	COMMANDMENTS_MAGIC	0x99447244
#define	COMMANDMENTS		struct commandments_head
#define	COMMANDMENTS_OBJ	struct commandments_obj


/* this is the shared-object description */
struct commandments_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct commandments_head {
	uint		magic ;
	const char 	*fname ;
	const char	*mapdata ;
	VECOBJ		db ;
	size_t		filesize ;		/* file size */
	size_t		mapsize ;		/* map length */
	time_t		ti_mod ;		/* file modification */
	time_t		ti_map ;		/* map */
	time_t		ti_lastcheck ;		/* last check of file */
	int		ncursors ;
} ;


#if	(! defined(COMMANDMENTS_MASTER)) || (COMMANDMENTS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	commandments_open(COMMANDMENTS *,const char *,const char *) ;
extern int	commandments_count(COMMANDMENTS *) ;
extern int	commandments_max(COMMANDMENTS *) ;
extern int	commandments_get(COMMANDMENTS *,int,char *,int) ;
extern int	commandments_enum(COMMANDMENTS *,int,int *,char *,int) ;
extern int	commandments_audit(COMMANDMENTS *) ;
extern int	commandments_close(COMMANDMENTS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* COMMANDMENTS_MASTER */

#endif /* COMMANDMENTS_INCLUDE */


