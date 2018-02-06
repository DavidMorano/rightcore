/* hexdecoder */
/* lang=C99 */

/* HEX decoder */


/* revision history:

	= 2016-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2016 David A­D­ Morano.  All rights reserved. */

#ifndef	HEXDECODER_INCLUDE
#define	HEXDECODER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#define	HEXDECODER_MAGIC	0x13f3c202
#define	HEXDECODER		struct hexdecoder_head


struct hexdecoder_head {
	uint		magic ;
	int		rl ;		/* residue length ('0' or '1') */
	void		*outbuf ;	/* output-buffer */
	char		rb[2] ;		/* residue buffer */
} ;


#if	(! defined(HEXDECODER_MASTER)) || (HEXDECODER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int hexdecoder_start(HEXDECODER *) ;
extern int hexdecoder_load(HEXDECODER *,cchar *,int) ;
extern int hexdecoder_read(HEXDECODER *,char *,int) ;
extern int hexdecoder_finish(HEXDECODER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* HEXDECODER_MASTER */

#endif /* HEXDECODER_INCLUDE */


