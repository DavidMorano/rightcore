/* bvcitekey */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	BVCITEKEY_INCLUDE
#define	BVCITEKEY_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


#define	BVCITEKEY		struct bvcitekey_head


struct bvcitekey_head {
	uchar		nlines ;
	uchar		b ;
	uchar		c ;
	uchar		v ;
 } ;


#if	(! defined(BVCITEKEY_MASTER)) || (BVCITEKEY_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bvcitekey_set(BVCITEKEY *,uint *) ;
extern int	bvcitekey_get(BVCITEKEY *,uint *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BVCITEKEY_MASTER */

#endif /* BVCITEKEY_INCLUDE */


