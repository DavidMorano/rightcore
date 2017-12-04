/* msfilee */

/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1999-06-01, David A­D­ Morano
        This is a whole rdebugwrite of the marshalling for dealing with MS
        entries. The previous stuff (now eradicated!) was too error-prone and
        inflexible to deal with partial updates.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines provide for marshalling (and unmarshalling) of an MS
        file entry.


*******************************************************************************/


#define	MSFILEE_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<inttypes.h>

#include	<vsystem.h>
#include	<serialbuf.h>
#include	<stdorder.h>
#include	<localmisc.h>

#include	"msfilee.h"


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* exported subroutines */


int msfilee_all(ep,f_read,buf,buflen)
char		buf[] ;
int		buflen ;
int		f_read ;
MSFILEE_ALL	*ep ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;
	int		i ;

#if	CF_DEBUGS
	char	timebuf[TIMEBUFLEN + 1] ;
#endif

#if	CF_DEBUGS
	debugprintf("msfilee_all: buf=%p buflen=%d\n",buf,buflen) ;
#endif

	if (ep == NULL)
	    return SR_FAULT ;

	if (buflen < 0)
	    buflen = INT_MAX ;

	if ((rs = serialbuf_start(&msgbuf,buf,buflen)) >= 0) {

	if (f_read) {

	    serialbuf_ruint(&msgbuf,&ep->btime) ;

	    serialbuf_ruint(&msgbuf,&ep->atime) ;

	    serialbuf_ruint(&msgbuf,&ep->utime) ;

	    serialbuf_ruint(&msgbuf,&ep->dtime) ;

	    serialbuf_ruint(&msgbuf,&ep->stime) ;

	    for (i = 0 ; i < 3 ; i += 1) {
	        serialbuf_ruint(&msgbuf,(ep->la + i)) ;
	    }

	    serialbuf_ruint(&msgbuf,&ep->nproc) ;

	    serialbuf_ruint(&msgbuf,&ep->nuser) ;

	    serialbuf_ruint(&msgbuf,&ep->pmtotal) ;

	    serialbuf_ruint(&msgbuf,&ep->pmavail) ;

	    serialbuf_ruint(&msgbuf,&ep->speed) ;

	    serialbuf_ruint(&msgbuf,&ep->ncpu) ;

	    serialbuf_ruint(&msgbuf,&ep->nodehash) ;

	    serialbuf_ruint(&msgbuf,&ep->pid) ;

	    serialbuf_rushort(&msgbuf,&ep->flags) ;

	    serialbuf_rstrn(&msgbuf,ep->nodename,MSFILEE_LNODENAME) ;

	    ep->nodename[MSFILEE_LNODENAME] = '\0' ;

	} else {

	    serialbuf_wuint(&msgbuf,ep->btime) ;

	    if (ep->atime != 0) {
	        serialbuf_wuint(&msgbuf,ep->atime) ;
	    } else {
	        serialbuf_adv(&msgbuf,sizeof(uint)) ;
	    }

	    serialbuf_wuint(&msgbuf,ep->utime) ;

	    serialbuf_wuint(&msgbuf,ep->dtime) ;

	    serialbuf_wuint(&msgbuf,ep->stime) ;

	    for (i = 0 ; i < 3 ; i += 1) {
	        serialbuf_wuint(&msgbuf,ep->la[i]) ;
	    }

	    serialbuf_wuint(&msgbuf,ep->nproc) ;

	    serialbuf_wuint(&msgbuf,ep->nuser) ;

	    serialbuf_wuint(&msgbuf,ep->pmtotal) ;

	    serialbuf_wuint(&msgbuf,ep->pmavail) ;

	    serialbuf_wuint(&msgbuf,ep->speed) ;

	    serialbuf_wuint(&msgbuf,ep->ncpu) ;

	    serialbuf_wuint(&msgbuf,ep->nodehash) ;

	    serialbuf_wuint(&msgbuf,ep->pid) ;

	    serialbuf_wushort(&msgbuf,ep->flags) ;

	    serialbuf_wstrn(&msgbuf,ep->nodename,MSFILEE_LNODENAME) ;

	} /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (msfilee_all) */


/* ARGSUSED */
int msfilee_la(ep,f_read,buf,buflen)
char		buf[] ;
int		buflen ;
int		f_read ;
MSFILEE_LA	*ep ;
{
	int		rs ;
	int		i ;
	char		*bp = (char *) (buf + MSFILEE_OLA) ;

	if (ep == NULL)
	    return SR_FAULT ;

	if (f_read) {
	    for (i = 0 ; i < 3 ; i += 1) {
		stdorder_ruint(bp,&ep->la[i]) ;
		bp += sizeof(uint) ;
	    }
	} else {
	    for (i = 0 ; i < 3 ; i += 1) {
		stdorder_wuint(bp,ep->la[i]) ;
		bp += sizeof(uint) ;
	    }
	} /* end if */

	rs = (MSFILEE_OLA + MSFILEE_LLA) ;
	return rs ;
}
/* end subroutine (msfilee_la) */


/* ARGSUSED */
int msfilee_atime(ep,f_read,buf,buflen)
char		buf[] ;
int		buflen ;
int		f_read ;
MSFILEE_ATIME	*ep ;
{
	int		rs ;
	char		*bp = (char *) (buf + MSFILEE_OATIME) ;

	if (ep == NULL) return SR_FAULT ;

	if (f_read) {
	    stdorder_ruint(bp,&ep->atime) ;
	} else {
	    stdorder_wuint(bp,ep->atime) ;
	} /* end if */

	rs = MSFILEE_OATIME + MSFILEE_LATIME ;
	return rs ;
}
/* end subroutine (msfilee_atime) */


/* ARGSUSED */
int msfilee_utime(ep,f_read,buf,buflen)
char		buf[] ;
int		buflen ;
int		f_read ;
MSFILEE_UTIME	*ep ;
{
	int		rs ;
	char		*bp = (char *) (buf + MSFILEE_OUTIME) ;

	if (ep == NULL)
	    return SR_FAULT ;

	if (f_read) {
	    stdorder_ruint(bp,&ep->utime) ;
	} else {
	    stdorder_wuint(bp,ep->utime) ;
	} /* end if */

	rs = (MSFILEE_OUTIME + MSFILEE_LUTIME) ;
	return rs ;
}
/* end subroutine (msfilee_utime) */


/* ARGSUSED */
int msfilee_dtime(ep,f_read,buf,buflen)
char		buf[] ;
int		buflen ;
int		f_read ;
MSFILEE_DTIME	*ep ;
{
	int		rs ;
	char		*bp = (char *) (buf + MSFILEE_ODTIME) ;

	if (ep == NULL)
	    return SR_FAULT ;

	if (f_read) {
	    stdorder_ruint(bp,&ep->dtime) ;
	} else {
	    stdorder_wuint(bp,ep->dtime) ;
	} /* end if */

	rs = MSFILEE_ODTIME + MSFILEE_LDTIME ;
	return rs ;
}
/* end subroutine (msfilee_dtime) */


/* ARGSUSED */
int msfilee_stime(ep,f_read,buf,buflen)
char		buf[] ;
int		buflen ;
int		f_read ;
MSFILEE_STIME	*ep ;
{
	int		rs ;
	char		*bp = (char *) (buf + MSFILEE_OSTIME) ;

	if (ep == NULL)
	    return SR_FAULT ;

	if (f_read) {
	    stdorder_ruint(bp,&ep->stime) ;
	} else {
	    stdorder_wuint(bp,ep->stime) ;
	} /* end if */

	rs = (MSFILEE_OSTIME + MSFILEE_LSTIME) ;
	return rs ;
}
/* end subroutine (msfilee_stime) */


