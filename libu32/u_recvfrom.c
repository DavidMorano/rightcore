/* u_recvfrom */

/* receive a "message" */
/* translation layer interface for UNIX® equivalents */


#define	CF_DEBUGS	0		/* non-switchable debug printo-outs */
#define	CF_XNET		1		/* use 'xnet' library */


/* revision history:

	= 1086-03-26, David A­D­ Morano
        This was first written to give a little bit to UNIX® what we have in our
        own circuit pack OSes!

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

#if	CF_XNET
#define	SALEN_T		socklen_t
#else
#define	SALEN_T		int
#endif

#define	TO_NOMEM	60
#define	TO_NOSR		(5 * 60)
#define	TO_NOBUFS	(5 * 60)


/* external subroutines */

extern int	msleep(int) ;


/* forward references */


/* exported subroutines */


int u_recvfrom(fd,dbuf,dlen,flags,asap,asalp)
int		fd ;
void		*dbuf ;
int		dlen ;
int		flags ;
void		*asap ;
int		*asalp ;
{
	struct sockaddr	*sap = (struct sockaddr *) asap ;
	SALEN_T		sal ;
	int		rs ;
	int		to_nomem = TO_NOMEM ;
	int		to_nosr = TO_NOSR ;
	int		to_nobufs = TO_NOBUFS ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("u_recvfrom: ent fd=%d len=%d\n",fd,buflen) ;
#endif

	repeat {
	    if (sap != NULL) sal = (SALEN_T) *asalp ;
	    if ((rs = recvfrom(fd,dbuf,dlen,flags,sap,&sal)) < 0) {
		rs = (- errno) ;
	    }
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

	if (asalp != NULL) *asalp = (int) sal ;

#if	CF_DEBUGS
	debugprintf("u_recvfrom: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (u_recvfrom) */


