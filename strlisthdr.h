/* strlisthdr */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	STRLISTHDR_INCLUDE
#define	STRLISTHDR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	STRLISTHDR		struct strlisthdr

#define	STRLISTHDR_MAGICSTR	"STRLISTHDR"
#define	STRLISTHDR_FSUF		"si"
#define	STRLISTHDR_MAGICLEN	sizeof(STRLISTHDR_MAGICSTR)
#define	STRLISTHDR_MAGICSIZE	16
#define	STRLISTHDR_VERSION	0


struct strlisthdr {
	uint		fsize ;		/* file size */
	uint		wtime ;		/* write time (creation time?) */
	uint		stoff ;		/* string table offset */
	uint		stlen ;		/* string table length (n-entries) */
	uint		rtoff ;		/* record-table offset */
	uint		rtlen ;		/* record-table length (n-entries) */
	uint		itoff ;		/* index-table offset */
	uint		itlen ;		/* index-table length (n-entries) */
	uint		nstrs ;		/* total number of strings */
	uint		nskip ;		/* a hash-loookup parameter */
	char		vetu[4] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int strlisthdr(STRLISTHDR *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* STRLISTHDR_INCLUDE */


