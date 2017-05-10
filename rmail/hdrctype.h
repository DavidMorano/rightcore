/* hdrctype */


/* revision history:

	= 1998-02-23, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	HDRCTYPE_INCLUDE
#define	HDRCTYPE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* object defines */

#define	HDRCTYPE		struct hdrctype_head


struct hdrctype_t {
	const char	*tp ;
	int		tl ;
} ;

struct hdrctype_head {
	struct hdrctype_t	main, sub ;
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



