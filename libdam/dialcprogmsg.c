/* dialcprogmsg */

/* create and parse the internal messages */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2003-07-21, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module contains the code to make and parse the internal messages
        that are used in this whole server facility.


*******************************************************************************/


#define	DIALCPROGMSG_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<sockaddress.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"dialcprogmsg.h"


/* local defines */


/* external subroutines */


/* local structures */


/* local variables */


/* exported subroutines */


int dialcprogmsg_end(mbuf,mlen,f,sp)
char		mbuf[] ;
int		mlen ;
int		f ;
struct dialcprogmsg_end		*sp ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&msgbuf,mbuf,mlen)) >= 0) {

	    if (f) { /* read */

	        serialbuf_ruchar(&msgbuf,&sp->type) ;

	        serialbuf_rushort(&msgbuf,&sp->len) ;

	        serialbuf_rushort(&msgbuf,&sp->flags) ;

	        serialbuf_rint(&msgbuf,&sp->opts) ;

	    } else { /* write */

	        sp->type = dialcprogmsgtype_end ;

	        serialbuf_wchar(&msgbuf,sp->type) ;

	        serialbuf_wushort(&msgbuf,4) ;

	        serialbuf_wushort(&msgbuf,sp->flags) ;

	        serialbuf_wint(&msgbuf,sp->opts) ;

	    } /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (dialcprogmsg_end) */


int dialcprogmsg_light(mbuf,mlen,f,sp)
char		mbuf[] ;
int		mlen ;
int		f ;
struct dialcprogmsg_light	*sp ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&msgbuf,mbuf,mlen)) >= 0) {
	    ushort	usw ;

	    if (f) { /* read */

	        serialbuf_ruchar(&msgbuf,&sp->type) ;

	        serialbuf_rushort(&msgbuf,&sp->len) ;

	        serialbuf_rushort(&msgbuf,&sp->salen1) ;

	        serialbuf_rushort(&msgbuf,&sp->salen2) ;

	        serialbuf_robj(&msgbuf,&sp->saout,(int) sp->salen1) ;

	        serialbuf_robj(&msgbuf,&sp->saerr,(int) sp->salen2) ;

	    } else { /* write */

	        sp->type = dialcprogmsgtype_light ;
	        serialbuf_wchar(&msgbuf,sp->type) ;

	        usw = sp->salen1 + sp->salen2 + (2 * sizeof(ushort)) ;
	        serialbuf_wushort(&msgbuf,usw) ;

	        serialbuf_wushort(&msgbuf,sp->salen1) ;

	        serialbuf_wushort(&msgbuf,sp->salen2) ;

	        serialbuf_wobj(&msgbuf,&sp->saout,(int) sp->salen1) ;

	        serialbuf_wobj(&msgbuf,&sp->saerr,(int) sp->salen2) ;

	    } /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (dialcprogmsg_light) */


