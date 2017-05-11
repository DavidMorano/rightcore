/* aiq */

/* asynchronous interrupt queue */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This module was originally written.

	= 1998-07-01, David A­D­ Morano
	This module was enhanced by adding the POSIX thread mutex calls.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	AIQ_INCLUDE
#define	AIQ_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<pthread.h>

#include	<vsystem.h>
#include	<ptm.h>
#include	<localmisc.h>


#define	AIQ		struct aiq_head
#define	AIQ_ENT		struct aiq_ent
#define	AIQ_MAGIC	0x76925634


struct aiq_ent {
	long		next ;
	long		prev ;
} ;

struct aiq_head {
	long		head ;
	long		tail ;
	PTM		lock ;
	int		magic ;
	int		count ;
} ;


#if	(! defined(AIQ_MASTER)) || (AIQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int aiq_start(AIQ *,int) ;
extern int aiq_finish(AIQ *) ;
extern int aiq_ins(AIQ *,AIQ_ENT *) ;
extern int aiq_rem(AIQ *,AIQ_ENT **) ;
extern int aiq_count(AIQ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(AIQ_MASTER)) || (AIQ_MASTER == 0) */


#endif /* AIQ_INCLUDE */



