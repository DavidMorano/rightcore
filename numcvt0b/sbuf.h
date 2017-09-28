/* INCLUDE sbuf */


/* revision history:

	= 1998-03-24, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SBUF_INCLUDE
#define	SBUF_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>


/* object defines */

#define	SBUF		struct sbuf_head


struct sbuf_head {
	char		*rbuf ;		/* result buffer base (constant) */
	int		rlen ;		/* result buffer length (constant) */
	int		index ;		/* current buffer index (changes) */
} ;


#if	(! defined(SBUF_MASTER)) || (SBUF_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	sbuf_start(SBUF *,char *,int) ;
extern int	sbuf_finish(SBUF *) ;
extern int	sbuf_buf(SBUF *,const char *,int) ;
extern int	sbuf_strw(SBUF *,const char *,int) ;
extern int	sbuf_dec(SBUF *,int) ;
extern int	sbuf_deci(SBUF *,int) ;
extern int	sbuf_decl(SBUF *,long) ;
extern int	sbuf_decll(SBUF *,longlong) ;
extern int	sbuf_decui(SBUF *,uint) ;
extern int	sbuf_decul(SBUF *,ulong) ;
extern int	sbuf_decull(SBUF *,ulonglong) ;
extern int	sbuf_hex(SBUF *,int) ;
extern int	sbuf_hexc(SBUF *,int) ;
extern int	sbuf_hexi(SBUF *,int) ;
extern int	sbuf_hexl(SBUF *,long) ;
extern int	sbuf_hexll(SBUF *,longlong) ;
extern int	sbuf_hexuc(SBUF *,uint) ;
extern int	sbuf_hexui(SBUF *,uint) ;
extern int	sbuf_hexul(SBUF *,ulong) ;
extern int	sbuf_hexull(SBUF *,ulonglong) ;
extern int	sbuf_char(SBUF *,int) ;
extern int	sbuf_nchar(SBUF *,int,int) ;
extern int	sbuf_blanks(SBUF *,int) ;
extern int	sbuf_adv(SBUF *,int,char **) ;
extern int	sbuf_rem(SBUF *) ;
extern int	sbuf_getlen(SBUF *) ;
extern int	sbuf_getpoint(SBUF *,const char **) ;
extern int	sbuf_getprev(SBUF *) ;
extern int	sbuf_printf(SBUF *,const char *,...) ;

#ifdef	__cplusplus
}
#endif

#endif /* SBUF_MASTER */

#endif /* SBUF_INCLUDE */


