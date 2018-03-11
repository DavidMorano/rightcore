/* intiq */

/* Integer-Interlocked Queue */
/* last modified %G% version %I% */


/* revision history:

	= 2017-11-24, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Manage interlocked FIFO-integer operations.


*******************************************************************************/


#ifndef	INTIQ_INCLUDE
#define	INTIQ_INCLUDE	1


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>

#include	<ptm.h>
#include	<fifoitem.h>
#include	<localmisc.h>


/* local defines */

#define	INTIQ		struct intiq


struct intiq {
	PTM		m ;
	FIFOITEM	q ;
} ;


#if	(! defined(INTIQ_MASTER)) || (INTIQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	intiq_start(INTIQ *) ;
extern int	intiq_ins(INTIQ *,int) ;
extern int	intiq_rem(INTIQ *,int *) ;
extern int	intiq_count(INTIQ *) ;
extern int	intiq_finish(INTIQ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* INTIQ_MASTER */

#endif /* INTIQ_INCLUDE */


