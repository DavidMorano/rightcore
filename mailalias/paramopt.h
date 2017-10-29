/* PARAMOPT */


/* revision history:

	= 1998-02-01, David A­D­ Morano

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
	struct paramopt_value	*next ;
	const char		*value ;
} ;

struct paramopt_name {
	const char		*name ;		/* key */
	struct paramopt_name	*next ;
	struct paramopt_value 	*head ;
	struct paramopt_value	*tail ;
	struct paramopt_value	*current ;	/* used for interation only */
	int			c ;		/* count of values */
} ;

struct paramopt_head {
	uint			magic ;
	struct paramopt_name	*head ;
	struct paramopt_name	*tail ;
	int			f_inited ;
} ;

struct paramopt_cur {
	struct paramopt_name	*keyp ;
	struct paramopt_value	*valuep ;
} ;


#if	(! defined(PARAMOPT_MASTER)) || (PARAMOPT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int paramopt_start(PARAMOPT *) ;
extern int paramopt_loadu(PARAMOPT *,const char *,int) ;
extern int paramopt_loads(PARAMOPT *,const char *,const char *,int) ;
extern int paramopt_load(PARAMOPT *,const char *,const char *,int) ;
extern int paramopt_havekey(PARAMOPT *,const char *) ;
extern int paramopt_haveval(PARAMOPT *,const char *,const char *,int) ;
extern int paramopt_countvals(PARAMOPT *,const char *) ;
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


