/* numincr */

/* number-incrementer */


/* revision history:

	- 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	NUMINCR_INCLUDE
#define	NUMINCR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>


/* object defines */

#define	NUMINCR		struct numincr_head
#define	NUMINCR_FL	struct numincr_flags
#define	NUMINCR_DEFENTS	10
#define	NUMINCR_MAXPREC	18


struct numincr_flags {
	uint		uc:1 ;		/* upper case */
	uint		alpha:1 ;	/* alpha numbers */
} ;

struct numincr_head {
	NUMINCR_FL	f ;
	int		v ;		/* value */
	int		b ;		/* base (not used?) */
	int		prec ;		/* precision */
} ;


#if	(! defined(NUMINCR_MASTER)) || (NUMINCR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int numincr_start(NUMINCR *,const char *,int) ;
extern int numincr_load(NUMINCR *,const char *,int) ;
extern int numincr_setprec(NUMINCR *,int) ;
extern int numincr_incr(NUMINCR *,int) ;
extern int numincr_cvtstr(NUMINCR *,char *,int,int) ;
extern int numincr_finish(NUMINCR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* NUMINCR_MASTER */

#endif /* NUMINCR_INCLUDE */


