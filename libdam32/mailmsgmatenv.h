/* mailmsgmatenv */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILMSGMATENV_INCLUDE
#define	MAILMSGMATENV_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>


#ifndef	UINT
#define	UINT	unsigned int
#endif


/* object defines */

#define	MAILMSGMATENV		struct mailmsgmatenv_head
#define	MAILMSGMATENV_FL	struct mailmsgmatenv_flags
#define	MAILMSGMATENV_ELEM	struct mailmsgmatenv_elem


struct mailmsgmatenv_flags {
	UINT		start:1 ;	/* starting envelope? (most are) */
} ;

struct mailmsgmatenv_elem {
	const char	*ep ;
	int		el ;
} ;

struct mailmsgmatenv_head {
	MAILMSGMATENV_ELEM	a ;	/* address (EMA) */
	MAILMSGMATENV_ELEM	d ;	/* date */
	MAILMSGMATENV_ELEM	r ;	/* remove-from (EMA) */
	MAILMSGMATENV_FL	f ;	/* flags */
	int			rt ;	/* remote type */
} ;


#if	(! defined(MAILMSGMATENV_MASTER)) || (MAILMSGMATENV_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsgmatenv(MAILMSGMATENV *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* MAILMSGMATENV_MASTER */

#endif /* MAILMSGMATENV_INCLUDE */


