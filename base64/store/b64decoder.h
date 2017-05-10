/* b64decoder */

/* decode data (encoded in BASE64) */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	B64DECODER_INCLUDE
#define	B64DECODER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<char.h>

#include	<localmisc.h>


/* object defines */

#define	B64DECODER		struct decoder_head
#define	B64DECODER_BUF		struct decoder_buf


struct decoder_head {
	char		*obuf ;
	int		olen ;
	int		bl ;
	int		sl ;
	char		stage[4] ;
} ;

struct decoder_buf {
	const char	*bp ;
	int		bl ;
} ;


#if	(! defined(B64DECODER_MASTER)) || (B64DECODER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	b64decoder_start(B64DECODER *,char *,int) ;
extern int	b64decoder_process(B64DECODER *,B64DECODER_BUF *,cchar **) ;
extern int	b64decoder_finish(B64DECODER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* B64DECODER_MASTER */

#endif /* B64DECODER_INCLUDE */


