/* libff */

/* header file for the FORTMAT subroutine */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This file was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	LIBFF_INCLUDE
#define	LIBFF_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdio.h>


#if	(! defined(LIBFF_MASTER)) || (LIBFF_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int ffclearerr(FILE *) ;
extern int ffclose(FILE *) ;
extern int ffeof(FILE *) ;
extern int fferror(FILE *) ;
extern int ffflush(FILE *) ;
extern int ffgetc(FILE *) ;
extern int ffprintf(FILE *,const char *,...) ;
extern int ffputc(FILE *,int) ;
extern int ffread(FILE *,void *,int) ;
extern int ffreadline(FILE *,char *,int) ;
extern int ffseek(FILE *,offset_t,int) ;
extern int fftell(FILE *,offset_t *) ;
extern int ffwrite(FILE *,const void *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* LIBFF_MASTER */

#endif /* LIBFF_INCLUDE */


