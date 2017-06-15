/* u_recv */

/* receive a "message" */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* non-switchable debug printo-outs */


/* revision history:

	= 1086-03-26, David A­D­ Morano
        This was first written to give a little bit to UNIX® what we already
        have in our own circuit pack OSes!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	LIBU_MASTER	1

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<poll.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_NOMEM	60
#define	TO_NOSR		(5 * 60)
#define	TO_NOBUFS	(5 * 60)


/* external subroutines */

extern int	msleep(int) ;


/* forward references */


/* exported subroutines */


int u_recv(int fd,void *vbuf,int vlen,int flags)
{
	int		rs ;
	int		to_nomem = TO_NOMEM ;
	int		to_nosr = TO_NOSR ;
	int		to_nobufs = TO_NOBUFS ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("u_recv: ent fd=%d len=%d\n",fd,vlen) ;
#endif

	repeat {
	    if ((rs = recv(fd,vbuf,vlen,flags)) < 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs)  {
	        case SR_NOMEM:
	            if (to_nomem-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
#if	defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0)
	        case SR_NOSR:
	            if (to_nosr-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
#endif /* defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0) */
	        case SR_NOBUFS:
	            if (to_nobufs-- > 0) {
			msleep(1000) ;
		    } else {
			f_exit = TRUE ;
		    }
	            break ;
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (u_recv) */


