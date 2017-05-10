/* procflow */

/* process a flow of messages */


#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that did
	similar types of functions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Here we process a flow of mail messages.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<mailmsgmatenv.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"systems.h"
#include	"dialer.h"
#include	"cm.h"



/* local defines */

#ifndef	ENVBUFLEN
#define	ENVBUFLEN		(2 * 1024)
#endif



/* external subroutines */

extern int	bufprintf(char *,int,const char *,...) ;

extern char	*timestr_edate(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int procflow(pip,conp,ifp,sip)
struct proginfo	*pip ;
CM		*conp ;
bfile		*ifp ;
int		*sip ;
{
	MSGMATENV	ei ;
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		elen ;
	int		len ;
	int		tlen = 0 ;
	int		f_envadd ;
	int		f_envin ;

	char	lbuf[LINEBUFLEN + 1] ;
	char	envbuf[ENVBUFLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procflow: entered f_signal=%u\n",*sip) ;
#endif

	f_envadd = TRUE ;
	f_envin = FALSE ;
	while ((! *sip) && ((rs = breadline(ifp,lbuf,llen)) > 0)) {
	    len = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("procflow: len=%d\n",len) ;
#endif

	    rs1 = mailmsgmatenv(&ei,lbuf,len) ;

	    if ((rs1 > 0) || f_envadd) {

	        if ((! f_envin) || f_envadd) {

	            f_envin = TRUE ;
	            f_envadd = FALSE ;
	            rs = bufprintf(envbuf,ENVBUFLEN,
	                "From %s %s remote from %s\n",
	                pip->username,
	                timestr_edate(pip->daytime,timebuf),
	                pip->domainname) ;

	            elen = rs ;
	            if (rs > 0) {
	                rs = cm_write(conp,envbuf,elen) ;
			tlen += rs ;
		    }

	        } /* end if (adding an envelope) */

	        if ((rs >= 0) && (rs1 >= 0)) {
	            envbuf[0] = '>' ;
	            rs = cm_write(conp,envbuf,1) ;
		    tlen += 1 ;
	        }

	    } else
	        f_envin = FALSE ;

	    if (rs >= 0) {
	        rs = cm_write(conp,lbuf,len) ;
	        tlen += len ;
	    }

	    if (rs < 0)
	        break ;

	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procflow: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (procflow) */



