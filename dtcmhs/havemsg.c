/* havemsg */

/* create and parse mail-calendar messages */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-07-21, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This module contains the subroutines to make and parse the HAVEMSG
	family of messages.


******************************************************************************/


#define	HAVEMSG_MASTER	0

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

#include	"havemsg.h"


/* local defines */


/* external subroutines */


/* local structures */


/* local variables */


/* exported subroutines */


int havemsg_request(buf,buflen,f,sp)
char			buf[] ;
int			buflen ;
int			f ;
struct havemsg_request	*sp ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&msgbuf,(char *) buf,buflen)) >= 0) {

	if (f) { /* read */

	    serialbuf_ruchar(&msgbuf,&sp->type) ;

	    serialbuf_ruchar(&msgbuf,&sp->seq) ;

	    serialbuf_ruint(&msgbuf,&sp->tag) ;

	    serialbuf_ruint(&msgbuf,&sp->timestamp) ;

	    serialbuf_rstrw(&msgbuf,sp->calendar,HAVEMSG_LCALENDAR) ;

	} else { /* write */

	    sp->type = havemsgtype_request ;

	    serialbuf_wchar(&msgbuf,sp->type) ;

	    serialbuf_wchar(&msgbuf,sp->seq) ;

	    serialbuf_wuint(&msgbuf,sp->tag) ;

	    serialbuf_wuint(&msgbuf,sp->timestamp) ;

	    serialbuf_wstrw(&msgbuf,sp->calendar,HAVEMSG_LCALENDAR) ;

	} /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (havemsg_request) */


int havemsg_report(buf,buflen,f,sp)
char			buf[] ;
int			buflen ;
int			f ;
struct havemsg_report	*sp ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

	if ((serialbuf_start(&msgbuf,(char *) buf,buflen)) >= 0) {

	if (f) { /* read */

	    serialbuf_ruchar(&msgbuf,&sp->type) ;

	    serialbuf_ruchar(&msgbuf,&sp->seq) ;

	    serialbuf_ruint(&msgbuf,&sp->tag) ;

	    serialbuf_ruint(&msgbuf,&sp->timestamp) ;

	    serialbuf_rstrw(&msgbuf,sp->calendar,HAVEMSG_LCALENDAR) ;

	    serialbuf_ruchar(&msgbuf,&sp->rc) ;

	} else { /* write */

	    sp->type = havemsgtype_request ;

	    serialbuf_wchar(&msgbuf,sp->type) ;

	    serialbuf_wchar(&msgbuf,sp->seq) ;

	    serialbuf_wuint(&msgbuf,sp->tag) ;

	    serialbuf_wuint(&msgbuf,sp->timestamp) ;

	    serialbuf_wstrw(&msgbuf,sp->calendar,HAVEMSG_LCALENDAR) ;

	    serialbuf_wchar(&msgbuf,sp->rc) ;

	} /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (havemsg_request) */


