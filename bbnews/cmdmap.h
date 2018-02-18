/* cmdmap */

/* manage the map of keys to commands */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	CMDMAP_INCLUDE
#define	CMDMAP_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<localmisc.h>


#define	CMDMAP		struct cmdmap_head
#define	CMDMAP_E	struct cmdmap_e
#define	CMDMAP_FL	struct cmdmap_flags
#define	CMDMAP_MAGIC	0x24182139


struct cmdmap_e {
	int		key ;
	int		cmd ;
} ;

struct cmdmap_flags {
	uint		sorted:1 ;
} ;

struct cmdmap_head {
	uint		magic ;
	vecobj		map ;
	CMDMAP_FL	f ;
} ;


#if	(! defined(CMDMAP_MASTER)) || (CMDMAP_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int cmdmap_start(CMDMAP *,const CMDMAP_E *) ;
extern int cmdmap_finish(CMDMAP *) ;
extern int cmdmap_load(CMDMAP *,int,int) ;
extern int cmdmap_lookup(CMDMAP *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* CMDMAP_MASTER */

#endif /* CMDMAP_INCLUDE */



