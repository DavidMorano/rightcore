/* cyifu */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CYIFU_INCLUDE
#define	CYIFU_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	CYIFU			struct cyifu

#define	CYIFU_MAGICSTR		"CALENDARINDEX"
#define	CYIFU_MAGICLEN		sizeof(CYIFU_MAGICSTR)
#define	CYIFU_MAGICSIZE		16
#define	CYIFU_VERSION		0


struct cyifu {
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

extern int cyifu(CYIFU *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* CYIFU_INCLUDE */


