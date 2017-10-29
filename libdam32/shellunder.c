/* shellunder */

/* parse out the shell-under information (as might be present) */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will parse out the shell-under information from a given
        string (containing the shell-under information).

	Synopsis:

	int shellunder(op,under)
	SHELLUNDER	*op ;
	const char	*under ;

	Arguments:

	op		pointer to object
	under		given string containing shell-under information

	Returns:

	>=0	length of program-execution filename
	<0	error


	= Implementation notes:

	The "shell-under" string takes the form:

		[*<ppid>*]<progename>

	where:

	ppid		PID of parent (shell) process
	progename	program exec-name of child process


	Enjoy.
	

*******************************************************************************/


#define	SHELLUNDER_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>

#include	"shellunder.h"


/* local defines */

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int shellunder(SHELLUNDER *op,cchar *under)
{
	int		rs = SR_OK ;
	int		pl = 0 ;

	if (under == NULL) return SR_FAULT ;

	op->pid = -1 ;
	op->progename = NULL ;
	if (under[0] != '\0') {

	    if (under[0] == '*') {
	        int	dl = -1 ;
	        cchar	*dp = (under+1) ;
	        cchar	*tp = strchr(under,'*') ;
	        if (tp != NULL) {
	            int	v ;
	            dl = (tp-dp) ;
	            under = (tp+1) ;
	            if ((rs = cfdeci(dp,dl,&v)) >= 0) {
	                op->pid = (pid_t) v ;
	            }
	        } else
	            rs = SR_INVALID ;
	    }

	    if ((rs >= 0) && (under[0] != '\0')) {
	        while (CHAR_ISWHITE(*under)) under += 1 ;
	        op->progename = under ;
	        pl = strlen(under) ;
	    }

	} /* end if (non-zero) */

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (shellunder_load) */


