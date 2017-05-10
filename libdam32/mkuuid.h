/* mkuuid */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MKUUID_INCLUDE
#define	MKUUID_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#define	MKUUID			struct mkuuid


struct mkuuid {
	uint64_t	time ;		/* 60-bits */
	uint64_t	node ;		/* 48-bits */
	uint16_t	clk ;		/* 14 (or 13¹) bits */
	uint		version:4 ;	/* 4-bits */
} ;

/* Note ¹: Micro$oft used 13 bits in the past (we always use 14 bits) */

#if	(! defined(MKUUID_MASTER)) || (MKUUID_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mkuuid(MKUUID *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* MKUUID_MASTER */

#endif /* MKUUID_INCLUDE */


