/* babycalc */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	BABYCALC_INCLUDE
#define	BABYCALC_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<modload.h>
#include	<localmisc.h>

#include	"babycalcs.h"


#define	BABYCALC_MAGIC		0x97147229
#define	BABYCALC		struct babycalc_head
#define	BABYCALC_INFO		struct babycalc_i
#define	BABYCALC_CALLS		struct babycalc_calls


struct babycalc_i {
	time_t		wtime ;
	time_t		atime ;
	uint		acount ;
} ;

struct babycalc_calls {
	int		(*open)(void *,cchar *,cchar *) ;
	int		(*check)(void *,time_t) ;
	int		(*lookup)(void *,time_t,uint *) ;
	int		(*info)(void *,BABYCALCS_INFO *) ;
	int		(*close)(void *) ;
} ;

struct babycalc_head {
	uint		magic ;
	MODLOAD		loader ;
	BABYCALC_CALLS	call ;
	void		*obj ;		/* object pointer */
} ;


#if	(! defined(BABYCALC_MASTER)) || (BABYCALC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	babycalc_open(BABYCALC *,cchar *,cchar *) ;
extern int	babycalc_check(BABYCALC *,time_t) ;
extern int	babycalc_lookup(BABYCALC *,time_t,uint *) ;
extern int	babycalc_info(BABYCALC *,BABYCALC_INFO *) ;
extern int	babycalc_close(BABYCALC *) ;

#ifdef	__cplusplus
}
#endif

#endif /* BABYCALC_MASTER */

#endif /* BABYCALC_INCLUDE */


