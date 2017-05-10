/* process */

/* process a name */


#define	F_DEBUG		0


/* revision history :

	= 96/03/01, David A­D­ Morano

	The subroutine was adapted from others programs that
	did similar types of functions.


*/


/******************************************************************************

	This module just provides optional expansion of directories.
	The real work is done by the 'checkname' module.



******************************************************************************/


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
#include	<field.h>

#include	"misc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	LINELEN
#define	LINELEN		100
#endif


/* external subroutines */

extern char	*strbasename(char *) ;


/* local forward references */


/* external variables */


/* global variables */


/* local variables */






int process(pip,name,pp)
struct proginfo	*pip ;
char		name[] ;
PARAMOPT	*pp ;
{
	bfile	infile ;

	int	rs ;
	int	i, len ;
	int	c_left, c_right ;

	char	linebuf[LINELEN + 1] ;


	if (name == NULL)
	    return SR_FAULT ;

#if	F_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("process: entered name=\"%s\"\n",name) ;
#endif

	if ((name[0] == '\0') || (strcmp(name,"-") == 0))
	    rs = bopen(&infile,BIO_STDIN,"r",0666) ;

	else
	    rs = bopen(&infile,name,"r",0666) ;

	if (rs < 0)
	    return rs ;

	c_left = c_right = 0 ;
	while ((len = bgetline(&infile,linebuf,LINELEN)) > 0) {

	    for (i = 0 ; i < len ; i += 1) {

	        if (linebuf[i] == '{')
	            c_left += 1 ;

	        else if (linebuf[i] == '}')
	            c_right += 1 ;

	    }

	} /* end while */

	bclose(&infile) ;


	bprintf(pip->ofp,"%d %d\n",c_left,c_right) ;

#if	F_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("process: rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */



