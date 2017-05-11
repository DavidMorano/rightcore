/* babiesfu */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	BABIESFU_INCLUDE
#define	BABIESFU_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#define	BABIESFU		struct babiesfu
#define	BABIESFU_MAGICSTR	"BABIES"
#define	BABIESFU_MAGICLEN	sizeof(BABIESFU_MAGICSTR)
#define	BABIESFU_MAGICSIZE	16
#define	BABIESFU_VERSION	0
#define	BABIESFU_IDLEN		20


enum babiesfuhs {
	babiesfuh_shmsize,
	babiesfuh_dbsize,
	babiesfuh_dbtime,
	babiesfuh_wtime,
	babiesfuh_atime,
	babiesfuh_acount,		/* access count */
	babiesfuh_muoff,
	babiesfuh_musize,
	babiesfuh_btoff,
	babiesfuh_btlen,
	babiesfuh_overlast
} ;

struct babiesfu {
	uint		shmsize ;
	uint		dbsize ;
	uint		dbtime ;
	uint		wtime ;
	uint		atime ;		/* access time */
	uint		acount ;	/* access count */
	uint		muoff ;
	uint		musize ;
	uint		btoff ;
	uint		btlen ;
	uchar		vetu[4] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int babiesfu(BABIESFU *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* BABIESFU_INCLUDE */


