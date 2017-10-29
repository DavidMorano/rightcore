/* calworder */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	CALWORDER_INCLUDE
#define	CALWORDER_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>

#include	"calent.h"


#define	CALWORDER		struct calworder


struct calworder {
	CALENT_LINE	*lines ;
	cchar		*md ;
	cchar		*sp ;
	int		sl ;
	int		i ;
	int		nlines ;
} ;


#if	(! defined(CALWORDER_MASTER)) || (CALWORDER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	calworder_start(CALWORDER *,cchar *,CALENT *) ;
extern int	calworder_finish(CALWORDER *) ;
extern int	calworder_get(CALWORDER *,cchar **) ;


#ifdef	__cplusplus
}
#endif

#endif /* CALWORDER_MASTER */

#endif /* CALWORDER_INCLUDE */


