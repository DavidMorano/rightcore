/* u_getcontext */

/* get the current process context */


#define	CF_DEBUGS	0		/* compile-time debugging */



#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/wait.h>
#include	<ucontext.h>
#include	<unistd.h>
#include	<poll.h>
#include	<errno.h>

#include	"vsystem.h"




int u_getcontext(ucp)
ucontext_t	*ucp ;
{
	int	rs ;


#if	CF_DEBUGS
	debugprintf("u_getcontext: ucp=%p\n",ucp) ;
#endif

	if ((rs = getcontext(ucp)) < 0) rs = (- errno) ;

#if	CF_DEBUGS
	debugprintf("u_getcontext: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (u_getcontext) */



