/* bvshdr (Bible Verse Structure) */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	BVSHDR_INCLUDE
#define	BVSHDR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	BVSHDR			struct bvshdr

#define	BVSHDR_MAGICSIZE	16
#define	BVSHDR_MAGICSTR		"BVS"
#define	BVSHDR_MAGICLEN		sizeof(BVSHDR_MAGICSTR)
#define	BVSHDR_VERSION		0


struct bvshdr {
	uint		fsize ;		/* file-size */
	uint		wtime ;		/* write-time */
	uint		nverses ;	/* total verses */
	uint		nzverses ;	/* non-zero verses */
	uint		nzbooks ;	/* number of non-zero books */
	uint		btoff ;		/* book-table */
	uint		btlen ;
	uint		ctoff ;		/* chapter-table */
	uint		ctlen ;
	uchar		vetu[4] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int bvshdr(BVSHDR *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* BVSHDR_INCLUDE */


