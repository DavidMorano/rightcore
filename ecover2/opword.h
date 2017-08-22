/* opword */


/* revision history:

	= 1999-06-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	OPWORD_INCLUDE
#define	OPWORD_INCLUDE		1


#include	<sys/types.h>

#include	<localmisc.h>


union opword {
	ULONG		w ;
	uint		h[2] ;
	uchar		c[8] ;
} ;


#endif /* OPWORD_INCLUDE */


