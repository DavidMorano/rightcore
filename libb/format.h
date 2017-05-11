/* format */

/* header file for the FORMAT subroutine */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This file was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	FORMAT_INCLUDE
#define	FORMAT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<stdarg.h>


#define	FORMAT_OCLEAN	(1<<0)		/* clean data */
#define	FORMAT_ONOOVERR	(1<<1)		/* do *not* return error on overflow */


#if	(! defined(FORMAT_MASTER)) || (FORMAT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	format(char *,int,int,const char *,va_list) ;

#ifdef	__cplusplus
}
#endif

#endif /* FORMAT_MASTER */

#endif /* FORMAT_INCLUDE */


