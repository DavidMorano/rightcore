/* bpihdr */

/* Bible-Paragraph-Index */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	BPIHDR_INCLUDE
#define	BPIHDR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	BPIHDR			struct bpihdr

#define	BPIHDR_MAGICSIZE	16
#define	BPIHDR_MAGICSTR		"BIBLEPARAINDEX"
#define	BPIHDR_MAGICLEN		sizeof(BPIHDR_MAGICSTR)
#define	BPIHDR_VERSION		0


struct bpihdr {
	uint		fsize ;
	uint		wtime ;
	uint		vioff ;
	uint		vilen ;
	uint		nverses ;
	uint		nzverses ;
	uint		maxbook ;
	uint		maxchapter ;
	uchar		vetu[4] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int bpihdr(BPIHDR *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* BPIHDR_INCLUDE */


