/* buffer */

/* variable-length buffer management */
/* last modified %G% version %I% */


/* revision history:

	= 1998-03-24, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	BUFFER_INCLUDE
#define	BUFFER_INCLUDE	1


#include	<envstandards.h>
#include	<stdarg.h>


#define	BUFFER		struct buffer_head


struct buffer_head {
	char		*buf ;		/* the "buffer" */
	int		startlen ;	/* saved for expansion purposes */
	int		len ;		/* occupied length */
	int		e ;		/* current buffer extent */
} ;


#if	(! defined(BUFFER_MASTER)) || (BUFFER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	buffer_start(BUFFER *,int) ;
extern int	buffer_reset(BUFFER *) ;
extern int	buffer_adv(BUFFER *,int) ;
extern int	buffer_char(BUFFER *,int) ;
extern int	buffer_strw(BUFFER *,const char *,int) ;
extern int	buffer_buf(BUFFER *,const char *,int) ;
extern int	buffer_deci(BUFFER *,int) ;
extern int	buffer_decui(BUFFER *,uint) ;
extern int	buffer_printf(BUFFER *,const char *,...) ;
extern int	buffer_vprintf(BUFFER *,const char *,va_list) ;
extern int	buffer_getprev(BUFFER *) ;
extern int	buffer_get(BUFFER *,const char **) ;
extern int	buffer_finish(BUFFER *) ;

#ifdef	COMMENT
extern int	buffer_strn(BUFFER *,const char *,int) ;
#endif

#ifdef	__cplusplus
}
#endif

#endif /* BUFFER_MASTER */

#endif /* BUFFER_INCLUDE */


