/* mkpath */

/* make a file path from components */


/* revision history:

	= 2001-12-03, David A­D­ Morano

	This code was born out of frustration with cleaning up bad legacy
	code (of which there is quite a bit -- like almost all of it).


*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine constructs a file path out of one or more path
	componets.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdarg.h>
#include	<string.h>

#include	<vsystem.h>


/* external subroutines */

extern int	mknpath(char *,int,int,...) ;


/* external variables */


/* local strutures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkpath1(ofname,s1)
char		ofname[] ;
const char	s1[] ;
{


	return mknpath(ofname,MAXPATHLEN,1,s1) ;
}
/* end subroutine (mkpath1) */


int mkpath2(ofname,s1,s2)
char		ofname[] ;
const char	s1[], s2[] ;
{


	return mknpath(ofname,MAXPATHLEN,2,s1,s2) ;
}
/* end subroutine (mkpath2) */


int mkpath3(ofname,s1,s2,s3)
char		ofname[] ;
const char	s1[], s2[], s3[] ;
{


	return mknpath(ofname,MAXPATHLEN,3,s1,s2,s3) ;
}
/* end subroutine (mkpath3) */


int mkpath4(ofname,s1,s2,s3,s4)
char		ofname[] ;
const char	s1[], s2[], s3[], s4[] ;
{


	return mknpath(ofname,MAXPATHLEN,4,s1,s2,s3,s4) ;
}
/* end subroutine (mkpath4) */


int mkpath5(ofname,s1,s2,s3,s4,s5)
char		ofname[] ;
const char	s1[], s2[], s3[], s4[], s5[] ;
{


	return mknpath(ofname,MAXPATHLEN,5,s1,s2,s3,s4,s5) ;
}
/* end subroutine (mkpath5) */



