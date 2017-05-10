/* progmsgid */

/* create a mail message ID */


#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_HOSTID	0		/* use host-id sometimes */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to create a mail message ID for certain PCS
        programs.

	Synopsis:

	int progmsgid(pip,mbuf,mlen,serial)
	PROGINFO	*pip ;
	char		mbuf[] ;
	char		mlen ;
	int		serial ;

	Arguments:

	pip		pointer to program information
	mbuf		caller supplied buffer to place result in
	mlen		length of caller supplied buffer
	serial		a serial number of some kind

	Returns:

	>=0		length of returned ID
	<0		error in process of creating ID


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"

#if	CF_DEBUGS || CF_DEBUG
#include	<debug.h>
#endif


/* local defines */


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int progmsgid(PROGINFO *pip,char *mbuf,int mlen,int serial)
{
	SBUF		ubuf ;
	int		rs ;
	int		len = 0 ;

	if (mbuf == NULL) return SR_FAULT ;

	if ((rs = sbuf_start(&ubuf,mbuf,mlen)) >= 0) {
	    const time_t	dt = pip->daytime ;
	    const pid_t		pid = pip->pid ;
	    cchar		*nn = pip->nodename ;
	    cchar		*dn = pip->domainname ;
	    uint		uv ;

#if	CF_HOSTID
	    {
		int	nl = strlen(nn) ;
	        if (nl > USERNAMELEN) {
		    uv = (uint) gethostid() ;
	            sbuf_hexui(&ubuf,uv) ;
	            sbuf_char(&ubuf,'-') ;
	        } else
	            sbuf_strw(&ubuf,nn,nl) ;
	    }
#else /* CF_HOSTID */
	    sbuf_strw(&ubuf,nn,-1) ;
#endif /* CF_HOSTID */

	    uv = (uint) pid ;
	    sbuf_decui(&ubuf,uv) ;

	    sbuf_char(&ubuf,'.') ;

	    uv = (uint) dt ;
	    sbuf_hexui(&ubuf,uv) ;

	    sbuf_char(&ubuf,'.') ;
	    sbuf_deci(&ubuf,pip->pserial) ;
	    sbuf_char(&ubuf,'.') ;
	    sbuf_deci(&ubuf,serial) ;

	    sbuf_char(&ubuf,'@') ;

	    sbuf_strw(&ubuf,dn,-1) ;

	    len = sbuf_finish(&ubuf) ;
	    if (rs >= 0) rs = len ;
	} /* end if (initialized) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progmsgid: ret msgid=>%s<\n",mbuf) ;
	    debugprintf("progmsgid: ret rs=%d len=%u\n",rs,len) ;
	}
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (progmsgid) */


