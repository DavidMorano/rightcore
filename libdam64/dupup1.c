/* dupup */

/* duplicate a file descriptor to be at some minimum number */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-10, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine will duplicate a file desciptor but the new
	file descriptor is guaranteed to be at or above some minimum
	number.  This sort of thing is required in a number of situations
	where file descriptors need to be juggled around (like around
	when programs do forks!).

	Synopsis:

	int dupup(fd,min)
	int	fd ;
	int	min ;

	Arguments:

	fd	file desciptor to duplicate
	min	minimum level to get the duplicate at

	Returns:

	<0	error
	>=0	the new file descriptor


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	MAXFDS		256


/* external subroutines */


/* external variables */


/* forward references */


/* exported subroutines */


int dupup(fd,min)
int	fd ;
int	min ;
{
	int	rs ;
	int	i, j ;
	int	fds[MAXFDS + 1] ;
	int	ufd ;


	if (fd < 0) return SR_INVALID ;

	if (min >= MAXFDS)
	    return SR_NOTSUP ;

	rs = u_dup(fd) ;
	ufd = rs ;
	if ((rs < 0) || (ufd >= min))
	    goto ret0 ;

	i = 0 ;
	while ((ufd >= 0) && (ufd < min) && (i < MAXFDS)) {

	    fds[i++] = ufd ;
	    rs = u_dup(ufd) ;
	    ufd = rs ;

	    if (rs < 0) break ;
	} /* end while */

	for (j = 0 ; j < i ; j += 1)
	    u_close(fds[j]) ;

	if ((rs >= 0) && (i == MAXFDS)) rs = SR_MFILE ;

ret0:
	return (rs >= 0) ? ufd : rs ;
}
/* end subroutine (dupup) */



