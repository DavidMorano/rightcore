/* pwihdr */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	PWIHDR_INCLUDE
#define	PWIHDR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#define	PWIHDR_MAGICSTR		"IPASSWD"
#define	PWIHDR_MAGICLEN		7
#define	PWIHDR_VERSION		0
#define	PWIHDR_TYPE		0


enum pwihdrs {
	pwihdr_wrtime,
	pwihdr_wrcount,
	pwihdr_rectab,
	pwihdr_reclen,
	pwihdr_recsize,
	pwihdr_strtab,
	pwihdr_strlen,
	pwihdr_strsize,
	pwihdr_idxlen,
	pwihdr_idxsize,
	pwihdr_idxl1,
	pwihdr_idxl3,
	pwihdr_idxf,
	pwihdr_idxfl3,
	pwihdr_idxun,
	pwihdr_overlast
} ;

struct pwihdr {
	uint		wrtime ;
	uint		wrcount ;
	uint		rectab ;
	uint		recsize ;
	uint		reclen ;
	uint		strtab ;
	uint		strsize ;
	uint		idxsize ;
	uint		idxlen ;
	uint		idxl1 ;
	uint		idxl3 ;
	uint		idxf ;
	uint		idxfl3 ;
	uint		idxun ;
} ;


#endif /* PWIHDR_INCLUDE */


