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


/* local forward references */


/* external variables */


/* global variables */


/* local variables */


/* exported subroutines */


int process(pip,name,pp)
PROGINFO	*pip ;
char		name[] ;
PARAMOPT	*pp ;
{
	bfile		infile ;
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		i, len ;
	int		c_left, c_right ;
	char		lbuf[LINEBUFLEN + 1] ;

	if (name == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("process: entered name=\"%s\"\n",name) ;
#endif

	if ((name[0] == '\0') || (strcmp(name,"-") == 0)) {
	    rs = bopen(&infile,BIO_STDIN,"r",0666) ;
	} else {
	    rs = bopen(&infile,name,"r",0666) ;
	}

	if (rs < 0)
	    return rs ;

	c_left = c_right = 0 ;
	while ((len = breadline(&infile,lbuf,llen)) > 0) {

	    for (i = 0 ; i < len ; i += 1) {
	        if (lbuf[i] == '{') {
	            c_left += 1 ;
	        } else if (lbuf[i] == '}') {
	            c_right += 1 ;
		}
	    }

	} /* end while */

	bclose(&infile) ;


	bprintf(pip->ofp,"%d %d\n",c_left,c_right) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("process: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */


