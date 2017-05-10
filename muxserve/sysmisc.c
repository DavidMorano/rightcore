/* sysmisc */

/* create and parse the internal messages */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-07-21, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module contains the subroutines to make and parse the SYSMISC
        family of messages.


*******************************************************************************/


#define	SYSMISC_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<stdorder.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"sysmisc.h"


/* local defines */

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif


/* external subroutines */


/* local structures */


/* local variables */


/* exported subroutines */


int sysmisc_request(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sysmisc_request	*sp ;
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

	        serialbuf_ruint(&msgbuf,&sp->duration) ;

	        serialbuf_ruint(&msgbuf,&sp->interval) ;

	        serialbuf_rushort(&msgbuf,&sp->addrfamily) ;

	        serialbuf_rushort(&msgbuf,&sp->addrport) ;

	        serialbuf_ruint(&msgbuf,&sp->addrhost[0]) ;

	        serialbuf_ruint(&msgbuf,&sp->addrhost[1]) ;

	        serialbuf_ruint(&msgbuf,&sp->addrhost[2]) ;

	        serialbuf_ruint(&msgbuf,&sp->addrhost[3]) ;

	        serialbuf_rushort(&msgbuf,&sp->opts) ;

	    } else { /* write */

	        sp->msgtype = sysmisctype_request ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuint(&msgbuf,sp->duration) ;

	        serialbuf_wuint(&msgbuf,sp->interval) ;

	        serialbuf_wushort(&msgbuf,sp->addrfamily) ;

	        serialbuf_wushort(&msgbuf,sp->addrport) ;

	        serialbuf_wuint(&msgbuf,sp->addrhost[0]) ;

	        serialbuf_wuint(&msgbuf,sp->addrhost[1]) ;

	        serialbuf_wuint(&msgbuf,sp->addrhost[2]) ;

	        serialbuf_wuint(&msgbuf,sp->addrhost[3]) ;

	        serialbuf_wushort(&msgbuf,sp->opts) ;

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
/* end subroutine (sysmisc_request) */


int sysmisc_loadave(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sysmisc_loadave	*sp ;
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

	        serialbuf_ruchar(&msgbuf,&sp->rc) ;

	        serialbuf_ruint(&msgbuf,&sp->timestamp) ;

	        serialbuf_ruint(&msgbuf,&sp->providerid) ;

	        serialbuf_ruint(&msgbuf,&sp->hostid) ;

	        serialbuf_ruint(&msgbuf,&sp->la_1min) ;

	        serialbuf_ruint(&msgbuf,&sp->la_5min) ;

	        serialbuf_ruint(&msgbuf,&sp->la_15min) ;

	    } else { /* write */

	        sp->msgtype = sysmisctype_loadave ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuchar(&msgbuf,sp->rc) ;

	        serialbuf_wuint(&msgbuf,sp->timestamp) ;

	        serialbuf_wuint(&msgbuf,sp->providerid) ;

	        serialbuf_wuint(&msgbuf,sp->hostid) ;

	        serialbuf_wuint(&msgbuf,sp->la_1min) ;

	        serialbuf_wuint(&msgbuf,sp->la_5min) ;

	        serialbuf_wuint(&msgbuf,sp->la_15min) ;

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
/* end subroutine (sysmisc_loadave) */


int sysmisc_extra(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sysmisc_extra	*sp ;
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

	        serialbuf_ruchar(&msgbuf,&sp->rc) ;

	        serialbuf_ruint(&msgbuf,&sp->timestamp) ;

	        serialbuf_ruint(&msgbuf,&sp->providerid) ;

	        serialbuf_ruint(&msgbuf,&sp->hostid) ;

	        serialbuf_ruint(&msgbuf,&sp->la_1min) ;

	        serialbuf_ruint(&msgbuf,&sp->la_5min) ;

	        serialbuf_ruint(&msgbuf,&sp->la_15min) ;

	        serialbuf_ruint(&msgbuf,&sp->boottime) ;

	        serialbuf_ruint(&msgbuf,&sp->nproc) ;

	    } else { /* write */

	        sp->msgtype = sysmisctype_extra ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuchar(&msgbuf,sp->rc) ;

	        serialbuf_wuint(&msgbuf,sp->timestamp) ;

	        serialbuf_wuint(&msgbuf,sp->providerid) ;

	        serialbuf_wuint(&msgbuf,sp->hostid) ;

	        serialbuf_wuint(&msgbuf,sp->la_1min) ;

	        serialbuf_wuint(&msgbuf,sp->la_5min) ;

	        serialbuf_wuint(&msgbuf,sp->la_15min) ;

	        serialbuf_wuint(&msgbuf,sp->boottime) ;

	        serialbuf_wuint(&msgbuf,sp->nproc) ;

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
/* end subroutine (sysmisc_extra) */


int sysmisc_hostinfo(sp,f,mbuf,mlen)
char			mbuf[] ;
int			mlen ;
int			f ;
struct sysmisc_hostinfo	*sp ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&msgbuf,mbuf,mlen)) >= 0) {
	    uint	hdr ;
	    int		hostnamelen ;

