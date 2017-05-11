/* outfmt */
/* output formats */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	OUTFMT_INCLUDE
#define	OUTFMT_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>


extern const char	*outfmts[] ;


enum ofis {
	outfmt_raw,
	outfmt_fill,
	outfmt_bible,
	outfmt_overlast
} ;


#endif /* OUTFMT_INCLUDE */


