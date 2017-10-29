/* ut_free */

/* TLI free */


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





int ut_free(p,stype)
void	*p ;
int	stype ;
{
	int	rs = SR_OK ;


	if (p == NULL)
		return SR_FAULT ;

again:
	rs = t_free(p,stype) ;

	if (rs < 0) {

	    switch (t_errno) {

	    case TSYSERR:
	        rs = (- errno) ;
		break ;

		default:
		rs = SR_NOANODE ;

	    } /* end switch */

	} /* end if */

	if (rs < 0) {

	    switch (rs) {

	    case SR_NOMEM:
	    case SR_NOSR:
	        sleep(1) ;

	    case SR_INTR:
	    case SR_AGAIN:
	        goto again ;

	    } /* end switch */

	} /* end if */

	return rs ;
}
/* end subroutine (ut_free) */



