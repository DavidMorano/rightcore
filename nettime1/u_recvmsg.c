/* u_recvmsg */

/* receive a "message" */


#define	CF_DEBUGS	0		/* non-switchable debug printo-outs */


/* revision history:

	= 1986-03-26, David A­D­ Morano

	This was first written to give a little bit to UNIX what
	we have in our own circuit pack OSes!


*/



#define	LIBU_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<poll.h>

#include	<vsystem.h>



/* local defines */

#define	TO_NOMEM	60
#define	TO_NOSR		(5 * 60)
#define	TO_NOBUFS	(5 * 60)



/* external subroutines */


/* forward references */






int u_recvmsg(fd,msgp,flags)
int		fd ;
struct msghdr	*msgp ;
int		flags ;
{
	int	rs ;
	int	to_nomem = TO_NOMEM ;
	int	to_nosr = TO_NOSR ;
	int	to_nobufs = TO_NOBUFS ;


#if	CF_DEBUGS
	debugprintf("u_recvmsg: entered \n") ;
#endif

again:
	rs = recvmsg(fd,msgp,flags) ;

	if (rs < 0)
	    rs = (- errno) ;

#if	CF_DEBUGS
	debugprintf("u_recvmsg: rs=%d\n",rs) ;
#endif

	if (rs < 0) {

	    switch (rs)  {

	    case SR_INTR:
	        goto again ;

	    case SR_NOMEM:
	        if (to_nomem-- > 0)
	            goto retry ;

	        break ;

#if	defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0)

	    case SR_NOSR:
	        if (to_nosr-- > 0)
	            goto retry ;

	        break ;

#endif /* defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0) */

	    case SR_NOBUFS:
	        if (to_nobufs-- > 0)
	            goto retry ;

	        break ;

	    } /* end switch */

	} /* end if */

	return rs ;

retry:
	sleep(1) ;

	goto again ;
}
/* end subroutine (u_recvmsg) */



