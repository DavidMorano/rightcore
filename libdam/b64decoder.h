/* b64decoder */

/* Base-64 (B64) decoder */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	B64DECODER_INCLUDE
#define	B64DECODER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#define	B64DECODER_MAGIC	0x13f3c204
#define	B64DECODER		struct b64decoder_head


struct b64decoder_head {
	uint		magic ;
	int		rl ;		/* stage length */
	void		*outbuf ;	/* output-buffer */
	char		rb[4+1] ;	/* stage buffer */
} ;


#if	(! defined(B64DECODER_MASTER)) || (B64DECODER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int b64decoder_start(B64DECODER *) ;
extern int b64decoder_load(B64DECODER *,cchar *,int) ;
extern int b64decoder_read(B64DECODER *,char *,int) ;
extern int b64decoder_finish(B64DECODER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* B64DECODER_MASTER */

#endif /* B64DECODER_INCLUDE */


