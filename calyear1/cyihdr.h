/* cyihdr */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CYIHDR_INCLUDE
#define	CYIHDR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	CYIHDR			struct cyihdr

#define	CYIHDR_MAGICSTR		"CALENDARINDEX"
#define	CYIHDR_MAGICLEN		sizeof(CYIHDR_MAGICSTR)
#define	CYIHDR_MAGICSIZE	16
#define	CYIHDR_VERSION		0


struct cyihdr {
	uint		fsize ;
	uint		wtime ;
	uint		diroff ;
	uint		caloff ;
	uint		vioff ;
	uint		vilen ;
	uint		vloff ;
	uint		vllen ;
	uint		nentries ;
	uint		nskip ;
	uint		year ;		/* the year index was made for */
	uchar		vetu[4] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int cyihdr(CYIHDR *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* CYIHDR_INCLUDE */


