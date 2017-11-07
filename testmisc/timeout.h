/* timeout */
/* lang=C99 */

/* time-out interface */


/* revision history:

	= 2014-04-04, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2014 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines and other definitions form the interface to the
	process-wide time manager.

	Synopsis:

	int uc_timeout(const cmd,TIMEOUT *entp)

	Arguments:

	cmd		command
	entp		pointer to TIMEOUT object

	Returns:

	<0		error
	>=0		OK


*******************************************************************************/


#ifndef	TIMEOUT_INCLUDE
#define	TIMEOUT_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */

#define	TIMEOUT		struct timeout


/* time-defs */

typedef int	(*timeout_met)(void *,uint,int) ;


/* external subroutines */


/* local structures */

enum timeoutcmds {
	timeoutcmd_set,
	timeoutcmd_cancel,
	timeoutcmd_overlast
} ;

struct timeout {
	void		*objp ;
	void		*metp ;
	time_t		val ;
	uint		tag ;
	int		arg ;
	int		id ; /* created by the system */
} ;


/* forward references */


/* exported subroutines */


#if	(! defined(TIMEOUT_MASTER)) || (TIMEOUT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int timeout_load(TIMEOUT *,time_t,void *,timeout_met,uint,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* TIMEOUT_MASTER */

#endif /* TIMEOUT_INCLUDE */


