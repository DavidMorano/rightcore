/* xfindbit */

/* find bits (meeting certain criteria) */


/* revision history:

	= 1999-03-09, David A­D­ Morano
        This group of subroutines was originally written here in C language but
        were inspired by single instructions on the VAX (Digital Equipment 1979)
        that did essentially the same functions.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Find bits that meet a certain criteria in an integer.

	xffbs[il]	find first bit set
	xffbc[il]	find first bit clear
	xflbs[i]	find last bit set
	xflbc[i]	find last bit clear


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


int xffbsi(UINT v)
{
	const int	n = (sizeof(uint)*8) ;
	int	i ;
	for (i = 0 ; i < n ; i += 1) {
	    if (v & 1) break ;
	    v = v >> 1 ;
	} /* end for */
	return (i < n) ? i : -1 ;
}
/* end subroutine (xffbsi) */


int xffbci(UINT v)
{
	const int	n = (sizeof(uint)*8) ;
	int	i ;
	for (i = 0 ; i < n ; i += 1) {
	    if (! (v & 1)) break ;
	    v = v >> 1 ;
	} /* end for */
	return (i < n) ? i : -1 ;
}
/* end subroutine (xffbci) */


int xffbsl(ULONG v)
{
	const int	n = (sizeof(ULONG)*8) ;
	int	i ;
	for (i = 0 ; i < n ; i += 1) {
	    if (v & 1) break ;
	    v = v >> 1 ;
	} /* end for */
	return (i < n) ? i : -1 ;
}
/* end subroutine (xffbsl) */


/* find first bit clear in LONG */
int xffbcl(ULONG v)
{
	const int	n = (sizeof(ULONG)*8) ;
	int	i ;
	for (i = 0 ; i < n ; i += 1) {
	    if (! (v & 1)) break ;
	    v = v >> 1 ;
	} /* end for */
	return (i < n) ? i : -1 ;
}
/* end subroutine (xffbcl) */


/* find last bit set in integer */
int xflbsi(UINT v)
{
	const int	n = (sizeof(UINT)*8) ;
	int	i ;
	for (i = (n-1) ; i >= 0 ; i -= 1) {
	    if ((v >> i) & 1) break ;
	} /* end for */
	return (i >= 0) ? i : -1 ;
}
/* end subroutine (xflbsi) */


/* find last bit clear in integer */
int xflbci(UINT v)
{
	const int	n = (sizeof(UINT)*8) ;
	int	i ;
	for (i = (n-1) ; i >= 0 ; i -= 1) {
	    if (! ((v >> i) & 1)) break ;
	} /* end for */
	return (i >= 0) ? i : -1 ;
}
/* end subroutine (xflbci) */


/* find last bit set in LONG */
int xflbsl(ULONG v)
{
	const int	n = (sizeof(ULONG)*8) ;
	int	i ;
	for (i = (n-1) ; i >= 0 ; i -= 1) {
	    if ((v >> i) & 1) break ;
	} /* end for */
	return (i >= 0) ? i : -1 ;
}
/* end subroutine (xflbsl) */


/* find last bit clear in LONG */
int xflbcl(ULONG v)
{
	const int	n = (sizeof(ULONG)*8) ;
	int	i ;
	for (i = (n-1) ; i >= 0 ; i -= 1) {
	    if (! ((v >> i) & 1)) break ;
	} /* end for */
	return (i >= 0) ? i : -1 ;
}
/* end subroutine (xflbcl) */


/* find the number of set bits in an *integer* */
int xfbscounti(uint v)
{
	const int	n = (sizeof(uint)*8) ;
	int	i ;
	int	c = 0 ;
	for (i = 0 ; i < n ; i += 1) {
	    if (v & 1) c += 1 ;
	    v = v >> 1 ;
	} /* end for */
	return c ;
}
/* end subroutine (xfbscounti) */


/* find the number of set bits in an *LONG* (64 bits) */
int xfbscountl(ULONG v)
{
	const int	n = (sizeof(ULONG)*8) ;
	int	i ;
	int	c = 0 ;
	for (i = 0 ; i < n ; i += 1) {
	    if (v & 1) c += 1 ;
	    v = v >> 1 ;
	} /* end for */
	return c ;
}
/* end subroutine (xfbscountl) */


