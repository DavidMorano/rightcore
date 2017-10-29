/* u_unixtime */


#define	F_DEBUGS	0



#include	<sys/types.h>
#include	<sys/wait.h>
#include	<limits.h>
#include	<time.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	"vsystem.h"

#include	"misc.h"





int u_unixtime(rp)
LONG	*rp ;
{
	int	rs ;

	ULONG	result ;

	time_t	daytime ;


	if (rp == NULL)
	    return SR_FAULT ;

again:
	rs = SR_OK ;
	if (time(&daytime) < 0) 
		rs = (- errno) ;

	if (rs < 0)
	    return rs ;

	if (sizeof(LONG) != sizeof(long)) {

	    result = (ULONG) daytime ;
	    result &= ((ULONG) UINT_MAX) ;

	} else
	    result = (LONG) daytime ;

	*rp = (LONG) result ;
	return rs ;
}
/* end subroutine (u_unixtime) */



