/* isValidMagic */

/* does the given unknown string container the given valid magic string? */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine verifies that the given unknown string contains the
        given valid magic string.

	Synopsis:

	int isValidMagic(tbuf,tlen,ms)

	Arguments:

	cchar	*tbuf		string buffer to test
	int	tlen		length of string buffer
	int	ms		givem valid magic string

	Returns:

	1		TRUE, YES it matches
	0		does not match


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<string.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int isValidMagic(char *tbuf,int tlen,char *ms)
{
	const int	ml = strlen(ms) ;
	int		f = FALSE ;
	if (tlen >= (ml+1)) {
	    f = TRUE ;
	    f = f && (strncmp(tbuf,ms,ml) == 0) ;
	    f = f && (tbuf[ml] == '\n') ;
	}
	return f ;
}
/* end subroutine (isValidMagic) */


