/* outbuf */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	OUTBUF_INCLUDE
#define	OUTBUF_INCLUDE	1


#define	OUTBUF		struct outbuf


struct outbuf {
	char		*obuf ;
	int		olen ;
	int		f_alloc ;
} ;


#if	(! defined(OUTBUF_MASTER)) || (OUTBUF_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	outbuf_start(OUTBUF *,char *,int) ;
extern int	outbuf_get(OUTBUF *,char **) ;
extern int	outbuf_finish(OUTBUF *) ;

#ifdef	__cplusplus
}
#endif

#endif /* OUTBUF_MASTER */

#endif /* OUTBUF_INCLUDE */


