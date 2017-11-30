/* testuterm (main) */
/* lang=C99 */

/* test the UTERM object */


#define	CF_DEBUGS	1		/* compile-time */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Testing of the UTERM object.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"uterm.h"


/* local defines */

#ifndef	FD_STDIN
#define	FD_STDIN	0
#endif

#ifndef	HEXBUFLEN
#define	HEXBUFLEN	80
#endif


/* external subroutines */


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	UTERM		ut ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		fd ;
	int		to, opts ;
	int		len, c ;
	int		sl ;
	int		ex = EX_INFO ;
	const char	*cp ;
	char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	fd = FD_STDIN ;
	if ((rs = uterm_start(&ut,fd)) >= 0) {

#if	CF_DEBUGS
	debugprintf("main: uterm_start() rs=%d\n",rs) ;
#endif

/* try to read something */

	to = 20 ;
	opts = 0 ;
	rs = uterm_reade(&ut,lbuf,LINEBUFLEN,to,opts,NULL,NULL) ;
	len = rs ;

#if	CF_DEBUGS
	debugprintf("main: uterm_reade() rs=%d\n",rs) ;
#endif

	if (len > 0) {

	    c = lbuf[len - 1] ;
	    if ((c == CH_ESC) || (c == CH_CSI)) {

#if	CF_DEBUGS
	debugprintf("main: escape sequence terminator=\\x%02X\n",c) ;
#endif

		opts = fm_noecho ;
	        rs = uterm_reade(&ut,lbuf,LINEBUFLEN,0,opts,NULL,NULL) ;

#if	CF_DEBUGS
	{
	char	hexbuf[HEXBUFLEN + 1] ;
	debugprintf("main: uterm_reade() rs=%d\n",rs) ;
	sl = mkhexstr(hexbuf,HEXBUFLEN,lbuf,rs) ;
	debugprintf("main: buf= %t\n",hexbuf,sl) ;
	}
#endif

	    }
	}

/* get out */

	uterm_finish(&ut) ;
	} /* end if (uterm) */

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


