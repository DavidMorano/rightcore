/* charq */

/* character queue */
/* last modified %G% version %I% */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This obejct module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CHARQ_INCLUDE
#define	CHARQ_INCLUDE	1


#define	CHARQ		struct charq_head


struct charq_head {
	char		*buf ;
	int		ri, wi ;
	int		size, count ;
} ;


#if	(! defined(CHARQ_MASTER)) || (CHARQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	charq_start(CHARQ *,int) ;
extern int	charq_ins(CHARQ *,int) ;
extern int	charq_rem(CHARQ *,char *) ;
extern int	charq_size(CHARQ *) ;
extern int	charq_count(CHARQ *) ;
extern int	charq_finish(CHARQ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CHARQ_MASTER */

#endif /* CHARQ_INCLUDE */


