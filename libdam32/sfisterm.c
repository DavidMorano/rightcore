/* sfisterm */

/* Safe-Fast Is-a-Terminal */


/* revision history:

	= 2004-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine determines if the given Safe-Fast stream is a terminal
	or not.

	Synopsis:

	int sfisterm(Sfio_t *f)

	Arguments:

	f		pointer to Shio_t object

	Returns:

	<0		error
	==0		NO (not a terminal)
	>0		YES (a terminal)


*******************************************************************************/


#include	<envstandards.h>

#include	<ast.h>			/* configures other stuff also */

#include	<sys/types.h>
#include	<unistd.h>

#include	<sfio_s.h>
#include	<sfio.h>


/* local defines */


/* exported subroutines */


int sfisterm(Sfio_t *f)
{
	int		f_isterm ;
	if (f == NULL) return -1 ;
	f_isterm = (! (f->_flags & SF_STRING)) ;
	f_isterm = f_isterm && (f->_file >= 0) ;
	if (f_isterm) f_isterm = isatty(f->_file) ;
	return f_isterm ;
}
/* end subroutine (sfisterm) */


