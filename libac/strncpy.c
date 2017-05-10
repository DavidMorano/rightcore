/* strncpy */

/* routine to copy a string to a buffer counted */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


char *strncpy(dst,src,n)
char	*src, *dst ;
int	n ;
{
	int	i ;
	for (i = 0 ; src[i] ; i += 1) {
	    dst[i] = src[i] ;
	}
	while (i < n) {
	    dst[i] = '\0' ;
	}
	return dst ;
}
/* end subroutine (strncpy) */


