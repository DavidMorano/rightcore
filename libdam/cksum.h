/* cksum */


/* revision history:

	= 1998-03-17, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	CKSUM_INCLUDE
#define	CKSUM_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>


/* object defines */

#define	CKSUM		struct cksum_head
#define	CKSUM_FL	struct cksum_flags


struct cksum_flags {
	uint		started:1 ;
	uint		finished:1 ;
} ;

struct cksum_head {
	CKSUM_FL	f ;
	uint		sum ;
	uint		len ;
	uint		totallen ;
	uint		totalsum ;
} ;


typedef	struct cksum_head	cksum ;


#if	(! defined(CKSUM_MASTER)) || (CKSUM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int cksum_start(CKSUM *) ;
extern int cksum_begin(CKSUM *) ;
extern int cksum_accum(CKSUM *,void *,int) ;
extern int cksum_end(CKSUM *) ;
extern int cksum_getsum(CKSUM *,uint *) ;
extern int cksum_gettotal(CKSUM *,uint *) ;
extern int cksum_finish(CKSUM *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CKSUM_MASTER */

#endif /* CKSUM_INCLUDE */


