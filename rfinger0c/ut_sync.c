/* ut_sync */


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



/* local defines */

#define	TO_RESTART	7			/* seconds */






int ut_sync(fd)
int	fd ;
{
	int	rs ;
	int	to_restart = TO_RESTART ;


again:
	rs = t_sync(fd) ;

	if (rs < 0) {

	    switch (t_errno) {

	    case TBADF:
	        rs = SR_NOTSOCK ;
	        break ;

	    case TSTATECHNG:
	        rs = SR_RESTART ;
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

	    case SR_RESTART:
	        if (--to_restart > 0) {

			sleep(1) ;

	            goto again ;
		}

	        break ;

	    } /* end switch */
	}

	return rs ;
}
/* end subroutine (ut_sync) */



