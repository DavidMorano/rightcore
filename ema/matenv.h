/* matenv */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MATENV_INCLUDE
#define	MATENV_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>


#ifndef	UINT
#define	UINT	unsigned int
#endif


/* object defines */

#define	MATENV		struct matenv_head


struct matenv_flags {
	UINT	start : 1 ;	/* was it a starting envelope? (most are) */
} ;

struct matenv_elem {
	const char	*p ;
	int		len ;
} ;

struct matenv_head {
	struct matenv_elem	address ;
	struct matenv_elem	date ;
	struct matenv_elem	remote ;
	struct matenv_flags	f ;
} ;


#if	(! defined(MATENV_MASTER)) || (MATENV_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int matenv(MATENV *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* MATENV_MASTER */

#endif /* MATENV_INCLUDE */


