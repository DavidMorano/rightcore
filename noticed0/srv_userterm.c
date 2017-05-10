/* srv_userterm */

/* do the user terminal thing */


#define	CF_DEBUGS	0		/* compile-time */


/* revision history:

	= 1988-02-01, David A­D­ Morano

	This subroutine was originally written.


	= 1988-02-10, David A­D­ Morano

	This subroutine was modified to not write out anything to
	standard output if the access time of the associated terminal
	has not been changed in 10 minutes.


*/


/************************************************************************

	This subroutine writes receive notices on user's terminals.
	It writes to the last terminal (if there are any) that last
	accepted some input data.

	Synopsis:

	int srv_userterm(pip,mhp,mlen,rfd)
	struct proginfo	*pip ;
	struct msghdr	*mhp ;
	int		mlen ;
	int		rfd ;

	Arguments:

	- pip		program information pointer
	- mbuf		message buffer
	- mbuflen	message buffer length
	- rfd		FD to reply to

	Returns:

	0		succesful
	<0		bad


*************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<netorder.h>
#include	<sbuf.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#undef	BUFLEN
#define	BUFLEN		2050

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	USERNAMELEN
#define	USERNAMELEN	32
#endif

#ifndef	LINENAMELEN
#define	LINENAMELEN	MAXPATHLEN
#endif



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const char *services[] = {
	"notice",
	"random",
	NULL
} ;

#define	ARGOPT_NOTICE		0
#define	ARGOPT_RANDOM		1

static const char	eol[] = "\r\n" ;






int srv_userterm(pip,mhp,mlen,fd_ipc)
struct proginfo	*pip ;
struct msghdr	*mhp ;
int		mlen ;
int		fd_ipc ;
{
	struct iovec	*vecs ;

	time_t	daytime ;

	int	rs, i, len, sl, ulen, tlen, wlen ;
	int	rs1 ;
	int	nvecs ;
	int	netbuflen ;
	int	fd_termdev ;

	char	timebuf[TIMEBUFLEN] ;
	char	termdev[MAXPATHLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	*netbuf ;
	char	*username ;
	char	*tbuf ;
	char	*sp, *cp ;


	vecs = mhp->msg_iov ;
	nvecs = mhp->msg_iovlen ;

	netbuf = vecs[0].iov_base + 6 ;
	netbuflen = vecs[0].iov_len - 6 ;

	if (netbuflen <= 0)
	    return SR_OK ;

	i = 0 ;

	ulen = netbuf[i++] & 255 ;
	username = netbuf + i ;

	tlen = MIN((netbuf[i++] & 255),(mlen - 6)) ;
	tbuf = netbuf + i ;

	rs = getuserterm(username,termdev,MAXPATHLEN,&fd_termdev) ;

	if (rs >= 0) {

	    SBUF	out = obj_sbuf(buf,BUFLEN) ;


/* form the notice to write out */

	    sbuf_char(&out,'\r') ;

	    sp = tbuf ;
	    len = tlen ;
	    while ((cp = strnchr(sp,len,'\n')) != NULL) {

	        sl = cp - sp ;
	        sbuf_strw(&out,sp,sl) ;

	        sbuf_strw(&out,eol,2) ;

	        sp = cp + 1 ;
	        len -= sl ;

	    } /* end while */

	    if (len > 0) {

	        sl = len ;
	        sbuf_strw(&out,sp,sl) ;

	        sbuf_strw(&out,eol,2) ;

	    }

	    sbuf_char(&out,'\n') ;

	    wlen = sbuf_getlen(&out) ;

/* write the notice out */

	    rs = wlen ;
	    if (wlen > 0)
	        rs = u_write(fd_termdev,buf,wlen) ;

	    sbuf_free(&out) ;

		u_close(fd_termdev) ;

	} /* end if (we were a go) */

	rs1 = rs ;

/* do we need to send a reply ? */

	        if (mhp->msg_namelen > 0) {

/* send reply of service completion status */

	            i = 0 ;
	            i += netorder_wint(netbuf + i,rs1) ;

	            sl = netbuflen - i - 1 ;
	            cp = strwcpy(netbuf + i,termdev,sl) ;

	            *cp++ = '\0' ;
	            wlen = 6 + (cp - netbuf) ;
	            vecs[0].iov_len = wlen ;
	            rs = u_sendmsg(fd_ipc,mhp,0) ;

	        } /* end if (there was an address to reply to) */


	return rs ;
}
/* end subroutine (srv_userterm) */



