/* manstr */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MANSTR_INCLUDE
#define	MANSTR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


#define	MANSTR		struct manstr


struct manstr {
	const char	*sp ;
	int		sl ;
} ;


#if	(! defined(MANSTR_MASTER)) || (MANSTR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	manstr_start(MANSTR *,const char *,int) ;
extern int	manstr_breakfield(MANSTR *,const char *,const char **) ;
extern int	manstr_span(MANSTR *,const char *) ;
extern int	manstr_whitedash(MANSTR *) ;
extern int	manstr_whitecolon(MANSTR *) ;
extern int	manstr_finish(MANSTR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MANSTR_MASTER */

#endif /* MANSTR_INCLUDE */


