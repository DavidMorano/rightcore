/* cvttemperature */

/* convert temperature */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>


/* exported subroutines */


/* convert degrees Fahrenheit to Centigrade */
double cvtfahtocen(double fah)
{

	return ((fah - 32.0) * 5.0 / 9.0) ;
}
/* end subroutine (cvtfahtocen) */


/* convert degrees Centigrade to Fahrenheit */
double cvtcentofah(double cen)
{

	return ((cen * 9.0 / 5.0) + 32.0) ;
}
/* end subroutine (cvtcentofah) */


