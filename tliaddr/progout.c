/* progout */

/* subroutine to handle dynamic output control */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1999-08-17, David A­D­ Morano
        This subroutine was taken from the LOGDIR/LOGNAME program (fist written
        for the SunOS 4.xx environment in 1989). This whole program is replacing
        serveral that did similar things in the past.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This subroutine manages the output file handling.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* exported subroutines */


int progout_open(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (! pip->open.outfile) {
	    rs = bopen(pip->ofp,pip->ofname,"rwc",0666) ;
	    pip->open.outfile = (rs >= 0) ;
	}

	return rs ;
}
/* end subroutine (progout_open) */


int progout_printf(PROGINFO *pip,const char *fmt,...)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (pip->open.outfile) {
	    va_list	ap ;
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    va_begin(ap,fmt) ;
	    rs = vbufprintf(lbuf,llen,fmt,ap) ;
	    len = rs ;
	    va_end(ap) ;

	    if ((rs >= 0) && (len > 0)) {
	        rs = bwrite(pip->ofp,lbuf,len) ;
	        wlen += len ;
	    } /* end if */

	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progout_printf) */


int progout_close(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->open.outfile) {
	    pip->open.outfile = FALSE ;
	    bclose(pip->ofp) ;
	}

	return rs ;
}
/* end subroutine (progout_close) */


