/* mkmsgbound */

/* create a mail message boundary for multipart MIME messages */


#define	CF_DEBUG	0		/* not-switchable debug print-outs */
#define	CF_MTIME	0		/* use MSG 'time' */
#define	CF_SBUFHEX	1		/* use |sbuf_hexxx()| */
#define	CF_SBUFDEC	1		/* use |sbuf_decxx()| */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine creates a unique (?) string no more than MSGBOUND
	characters.

	Synopsis:

	int mkmsgbound(pip,rbuf,rlen)
	PROGINFO	*pip ;
	char		rbuf[] ;
	int		rlen ;

	Arguments:

	pip		pointer to program information
	rbuf		caller-supplied buffer to receive result
	rlen		caller-supplied buffer length

	Returns:

	>=0		length of result
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<ctdec.h>
#include	<cthex.h>
#include	<dater.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	DIGBUFLEN
#define	HEXBUFLEN	45
#endif

#ifndef	HEXBUFLEN
#define	HEXBUFLEN	32
#endif


/* external subroutines */

extern int	mkrand(PROGINFO *) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkmsgbound(PROGINFO *pip,char *mbuf,int mlen)
{
	int		rs ;
	int		len = 0 ;

	mbuf[0] = '\0' ;
	if ((rs = mkrand(pip)) >= 0) {
	    SBUF	b ;
	    if ((rs = sbuf_start(&b,mbuf,mlen)) >= 0) {
	        time_t	t ;
	        uint	hostid ;

/* nodename */

	        sbuf_strw(&b,pip->nodename,-1) ;

/* PID */

	        sbuf_deci(&b,(int) pip->pid) ;

	        sbuf_char(&b,'.') ;

/* time-of-day */

#if	CF_MTIME
	        dater_gettime(&pip->mdate,&t) ;
#else
	        t = pip->daytime ;
#endif /* CF_MTIME */

#if	CF_SBUFHEX
	        sbuf_hexi(&b,(uint) t) ;
#else
		{
		    const int	hlen = HEXBUFLEN ;
		    char	hbuf[HEXBUFLEN+1] ;
	            if ((rs = cthexui(hbuf,hlen,((uint) t))) >= 0) {
	                sbuf_strw(&b,hbuf,rs) ;
		    }
		}
#endif /* CF_SBUFHEX */

/* hostid */

	        hostid = gethostid() ;

#if	CF_SBUFHEX
	        sbuf_hexui(&b,hostid) ;
#else
		{
		    const int	hlen = HEXBUFLEN ;
		    char	hbuf[HEXBUFLEN+1] ;
	            if ((rs = cthexui(hbuf,hlen,hostid)) >= 0) {
	                sbuf_strw(&b,hbuf,rs) ;
		    }
		}
#endif /* CF_SBUFHEX */

/* random number */

#if	CF_SBUFHEX
	        sbuf_hexull(&b,pip->rand) ;
#else
		{
		    const int	hlen = HEXBUFLEN ;
		    char	hbuf[HEXBUFLEN+1] ;
	            if ((rs = cthexull(hbuf,hlen,pip->rand)) >= 0) {
	                sbuf_strw(&b,hbuf,rs) ;
		    }
		}
#endif /* CF_SBUFHEX */

	        sbuf_char(&b,'.') ;

#if	CF_SBUFDEC
	        sbuf_decui(&b,pip->serial) ;
#else
		{
		    const int	dlen = DIGBUFLEN ;
		    char	dbuf[DIGBUFLEN+1] ;
	            if ((rs = ctdecui(dbuf,dlen,pip->serial)) >= 0) {
	                sbuf_strw(&b,dbuf,rs) ;
		    }
		}
#endif /* CF_SBUFDEC */

	        sbuf_char(&b,'@') ;

	        if (pip->domainname != NULL) {
	            sbuf_strw(&b,pip->domainname,-1) ;
		}

	        len = sbuf_finish(&b) ;
	        if (rs >= 0) rs = len ;
	    } /* end if (sbuf) */
	} /* end if (mkrand) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkmsgbound) */


