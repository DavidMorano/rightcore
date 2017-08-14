/* ecinfo */


/* revision history:

	= 1998-04-19, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	ECINFO_INCLUDE
#define	ECINFO_INCLUDE	1


#include	<sys/types.h>

#include	<localmisc.h>		/* for 'uint' */


#define	ECINFO			struct ecinfo_data
#define	ECINFO_DATA		struct ecinfo_data
#define	ECINFO_REASONLEN	100


/* general acknowledgement response */
struct ecinfo_data {
	uint	filelen ;
	uint	filetime ;
	uint	filesum ;
	uint	msglen ;
	uint	msgtime ;
	uint	msgsum ;
	uint	tag ;
	uint	type ;			/* message type */
} ;


/* message types */
enum ecinfotypes {
	ecinfotype_data,
	ecinfotype_overlast
} ;

/* response codes */
enum ecinforcs {
	ecinforc_ok,
	ecinforc_invalid,
	ecinforc_overlast
} ;


#if	(! defined(ECINFO_MASTER)) || (ECINFO_MASTER == 0)

extern int	ecinfo_data(char *,int,int,struct ecinfo_data *) ;

#endif /* ECINFO_MASTER */


#endif /* ECINFO_INCLUDE */


