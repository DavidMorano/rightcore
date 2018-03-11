/* chariq */

/* Character-Interlocked Queue */
/* last modified %G% version %I% */


/* revision history:

	= 1994-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1994 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Manage interlocked FIFO-character operations.


*******************************************************************************/


#ifndef	CHARIQ_INCLUDE
#define	CHARIQ_INCLUDE	1


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<charq.h>
#include	<ptm.h>
#include	<localmisc.h>


/* local defines */

#define	CHARIQ		struct chariq


struct chariq {
	CHARQ		q ;
	PTM		m ;
} ;


#if	(! defined(CHARIQ_MASTER)) || (CHARIQ_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	chariq_start(CHARIQ *,int) ;
extern int	chariq_ins(CHARIQ *,int) ;
extern int	chariq_rem(CHARIQ *,char *) ;
extern int	chariq_size(CHARIQ *) ;
extern int	chariq_count(CHARIQ *) ;
extern int	chariq_finish(CHARIQ *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CHARIQ_MASTER */

#endif /* CHARIQ_INCLUDE */


