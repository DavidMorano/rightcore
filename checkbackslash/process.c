/* process */

/* process a name */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history :

	= 1996-03-01, David A­D­ Morano
        The subroutine was adapted from others programs that did similar types
        of functions.

*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This module just provides optional expansion of directories. The real
        work is done by the 'checkname' module.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

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

extern char	*strbasename(char *) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int process(pip,fname,pp)
PROGINFO	*pip ;
char		fname[] ;
PARAMOPT	*pp ;
{
	bfile		infile ;
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		i, len ;
	int		line ;
	int		c_total, c_bad ;
	char		lbuf[LINEBUFLEN + 1] ;

	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("process: entered fname=\"%s\"\n",fname) ;
#endif

	if ((fname[0] == '\0') || (strcmp(fname,"-") == 0)) {
	    rs = bopen(&infile,BIO_STDIN,"r",0666) ;
	] else {
	    rs = bopen(&infile,fname,"r",0666) ;
	}

	if (rs < 0)
	    return rs ;

	c_total = 0 ;
	c_bad = 0 ;

	line = 0 ;
	while ((len = breadline(&infile,lbuf,LINEBUFLEN)) > 0) {

	    if (lbuf[len - 1] == '\n') line += 1 ;

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
	    } /* end for */

	} /* end while */

	bclose(&infile) ;

	bprintf(pip->ofp,"file=%s total=%d bad=%d\n",
		fname,c_total,c_bad) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("process: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */


