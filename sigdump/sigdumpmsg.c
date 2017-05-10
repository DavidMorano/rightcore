/* sigdumpmsg */

/* create and parse the internal messages */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-07-21, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module contains the subroutines to make and parse the SIGDUMPMSG
        family of messages.


*******************************************************************************/


#define	SIGDUMPMSG_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"sigdumpmsg.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		1024
#endif


/* external subroutines */


/* local structures */


/* local variables */


/* exported subroutines */


int sigdumpmsg_request(sp,f,mbuf,mlen)
struct sigdumpmsg_request	*sp ;
int		f ;
char		mbuf[] ;
int		mlen ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("sigdumpmsg_request: mlen=%d\n",mlen) ;
#endif

	if ((rs = serialbuf_start(&msgbuf,mbuf,mlen)) >= 0) {

	    if (f) { /* read */

	        serialbuf_ruchar(&msgbuf,&sp->type) ;

	        serialbuf_ruint(&msgbuf,&sp->tag) ;

	        serialbuf_ruint(&msgbuf,&sp->pid) ;

	        serialbuf_rstrw(&msgbuf,sp->fname,MAXNAMELEN) ;

	    } else { /* write */

	        sp->type = sigdumpmsgtype_request ;

	        serialbuf_wchar(&msgbuf,sp->type) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuint(&msgbuf,sp->pid) ;

	        serialbuf_wstrw(&msgbuf,sp->fname,MAXNAMELEN) ;

	    } /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (sigdumpmsg_request) */


