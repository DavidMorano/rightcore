/* readline */
/* lang=C++11 */

/* read a line from a file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2013-07-11, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int readline(ifstream &is,char *lbuf,int llen)

	Arguments:

	is		reference to stream (file)
	lbuf		buffer to receive
	llen		length of supplied buffer

	Returns:

	-		lenght of bytes read


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<limits.h>
#include	<cinttypes>
#include	<new>
#include	<initializer_list>
#include	<utility>
#include	<functional>
#include	<algorithm>
#include	<vector>
#include	<string>
#include	<fstream>
#include	<iostream>
#include	<iomanip>
#include	<vsystem.h>
#include	<localmisc.h>


/* name-spaces */

using namespace std ;


/* external subroutines */


/* global variables */


/* local structures (and methods) */


/* forward references */


/* local variables */


/* exported subroutines */


int readline(ifstream &is,char *lbuf,int llen)
{
	int		rs = SR_OK ;
	if (is.getline(lbuf,llen)) {
	    rs = is.gcount() ;
	}
	return rs ;
}
/* end subroutine (readline) */


