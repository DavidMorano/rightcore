/* txtindexfu */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	TXTINDEXFU_INCLUDE
#define	TXTINDEXFU_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	TXTINDEXFU		struct txtindexfu
#define	TXTINDEXFU_MAGICSIZE	16
#define	TXTINDEXFU_MAGICSTR	"MKINVHASH"
#define	TXTINDEXFU_MAGICLEN	sizeof(TXTINDEXFU_MAGICSTR)
#define	TXTINDEXFU_VERSION	0


struct txtindexfu {
	uint		hfsize ;	/* hash-file size */
	uint		tfsize ;	/* tag-file size */
	uint		ersize ;	/* eigen-record table size */
	uint		eisize ;	/* eigen-index table size */
	uint		wtime ;		/* write-time */
	uint		sdnoff ;	/* dirname offset */
	uint		sfnoff ;	/* filename offset */
	uint		listoff ;	/* list offset */
	uint		esoff ;		/* eigen-string table offset */
	uint		essize ;	/* eigen-string table length (size) */
	uint		eroff ;		/* eigen-record table offset */
	uint		erlen ;
	uint		eioff ;		/* eigen-index table offset */
	uint		eilen ;
	uint		eiskip ;	/* eigen-index table skip-factor */
	uint		taboff ;	/* main hash-table offset */
	uint		tablen ;	/* main hash-table length */
	uint		taglen ;	/* total number of tags */
	uint		maxtags ;	/* maximum tags in any list */
	uint		minwlen ;	/* minimum word length */
	uint		maxwlen ;	/* maximum word length */
	uchar		vetu[4] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int txtindexfu(TXTINDEXFU *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* TXTINDEXFU_INCLUDE */


