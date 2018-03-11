/* taginfo */

/* parse a tag (given in a string) */


/* revision history:

	= 1994-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	TAGINFO_INCLUDE
#define	TAGINFO_INCLUDE		1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>


#define	TAGINFO		struct taginfo


struct taginfo {
	ulong		recoff ;
	ulong		reclen ;
	int		fnamelen ;
} ;


#if	(! defined(TAGINFO_MASTER)) || (TAGINFO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	taginfo_parse(TAGINFO *,const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* TAGINFO_MASTER */

#endif /* TAGINFO_INCLUDE */


