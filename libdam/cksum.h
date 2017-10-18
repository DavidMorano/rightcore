/* cksum */

/* perform the POSIX 1003.2 CRC checksum algorithm */


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
#define	CKSUM_STATE	struct cksum_stage


struct cksum_flags {
	uint		local:1 ;
} ;

struct cksum_stage {
	uint		len, sum ;
} ;

struct cksum_head {
	CKSUM_FL	f ;
	CKSUM_STATE	local, total ;
} ;


typedef	struct cksum_head	cksum ;


#if	(! defined(CKSUM_MASTER)) || (CKSUM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int cksum_start(CKSUM *) ;
extern int cksum_begin(CKSUM *) ;
extern int cksum_accum(CKSUM *,const void *,int) ;
extern int cksum_end(CKSUM *) ;
extern int cksum_getsum(CKSUM *,uint *) ;
extern int cksum_getsumall(CKSUM *,uint *) ;
extern int cksum_finish(CKSUM *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CKSUM_MASTER */

#endif /* CKSUM_INCLUDE */


