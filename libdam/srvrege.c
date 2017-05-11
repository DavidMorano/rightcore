/* srvrege */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-06-25, David A­D­ Morano

	This is a whole rewrite of the marshalling for deadling
	with MS entries.  The previous stuff (now eradicated !)
	was too error-prone and inflexible to deal with partial
	updates.


*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	These subroutines provide for marshalling (and unmarshalling)
	of a SRVREGE file entry.


******************************************************************************/


#define	SRVREGE_MASTER	0


#include	<sys/types.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<inttypes.h>
#include	<time.h>

#include	<vsystem.h>
#include	<serialbuf.h>
#include	<stdorder.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#include	"srvrege.h"


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */


/* exported subroutines */


int srvrege_all(buf,buflen,f_read,ep)
char		buf[] ;
int		buflen ;
int		f_read ;
SRVREGE_ALL	*ep ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

	if (ep == NULL)
	    return SR_FAULT ;

	if (buflen < 0)
	    buflen = INT_MAX ;

	if ((rs = serialbuf_start(&msgbuf,(char *) buf,buflen)) >= 0) {

	if (f_read) {

	    serialbuf_ruint(&msgbuf,&ep->utime) ;

	    serialbuf_ruint(&msgbuf,&ep->stime) ;

	    serialbuf_ruint(&msgbuf,&ep->hostid) ;

	    serialbuf_ruint(&msgbuf,&ep->itype) ;

	    serialbuf_ruint(&msgbuf,&ep->pf) ;

	    serialbuf_ruint(&msgbuf,&ep->ptype) ;

	    serialbuf_ruint(&msgbuf,&ep->proto) ;

	    serialbuf_ruint(&msgbuf,&ep->pid) ;

	    serialbuf_robj(&msgbuf,&ep->a,sizeof(union srvrege_addr)) ;

	    serialbuf_rstrn(&msgbuf,ep->tag,SRVREGE_LTAG) ;

	    serialbuf_rstrn(&msgbuf,ep->svc,SRVREGE_LSVC) ;

	    serialbuf_rstrn(&msgbuf,ep->ss,SRVREGE_LSS) ;

	    serialbuf_rstrn(&msgbuf,ep->host,SRVREGE_LHOST) ;

	} else {

	    serialbuf_wuint(&msgbuf,ep->utime) ;

	    serialbuf_wuint(&msgbuf,ep->stime) ;

	    serialbuf_wuint(&msgbuf,ep->hostid) ;

	    serialbuf_wuint(&msgbuf,ep->itype) ;

	    serialbuf_wuint(&msgbuf,ep->pf) ;

	    serialbuf_wuint(&msgbuf,ep->ptype) ;

	    serialbuf_wuint(&msgbuf,ep->proto) ;

	    serialbuf_wuint(&msgbuf,ep->pid) ;

	    serialbuf_wobj(&msgbuf,&ep->a,sizeof(union srvrege_addr)) ;

	    serialbuf_wstrn(&msgbuf,ep->tag,SRVREGE_LTAG) ;

	    serialbuf_wstrn(&msgbuf,ep->svc,SRVREGE_LSVC) ;

	    serialbuf_wstrn(&msgbuf,ep->ss,SRVREGE_LSS) ;

	    serialbuf_wstrn(&msgbuf,ep->host,SRVREGE_LHOST) ;

	} /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (srvrege_all) */


int srvrege_utime(rbuf,rlen,f_read,ep)
char		rbuf[] ;
int		rlen ;
int		f_read ;
SRVREGE_UTIME	*ep ;
{
	int		rs ;
	char		*bp = (char *) (rbuf + SRVREGE_OUTIME) ;

	if (rlen <= 0) {
	    rs = SR_OVERFLOW ;
	    goto ret0 ;
	}

	if (ep == NULL)
	    return SR_FAULT ;

	if (f_read) {

	    stdorder_ruint(bp,&ep->utime) ;

	} else {

	    stdorder_wuint(bp,ep->utime) ;

	}

	rs = (SRVREGE_OUTIME + SRVREGE_LUTIME) ;

ret0:
	return rs ;
}
/* end subroutine (srvrege_utime) */


