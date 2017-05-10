/* procfile */

/* process a file */


#define	CF_DEBUG	0


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that
	did similar types of functions.


*/


/******************************************************************************

	This module just provides optional expansion of directories.
	The real work is done by the 'checkname' module.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<paramopt.h>
#include	<ascii.h>
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


/* local variables */


/* exported subroutines */


int procfile(pip,pp,fname)
struct proginfo	*pip ;
PARAMOPT	*pp ;
char		fname[] ;
{
	bfile	infile ;

	int	rs = SR_OK ;
	int	i, len ;
	int	c_left, c_right ;

	char	linebuf[LINEBUFLEN + 1] ;


	if (fname == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procfile: entered fname=\"%s\"\n",fname) ;
#endif

	if ((fname[0] != '\0') && (strcmp(fname,"-") != 0))
	    rs = bopen(&infile,fname,"r",0666) ;

	else
	    rs = bopen(&infile,BFILE_STDIN,"r",0666) ;

	if (rs < 0)
	    goto ret0 ;

	c_left = c_right = 0 ;
	while ((rs = breadline(&infile,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    for (i = 0 ; i < len ; i += 1) {

	        if (linebuf[i] == CH_LBRACE)
	            c_left += 1 ;

	        else if (linebuf[i] == CH_RBRACE)
	            c_right += 1 ;

	    } /* end for */

	} /* end while */

	bclose(&infile) ;

	if (rs >= 0)
	    bprintf(pip->ofp,"%d %d\n",c_left,c_right) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procfile) */



