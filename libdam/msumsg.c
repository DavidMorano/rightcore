/* msu-msg */

/* messages for MSU requests-responses */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-07-21, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module contains the subroutines to make and parse the MSUMSG
	family of messages.


*******************************************************************************/


#define	MSUMSG_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<stdorder.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"msumsg.h"


/* local defines */


/* external subroutines */


/* local structures */


/* local variables */


/* exported subroutines */


int msumsg_getstatus(struct msumsg_getstatus *sp,int f,char *mbuf,int mlen)
{
	SERIALBUF	m ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&m,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&m,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&m,&sp->tag) ;

	    } else { /* write */

	        sp->msgtype = msumsgtype_getstatus ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;

	        serialbuf_wuint(&m,sp->tag) ;

	        if ((sp->msglen = serialbuf_getlen(&m)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (msumsg_getstatus) */


int msumsg_status(struct msumsg_status *sp,int f,char *mbuf,int mlen)
{
	SERIALBUF	m ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&m,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&m,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&m,&sp->tag) ;

	        serialbuf_ruint(&m,&sp->pid) ;

	        serialbuf_ruchar(&m,&sp->rc) ;

	    } else { /* write */

	        sp->msgtype = msumsgtype_status ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;

	        serialbuf_wuint(&m,sp->tag) ;

	        serialbuf_wuint(&m,sp->pid) ;

	        serialbuf_wuchar(&m,sp->rc) ;

	        if ((sp->msglen = serialbuf_getlen(&m)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (msumsg_status) */


int msumsg_getsysmisc(struct msumsg_getsysmisc *sp,int f,char *mbuf,int mlen)
{
	SERIALBUF	m ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&m,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&m,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&m,&sp->tag) ;

	    } else { /* write */

	        sp->msgtype = msumsgtype_getsysmisc ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;

	        serialbuf_wuint(&m,sp->tag) ;

	        if ((sp->msglen = serialbuf_getlen(&m)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (msumsg_getsysmisc) */


int msumsg_sysmisc(struct msumsg_sysmisc *sp,int f,char *mbuf,int mlen)
{
	SERIALBUF	m ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&m,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&m,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&m,&sp->tag) ;

	        serialbuf_ruint(&m,&sp->pid) ;

	        serialbuf_ruint(&m,&sp->utime) ;

	        serialbuf_ruint(&m,&sp->btime) ;

	        serialbuf_ruint(&m,&sp->ncpu) ;

	        serialbuf_ruint(&m,&sp->nproc) ;

	        serialbuf_ruinta(&m,sp->la,3) ;

	        serialbuf_ruchar(&m,&sp->rc) ;

	    } else { /* write */

	        sp->msgtype = msumsgtype_sysmisc ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;

	        serialbuf_wuint(&m,sp->tag) ;

	        serialbuf_wuint(&m,sp->pid) ;

	        serialbuf_wuint(&m,sp->utime) ;

	        serialbuf_wuint(&m,sp->btime) ;

	        serialbuf_wuint(&m,sp->ncpu) ;

	        serialbuf_wuint(&m,sp->nproc) ;

	        serialbuf_wuinta(&m,sp->la,3) ;

	        serialbuf_wuchar(&m,sp->rc) ;

	        if ((sp->msglen = serialbuf_getlen(&m)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (msumsg_sysmisc) */


int msumsg_exit(struct msumsg_exit *sp,int f,char *mbuf,int mlen)
{
	SERIALBUF	m ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&m,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&m,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&m,&sp->tag) ;

	    } else { /* write */

	        sp->msgtype = msumsgtype_exit ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;

	        serialbuf_wuint(&m,sp->tag) ;

	        if ((sp->msglen = serialbuf_getlen(&m)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (msumsg_exit) */


int msumsg_mark(struct msumsg_mark *sp,int f,char *mbuf,int mlen)
{
	SERIALBUF	m ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&m,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&m,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&m,&sp->tag) ;

	    } else { /* write */

	        sp->msgtype = msumsgtype_mark ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;

	        serialbuf_wuint(&m,sp->tag) ;

	        if ((sp->msglen = serialbuf_getlen(&m)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (msumsg_mark) */


int msumsg_report(struct msumsg_report *sp,int f,char *mbuf,int mlen)
{
	SERIALBUF	m ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&m,mbuf,mlen)) >= 0) {
	    uint	hdr ;

	    if (f) { /* read */

	        serialbuf_ruint(&m,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&m,&sp->tag) ;

	    } else { /* write */

	        sp->msgtype = msumsgtype_report ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;

	        serialbuf_wuint(&m,sp->tag) ;

	        if ((sp->msglen = serialbuf_getlen(&m)) > 0) {
	            hdr |= (sp->msglen << 8) ;
	            stdorder_wuint(mbuf,hdr) ;
	        }

	    } /* end if */

	    rs1 = serialbuf_finish(&m) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (msumsg_report) */


