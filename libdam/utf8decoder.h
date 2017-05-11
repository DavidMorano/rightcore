/* utf8decoder */

/* UTF-8 decoder */


/* revision history:

	= 2016-06-29, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2016 David A­D­ Morano.  All rights reserved. */

#ifndef	UTF8DECODER_INCLUDE
#define	UTF8DECODER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<stddef.h>		/* for 'wchar_t' */
#include	<localmisc.h>


#define	UTF8DECODER_MAGIC	0x13f3c205
#define	UTF8DECODER		struct utf8decoder_head


struct utf8decoder_head {
	uint		magic ;
	void		*outbuf ;	/* output-buffer */
	uint		code ;		/* UNICODE® code point */
	int		rem ;		/* remaining bytes */
} ;


#if	(! defined(UTF8DECODER_MASTER)) || (UTF8DECODER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int utf8decoder_start(UTF8DECODER *) ;
extern int utf8decoder_load(UTF8DECODER *,cchar *,int) ;
extern int utf8decoder_read(UTF8DECODER *,wchar_t *,int) ;
extern int utf8decoder_finish(UTF8DECODER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* UTF8DECODER_MASTER */

#endif /* UTF8DECODER_INCLUDE */


