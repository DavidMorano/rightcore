/* bvihdr */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	BVIHDR_INCLUDE
#define	BVIHDR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	BVIHDR			struct bvihdr

#define	BVIHDR_MAGICSTR		"BIBLEVERSEINDEX"
#define	BVIHDR_MAGICLEN		sizeof(BVIHDR_MAGICSTR)
#define	BVIHDR_MAGICSIZE		16
#define	BVIHDR_VERSION		0


struct bvihdr {
	uint		fsize ;
	uint		wtime ;
	uint		vioff ;
	uint		vilen ;
	uint		vloff ;
	uint		vllen ;
	uint		nverses ;
	uint		nzverses ;
	uint		maxbook ;
	uint		maxchapter ;
	uchar		vetu[4] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int bvihdr(BVIHDR *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* BVIHDR_INCLUDE */


