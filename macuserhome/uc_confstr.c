/* uc_confstr */

/* interface component for UNIX® library-3c */
/* knock-off for the UNIX®-System version */


#define	CF_DEBUGS	0


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is NOT the same as the UNIX®-System version. This
        subroutine returns "OVERFLOW" when the user supplied buffer is not big
        enough to hold the associated value.

	Synopsis:

	int uc_confstr(request,ubuf,ulen)
	int	request ;
	char	ubuf[] ;
	int	ulen ;

	Arguments:

	request		configuration value to request
	ubuf		user supplied buffer to hold result
	ulen		length of user supplied value

	Returns:

	>0		valid and the value is returned with this length
	0		valid but there was no value associated
	<0		the requested configuration value was invalid


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<unistd.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */


/* external variables */


/* exported subroutines */


int uc_confstr(int request,char *ubuf,int ulen)
{
	int		rs = SR_OVERFLOW ;
	int		len = 0 ;

	if (ubuf == NULL) ulen = 0 ; /* indicate return length only */

	if ((ubuf == NULL) || (ulen >= 1)) {
	    size_t	result ;
	    size_t	llen = (ulen+1) ;
	    rs = SR_OK ;
	    errno = 0 ;
	    result = confstr(request,ubuf,llen) ;
	    if ((result == 0) && (errno != 0)) rs = (- errno) ;
	    len = (result-1) ;
	    if ((rs >= 0) && (result > llen)) rs = SR_OVERFLOW ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uc_confstr) */


