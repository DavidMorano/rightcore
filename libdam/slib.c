/* slib */

/* string library */
/* last modified %G% version %I% */


/* revision history:

	= 1983-12-02, David A.D. Morano
	This library module was originally written being traslated from
	MC68000 assembly.

*/


/*
; string library subroutines

;	This file contains many of the string manipulation subroutines
;	used in other modules.
*/


#include	<localmisc.h>


/* exported subroutines */


/*
; routine to convert a left justified HEX string to its binary value

;	Arguments:
;	- length of string to be converted
;	- address of string to be converted
;	- address of longword to store result

;	Outputs :
;	- return status is zero for OK else not zero
*/

int cfhex(len,s,rp)
int	len ;
long	*rp ;
char	*s ;
{
	int		val, n, si ;
	char		c ;

	val = 0 ;
	if (len == 0) 
	    goto cfh_done ;

	n = 0 ;
	si = len - 1 ;

	while (s[si] == '\t' || s[si] == ' ') {
		if (si == 0) 
		goto cfh_done ;
		si -= 1 ;
	}

	c = s[si] ;
	goto cfh_enter ;

/*; start to form value from the low end of the string */
cfh_loop:
	c = s[--si] ;

cfh_enter:
	if ((c == ' ') || (c == '\t')) 
		goto cfh_done ;

	if (c < '0') 
		goto cfh_nodig ;

	if (c <= '9') 
		goto cfh_dig ;

	c &= 0xDF ;			/* clear bit 5 for upper case */

	if (c > 'F') 
		goto cfh_nodig ;

	if (c < 'A') 
		goto cfh_nodig ;

	c -= 7 ;

/*; continue here if a good character */
cfh_dig:
	if (n >= 8) 
		goto cfh_more ;

	c &= 0x0F ;
	val |= (((int) c) << (n * 4)) ;
	n += 1 ;

/*; is there any more string left? */
cfh_more:
	if (si > 0) 
		goto cfh_loop ;	/* jump if no more string left */

/*; we are done looping, finish up */
cfh_done:
	*rp = val ;
	return OK ;

cfh_nodig:
	while (s[si] == '\t' || s[si] == ' ') {

		if (si == 0) 
		goto cfh_done ;

		si -= 1 ;
	}

	c = s[si] ;
	if (c != '-') 
		goto cfh_bad ;

	val = -val ;
	goto cfh_done ;

cfh_bad:
	return BAD ;
}
/*; end subroutine (cfhex) */


/* routine to compare two character strings for equallity */
int cmpc(len,src,dst)
long	len ;
char	*src, *dst ;
{
	int		i ;

	for (i = 0 ; i < len ; i++) {
	    if (*src++ != *dst++) 
		goto bad ;
	}

	return (0L) ;

bad:
	src-- ; dst-- ;
	return ((int) *src) - ((int) *dst) ;
}
/* end subroutine (cmpc) */


/* routine to convert a counted string to upper case */
char *cup(len,src,dst)
long	len ;
char	*src, *dst ;
{
	long		i ;

	for (i = 0 ; i < len ; i++) {
	    *dst++ = 
		((*src >= 'a') && (*src <= 'z')) ? (*src++ - 32) : *src++ ;
	}

	return (dst) ;
}
/* end subroutine (cup) */


/* routine to convert a counted string to lower case */
char *clow(len,src,dst)
long	len ;
char	*src, *dst ;
{
	long		i ;

	for (i = 0 ; i < len ; i++) {
	    *dst++ = 
		((*src >= 'A') && (*src <= 'Z')) ? (*src++ + 32) : *src++ ;
	}

	return (dst) ;
}
/* end subroutine (clow) */


