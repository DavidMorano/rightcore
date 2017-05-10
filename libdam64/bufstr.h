/* BUFSTR */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	BUFSTR_INCLUDE
#define	BUFSTR_INCLUDE	1


#define	BUFSTR		struct bufstr_head

#define	BUFSTR_LEN	100		/* default value */


struct bufstr_head {
	char		*dbuf ;		/* dynamic- buffer */
	int		len ;		/* index (active length) */
	int		dlen ;		/* extent */
	char		sbuf[BUFSTR_LEN + 1] ; /* static-buffer */
} ;


#if	(! defined(BUFSTR_MASTER)) || (BUFSTR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	bufstr_start(BUFSTR *) ;
extern int	bufstr_char(BUFSTR *,int) ;
extern int	bufstr_strw(BUFSTR *,const char *,int) ;
extern int	bufstr_buf(BUFSTR *,const char *,int) ;
extern int	bufstr_get(BUFSTR *,const char **) ;
extern int	bufstr_finish(BUFSTR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BUFSTR_MASTER */

#endif /* BUFSTR_INCLUDE */


