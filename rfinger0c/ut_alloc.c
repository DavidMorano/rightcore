/* ut_alloc */

/* TLI allocate */


#define	CF_DEBUGS	0		/* compile-time debugging */


#define	LIBUT_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<tiuser.h>
#include	<errno.h>

#include	<vsystem.h>





int ut_alloc(fd,stype,fields,rpp)
int	fd ;
int	stype ;
int	fields ;
void	**rpp ;
{
	int	rs = SR_OK ;

	void	*p ;


	if (rpp == NULL)
		return SR_FAULT ;

again:
	p = t_alloc(fd,stype,fields) ;

	if (p == NULL) {

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

	*rpp = ((rs >= 0) ? p : NULL) ;

	return rs ;
}
/* end subroutine (ut_alloc) */



