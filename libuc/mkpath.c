/* mkpath */

/* make a file path from components */


/* revision history:

	= 2001-12-03, David A­D­ Morano
	This code was born out of frustration with cleaning up bad legacy code
	(of which there is quite a bit -- like almost all of it).

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine constructs a file path out of one or more path
        componets.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>


/* external subroutines */

extern int	mknpath(char *,int,int,...) ;


/* external variables */


/* local strutures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkpath1(rbuf,s1)
char		rbuf[] ;
const char	s1[] ;
{
	const int	rlen = MAXPATHLEN ;
	return mknpath(rbuf,rlen,1,s1) ;
}
/* end subroutine (mkpath1) */


int mkpath2(rbuf,s1,s2)
char		rbuf[] ;
const char	s1[], s2[] ;
{
	const int	rlen = MAXPATHLEN ;
	return mknpath(rbuf,rlen,2,s1,s2) ;
}
/* end subroutine (mkpath2) */


int mkpath3(rbuf,s1,s2,s3)
char		rbuf[] ;
const char	s1[], s2[], s3[] ;
{
	const int	rlen = MAXPATHLEN ;
	return mknpath(rbuf,rlen,3,s1,s2,s3) ;
}
/* end subroutine (mkpath3) */


int mkpath4(rbuf,s1,s2,s3,s4)
char		rbuf[] ;
const char	s1[], s2[], s3[], s4[] ;
{
	const int	rlen = MAXPATHLEN ;
	return mknpath(rbuf,rlen,4,s1,s2,s3,s4) ;
}
/* end subroutine (mkpath4) */


int mkpath5(rbuf,s1,s2,s3,s4,s5)
char		rbuf[] ;
const char	s1[], s2[], s3[], s4[], s5[] ;
{
	const int	rlen = MAXPATHLEN ;
	return mknpath(rbuf,rlen,5,s1,s2,s3,s4,s5) ;
}
/* end subroutine (mkpath5) */


