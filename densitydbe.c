/* densitydbe */

/* density DB entry */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-06-25, David A­D­ Morano
	This is being writen to support the DENSITYDB object.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These module implements the messages (reads and writes) to the DENSITYDB
        database file. It actually does the subroutine marshalling for the file
        reads and writes.


*******************************************************************************/


#define	DENSITYDBE_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<inttypes.h>

#include	<vsystem.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"densitydbe.h"


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* exported subroutines */


int densitydbe_all(buf,buflen,f_read,ep)
char		buf[] ;
int		buflen ;
int		f_read ;
DENSITYDBE_ALL	*ep ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("densitydbe_all: buf=%p buflen=%d\n",buf,buflen) ;
#endif

	if (ep == NULL) return SR_FAULT ;

	if (buflen < 0) 
	    buflen = INT_MAX ;

	if ((rs = serialbuf_start(&msgbuf,buf,buflen)) >= 0) {

	    if (f_read) {
	        serialbuf_ruint(&msgbuf,&ep->count) ;
	        serialbuf_ruint(&msgbuf,&ep->utime) ;
	    } else {
	        serialbuf_wuint(&msgbuf,ep->count) ;
	        serialbuf_wuint(&msgbuf,ep->utime) ;
	    }

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (densitydbe_all) */


int densitydbe_upd(buf,buflen,f_read,ep)
char		buf[] ;
int		buflen ;
int		f_read ;
DENSITYDBE_UPD	*ep ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("densitydbe_upd: buf=%p buflen=%d\n",buf,buflen) ;
#endif

	if (ep == NULL) return SR_FAULT ;

	if (buflen < 0) 
	    buflen = INT_MAX ;

	if ((rs = serialbuf_start(&msgbuf,buf,buflen)) >= 0) {

	    if (f_read) {
	        serialbuf_ruint(&msgbuf,&ep->count) ;
	        serialbuf_ruint(&msgbuf,&ep->utime) ;
	    } else {
	        serialbuf_wuint(&msgbuf,ep->count) ;
	        serialbuf_wuint(&msgbuf,ep->utime) ;
	    }

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (densitydbe_upd) */


