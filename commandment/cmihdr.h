/* cmihdr */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CMIHDR_INCLUDE
#define	CMIHDR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#define	CMIHDR			struct cmihdr

#define	CMIHDR_MAGICSTR		"CMDINDEX"
#define	CMIHDR_MAGICLEN		sizeof(CMIHDR_MAGICSTR)
#define	CMIHDR_MAGICSIZE	16
#define	CMIHDR_VERSION		0


struct cmihdr {
	uint		dbsize ;	/* DB-file size */
	uint		dbtime ;	/* DB modification-time */
	uint		idxsize ;	/* IDX-file size */
	uint		idxtime ;	/* IDX creation-time */
	uint		vioff ;
	uint		vilen ;
	uint		vloff ;
	uint		vllen ;
	uint		nents ;
	uint		maxent ;
	uchar		vetu[4] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int cmihdr(CMIHDR *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* CMIHDR_INCLUDE */


