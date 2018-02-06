/* qpdecoder */

/* Quoted-Printable (QP) decoder */


/* revision history:

	= 2016-06-29, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2016 David A­D­ Morano.  All rights reserved. */

#ifndef	QPDECODER_INCLUDE
#define	QPDECODER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#define	QPDECODER_MAGIC		0x13f3c205
#define	QPDECODER		struct qpdecoder_head
#define	QPDECODER_FL		struct qpdecoder_flags


struct qpdecoder_flags {
	uint		esc:1 ;
	uint		space:1 ;
} ;

struct qpdecoder_head {
	uint		magic ;
	int		rl ;		/* stage length */
	QPDECODER_FL	f ;
	void		*outbuf ;	/* output-buffer */
	char		rb[4+1] ;	/* stage buffer */
} ;


#if	(! defined(QPDECODER_MASTER)) || (QPDECODER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int qpdecoder_start(QPDECODER *,int) ;
extern int qpdecoder_load(QPDECODER *,cchar *,int) ;
extern int qpdecoder_read(QPDECODER *,char *,int) ;
extern int qpdecoder_finish(QPDECODER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* QPDECODER_MASTER */

#endif /* QPDECODER_INCLUDE */


