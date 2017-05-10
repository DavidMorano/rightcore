/* strupper */

/* string library subroutines */
/* last modified %G% version %I% */



/* revision history:

	= 82/11/01, David A­D­ Morano

	Originally written for Audix Database Processor work.


*/


/*

;	This subroutine converts a string of characters to upper case.

*/





/* routine to convert a counted string to upper case */

char *strupper(dst,src)
char	*src, *dst ;
{


	while (*src != '\0') {

	    *dst++ = ((*src >= 'a') && (*src <= 'z')) ? 
	        (*src++ & (~ 0x20)) : *src++ ;

	} /* end while */

	*dst = '\0' ;
	return dst ;
}
/* end subroutine (strupper) */



