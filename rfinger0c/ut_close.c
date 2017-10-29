/* ut_close */


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

#define	TO_NOMEM	5





int ut_close(fd)
int	fd ;
{
	int	rs ;
	int	to_nomem = TO_NOMEM ;


again:
	rs = t_close(fd) ;

	if (rs < 0) {

		switch (t_errno) {

		case TBADF:
			rs = SR_NOTSOCK ;
			break ;

		case TSYSERR:
			rs = (- errno) ;
			break ;

		}
	}

	if (rs < 0) {

		switch (rs) {

		case SR_NOMEM:
		case SR_NOSR:
			to_nomem -= 1 ;
			if (to_nomem <= 0)
				break ;

			sleep(1) ;

		case SR_INTR:
		case SR_AGAIN:
			goto again ;

		} /* end switch */
	}

	return rs ;
}
/* end subroutine (ut_close) */



