/* cvtdater */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CVTDATER_INCLUDE
#define	CVTDATER_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	CVTDATER	struct cvtdater


struct cvtdater {
	time_t		daytime ;
} ;


#if	(! defined(CVTDATER_MASTER)) || (CVTDATER_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	cvtdater_start(CVTDATER *,time_t) ;
extern int	cvtdater_load(CVTDATER *,time_t *,cchar *,int) ;
extern int	cvtdater_finish(CVTDATER *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CVTDATER_MASTER */

#endif /* CVTDATER_INCLUDE */


