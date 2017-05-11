/* uc_exit */

/* interface component for UNIX® library-3c */
/* regular exit (like |exit(3c)|) */


/* revision history:

	= 1998-11-28, David A­D­ Morano
	How did we get along without this for over 10 years?

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine exits in a way that calls any "exit" subroutines that
	have been registered to be called on "exit."


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<vsystem.h>


/* local defines */


/* forward references */


/* exported subroutines */


int uc_exit(int ex)
{
	exit(ex) ;
	return SR_NOSYS ;
}
/* end subroutine (uc_exit) */


