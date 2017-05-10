/* sigdumper */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"sigdumpmsg.h"


/* local defines */

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#define	O_FLAGS		(O_NDELAY | O_WRONLY)


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* exported subroutines */


int sigdumper(cchar *fname,pid_t pid,cchar *s)
{
	int		rs ;
	char		msgbuf[MSGBUFLEN + 1] ;

	if ((rs = u_open(fname,O_FLAGS,0666)) >= 0) {
	    struct sigdumpmsg_request	m0 ;
	    const int	fd = rs ;
	    int		ml ;

	m0.tag = 0 ;
	m0.pid = pid ;
	strwcpy(m0.fname,s,(MAXNAMELEN - 1)) ;

	ml = sigdumpmsg_request(&m0,0,msgbuf,MSGBUFLEN) ;

	rs = uc_writen(fd,msgbuf,ml) ;

	u_close(fd) ;
	} /* end if (file) */

	if (rs > 0)
		sleep(1) ;

	return rs ;
}
/* end subroutine (sigdumper) */


