/* ttihdr */

/* Termianl-Translate-Index file management */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	TTIHDR_INCLUDE
#define	TTIHDR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	TTIHDR			struct ttihdr

#define	TTIHDR_MAGICSTR		"TERMTRANSINDEX"
#define	TTIHDR_MAGICLEN		sizeof(TTIHDR_MAGICSTR)
#define	TTIHDR_MAGICSIZE	16
#define	TTIHDR_VERSION		0


struct ttihdr {
	uint		fsize ;		/* file-size */
	uint		ctime ;		/* create-time */
	uint		rectab ;	/* record-table */
	uint		reclen ;	/* recotd-table-length */
	uint		ostrtab ;	/* overflow-string-table */
	uint		ostrlen ;	/* overflow-string-table length */
	uchar		vetu[4] ;	/* VETU */
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int ttihdr(TTIHDR *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* TTIHDR_INCLUDE */


