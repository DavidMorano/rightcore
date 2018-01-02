/* storebuf INCLUDE */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	STOREBUF_INCLUDE
#define	STOREBUF_INCLUDE	1

#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>

#if	(! defined(STOREBUF_MASTER)) || (STOREBUF_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int storebuf_buf(char *,int,int,cchar *,int) ;
extern int storebuf_strw(char *,int,int,cchar *,int) ;
extern int storebuf_char(char *,int,int,int) ;
extern int storebuf_dec(char *,int,int,int) ;
extern int storebuf_deci(char *,int,int,int) ;
extern int storebuf_decui(char *,int,int,uint) ;
extern int storebuf_hexi(char *,int,int,int) ;
extern int storebuf_hexui(char *,int,int,uint) ;

#ifdef	__cplusplus
}
#endif

#endif /* STOREBUF_MASTER */

#endif /* STOREBUF_INCLUDE */


