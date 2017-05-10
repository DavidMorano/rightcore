/* pcs-msg */

/* messages for PCS requests-responses */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-07-21, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module contains the subroutines to make and parse the PCSMSG
	family of messages.


*******************************************************************************/


#define	PCSMSG_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>

#include	<vsystem.h>
#include	<stdorder.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"pcsmsg.h"


/* local defines */


/* external subroutines */


/* local structures */


/* local variables */


/* exported subroutines */


int pcsmsg_getstatus(struct pcsmsg_getstatus *sp,int f,char *mbuf,int mlen)
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
	        sp->msgtype = pcsmsgtype_getstatus ;
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
/* end subroutine (pcsmsg_getstatus) */


int pcsmsg_status(struct pcsmsg_status *sp,int f,char *mbuf,int mlen)
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
	        serialbuf_ruint(&m,&sp->queries) ;
	        serialbuf_ruchar(&m,&sp->rc) ;
	    } else { /* write */
	        sp->msgtype = pcsmsgtype_status ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;
	        serialbuf_wuint(&m,sp->tag) ;
	        serialbuf_wuint(&m,sp->pid) ;
	        serialbuf_wuint(&m,sp->queries) ;
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
/* end subroutine (pcsmsg_status) */


int pcsmsg_getval(struct pcsmsg_getval *sp,int f,char *mbuf,int mlen)
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
	        serialbuf_ruchar(&m,&sp->w) ;
	        serialbuf_rstrw(&m,sp->key,PCSMSG_KEYLEN) ;
	    } else { /* write */
	        sp->msgtype = pcsmsgtype_getval ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;
	        serialbuf_wuint(&m,sp->tag) ;
	        serialbuf_wuchar(&m,sp->w) ;
	        serialbuf_wstrw(&m,sp->key,PCSMSG_KEYLEN) ;
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
/* end subroutine (pcsmsg_getval) */


int pcsmsg_val(struct pcsmsg_val *sp,int f,char *mbuf,int mlen)
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
	        serialbuf_ruchar(&m,&sp->w) ;
	        serialbuf_ruchar(&m,&sp->rc) ;
	        serialbuf_ruchar(&m,&sp->vl) ;
	        serialbuf_rstrw(&m,sp->val,REALNAMELEN) ;
	    } else { /* write */
	        sp->msgtype = pcsmsgtype_val ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;
	        serialbuf_wuint(&m,sp->tag) ;
	        serialbuf_wuchar(&m,sp->w) ;
	        serialbuf_wuchar(&m,sp->rc) ;
	        serialbuf_wuchar(&m,sp->vl) ;
	        serialbuf_wstrw(&m,sp->val,REALNAMELEN) ;
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
/* end subroutine (pcsmsg_val) */


int pcsmsg_gethelp(struct pcsmsg_gethelp *sp,int f,char *mbuf,int mlen)
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
	        serialbuf_ruchar(&m,&sp->idx) ;
	    } else { /* write */
	        sp->msgtype = pcsmsgtype_gethelp ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;
	        serialbuf_wuint(&m,sp->tag) ;
	        serialbuf_wuchar(&m,sp->idx) ;
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
/* end subroutine (pcsmsg_gethelp) */


int pcsmsg_help(struct pcsmsg_help *sp,int f,char *mbuf,int mlen)
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
	        serialbuf_ruchar(&m,&sp->idx) ;
	        serialbuf_ruchar(&m,&sp->rc) ;
	        serialbuf_ruchar(&m,&sp->vl) ;
	        serialbuf_rstrw(&m,sp->val,REALNAMELEN) ;
	    } else { /* write */
	        sp->msgtype = pcsmsgtype_help ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;
	        serialbuf_wuint(&m,sp->tag) ;
	        serialbuf_wuchar(&m,sp->idx) ;
	        serialbuf_wuchar(&m,sp->rc) ;
	        serialbuf_wuchar(&m,sp->vl) ;
	        serialbuf_wstrw(&m,sp->val,REALNAMELEN) ;
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
/* end subroutine (pcsmsg_help) */


int pcsmsg_getname(struct pcsmsg_getname *sp,int f,char *mbuf,int mlen)
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
	        serialbuf_rstrw(&m,sp->un,USERNAMELEN) ;
	    } else { /* write */
	        sp->msgtype = pcsmsgtype_getname ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;
	        serialbuf_wuint(&m,sp->tag) ;
	        serialbuf_wstrw(&m,sp->un,USERNAMELEN) ;
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
/* end subroutine (pcsmsg_getname) */


int pcsmsg_name(struct pcsmsg_name *sp,int f,char *mbuf,int mlen)
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
	        serialbuf_ruchar(&m,&sp->rc) ;
	        serialbuf_rstrw(&m,sp->rn,REALNAMELEN) ;
	    } else { /* write */
	        sp->msgtype = pcsmsgtype_name ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;
	        serialbuf_wuint(&m,sp->tag) ;
	        serialbuf_wuchar(&m,sp->rc) ;
	        serialbuf_wstrw(&m,sp->rn,REALNAMELEN) ;
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
/* end subroutine (pcsmsg_name) */


int pcsmsg_getuser(struct pcsmsg_getuser *sp,int f,char *mbuf,int mlen)
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
	        serialbuf_rstrw(&m,sp->spec,REALNAMELEN) ;
	    } else { /* write */
	        sp->msgtype = pcsmsgtype_getuser ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;
	        serialbuf_wuint(&m,sp->tag) ;
	        serialbuf_wstrw(&m,sp->spec,REALNAMELEN) ;
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
/* end subroutine (pcsmsg_getuser) */


int pcsmsg_user(struct pcsmsg_user *sp,int f,char *mbuf,int mlen)
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
	        serialbuf_ruchar(&m,&sp->rc) ;
	        serialbuf_rstrw(&m,sp->un,USERNAMELEN) ;
	    } else { /* write */
	        sp->msgtype = pcsmsgtype_user ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;
	        serialbuf_wuint(&m,sp->tag) ;
	        serialbuf_wuchar(&m,sp->rc) ;
	        serialbuf_wstrw(&m,sp->un,USERNAMELEN) ;
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
/* end subroutine (pcsmsg_user) */


int pcsmsg_exit(struct pcsmsg_exit *sp,int f,char *mbuf,int mlen)
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
	        serialbuf_rstrw(&m,sp->reason,REALNAMELEN) ;
	    } else { /* write */
	        sp->msgtype = pcsmsgtype_exit ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;
	        serialbuf_wuint(&m,sp->tag) ;
	        serialbuf_wstrw(&m,sp->reason,REALNAMELEN) ;
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
/* end subroutine (pcsmsg_exit) */


int pcsmsg_mark(struct pcsmsg_mark *sp,int f,char *mbuf,int mlen)
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
	        sp->msgtype = pcsmsgtype_mark ;
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
/* end subroutine (pcsmsg_mark) */


int pcsmsg_ack(struct pcsmsg_ack *sp,int f,char *mbuf,int mlen)
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
	        serialbuf_ruchar(&m,&sp->rc) ;
	    } else { /* write */
	        sp->msgtype = pcsmsgtype_mark ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&m,hdr) ;
	        serialbuf_wuint(&m,sp->tag) ;
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
/* end subroutine (pcsmsg_ack) */


