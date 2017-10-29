/* strop */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	STROP_INCLUDE
#define	STROP_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>


#define	STROP		struct xstrop


struct xstrop {
	const char	*sp ;
	int		sl ;
} ;


#if	(! defined(STROP_MASTER)) || (STROP_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	strop_start(STROP *,cchar *,int) ;
extern int	strop_breakfield(STROP *,cchar *,cchar **) ;
extern int	strop_span(STROP *,const char *) ;
extern int	strop_whitedash(STROP *) ;
extern int	strop_whitecolon(STROP *) ;
extern int	strop_finish(STROP *) ;

#ifdef	__cplusplus
}
#endif

#endif /* STROP_MASTER */

#endif /* STROP_INCLUDE */


