/* havenis */

/* do we have NIS running? */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	We determine is NIS is activated.  Note that NIS may not be active.

	Synopsis:

	int havenis() ;

	Arguments:

	*

	Returns:

	<0	error
	==	NIS not activated on current system
	>0	NIS is activate (but not necessarily running)


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	NISDATADNAME
#define	NISDATADNAME	"/var/nis/data"
#endif


/* exported subroutines */


int havenis()
{
	struct ustat	sb ;
	const int	nrs = SR_NOENT ;
	int		rs ;
	int		f = FALSE ;
	const char	*dname = NISDATADNAME ;

	if ((rs = u_stat(dname,&sb)) >= 0) {
	    if (S_ISDIR(sb.st_mode)) {
		f = TRUE ;
	    }
	} else if (rs == nrs) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (havenis) */


