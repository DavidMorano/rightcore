/* unsetenv */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


/* external subroutines */

extern char	*delenv(const char *) ;


/* exported subroutines */


void unsetenv(cchar *name)
{
	(void) delenv(name) ;
}
/* end subroutine (unsetenv) */


