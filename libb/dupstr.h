/* dupstr */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DUPSTR_INCLUDE
#define	DUPSTR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<localmisc.h>


#define	DUPSTR		struct dupstr
#define	DUPSTR_SHORTLEN	32


struct dupstr {
	char		*as ;	/* allocated string */
	char		buf[DUPSTR_SHORTLEN+1] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	dupstr_start(DUPSTR *,const char *,int,char **) ;
extern int	dupstr_finish(DUPSTR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DUPSTR_INCLUDE */


