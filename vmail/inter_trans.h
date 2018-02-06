/* inter_trans */

/* translation adjunct for the INTER object */


/* revision history:

	= 1994-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Manage character translation services for the INTER object.


*******************************************************************************/


#ifndef	INTERTRANS_INCLUDE
#define	INTERTRANS_INCLUDE	1


#include	<envstandards.h>	/* must be before others */
#include	<sys/types.h>
#include	<localmisc.h>


#if	(! defined(INTERTRANS_MASTER)) || (INTERTRANS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int inter_transbegin(INTER *) ;
extern int inter_transend(INTER *) ;
extern int inter_transhd(INTER *) ;
extern int inter_transproc(INTER *,char *,int,cchar *,int,int) ;
 
#ifdef	__cplusplus
}
#endif

#endif /* INTERTRANS_MASTER */

#endif /* INTERTRANS_INCLUDE */


