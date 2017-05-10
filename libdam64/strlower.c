/* strlower */

/* last modified %G% version %I% */



/* revision history:

	= 82/11/01, David A­D­ Morano

	Originally written for Audix Database Processor work.


*/



/*
; string library subroutines

;	This subroutine converts a string of characters to lower case.

*/





/* routine to convert a counted string to upper case */

char *strlower(dst,src)
char	*src, *dst ;
{


	while (*src != '\0') {

	    *dst++ = ((*src >= 'A') && (*src <= 'Z')) ? 
	        (*src++ | 0x20) : *src++ ;

	} /* end for */

	*dst = '\0' ;
	return dst ;
}
/* end subroutine (strlower) */



