/* outline */

/* output lines */


/* revision history:

	= 1998-04-01, David A­D­ Morano
	This subroutine was originally written.

	= 1999-02-01, David A­D­ Morano
        I added a little code to "post" articles that do not have a valid
        newsgroup to a special "dead article" directory in the BB spool area.

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object deals with printing lines.


*******************************************************************************/


#ifndef	OUTLINE_INCLUDE
#define	OUTLINE_INCLUDE		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<bfile.h>
#include	<localmisc.h>


#define	OUTLINE		struct outline
#define	OUTLINE_FL	struct outline_flags


struct outline_flags {
	uint		comma:1 ;
} ;

struct outline {
	bfile		*ofp ;
	OUTLINE_FL	f ;
	int		maxlen ;
	int		rlen ;
	int		llen ;
	int		wlen ;
	int		c_values ;
	int		c_items ;
} ;


#if	(! defined(OUTLINE_MASTER)) || (OUTLINE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	outline_start(OUTLINE *,bfile *,int) ;
extern int	outline_finish(OUTLINE *) ;
extern int	outline_write(OUTLINE *,cchar *,int) ;
extern int	outline_printf(OUTLINE *,cchar *,...) ;
extern int	outline_item(OUTLINE *,cchar *,int) ;
extern int	outline_value(OUTLINE *,cchar *,int) ;
extern int	outline_needlength(OUTLINE *,int) ;

#ifdef	__cplusplus
}
#endif

#endif	/* (! defined(OUTLINE_MASTER)) || (OUTLINE_MASTER == 0) */

#endif	/* OUTLINE_INCLUDE */


