/* timemgr */
/* lang=C99 */

/* time manger interface */


/* revision history:

	= 2017-10-04, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines and other definitions form the interface to the
	process-wide time manager.

	Synopsis:

	int uc_timemgr(const cmd,TIMEMGR *entp)

	Arguments:

	cmd		command
	entp		pointer to TIMEMGR object

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#ifndef	TIMENGR_INCLUDE
#define	TIMEMGR_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */

#define	TIMEMGR		struct timemgr


/* external subroutines */


/* local structures */

enum timemgrcmds {
	timemgrcmd_set,
	timemgrcmd_cancel,
	timemgrcmd_overlast
} ;

struct timemgr {
	void		*objp ;
	void		*metp ;
	time_t		val ;
	uint		tag ;
	int		arg ;
	int		id ; /* created by the system */
} ;


/* forward references */


/* exported subroutines */


#if	(! defined(TIMEMGR_MASTER)) || (TIMEMGR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int uc_timemgr(int,TIMEMGR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TIMEMGR_MASTER */

#endif /* TIMEMGR_INCLUDE */


