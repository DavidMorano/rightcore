/* msgbuf */

/* message buffering */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object performs some simple message buffering.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"msgbuf.h"


/* local defines */

#ifndef	TO_READ
#define	TO_READ		4
#endif

#define	NEOF		3

#define	NDF	"/tmp/msgbuf.deb"


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int msgbuf_start(MSGBUF *mbp,int fd,int bufsize,int to)
{
	int		rs ;
	char		*bp ;

	if (mbp == NULL) return SR_FAULT ;

	if (fd < 0) return SR_INVALID ;

	if (bufsize <= 0)
	    bufsize = getpagesize() ;

	if (to < 1)
	    to = TO_READ ;

	memset(mbp,0,sizeof(MSGBUF)) ;
	mbp->fd = fd ;
	mbp->bufsize = bufsize ;
	mbp->to = to ;

	if ((rs = uc_malloc(bufsize,&bp)) >= 0) {
	    mbp->buf = bp ;
	}

	return rs ;
}
/* end subroutine (msgbuf_start) */


int msgbuf_finish(MSGBUF *mbp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mbp == NULL) return SR_FAULT ;

	if (mbp->buf != NULL) {
	    rs1 = uc_free(mbp->buf) ;
	    if (rs >= 0) rs = rs1 ;
	    mbp->buf = NULL ;
	}

	mbp->bp = NULL ;
	mbp->bl = 0 ;
	mbp->fd = -1 ;
	return rs ;
}
/* end subroutine (msgbuf_finish) */


int msgbuf_read(MSGBUF *mbp,cchar **rpp)
{
	int		rs = SR_OK ;
	int		opts = 0 ;
	int		len = 0 ;

	if (mbp == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;

	if ((mbp->bl == 0) && (mbp->neof < NEOF)) {
	    rs = uc_reade(mbp->fd,mbp->buf,mbp->bufsize,mbp->to,opts) ;

#if	CF_DEBUGS
	    debugprintf("msgbuf_read: uc_reade() rs=%d\n",rs) ;
#endif

#if	CF_DEBUGN && defined(NDF)
	    nprintf(NDF,"msgbuf_read: uc_reade() rs=%d\n",rs) ;
#endif

	    mbp->bp = mbp->buf ;
	    mbp->bl = rs ;
	    mbp->neof = (rs == 0) ? (mbp->neof+1) : 0 ;
	}

	if (rs >= 0) {
	    *rpp = mbp->bp ;
	    len = mbp->bl ;
	} else {
	    *rpp = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("msgbuf_read: ret rs=%d len=%u\n",rs,len) ;
	debugprintf("msgbuf_read: bl=%d\n",mbp->bl) ;
	debugprintf("msgbuf_read: bp=%08x\n",mbp->bp) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (msgbuf_read) */


int msgbuf_adv(MSGBUF *mbp,int mlen)
{
	int		rs = SR_OK ;
	int		rlen ;
	int		opts = 0 ;
	int		len = 0 ;
	char		*rbuf ;

	if (mbp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("msgbuf_adv: mlen=%d\n",mlen) ;
	debugprintf("msgbuf_adv: bl=%d\n",mbp->bl) ;
	debugprintf("msgbuf_adv: bp=%08x\n",mbp->bp) ;
#endif

	if (mlen < 0) {
	    if ((mbp->bl > 0) && (mbp->bp != mbp->buf)) {
		int	i ;
	        for (i = 0 ; i < mbp->bl ; i += 1) {
	            mbp->buf[i] = *mbp->bp++ ;
		}
	        mbp->bp = mbp->buf ;
	    }
	} else if (mlen <= mbp->bl) {
	    mbp->bp += mlen ;
	    mbp->bl -= mlen ;
	    if (mbp->bl == 0)
		mbp->bp = mbp->buf ;
	} else if (mlen > mbp->bl) {
	    rs = SR_RANGE ;
	}

	if ((rs >= 0) && (mlen < 0)) {

	    rbuf = (mbp->buf + mbp->bl) ;
	    rlen = (mbp->bufsize - mbp->bl) ;
	    if (rlen > 0) {
	        rs = uc_reade(mbp->fd,rbuf,rlen,mbp->to,opts) ;
	        len = rs ;
	    }

	    if (rs >= 0) {
	        mbp->neof = (rs == 0) ? (mbp->neof+1) : 0 ;
	        mbp->bl += len ;
	        len = mbp->bl ;
	    }

	} /* end if */

#if	CF_DEBUGS
	debugprintf("msgbuf_adv: ret rs=%d len=%u\n",rs,len) ;
	debugprintf("msgbuf_adv: bl=%d\n",mbp->bl) ;
	debugprintf("msgbuf_adv: bp=%08x\n",mbp->bp) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (msgbuf_adv) */


int msgbuf_update(MSGBUF *mbp,int mlen)
{

	return msgbuf_adv(mbp,mlen) ;
}
/* end subroutine (msgbuf_update) */


