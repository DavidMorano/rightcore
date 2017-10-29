/* ut_look */


#define	CF_DEBUGS	0


#define	LIBUT_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<tiuser.h>
#include	<errno.h>

#include	<vsystem.h>





int ut_look(fd)
int	fd ;
{
	int	rs ;


again:
	rs = t_look(fd) ;

	if (rs < 0) {

	    switch (t_errno) {

	    case TBADF:
	        rs = SR_NOTSOCK ;
		break ;

	    case TSYSERR:
	        rs = (- errno) ;
		break ;

		default:
		rs = SR_NOANODE ;

	    } /* end switch */
	}

	if (rs < 0) {

	    switch (rs) {

	    case SR_NOMEM:
	    case SR_NOSR:
	        sleep(1) ;

	    case SR_INTR:
	    case SR_AGAIN:
	        goto again ;

	    } /* end switch */
	}

	return rs ;
}
/* end subroutine (ut_look) */



