/* mkchar */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	This was written for Rightcore Network Services (RNS).

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We make a character (an 8-bit entity) out of an integer.  We do this
	quite simply.


*******************************************************************************/


/* exported subroutines */


int mkchar(int ch)
{
	return (ch & 255) ;
}
/* end subroutine (mkchar) */


