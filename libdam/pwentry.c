/* pentry */

/* some miscellaneous PWENTRY subroutines */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We provide some miscelleneous utility subroutines for the PWENTRY
	object.

	int pentry_bufsize()

	Arguments:

	Returns:

	>=0		size of necessary buffer to hold the data for 
			a PWENTRY object


*******************************************************************************/


#define	PWENTRY_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<sys/param.h>		/* for MAXPATHLEN */
#include	<vsystem.h>
#include	<getbufsize.h>
#include	<localmisc.h>
#include	"pwentry.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int pwentry_bufsize()
{
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	rs = (pwlen+MAXPATHLEN) ;
	return rs ;
}
/* end subroutine (pwentry_bufsize) */


