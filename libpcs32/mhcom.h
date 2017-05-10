/* mhcom */

/* comment-parse (for RFC822) small strings (like for stupid RFC822 date) */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MHCOM_INCLUDE
#define	MHCOM_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


/* object defines */

#define	MHCOM_MAGIC	0x98638451
#define	MHCOM		struct mhcom_head


struct mhcom_head {
	uint		magic ;
	int		vlen, clen ;
	char		*a ;
	char		*value ;
	char		*comment ;
} ;


#if	(! defined(MHCOM_MASTER)) || (MHCOM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mhcom_start(MHCOM *,const char *,int) ;
extern int mhcom_getval(MHCOM *,const char **) ;
extern int mhcom_getcom(MHCOM *,const char **) ;
extern int mhcom_finish(MHCOM *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MHCOM_MASTER */

#endif /* MHCOM_INCLUDE */


