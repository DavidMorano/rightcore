/* ulmsg */

/* User-Location-Manager - create and parse the UL messages */


#define	CF_DEBUGS	0


/* revision history:

	= 1999-07-21, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This module contains the subroutines to make and parse the 
	User-Location-Manager (ULMSG) family of messages.


******************************************************************************/


#define	ULMSG_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<stdorder.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"ulmsg.h"


/* local defines */


/* external subroutines */


/* local structures */


/* local variables */


/* exported subroutines */


int ulmsg_update(sp,f,buf,buflen)
char			buf[] ;
int			buflen ;
int			f ;
struct ulmsg_update	*sp ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("ulmsg_update: buflen=%d\n",buflen) ;
#endif

	if ((rs = serialbuf_start(&msgbuf,buf,buflen)) >= 0) {
	uint	hdr ;

	if (f) { /* read */

	    serialbuf_ruint(&msgbuf,&hdr) ;
	    sp->msgtype = (hdr & 0xff) ;
	    sp->msglen = (hdr >> 8) ;

	    serialbuf_ruint(&msgbuf,&sp->ts) ;

	    serialbuf_ruint(&msgbuf,&sp->ta) ;

	    serialbuf_ruint(&msgbuf,&sp->tm) ;

	    serialbuf_ruint(&msgbuf,&sp->tu_sec) ;

	    serialbuf_ruint(&msgbuf,&sp->tu_usec) ;

	    serialbuf_ruint(&msgbuf,&sp->pid) ;

	    serialbuf_ruint(&msgbuf,&sp->sid) ;

	    serialbuf_rushort(&msgbuf,&sp->termination) ;

	    serialbuf_rushort(&msgbuf,&sp->exit) ;

	    serialbuf_rshort(&msgbuf,&sp->utype) ;

	    serialbuf_ruchar(&msgbuf,&sp->stype) ;

	    serialbuf_ruchar(&msgbuf,&sp->status) ;

	    serialbuf_rstrw(&msgbuf,sp->username,
		ULMSG_LUSERNAME) ;

	    serialbuf_rstrw(&msgbuf,sp->line,
		ULMSG_LLINE) ;

	    serialbuf_rstrw(&msgbuf,sp->node,
		ULMSG_LNODE) ;

	    serialbuf_rstrw(&msgbuf,sp->cluster,
		ULMSG_LCLUSTER) ;

	    serialbuf_rstrw(&msgbuf,sp->domain,
		ULMSG_LDOMAIN) ;

	    serialbuf_rstrw(&msgbuf,sp->host,
		ULMSG_LHOST) ;

	    serialbuf_rstrw(&msgbuf,sp->termtype,
		ULMSG_LTERMTYPE) ;

	    serialbuf_rstrw(&msgbuf,sp->id,
		ULMSG_LID) ;

	} else { /* write */

	    sp->msgtype = ulmsgtype_update ;
	    hdr = sp->msgtype ;
	    serialbuf_wuint(&msgbuf,hdr) ;

	    serialbuf_wuint(&msgbuf,sp->ts) ;

	    serialbuf_wuint(&msgbuf,sp->ta) ;

	    serialbuf_wuint(&msgbuf,sp->tm) ;

	    serialbuf_wuint(&msgbuf,sp->tu_sec) ;

	    serialbuf_wuint(&msgbuf,sp->tu_usec) ;

	    serialbuf_wuint(&msgbuf,sp->pid) ;

	    serialbuf_wuint(&msgbuf,sp->sid) ;

	    serialbuf_wushort(&msgbuf,sp->termination) ;

	    serialbuf_wushort(&msgbuf,sp->exit) ;

	    serialbuf_wshort(&msgbuf,sp->utype) ;

	    serialbuf_wchar(&msgbuf,sp->stype) ;

	    serialbuf_wchar(&msgbuf,sp->status) ;

	    serialbuf_wstrw(&msgbuf,sp->username,
		ULMSG_LUSERNAME) ;

	    serialbuf_wstrw(&msgbuf,sp->line,
		ULMSG_LLINE) ;

	    serialbuf_wstrw(&msgbuf,sp->node,
		ULMSG_LNODE) ;

	    serialbuf_wstrw(&msgbuf,sp->cluster,
		ULMSG_LCLUSTER) ;

	    serialbuf_wstrw(&msgbuf,sp->domain,
		ULMSG_LDOMAIN) ;

	    serialbuf_wstrw(&msgbuf,sp->host,
		ULMSG_LHOST) ;

	    serialbuf_wstrw(&msgbuf,sp->termtype,
		ULMSG_LTERMTYPE) ;

	    serialbuf_wstrw(&msgbuf,sp->id,
		ULMSG_LID) ;

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
/* end subroutine (ulmsg_update) */


int ulmsg_request(sp,f,buf,buflen)
char			buf[] ;
int			buflen ;
int			f ;
struct ulmsg_request	*sp ;
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

	    serialbuf_ruint(&msgbuf,&sp->tag) ;

	    serialbuf_ruint(&msgbuf,&sp->ts) ;

	    serialbuf_ruint(&msgbuf,&sp->expire) ;

	    serialbuf_ruchar(&msgbuf,&sp->stype) ;

	    serialbuf_rstrw(&msgbuf,sp->username,
		ULMSG_LUSERNAME) ;

	    serialbuf_rstrw(&msgbuf,sp->line,
		ULMSG_LLINE) ;

	    serialbuf_rstrw(&msgbuf,sp->node,
		ULMSG_LNODE) ;

	    serialbuf_rstrw(&msgbuf,sp->cluster,
		ULMSG_LCLUSTER) ;

	    serialbuf_rstrw(&msgbuf,sp->domain,
		ULMSG_LDOMAIN) ;

	    serialbuf_rstrw(&msgbuf,sp->host,
		ULMSG_LHOST) ;

	    serialbuf_rstrw(&msgbuf,sp->termtype,
		ULMSG_LTERMTYPE) ;

	    serialbuf_rstrw(&msgbuf,sp->id,
		ULMSG_LID) ;

	} else { /* write */

	    sp->msgtype = ulmsgtype_request ;
	    hdr = sp->msgtype ;
	    serialbuf_wuint(&msgbuf,hdr) ;

	    serialbuf_wuint(&msgbuf,sp->tag) ;

	    serialbuf_wuint(&msgbuf,sp->ts) ;

	    serialbuf_wuint(&msgbuf,sp->expire) ;

	    serialbuf_wchar(&msgbuf,sp->stype) ;

	    serialbuf_wstrw(&msgbuf,sp->username,
		ULMSG_LUSERNAME) ;

	    serialbuf_wstrw(&msgbuf,sp->line,
		ULMSG_LLINE) ;

	    serialbuf_wstrw(&msgbuf,sp->node,
		ULMSG_LNODE) ;

	    serialbuf_wstrw(&msgbuf,sp->cluster,
		ULMSG_LCLUSTER) ;

	    serialbuf_wstrw(&msgbuf,sp->domain,
		ULMSG_LDOMAIN) ;

	    serialbuf_wstrw(&msgbuf,sp->host,
		ULMSG_LHOST) ;

	    serialbuf_wstrw(&msgbuf,sp->termtype,
		ULMSG_LTERMTYPE) ;

	    serialbuf_wstrw(&msgbuf,sp->id,
		ULMSG_LID) ;

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
/* end subroutine (ulmsg_request) */


int ulmsg_response(sp,f,buf,buflen)
char			buf[] ;
int			buflen ;
int			f ;
struct ulmsg_response	*sp ;
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

	    serialbuf_ruint(&msgbuf,&sp->tag) ;

	    serialbuf_ruint(&msgbuf,&sp->ts) ;

	    serialbuf_ruint(&msgbuf,&sp->ta) ;

	    serialbuf_ruint(&msgbuf,&sp->tm) ;

	    serialbuf_ruint(&msgbuf,&sp->tu_sec) ;

	    serialbuf_ruint(&msgbuf,&sp->tu_usec) ;

	    serialbuf_ruint(&msgbuf,&sp->pid) ;

	    serialbuf_ruint(&msgbuf,&sp->sid) ;

	    serialbuf_rushort(&msgbuf,&sp->termination) ;

	    serialbuf_rushort(&msgbuf,&sp->exit) ;

	    serialbuf_rshort(&msgbuf,&sp->utype) ;

	    serialbuf_ruchar(&msgbuf,&sp->stype) ;

	    serialbuf_ruchar(&msgbuf,&sp->status) ;

	    serialbuf_ruchar(&msgbuf,&sp->rc) ;

	    serialbuf_rstrw(&msgbuf,sp->username,
		ULMSG_LUSERNAME) ;

	    serialbuf_rstrw(&msgbuf,sp->line,
		ULMSG_LLINE) ;

	    serialbuf_rstrw(&msgbuf,sp->node,
		ULMSG_LNODE) ;

	    serialbuf_rstrw(&msgbuf,sp->cluster,
		ULMSG_LCLUSTER) ;

	    serialbuf_rstrw(&msgbuf,sp->domain,
		ULMSG_LDOMAIN) ;

	    serialbuf_rstrw(&msgbuf,sp->host,
		ULMSG_LHOST) ;

	    serialbuf_rstrw(&msgbuf,sp->termtype,
		ULMSG_LTERMTYPE) ;

	    serialbuf_rstrw(&msgbuf,sp->id,
		ULMSG_LID) ;

	} else { /* write */

	    sp->msgtype = ulmsgtype_request ;
	    hdr = sp->msgtype ;
	    serialbuf_wuint(&msgbuf,hdr) ;

	    serialbuf_wuint(&msgbuf,sp->tag) ;

	    serialbuf_wuint(&msgbuf,sp->ts) ;

	    serialbuf_wuint(&msgbuf,sp->ta) ;

	    serialbuf_wuint(&msgbuf,sp->tm) ;

	    serialbuf_wuint(&msgbuf,sp->tu_sec) ;

	    serialbuf_wuint(&msgbuf,sp->tu_usec) ;

	    serialbuf_wuint(&msgbuf,sp->pid) ;

	    serialbuf_wuint(&msgbuf,sp->sid) ;

	    serialbuf_wushort(&msgbuf,sp->termination) ;

	    serialbuf_wushort(&msgbuf,sp->exit) ;

	    serialbuf_wshort(&msgbuf,sp->utype) ;

	    serialbuf_wchar(&msgbuf,sp->stype) ;

	    serialbuf_wchar(&msgbuf,sp->status) ;

	    serialbuf_wchar(&msgbuf,sp->rc) ;

	    serialbuf_wstrw(&msgbuf,sp->username,
		ULMSG_LUSERNAME) ;

	    serialbuf_wstrw(&msgbuf,sp->line,
		ULMSG_LLINE) ;

	    serialbuf_wstrw(&msgbuf,sp->node,
		ULMSG_LNODE) ;

	    serialbuf_wstrw(&msgbuf,sp->cluster,
		ULMSG_LCLUSTER) ;

	    serialbuf_wstrw(&msgbuf,sp->domain,
		ULMSG_LDOMAIN) ;

	    serialbuf_wstrw(&msgbuf,sp->host,
		ULMSG_LHOST) ;

	    serialbuf_wstrw(&msgbuf,sp->termtype,
		ULMSG_LTERMTYPE) ;

	    serialbuf_wstrw(&msgbuf,sp->id,
		ULMSG_LID) ;

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
/* end subroutine (ulmsg_response) */


