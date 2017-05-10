/* mcmsg */

/* create and parse mail-cluster messages */


#define	CF_DEBUGS	0


/* revision history:

	= 1999-07-21, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This module contains the subroutines to make and parse the MCMSG family
	of messages.


******************************************************************************/


#define	MCMSG_MASTER	0


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
#include	<stdorder.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"mcmsg.h"


/* local defines */


/* external subroutines */


/* local structures */


/* local variables */


/* exported subroutines */


int mcmsg_request(sp,f,buf,buflen)
char			buf[] ;
int			buflen ;
int			f ;
struct mcmsg_request	*sp ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&msgbuf,buf,buflen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&msgbuf,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruchar(&msgbuf,&sp->seq) ;

	        serialbuf_ruint(&msgbuf,&sp->tag) ;

	        serialbuf_ruint(&msgbuf,&sp->timestamp) ;

	        serialbuf_rstrw(&msgbuf,sp->cluster,MCMSG_LCLUSTER) ;

	        serialbuf_rstrw(&msgbuf,sp->mailuser,MCMSG_LMAILUSER) ;

	    } else { /* write */

	        sp->msgtype = mcmsgtype_request ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wchar(&msgbuf,sp->seq) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuint(&msgbuf,sp->timestamp) ;

	        serialbuf_wstrw(&msgbuf,sp->cluster,MCMSG_LCLUSTER) ;

	        serialbuf_wstrw(&msgbuf,sp->mailuser,MCMSG_LMAILUSER) ;

	        if ((sp->msglen = serialbuf_getlen(&msgbuf)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(buf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (mcmsg_request) */


int mcmsg_report(sp,f,buf,buflen)
char			buf[] ;
int			buflen ;
int			f ;
struct mcmsg_report	*sp ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&msgbuf,buf,buflen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&msgbuf,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruchar(&msgbuf,&sp->seq) ;

	        serialbuf_ruint(&msgbuf,&sp->tag) ;

	        serialbuf_ruint(&msgbuf,&sp->timestamp) ;

	        serialbuf_ruint(&msgbuf,&sp->mlen) ;

	        serialbuf_rstrw(&msgbuf,sp->cluster,MCMSG_LCLUSTER) ;

	        serialbuf_rstrw(&msgbuf,sp->mailuser,MCMSG_LMAILUSER) ;

	        serialbuf_rstrw(&msgbuf,sp->msgid,MCMSG_LMSGID) ;

	        serialbuf_rstrw(&msgbuf,sp->from,MCMSG_LFROM) ;

	        serialbuf_ruchar(&msgbuf,&sp->flags) ;

	        serialbuf_ruchar(&msgbuf,&sp->rc) ;

	    } else { /* write */

	        sp->msgtype = mcmsgtype_report ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wchar(&msgbuf,sp->seq) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuint(&msgbuf,sp->timestamp) ;

	        serialbuf_wuint(&msgbuf,sp->mlen) ;

	        serialbuf_wstrw(&msgbuf,sp->cluster,MCMSG_LCLUSTER) ;

	        serialbuf_wstrw(&msgbuf,sp->mailuser,MCMSG_LMAILUSER) ;

	        serialbuf_wstrw(&msgbuf,sp->msgid,MCMSG_LMSGID) ;

	        serialbuf_wstrw(&msgbuf,sp->from,MCMSG_LFROM) ;

	        serialbuf_wchar(&msgbuf,sp->flags) ;

	        serialbuf_wchar(&msgbuf,sp->rc) ;

	        if ((sp->msglen = serialbuf_getlen(&msgbuf)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(buf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (mcmsg_report) */


int mcmsg_ack(sp,f,buf,buflen)
char			buf[] ;
int			buflen ;
int			f ;
struct mcmsg_ack	*sp ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&msgbuf,buf,buflen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&msgbuf,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruchar(&msgbuf,&sp->seq) ;

	        serialbuf_ruint(&msgbuf,&sp->tag) ;

	        serialbuf_ruchar(&msgbuf,&sp->rc) ;

	    } else { /* write */

	        sp->msgtype = mcmsgtype_ack ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wchar(&msgbuf,sp->seq) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wchar(&msgbuf,sp->rc) ;

	        if ((sp->msglen = serialbuf_getlen(&msgbuf)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(buf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (mcmsg_ack) */


