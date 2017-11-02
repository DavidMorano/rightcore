/* uc_sigdefault */

/* interface component for UNIX® library-3c */


#define	CF_ISAEXEC	1		/* try ISA-EXEC */


/* revision history:

	= 1998-11-28, David A­D­ Morano
	How did we get along without this for over 10 years?

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        For now, this just is a regular (fairly so at any rate) 'exec(2)'-like
        subroutine.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<signal.h>
#include	<stdlib.h>

#include	<vsystem.h>


/* local defines */


/* forward references */


/* exported subroutines */


int uc_sigdefault(int sn)
{
	int		rs = SR_OK ;
	void		(*ret)(int) ;

	ret = signal(sn,SIG_DFL) ;
	if (ret == SIG_ERR) rs = (- errno) ;

	return rs ;
}
/* end subroutine (uc_sigdefault) */


