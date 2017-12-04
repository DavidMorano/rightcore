/* hashelf */

/* perform the hash done in processing ELF files */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.  The idea of this hash is taken
	from the ELF-type object manipulation procedures.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine implements the ELF object file procedure hashing
        function.

	Synopsis:

	unsigned int hashelf(const char *sp,int sl)

	Arguments:

	sp		buffer to be hashed
	sl		len of buffer data to be hashed

	Returns:

	value		the hash value (unsigned)


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<string.h>
#include	<localmisc.h>


/* external subroutines */


/* forward references */


/* exported subroutines */


uint hashelf(const char *sp,int sl)
{
	unsigned	h = 0 ;
	unsigned	g ;
	unsigned	v ;

	if (sl < 0) sl = strlen(sp) ;

	while (sl-- > 0) {

	    v = (uchar) *sp++ ;
	    h <<= 4 ;
	    h += v ;
	    g = (h & 0xF0000000) ;
	    if (g != 0) {
	        h ^= (g >> 24) ;
	        h &= (~ g) ;
	    }

	} /* end while */

	return h ;
}
/* end subroutine (hashelf) */


int elfhash(const char *sp,int sl)
{
	return hashelf(sp,sl) ;
}
/* end subroutine (elfhash) */


