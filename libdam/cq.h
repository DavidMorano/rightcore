/* cq */
/* Container-Queue */

/* container Q */


/* revision history:

	= 1998-07-17, David A­D­ Morano
	Oh what a cheap Q!  I do not know why I am doing this!

	= 2017-11-21, David A­D­ Morano
	Added new method |cq_unlink()|.

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */

#ifndef	CQ_INCLUDE
#define	CQ_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<vechand.h>


#define	CQ_MAGIC	0x65748392
#define	CQ_DEFENTS	10
#define	CQ		struct cq_head
#define	CQ_CUR		struct cq_c


struct cq_head {
	uint		magic ;
	vechand		q ;
} ;

struct cq_c {
	int		i ;
} ;


#if	(! defined(CQ_MASTER)) || (CQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int cq_start(CQ *) ;
extern int cq_finish(CQ *) ;
extern int cq_ins(CQ *,void *) ;
extern int cq_rem(CQ *,void *) ;
extern int cq_unlink(CQ *,void *) ;
extern int cq_count(CQ *) ;
extern int cq_curbegin(CQ *,CQ_CUR *) ;
extern int cq_curend(CQ *,CQ_CUR *) ;
extern int cq_enum(CQ *,CQ_CUR *,void *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CQ_MASTER */

#endif /* CQ_INCLUDE */


