/* openportmsg */

/* message for 'openport(3dam)' support IPC */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-07-21, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module contains the code to make and parse the internal
        messages to support 'openport(3dam)' IPC.


*******************************************************************************/


#define	MUXIMSG_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<stdorder.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"openportmsg.h"


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* local structures */


/* local variables */


/* exported subroutines */


int openportmsg_request(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct openportmsg_request	*sp ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&msgbuf,mbuf,mlen)) >= 0) {
	    uint	hdr ;
	    int		size ;
	    uchar	*ubp = (uchar *) &sp->sa ;

	    if (f) { /* read */

	        serialbuf_ruint(&msgbuf,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_rint(&msgbuf,&sp->pf) ;

	        serialbuf_rint(&msgbuf,&sp->ptype) ;

	        serialbuf_rint(&msgbuf,&sp->proto) ;

	        size = sizeof(struct sockaddress_inet6) ;
	        serialbuf_rubuf(&msgbuf,ubp,size) ;

	        serialbuf_rstrw(&msgbuf,sp->username,USERNAMELEN) ;

	    } else { /* write */

	        sp->msgtype = openportmsgtype_request ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wint(&msgbuf,sp->pf) ;

	        serialbuf_wint(&msgbuf,sp->ptype) ;

	        serialbuf_wint(&msgbuf,sp->proto) ;

	        size = sizeof(struct sockaddress_inet6) ;
	        rs1 = serialbuf_wubuf(&msgbuf,ubp,size) ;

#if	CF_DEBUGS
	        debugprintf("openportmsg_request: serialbuf_wubuf() rs=%d\n",
	            rs1) ;
#endif

	        serialbuf_wstrw(&msgbuf,sp->username,USERNAMELEN) ;

	        if ((sp->msglen = serialbuf_getlen(&msgbuf)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (openportmsg_request) */


/* general response message */
int openportmsg_response(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct openportmsg_response	*sp ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&msgbuf,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&msgbuf,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_rint(&msgbuf,&sp->rs) ;

	    } else { /* write */

	        sp->msgtype = openportmsgtype_response ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wint(&msgbuf,sp->rs) ;

	        if ((sp->msglen = serialbuf_getlen(&msgbuf)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (openportmsg_response) */


