/* mkfname */

/* make a file name from several parts */


/* revision history:

	= 2001-12-03, David A­D­ Morano
	This was made specifically for the HDB UUCP modified code.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs a filename (a single filename component) out
	the given source strings.  This subroutine it totally just a bif
	concatenation of strings but with the restriction that the destination
	buffer is only MAXPATHLEN long.

	Synopsis:

	int mkfname(ofname,p1,p2,p3)
	char		ofname[] ;
	const char	p1[] ;
	const char	p2[] ;
	const char	p2[] ;

	Arguments:

	ofname		buffer for the resulting filename
	p1		first component
	p2		second component
	p3		third component

	Returns:

	<0		error
	>=0		OK, length of resulting filename


	Notes: 

	1. Note that |mkfname1()| is sematically identical to |mkpath1()|
	but uses different code.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>


/* external subroutines */

extern int	sncpy(char *,int,int,...) ;


/* forward references */


/* exported subroutines */


int mkfname1(rbuf,p1)
char		rbuf[] ;
const char	p1[] ;
{
	const int	rlen = MAXPATHLEN ;
	return sncpy(rbuf,rlen,1,p1) ;
}
/* end subroutine (mkfname1) */


int mkfname2(rbuf,p1,p2)
char		rbuf[] ;
const char	p1[], p2[] ;
{
	const int	rlen = MAXPATHLEN ;
	return sncpy(rbuf,rlen,2,p1,p2) ;
}
/* end subroutine (mkfname2) */


int mkfname3(rbuf,p1,p2,p3)
char		rbuf[] ;
const char	p1[], p2[], p3[] ;
{
	const int	rlen = MAXPATHLEN ;
	return sncpy(rbuf,rlen,3,p1,p2,p3) ;
}
/* end subroutine (mkfname3) */


