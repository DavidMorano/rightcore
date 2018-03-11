/* tmz */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	TMZ_INCLUDE
#define	TMZ_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


#define	TMZ_MAGIC	0x26292511
#define	TMZ		struct tmz_head
#define	TMZ_FLAGS	struct tmz_flags
#define	TMZ_ZNAMESIZE	8


struct tmz_flags {
	uint		zoff:1 ;		/* zone offset is present */
	uint		year:1 ;		/* year is present */
} ;

struct tmz_head {
	struct tm	st ;
	TMZ_FLAGS	f ;
	short		zoff ;			/* minutes west of GMT */
	char		zname[TMZ_ZNAMESIZE] ;
} ;


#if	(! defined(TMZ_MASTER)) || (TMZ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	tmz_init(TMZ *) ;
extern int	tmz_std(TMZ *,const char *,int) ;
extern int	tmz_msg(TMZ *,const char *,int) ;
extern int	tmz_touch(TMZ *,const char *,int) ;
extern int	tmz_toucht(TMZ *,const char *,int) ;
extern int	tmz_strdig(TMZ *,const char *,int) ;
extern int	tmz_logz(TMZ *,const char *,int) ;
extern int	tmz_day(TMZ *,const char *,int) ;
extern int	tmz_isset(TMZ *) ;
extern int	tmz_hasyear(TMZ *) ;
extern int	tmz_haszoff(TMZ *) ;
extern int	tmz_haszone(TMZ *) ;
extern int	tmz_setday(TMZ *,int,int,int) ;
extern int	tmz_setyear(TMZ *,int) ;
extern int	tmz_setzone(TMZ *,const char *,int) ;
extern int	tmz_gettm(TMZ *,struct tm *) ;
extern int	tmz_getdst(TMZ *) ;
extern int	tmz_getzoff(TMZ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(TMZ_MASTER)) || (TMZ_MASTER == 0) */

#endif /* TMZ_INCLUDE */


