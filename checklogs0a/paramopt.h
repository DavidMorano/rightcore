/* PARAMOPT */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PARAMOPT_INCLUDE
#define	PARAMOPT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>


/* object defines */

#define	PARAMOPT	struct paramopt_head
#define	PARAMOPT_NAME	struct paramopt_name
#define	PARAMOPT_VALUE	struct paramopt_value
#define	PARAMOPT_CUR	struct paramopt_cur
#define	PARAMOPT_MAGIC	0x87892334


struct paramopt_value {
	PARAMOPT_VALUE	*next ;
	const char	*value ;
} ;

struct paramopt_name {
	const char	*name ;		/* key */
	PARAMOPT_NAME	*next ;
	PARAMOPT_VALUE	*head ;
	PARAMOPT_VALUE	*tail ;
	PARAMOPT_VALUE	*current ;	/* used for interation only */
	int		c ;		/* count of values */
} ;

struct paramopt_head {
	uint		magic ;
	PARAMOPT_NAME	*head ;
	PARAMOPT_NAME	*tail ;
	int		f_inited ;
} ;

struct paramopt_cur {
	PARAMOPT_NAME	*keyp ;
	PARAMOPT_VALUE	*valuep ;
} ;


#if	(! defined(PARAMOPT_MASTER)) || (PARAMOPT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int paramopt_start(PARAMOPT *) ;
extern int paramopt_loadu(PARAMOPT *,cchar *,int) ;
extern int paramopt_loads(PARAMOPT *,cchar *,cchar *,int) ;
extern int paramopt_load(PARAMOPT *,cchar *,cchar *,int) ;
extern int paramopt_loaduniq(PARAMOPT *,cchar *,cchar *,int) ;
extern int paramopt_havekey(PARAMOPT *,cchar *) ;
extern int paramopt_haveval(PARAMOPT *,cchar *,cchar *,int) ;
extern int paramopt_countvals(PARAMOPT *,cchar *) ;
extern int paramopt_curbegin(PARAMOPT *,PARAMOPT_CUR *) ;
extern int paramopt_curend(PARAMOPT *,PARAMOPT_CUR *) ;
extern int paramopt_enumkeys(PARAMOPT *,PARAMOPT_CUR *,cchar **) ;
extern int paramopt_fetch(PARAMOPT *,cchar *,PARAMOPT_CUR *,cchar **) ;
extern int paramopt_enumvalues(PARAMOPT *,cchar *,PARAMOPT_CUR *,cchar **) ;
extern int paramopt_incr(PARAMOPT *) ;
extern int paramopt_finish(PARAMOPT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PARAMOPT_MASTER */

#endif /* PARAMOPT_INCLUDE */


