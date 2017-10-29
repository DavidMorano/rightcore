/* ut_bind */


#define	CF_DEBUGS	0


#define	LIBUT_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<tiuser.h>
#include	<errno.h>

#include	<vsystem.h>

#include	"localmisc.h"



/* time outs */

#define	TO_NOMEM	60
#define	TO_NOSR		(5 * 60)





int ut_bind(fd,req,ret)
int		fd ;
struct t_bind	*req, *ret ;
{
	int	rs ;
	int	to_nosr = 0 ;


again:
	rs = t_bind(fd,req,ret) ;

	if (rs < 0) {

		switch (t_errno) {

		case TACCES:
			rs = SR_ACCES ;
			break ;

		case TBADADDR:
			rs = SR_BADFMT;
			break ;

		case TBADF:
			rs = SR_NOTSOCK ;
			break ;

		case TBUFOVFLW:
			rs = SR_OVERFLOW;
			break ;

		case TNOADDR:
			rs = SR_ADDRNOTAVAIL;
			break ;

		case TOUTSTATE:
			rs = SR_PROTO;
			break ;

		case TSYSERR:
			rs = (- errno) ;
			break ;

		}
	}

	if (rs < 0) {

		switch (rs) {

		case SR_NOSR:
			to_nosr += 1 ;
			if (to_nosr > TO_NOSR) break ;

			sleep(1) ;

			goto again ;

		case SR_INTR:
			goto again ;

		} /* end switch */
	}

	return rs ;
}
/* end subroutine (ut_bind) */



