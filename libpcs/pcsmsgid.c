/* pcsmsgid */

/* create a mail message ID (for PCS) */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_PCSGETSERIAL	1		/* usage 'pcsgetserial(3pcs)' */
#define	CF_LENIENT	0		/* be lenient about program-root */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used to create a mail message ID for PCS programs.

	Synopsis:

	int pcsmsgid(pcsroot,rbuf,rlen)
	const char	pcsroot[] ;
	char		rbuf[] ;
	char		rlen ;

	Arguments:

	pcsroot		PCS program root path
	rbuf		caller supplied buffer to place result in
	rlen		length of caller supplied buffer

	Returns:

	>=0		length of returned ID
	<0		error in process of creating ID


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARPRPCS
#define	VARPRPCS	"PCS"
#endif

#define	SERIALFNAME1	"var/serial"
#define	SERIALFNAME2	"/tmp/serial"

#ifndef	HEXBUFLEN
#define	HEXBUFLEN	16
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	getnodedomain(char *,char *) ;
extern int	pcsgetserial(const char *) ;
extern int	getserial(const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int pcsmsgid(cchar *pr,char *rbuf,int rlen)
{
	const time_t	daytime = time(NULL) ;
	const pid_t	pid = getpid() ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		nl ;
	int		serial = 0 ;
	int		len = 0 ;
	char		nodename[NODENAMELEN + 1] ;
	char		domainname[MAXHOSTNAMELEN + 1] ;

	if (rbuf == NULL) return SR_FAULT ;

	nodename[0] = '\0' ;
	domainname[0] = '\0' ;

/* get the current program root if we can */

#if	CF_LENIENT

	if ((pr == NULL) || (pr[0] == '\0')) {
	    char	prbuf[MAXPATHLEN + 1] ;

	    pr = prbuf ;
	    if (domainname[0] == '\0')
	        rs = getnodedomain(nodename,domainname) ;

	    if (rs >= 0)
	        rs = mkpr(prbuf,MAXPATHLEN,VARPRPCS,domainname) ;

	    if (rs < 0)
	        goto ret0 ;

	} /* end if */

#else /* CF_LENIENT */

	if (pr == NULL)
	    return SR_FAULT ;

	if (pr[0] == '\0')
	    return SR_INVALID ;

#endif /* CF_LENIENT */

/* get a serial number if we can */

#if	CF_PCSGETSERIAL
	serial = pcsgetserial(pr) ;
#else
	{
	    char	tmpfname[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(tmpfname,pr,SERIALFNAME1)) >= 0) {
	        serial = getserial(tmpfname) ;
	    }
	}
#endif /* CF_PCSGETSERIAL */

	if ((rs >= 0) && (serial < 0)) {
	    serial = getserial(SERIALFNAME2) ;
	    if (serial < 0) serial = 0 ;
	}

/* more initialization */

	if ((rs >= 0) && (nodename[0] == '\0')) {
	    rs = getnodedomain(nodename,domainname) ;
	}

/* create the message ID number */

	if (rs >= 0) {
	    SBUF	ubuf ;
	    if ((rs = sbuf_start(&ubuf,rbuf,rlen)) >= 0) {
	        uint	tv = (uint) daytime ;

	        nl = strlen(nodename) ;
	        if (nl > USERNAMELEN) {
	            rs1 = (int) gethostid() ;
	            sbuf_hexi(&ubuf,rs1) ;
	            sbuf_char(&ubuf,'-') ;
	        } else {
	            sbuf_strw(&ubuf,nodename,nl) ;
		}

	        sbuf_deci(&ubuf,(int) pid) ;

	        sbuf_char(&ubuf,'.') ;

	        sbuf_hexui(&ubuf,tv) ;

	        sbuf_char(&ubuf,'.') ;

	        sbuf_deci(&ubuf,serial) ;

	        sbuf_char(&ubuf,'@') ;

	        sbuf_strw(&ubuf,domainname,-1) ;

	        len = sbuf_finish(&ubuf) ;
	        if (rs >= 0) rs = len ;
	    } /* end if (sbuf) */
	} /* end if (ok) */

ret0:

#if	CF_DEBUGS
	debugprintf("pcsmsgid: ret rs=%d msgid=>%s<\n",rs,rbuf) ;
#endif

	return rs ;
}
/* end subroutine (pcsmsgid) */


