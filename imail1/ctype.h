/* hdrctype */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	HDRCTYPE_INCLUDE
#define	HDRCTYPE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* object defines */

#define	HDRCTYPE		struct hdrctype_head


struct hdrctype_head {
	const char	*mtp ;		/* main-type */
	const char	*stp ;		/* sub-type */
	int		mtl ;		/* main-type */
	int		stl ;		/* sub-type */
} ;


#if	(! defined(HDRCTYPE_MASTER)) || (HDRCTYPE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int hdrctype_decode(HDRCTYPE *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* HDRCTYPE_MASTER */

#endif /* HDRCTYPE_INCLUDE */



