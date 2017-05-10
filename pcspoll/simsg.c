/* simsg */

/* messages for SIMSG requests-responses */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-07-21, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module contains the subroutines to make and parse the SIMSG family
	of messages.


*******************************************************************************/


#define	SIMSG_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<stdorder.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"simsg.h"


/* local defines */


/* external subroutines */


/* local structures */


/* local variables */


/* exported subroutines */


int simsg_getsysmisc(struct simsg_getsysmisc *sp,int f,char *mbuf,int mlen)
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

	        serialbuf_ruint(&msgbuf,&sp->tag) ;

	    } else { /* write */

	        sp->msgtype = simsgtype_getsysmisc ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

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
/* end subroutine (simsg_getsysmisc) */


int simsg_sysmisc(struct simsg_sysmisc *sp,int f,char *mbuf,int mlen)
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

	        serialbuf_ruint(&msgbuf,&sp->tag) ;

	        serialbuf_ruint(&msgbuf,&sp->la1m) ;

	        serialbuf_ruint(&msgbuf,&sp->la5m) ;

	        serialbuf_ruint(&msgbuf,&sp->la15m) ;

	        serialbuf_ruint(&msgbuf,&sp->btime) ;

	        serialbuf_ruint(&msgbuf,&sp->ncpu) ;

	        serialbuf_ruint(&msgbuf,&sp->nproc) ;

	        serialbuf_ruchar(&msgbuf,&sp->rc) ;

	    } else { /* write */

	        sp->msgtype = simsgtype_sysmisc ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuint(&msgbuf,sp->la1m) ;

	        serialbuf_wuint(&msgbuf,sp->la5m) ;

	        serialbuf_wuint(&msgbuf,sp->la15m) ;

	        serialbuf_wuint(&msgbuf,sp->btime) ;

	        serialbuf_wuint(&msgbuf,sp->ncpu) ;

	        serialbuf_wuint(&msgbuf,sp->nproc) ;

	        serialbuf_wuchar(&msgbuf,sp->rc) ;

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
/* end subroutine (simsg_sysmisc) */


