/* biblebooks */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written for hardware CAD support.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	BIBLEBOOKS_INCLUDE
#define	BIBLEBOOKS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<vecpstr.h>
#include	<localmisc.h>


#define	BIBLEBOOKS_MAGIC	0x99447243
#define	BIBLEBOOKS		struct biblebooks_head
#define	BIBLEBOOKS_OBJ		struct biblebooks_obj


/* this is the shared-object descritoption */
struct biblebooks_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct biblebooks_head {
	uint		magic ;
	vecpstr		db ;
} ;


#if	(! defined(BIBLEBOOKS_MASTER)) || (BIBLEBOOKS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	biblebooks_open(BIBLEBOOKS *,const char *,const char *) ;
extern int	biblebooks_count(BIBLEBOOKS *) ;
extern int	biblebooks_max(BIBLEBOOKS *) ;
extern int	biblebooks_lookup(BIBLEBOOKS *,char *,int,int) ;
extern int	biblebooks_get(BIBLEBOOKS *,int,char *,int) ;
extern int	biblebooks_size(BIBLEBOOKS *) ;
extern int	biblebooks_audit(BIBLEBOOKS *) ;
extern int	biblebooks_close(BIBLEBOOKS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BIBLEBOOKS_MASTER */

#endif /* BIBLEBOOKS_INCLUDE */


