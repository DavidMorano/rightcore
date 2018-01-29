/* mknpathw */

/* make a file path from components */


#define	CF_DEBUGS	0		/* compile-time debugging */


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


/* external subroutines */

extern int	mknpathw(char *,int,int,...) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local strutures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkpath1w(rbuf,s1,s1len)
char		rbuf[] ;
const char	s1[] ;
int		s1len ;
{
	const int	rlen = MAXPATHLEN ;
	return mknpathw(rbuf,rlen,1,s1,s1len) ;
}
/* end subroutine (mkpath1w) */


int mkpath2w(rbuf,s1,s2,s2len)
char		rbuf[] ;
const char	s1[], s2[] ;
int		s2len ;
{
	const int	rlen = MAXPATHLEN ;
	return mknpathw(rbuf,rlen,2,s1,s2,s2len) ;
}
/* end subroutine (mkpath2w) */


int mkpath3w(rbuf,s1,s2,s3,s3len)
char		rbuf[] ;
const char	s1[], s2[], s3[] ;
int		s3len ;
{
	const int	rlen = MAXPATHLEN ;
	return mknpathw(rbuf,rlen,3,s1,s2,s3,s3len) ;
}
/* end subroutine (mkpath3w) */


int mkpath4w(rbuf,s1,s2,s3,s4,s4len)
char		rbuf[] ;
const char	s1[], s2[], s3[], s4[] ;
int		s4len ;
{
	const int	rlen = MAXPATHLEN ;
	return mknpathw(rbuf,rlen,4,s1,s2,s3,s4,s4len) ;
}
/* end subroutine (mkpath4w) */


int mkpath5w(rbuf,s1,s2,s3,s4,s5,s5len)
char		rbuf[] ;
const char	s1[], s2[], s3[], s4[], s5[] ;
int		s5len ;
{
	const int	rlen = MAXPATHLEN ;
	return mknpathw(rbuf,rlen,5,s1,s2,s3,s4,s5,s5len) ;
}
/* end subroutine (mkpath5w) */


