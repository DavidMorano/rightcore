/* nlsdialassist */

/* we provide NLS dialing assistance */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-02-17, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We provide NLS dialing assistance.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<listen.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"nlsdialassist.h"


/* local defines */

#define	SVC_TCPMUX	"tcpmux"
#define	SVC_LISTEN	"listen"
#define	SVC_TCPNLS	"tcpnls"

#ifndef	SVCLEN
#define	SVCLEN		MAXNAMELEN
#endif

#define	RBUFLEN		MAXNAMELEN


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	nlsresponse(char *,int,const char *,int) ;


/* local variables */


/* exported subroutines */


int mknlsreq(char *nlsbuf,int nlslen,cchar *svcbuf,int svclen)
{
	SBUF		svc ;
	int		rs ;
	int		len = 0 ;

	if ((rs = sbuf_start(&svc,nlsbuf,nlslen)) >= 0) {

	    sbuf_strw(&svc,NLPS_REQ2,-1) ;

	    sbuf_strw(&svc,svcbuf,svclen) ;

	    sbuf_char(&svc,0) ;

	    len = sbuf_finish(&svc) ;
	    if (rs >= 0) rs = len ;
	} /* end if (nlsbuf) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mknlsreq) */


int readnlsresp(int fd,char *tbuf,int tlen,int to)
{
	const int	rlen = RBUFLEN ;
	int		rs ;
	char		rbuf[RBUFLEN+1] = { 0 } ;

	if ((rs = uc_readlinetimed(fd,rbuf,rlen,to)) >= 0) {
#if	CF_DEBUGS
	    debugprintf("readnlsresp: uc_readlinetimed() rs=%d\n",rs) ;
	    debugprinthex("readnlsresp: resp=",80,rbuf,rs) ;
#endif
	    if ((rs = nlsresponse(tbuf,tlen,rbuf,rs)) >= 0) {
	        int	code = rs ;
#if	CF_DEBUGS
	        debugprintf("readnlsresp: code=%d\n",code) ;
#endif
	        switch (code) {
	        case NLSSTART:
	            break ;
	        case NLSFORMAT:
	        case NLSUNKNOWN:
	            rs = SR_BADREQUEST ;
	            break ;
	        case NLSDISABLED:
	            rs = SR_NOTAVAIL ;
	            break ;
	        default:
	            rs = SR_PROTO ;
	            break ;
	        } /* end switch */
	    } /* end if (parsing response) */
#if	CF_DEBUGS
	    debugprintf("readnlsresp: parse-out rs=%d\n",rs) ;
#endif
	} /* end if (reading response) */

#if	CF_DEBUGS
	debugprintf("readnlsresp: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (readnlsresp) */


/* local subroutine */


static int nlsresponse(char *tbuf,int tlen,cchar *rbuf,int rlen)
{
	int		rs = SR_PROTO ;
	int		sl = rlen ;
	int		pv = 0 ;
	int		code = 0 ;
	const char	*tp ;
	const char	*sp = rbuf ;
	if ((tp = strnchr(sp,sl,':')) != NULL) {
	    if ((rs = cfdeci(sp,(tp-sp),&pv)) >= 0) {
	        sl -= ((tp+1)-sp) ;
	        sp = (tp+1) ;
	        if ((tp = strnchr(sp,sl,':')) != NULL) {
	            if ((rs = cfdeci(sp,(tp-sp),&code)) >= 0) {
	                sl -= ((tp+1)-sp) ;
	                sp = (tp+1) ;
	                rs = snwcpy(tbuf,tlen,sp,sl) ;
	            }
	        } else {
	            rs = SR_PROTO ;
		}
	    }
	}
	if ((rs >= 0) && (pv != NLPS_REQVERSION)) rs = SR_NOPROTOOPT ;
#if	CF_DEBUGS
	debugprintf("nlpresponse: ret rs=%d pv=%u\n",rs,pv) ;
#endif
	return (rs >= 0) ? code : rs ;
}
/* end subroutine (nlsresponse) */


