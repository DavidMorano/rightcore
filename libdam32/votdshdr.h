/* votdchdr */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VOTDCHDR_INCLUDE
#define	VOTDCHDR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#define	VOTDCHDR		struct votdchdr
#define	VOTDCHDR_MAGICSTR	"VOTDC"
#define	VOTDCHDR_MAGICLEN	sizeof(VOTDCHDR_MAGICSTR)
#define	VOTDCHDR_MAGICSIZE	16
#define	VOTDCHDR_VERSION	0
#define	VOTDCHDR_IDLEN		20	/* front matter */


enum votdchdrhs {
	votdchdrh_shmsize,
	votdchdrh_wtime,
	votdchdrh_atime,
	votdchdrh_wcount,		/* write count */
	votdchdrh_acount,		/* access count */
	votdchdrh_muoff,		/* MUTEX offset */
	votdchdrh_musize,
	votdchdrh_bookoff,		/* book table */
	votdchdrh_booklen,		
	votdchdrh_recoff,		/* verse (record) table */
	votdchdrh_reclen,		
	votdchdrh_balloff,		/* SHM allocator (for books) */
	votdchdrh_ballsize,
	votdchdrh_valloff,		/* SHM allocator (for verses) */
	votdchdrh_vallsize,
	votdchdrh_bstroff,		/* book-name string table */
	votdchdrh_bstrlen,
	votdchdrh_vstroff,		/* verse string table */
	votdchdrh_vstrlen,
	votdchdrh_overlast
} ;

struct votdchdr {
	uint		shmsize ;
	uint		wtime ;
	uint		atime ;
	uint		wcount ;	/* write count */
	uint		acount ;	/* access count */
	uint		muoff ;		/* MUTEX offset */
	uint		musize ;
	uint		bookoff ;	/* book table */
	uint		booklen ;
	uint		recoff ;	/* record (verse) table */
	uint		reclen ;
	uint		balloff ;	/* book SHM allocator object */
	uint		ballsize ;
	uint		valloff ;	/* verse SHM allocator object */
	uint		vallsize ;
	uint		bstroff ;	/* book-name string table (aligned) */
	uint		bstrlen ;
	uint		vstroff ;	/* verse string table (aligned) */
	uint		vstrlen ;
	uchar		vetu[4] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int votdchdr(VOTDCHDR *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* VOTDCHDR_INCLUDE */


