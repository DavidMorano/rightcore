/* muximsg */

/* create and parse the internal messages */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-07-21, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module contains the code to make and parse the internal messages
        that are used in this whole server facility.


*******************************************************************************/


#define	MUXIMSG_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<stdorder.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"muximsg.h"


/* local defines */


/* external subroutines */


/* local structures */


/* local variables */


/* exported subroutines */


/* general response message */
int muximsg_response(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_response	*sp ;
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

	        serialbuf_ruint(&msgbuf,&sp->pid) ;

	        serialbuf_ruchar(&msgbuf,&sp->rc) ;

	    } else { /* write */

	        sp->msgtype = muximsgtype_response ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuint(&msgbuf,sp->pid) ;

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
/* end subroutine (muximsg_response) */


int muximsg_noop(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_noop	*sp ;
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

	        sp->msgtype = muximsgtype_noop ;
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
/* end subroutine (muximsg_noop) */


int muximsg_passfd(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_passfd	*sp ;
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

	        serialbuf_rstrw(&msgbuf,sp->svc,MUXIMSG_SVCLEN) ;

	    } else { /* write */

	        sp->msgtype = muximsgtype_passfd ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wchar(&msgbuf,sp->msgtype) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wstrw(&msgbuf,sp->svc,MUXIMSG_SVCLEN) ;

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
/* end subroutine (muximsg_passfd) */


int muximsg_exit(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_exit	*sp ;
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

	        serialbuf_rstrw(&msgbuf,sp->reason,MUXIMSG_REASONLEN) ;

	    } else { /* write */

	        sp->msgtype = muximsgtype_exit ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wstrw(&msgbuf,sp->reason,MUXIMSG_REASONLEN) ;

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
/* end subroutine (muximsg_exit) */


int muximsg_getsysmisc(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_getsysmisc	*sp ;
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

	        sp->msgtype = muximsgtype_getsysmisc ;
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
/* end subroutine (muximsg_getsysmisc) */


int muximsg_sysmisc(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_sysmisc	*sp ;
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

	        serialbuf_ruint(&msgbuf,&sp->la_1min) ;

	        serialbuf_ruint(&msgbuf,&sp->la_5min) ;

	        serialbuf_ruint(&msgbuf,&sp->la_15min) ;

	        serialbuf_ruint(&msgbuf,&sp->boottime) ;

	        serialbuf_ruint(&msgbuf,&sp->nproc) ;

	        serialbuf_ruchar(&msgbuf,&sp->rc) ;

	    } else { /* write */

	        sp->msgtype = muximsgtype_sysmisc ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuint(&msgbuf,sp->la_1min) ;

	        serialbuf_wuint(&msgbuf,sp->la_5min) ;

	        serialbuf_wuint(&msgbuf,sp->la_15min) ;

	        serialbuf_wuint(&msgbuf,sp->boottime) ;

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
/* end subroutine (muximsg_sysmisc) */


int muximsg_getloadave(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_getloadave	*sp ;
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

	        sp->msgtype = muximsgtype_getloadave ;
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
/* end subroutine (muximsg_getloadve) */


int muximsg_loadave(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_loadave	*sp ;
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

	        serialbuf_ruint(&msgbuf,&sp->la_1min) ;

	        serialbuf_ruint(&msgbuf,&sp->la_5min) ;

	        serialbuf_ruint(&msgbuf,&sp->la_15min) ;

	        serialbuf_ruchar(&msgbuf,&sp->rc) ;

	    } else { /* write */

	        sp->msgtype = muximsgtype_loadave ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuint(&msgbuf,sp->la_1min) ;

	        serialbuf_wuint(&msgbuf,sp->la_5min) ;

	        serialbuf_wuint(&msgbuf,sp->la_15min) ;

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
/* end subroutine (muximsg_loadave) */


int muximsg_reploadave(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_reploadave	*sp ;
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

	        serialbuf_ruint(&msgbuf,&sp->utag) ;

	        serialbuf_ruint(&msgbuf,&sp->duration) ;

	        serialbuf_ruint(&msgbuf,&sp->interval) ;

	        serialbuf_rushort(&msgbuf,&sp->addrfamily) ;

	        serialbuf_rushort(&msgbuf,&sp->addrport) ;

	        serialbuf_ruint(&msgbuf,&sp->addrhost[0]) ;

	        serialbuf_ruint(&msgbuf,&sp->addrhost[1]) ;

	        serialbuf_ruint(&msgbuf,&sp->addrhost[2]) ;

	        serialbuf_ruint(&msgbuf,&sp->addrhost[3]) ;

	    } else { /* write */

	        sp->msgtype = muximsgtype_reploadave ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuint(&msgbuf,sp->utag) ;

	        serialbuf_wuint(&msgbuf,sp->duration) ;

	        serialbuf_wuint(&msgbuf,sp->interval) ;

	        serialbuf_wushort(&msgbuf,sp->addrfamily) ;

	        serialbuf_wushort(&msgbuf,sp->addrport) ;

	        serialbuf_wuint(&msgbuf,sp->addrhost[0]) ;

	        serialbuf_wuint(&msgbuf,sp->addrhost[1]) ;

	        serialbuf_wuint(&msgbuf,sp->addrhost[2]) ;

	        serialbuf_wuint(&msgbuf,sp->addrhost[4]) ;

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
/* end subroutine (muximsg_reploadave) */


int muximsg_getlistener(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_getlistener	*sp ;
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

	        serialbuf_ruint(&msgbuf,&sp->idx) ;

	    } else { /* write */

	        sp->msgtype = muximsgtype_getlistener ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuint(&msgbuf,sp->idx) ;

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
/* end subroutine (muximsg_getlistener) */


int muximsg_listener(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_listener	*sp ;
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

	        serialbuf_ruint(&msgbuf,&sp->idx) ;

	        serialbuf_ruint(&msgbuf,&sp->pid) ;

	        serialbuf_ruchar(&msgbuf,&sp->rc) ;

	        serialbuf_ruchar(&msgbuf,&sp->ls) ;

	        serialbuf_rstrw(&msgbuf,sp->name,MUXIMSG_LNAMELEN) ;

	        serialbuf_rstrw(&msgbuf,sp->addr,MUXIMSG_LADDRLEN) ;

	    } else { /* write */

	        sp->msgtype = muximsgtype_listener ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuint(&msgbuf,sp->idx) ;

	        serialbuf_wuint(&msgbuf,sp->pid) ;

	        serialbuf_wuchar(&msgbuf,sp->rc) ;

	        serialbuf_wuchar(&msgbuf,sp->ls) ;

	        serialbuf_wstrw(&msgbuf,sp->name,MUXIMSG_LNAMELEN) ;

	        serialbuf_wstrw(&msgbuf,sp->addr,MUXIMSG_LADDRLEN) ;

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
/* end subroutine (muximsg_listener) */


int muximsg_mark(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_mark	*sp ;
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

	        sp->msgtype = muximsgtype_mark ;
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
/* end subroutine (muximsg_mark) */


int muximsg_unknown(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_unknown	*sp ;
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

	    } else { /* write */

	        sp->msgtype = muximsgtype_unknown ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

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
/* end subroutine (muximsg_unknown) */


int muximsg_gethelp(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_gethelp	*sp ;
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

	        serialbuf_ruint(&msgbuf,&sp->idx) ;

	    } else { /* write */

	        sp->msgtype = muximsgtype_gethelp ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuint(&msgbuf,sp->idx) ;

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
/* end subroutine (muximsg_gethelp) */


int muximsg_help(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_help	*sp ;
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

	        serialbuf_ruint(&msgbuf,&sp->idx) ;

	        serialbuf_ruint(&msgbuf,&sp->pid) ;

	        serialbuf_ruchar(&msgbuf,&sp->rc) ;

	        serialbuf_rstrw(&msgbuf,sp->name,MUXIMSG_LNAMELEN) ;

	    } else { /* write */

	        sp->msgtype = muximsgtype_help ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuint(&msgbuf,sp->idx) ;

	        serialbuf_wuint(&msgbuf,sp->pid) ;

	        serialbuf_wuchar(&msgbuf,sp->rc) ;

	        serialbuf_wstrw(&msgbuf,sp->name,MUXIMSG_LNAMELEN) ;

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
/* end subroutine (muximsg_help) */


int muximsg_cmd(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct muximsg_cmd	*sp ;
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

	        serialbuf_rstrw(&msgbuf,sp->cmd,MUXIMSG_CMDLEN) ;

	    } else { /* write */

	        sp->msgtype = muximsgtype_cmd ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wstrw(&msgbuf,sp->cmd,MUXIMSG_CMDLEN) ;

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
/* end subroutine (muximsg_cmd) */


