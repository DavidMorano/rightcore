/* progcs */

/* Program-ComSat (ProgCS) */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Sub-program for the COMSAT server.


*******************************************************************************/


#ifndef	PROGCS_INCLUDE
#define	PROGCS_INCLUDE	1


#include	<envstandards.h>	/* must be before others */
#include	<sys/types.h>
#include	<ids.h>
#include	<hdrdecode.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"defs.h"


#define	PROGCS		struct progcs
#define	PROGCS_FL	struct progcs_flags
#define	PROGCS_JOB	struct progcs_job


struct progcs_job {
	cchar		*mp ;
	int		ml ;
} ;

struct progcs_flags {
	uint		hdrdecode:1 ;
} ;

struct progcs {
	PROGINFO	*pip ;
	PROGCS_FL	open ;
	IDS		id ;
	HDRDECODE	d ;		/* header-decode */
	PTM		dm ;		/* header-decode mutex */
} ;


#if	(! defined(PROGCS_MASTER)) || (PROGCS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	progcs_trans(PROGINFO *,wchar_t *,int,cchar *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* PROGCS_MASTER */

#endif /* PROGCS_INCLUDE */


