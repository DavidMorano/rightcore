/* sfio */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	SFIO_INCLUDE
#define	SFIO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<ast.h>			/* configures other stuff also */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdarg.h>

#include	<localmisc.h>

#include	<sfio.h>


#define	SFIO_CNOP		0
#define	SFIO_CSETBUFWHOLE	1
#define	SFIO_CSETBUFLINE	2
#define	SFIO_CSETBUFUN		3
#define	SFIO_CFD		4
#define	SFIO_CSETFLAGS		5


#if	(! defined(SFIO_MASTER)) || (SFIO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sfio_reade(Sfio_t *,void *,int,int,int) ;
extern int sfio_read(Sfio_t *,void *,int) ;
extern int sfio_readintr(Sfio_t *,void *,int,int,int *) ;
extern int sfio_readlinetimed(Sfio_t *,char *,int,int) ;
extern int sfio_readline(Sfio_t *,char *,int) ;
extern int sfio_write(Sfio_t *,const void *,int) ;
extern int sfio_printline(Sfio_t *,const char *,int) ;
extern int sfio_printf(Sfio_t *,const char *,...) ;
extern int sfio_putc(Sfio_t *,int) ;
extern int sfio_seek(Sfio_t *,offset_t,int) ;
extern int sfio_flush(Sfio_t *) ;
extern int sfio_control(Sfio_t *,int,...) ;
extern int sfio_getfd(Sfio_t *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SFIO_MASTER */


#endif /* SFIO_INCLUDE */


