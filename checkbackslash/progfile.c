/* progfile */

/* process a name */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that did similar types
	of functions.


*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This module just provides optional expansion of directories.  The real
	work is done by the 'checkname' module.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<paramopt.h>
#include	<field.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int progfile(PROGINFO *pip,PARAMOPT *pp,cchar *fname)
{
	bfile		ifile, *ifp = &ifile ;
	int		rs ;
	int		c_total = 0 ;
	int		c_bad = 0 ;

	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("progfile: ent fname=\"%s\"\n",fname) ;
#endif

	if ((fname[0] == '\0') || (strcmp(fname,"-") == 0))
	    fname = BFILE_STDIN ;

	if ((rs = bopen(ifp,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		line = 0 ;
	    int		i ;
	    char		lbuf[LINEBUFLEN + 1] ;
	    while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	        int	len = rs ;

	        for (i = 0 ; i < len ; i += 1) {

	            if ((lbuf[i] == '\\') && ((i + 1) < len)) {

	                if (lbuf[i + 1] == '\n') {

	                    c_total += 1 ;

	                } else if (lbuf[i + 1] == ' ') {

	                    c_total += 1 ;
	                    c_bad += 1 ;

	                    bprintf(pip->ofp,"file=%s line=%d\n",
	                        fname,line) ;

	                }

	            } /* end something (had one) */
	            if (rs < 0) break ;
	        } /* end for */
	        line += 1 ;
	        if (rs < 0) break ;
	    } /* end while */
	    bclose(&ifile) ;
	} /* end if (file) */

	if ((rs >= 0) && (pip->verboselevel > 0)) {
	    bprintf(pip->ofp,"file=%s total=%d bad=%d\n",
	        fname,c_total,c_bad) ;
	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("progfile: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progfile) */


