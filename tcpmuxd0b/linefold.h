/* linefold */


/* revision history:

	= 2009-04-10, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

#ifndef	LINEFOLD_INCLUDE
#define	LINEFOLD_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<localmisc.h>


/* object defines */
#define	LINEFOLD_MAGIC		0x88431773
#define	LINEFOLD		struct linefold_head
#define	LINEFOLD_FL		struct linefold_flags

/* options */
#define	LINEFOLD_MCARRIAGE	0x0001


struct linefold_flags {
	uint		dummy:1 ;
} ;

struct linefold_head {
	uint		magic ;
	LINEFOLD_FL	f ;
	vecobj		lines ;
} ;


#if	(! defined(LINEFOLD_MASTER)) || (LINEFOLD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int linefold_start(LINEFOLD *,int,int,const char *,int) ;
extern int linefold_get(LINEFOLD *,int,const char **) ;
extern int linefold_getline(LINEFOLD *,int,const char **) ;
extern int linefold_finish(LINEFOLD *) ;

#ifdef	__cplusplus
}
#endif

#endif /* LINEFOLD_MASTER */

#endif /* LINEFOLD_INCLUDE */


