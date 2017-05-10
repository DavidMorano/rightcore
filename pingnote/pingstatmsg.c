/* pingstatmsg */

/* create and parse the internal messages */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2001-07-21, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This module contains the subroutines to make and parse the 
	PINGSTAT family of messages.


******************************************************************************/


#define	PINGSTATMSG_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<serialbuf.h>
#include	<stdorder.h>
#include	<localmisc.h>

#include	"pingstatmsg.h"


/* local defines */


/* external subroutines */


/* local structures */


/* local variables */


/* exported subroutines */


int pingstatmsg_update(sp,f,buf,buflen)
struct pingstatmsg_update	*sp ;
int			f ;
char			buf[] ;
int			buflen ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		len = 0 ;

	if ((rs = serialbuf_start(&msgbuf,buf,buflen)) >= 0) {
	uint	hdr ;

	if (f) { /* read */

	    serialbuf_ruint(&msgbuf,&hdr) ;
	    sp->msgtype = (hdr & 0xff) ;
	    sp->msglen = (hdr >> 8) ;

	    serialbuf_ruint(&msgbuf,&sp->timestamp) ;

	    serialbuf_rshort(&msgbuf,&sp->hostnamelen) ;

	    serialbuf_rstrw(&msgbuf,sp->hostname,MAXHOSTNAMELEN) ;

	} else { /* write */

	    len = MIN(sp->hostnamelen,MAXHOSTNAMELEN) ;
	    if (sp->hostnamelen < 0)
	        len = strnlen(sp->hostname,MAXHOSTNAMELEN) ;

	    sp->msgtype = pingstatmsgtype_update ;
	    hdr = sp->msgtype ;
	    serialbuf_wuint(&msgbuf,hdr) ;

	    serialbuf_wuint(&msgbuf,sp->timestamp) ;

	    serialbuf_wshort(&msgbuf,len) ;

	    serialbuf_wstrw(&msgbuf,sp->hostname,len) ;

	    if ((sp->msglen = serialbuf_getlen(&msgbuf)) > 0) {
	        hdr |= (sp->msglen << 8) ;
		stdorder_wuint(buf,hdr) ;
	    }

	} /* end if */

	    len = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = len ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (pingstatmsg_update) */


int pingstatmsg_uptime(sp,f,buf,buflen)
struct pingstatmsg_uptime	*sp ;
int			f ;
char			buf[] ;
int			buflen ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		len = 0 ;

	if ((rs = serialbuf_start(&msgbuf,buf,buflen)) >= 0) {
	uint	hdr ;

	if (f) { /* read */

	    serialbuf_ruint(&msgbuf,&hdr) ;
	    sp->msgtype = (hdr & 0xff) ;
	    sp->msglen = (hdr >> 8) ;

	    serialbuf_ruint(&msgbuf,&sp->timestamp) ;

	    serialbuf_ruint(&msgbuf,&sp->timechange) ;

	    serialbuf_ruint(&msgbuf,&sp->count) ;

	    serialbuf_rshort(&msgbuf,&sp->hostnamelen) ;

	    serialbuf_rstrw(&msgbuf,sp->hostname,MAXHOSTNAMELEN) ;

	} else { /* write */

	    len = MIN(sp->hostnamelen,MAXHOSTNAMELEN) ;
	    if (sp->hostnamelen < 0)
	        len = strnlen(sp->hostname,MAXHOSTNAMELEN) ;

	    sp->msgtype = pingstatmsgtype_uptime ;
	    hdr = sp->msgtype ;
	    serialbuf_wuint(&msgbuf,hdr) ;

	    serialbuf_wuint(&msgbuf,sp->timestamp) ;

	    serialbuf_wuint(&msgbuf,sp->timechange) ;

	    serialbuf_wuint(&msgbuf,sp->count) ;

	    serialbuf_wshort(&msgbuf,len) ;

	    serialbuf_wstrw(&msgbuf,sp->hostname,len) ;

	    if ((sp->msglen = serialbuf_getlen(&msgbuf)) > 0) {
	        hdr |= (sp->msglen << 8) ;
		stdorder_wuint(buf,hdr) ;
	    }

	} /* end if */

	    len = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = len ;
	} /* end if (serialbuf) */

#if	CF_DEBUGS
	debugprintf("pingstatmsg_update: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pingstatmsg_uptime) */


/* unknown message */
int pingstatmsg_unknown(sp,f,buf,buflen)
char			buf[] ;
int			buflen ;
int			f ;
struct pingstatmsg_unknown	*sp ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		len = 0 ;

	if ((rs = serialbuf_start(&msgbuf,buf,buflen)) >= 0) {
	uint	hdr ;

	if (f) { /* read */

	    serialbuf_ruint(&msgbuf,&hdr) ;
	    sp->msgtype = (hdr & 0xff) ;
	    sp->msglen = (hdr >> 8) ;

	} else { /* write */

	    sp->msgtype = pingstatmsgtype_unknown ;
	    hdr = sp->msgtype ;
	    serialbuf_wuint(&msgbuf,hdr) ;

	    if ((sp->msglen = serialbuf_getlen(&msgbuf)) > 0) {
	        hdr |= (sp->msglen << 8) ;
		stdorder_wuint(buf,hdr) ;
	    }

	} /* end if */

	    len = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = len ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (pingstatmsg_unknown) */


#ifdef	COMMENT

int pingstatmsg_msglen(type)
int	type ;
{
	int	rs ;

	switch (type) {

	case pingstatmsgtype_update:
	    rs = PINGSTATMSG_SUPDATE ;
	    break ;

	case pingstatmsgtype_uptime:
	    rs = PINGSTATMSG_SUPTIME ;
	    break ;

	default:
	    rs = SR_INVALID ;

	} /* end switch */

	return rs ;
}
/* end subroutine (pingstatmsg_msglen) */

#endif /* COMMENT */