	    if (f) { /* read */

	        serialbuf_ruint(&msgbuf,&hdr) ;
	        sp->msgtype = (hdr & 0xff) ;
	        sp->msglen = (hdr >> 8) ;

	        serialbuf_ruint(&msgbuf,&sp->tag) ;

	        serialbuf_ruchar(&msgbuf,&sp->rc) ;

	        serialbuf_ruint(&msgbuf,&sp->timestamp) ;

	        serialbuf_ruint(&msgbuf,&sp->providerid) ;

	        serialbuf_ruint(&msgbuf,&sp->hostid) ;

	        serialbuf_ruint(&msgbuf,&sp->la_1min) ;

	        serialbuf_ruint(&msgbuf,&sp->la_5min) ;

	        serialbuf_ruint(&msgbuf,&sp->la_15min) ;

	        serialbuf_ruint(&msgbuf,&sp->boottime) ;

	        serialbuf_ruint(&msgbuf,&sp->nproc) ;

	        serialbuf_rstrw(&msgbuf,sp->provider,SYSMISC_PROVIDERLEN) ;

	        serialbuf_rushort(&msgbuf,&sp->hostnamelen) ;

	        serialbuf_rstrw(&msgbuf,sp->hostname,(MAXHOSTNAMELEN - 1)) ;

	    } else { /* write */

	        sp->msgtype = sysmisctype_hostinfo ;
	        hdr = sp->msgtype ;
	        serialbuf_wuint(&msgbuf,hdr) ;

	        serialbuf_wuchar(&msgbuf,sp->msgtype) ;

	        serialbuf_wuint(&msgbuf,sp->tag) ;

	        serialbuf_wuchar(&msgbuf,sp->rc) ;

	        serialbuf_wuint(&msgbuf,sp->timestamp) ;

	        serialbuf_wuint(&msgbuf,sp->providerid) ;

	        serialbuf_wuint(&msgbuf,sp->hostid) ;

	        serialbuf_wuint(&msgbuf,sp->la_1min) ;

	        serialbuf_wuint(&msgbuf,sp->la_5min) ;

	        serialbuf_wuint(&msgbuf,sp->la_15min) ;

	        serialbuf_wuint(&msgbuf,sp->boottime) ;

	        serialbuf_wuint(&msgbuf,sp->nproc) ;

	        serialbuf_wstrw(&msgbuf,sp->provider,
	            SYSMISC_PROVIDERLEN) ;

	        hostnamelen = MIN(sp->hostnamelen,(MAXHOSTNAMELEN - 1)) ;
	        if (hostnamelen < 0)
	            hostnamelen = strlen(sp->hostname) ;

	        serialbuf_wushort(&msgbuf,hostnamelen) ;

	        serialbuf_wstrw(&msgbuf,sp->hostname,hostnamelen) ;

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
/* end subroutine (sysmisc_hostinfo) */


#ifdef	COMMENT
int sysmisc_msglen(type)
int	type ;
{
	int		rs ;

	switch (type) {
	case sysmisctype_request:
	    rs = SYSMISC_SREQUEST ;
	    break ;
	case sysmisctype_loadave:
	    rs = SYSMISC_SLOADAVE ;
	    break ;
	case sysmisctype_extra:
	    rs = SYSMISC_SEXTRA ;
	    break ;
	case sysmisctype_hostinfo:
	    rs = SYSMISC_SHOSTINFO ;
	    break ;
	default:
	    rs = SR_INVALID ;
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (sysmisc_msglen) */
#endif /* COMMENT */


