/* msgide */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-06-25, David A­D­ Morano
	This is being writen to support the MSGID object.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These module implements the messages (reads and writes) to the MSGID
        database file. It actually does the subroutine marshalling for the file
        reads and writes.


*******************************************************************************/


#define	MSGIDE_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<inttypes.h>

#include	<vsystem.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"msgide.h"


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* exported subroutines */


int msgide_all(ep,f_read,mbuf,mlen)
char		mbuf[] ;
int		mlen ;
int		f_read ;
MSGIDE_ALL	*ep ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("msgide_all: buf=%p buflen=%d\n",buf,buflen) ;
#endif

	if (ep == NULL) return SR_FAULT ;
	if (mbuf == NULL) return SR_FAULT ;

	if (mlen < 0) mlen = INT_MAX ;

	if ((rs = serialbuf_start(&msgbuf,mbuf,mlen)) >= 0) {

	if (f_read) {

	    serialbuf_ruint(&msgbuf,&ep->count) ;

	    serialbuf_ruint(&msgbuf,&ep->utime) ;

	    serialbuf_ruint(&msgbuf,&ep->ctime) ;

	    serialbuf_ruint(&msgbuf,&ep->mtime) ;

	    serialbuf_ruint(&msgbuf,&ep->hash) ;

	    serialbuf_rstrn(&msgbuf,ep->recipient,MSGIDE_LRECIPIENT) ;

	    serialbuf_rstrn(&msgbuf,ep->messageid,MSGIDE_LMESSAGEID) ;

	    serialbuf_rstrn(&msgbuf,ep->from,MSGIDE_LFROM) ;

	} else {

	    serialbuf_wuint(&msgbuf,ep->count) ;

	    serialbuf_wuint(&msgbuf,ep->utime) ;

	    serialbuf_wuint(&msgbuf,ep->ctime) ;

	    serialbuf_wuint(&msgbuf,ep->mtime) ;

	    serialbuf_wuint(&msgbuf,ep->hash) ;

	    serialbuf_wstrn(&msgbuf,ep->recipient,MSGIDE_LRECIPIENT) ;

	    serialbuf_wstrn(&msgbuf,ep->messageid,MSGIDE_LMESSAGEID) ;

	    serialbuf_wstrn(&msgbuf,ep->from,MSGIDE_LFROM) ;

	} /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (entry_all) */


int msgide_update(ep,f_read,mbuf,mlen)
char		mbuf[] ;
int		mlen ;
int		f_read ;
MSGIDE_UPDATE	*ep ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;
	if (mbuf == NULL) return SR_FAULT ;

/* go to where this data is in the message buffer */

	mbuf += MSGIDE_OCOUNT ;
	mlen -= MSGIDE_OCOUNT ;

/* proceed as normal (?) */

	if ((rs = serialbuf_start(&msgbuf,mbuf,mlen)) >= 0) {

	if (f_read) {

	    serialbuf_ruint(&msgbuf,&ep->count) ;

	    serialbuf_ruint(&msgbuf,&ep->utime) ;

	} else {

	    serialbuf_wuint(&msgbuf,ep->count) ;

	    serialbuf_wuint(&msgbuf,ep->utime) ;

	} /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	if (rs >= 0)
		rs += MSGIDE_OCOUNT ;

	return rs ;
}
/* end subroutine (msgide_update) */


