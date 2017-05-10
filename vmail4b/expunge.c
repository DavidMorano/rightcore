/* expunge */

/* subroutine to do something */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Expunge something!

        When leave mailbox (change,quit), ask whether should mbrewrite. If no,
        delete marks disappear.

        return TRUE if should mbrewrite (some marked and user says yes). return
        FALSE otherwise (none marked or user says no).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<bfile.h>
#include	<baops.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"mb.h"
#include	"display.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local (forward) subroutines */


/* exported subroutines */


int inter_expunge(pip)
struct proginfo	*pip ;
{
	int	rs, i ;
	int	pv, answer ;


/* there is at least one message marked for deletion */

	pv = profile("confirm") ;

	if (pv == 0) 
		return TRUE ;

	answer = 
	    inter_inchar(pip,"delete marked messages? [yes] ") ;

	if (answer < 0) 
		return FALSE ;

	if (answer == '\r' || answer == '\n' ||
	    answer == 'y' || answer == 'Y') {

	    display_info(pip->dip,
		"marked messages are scheduled for deletion\n") ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("mbrewrite: OK to delete\n") ;
#endif

	    rs = TRUE ;

	} else {

	    rs = FALSE ;
	    display_info(pip->dip,"not deleted\n") ;

	}

	display_flush(pip->dip) ;

	return rs ;
}
/* end subroutine (inter_expunge) */



