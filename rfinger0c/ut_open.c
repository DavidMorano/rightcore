/* ut_open */

/* UNIX® XTI subroutine */


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


/* exported subroutines */


int ut_open(fname,f,ip)
char		fname[] ;
int		f ;
struct t_info	*ip ;
{
	int	rs ;


again:
	rs = t_open(fname,f,ip) ;

	if (rs < 0) {
	    switch (t_errno) {
	    case TBADFLAG:
	        rs = SR_INVALID ;
		break ;
	    case TSYSERR:
	        rs = (- errno) ;
		break ;
	    } /* end switch */
	}

	if (rs < 0) {
	    switch (rs) {
	    case SR_NOMEM:
	    case SR_NOSR:
	        sleep(1) ;
/* FALLTHROUGH */
	    case SR_INTR:
	    case SR_AGAIN:
	        goto again ;
	    } /* end switch */
	}

	return rs ;
}
/* end subroutine (ut_open) */



