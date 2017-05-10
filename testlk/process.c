/* process */

/* process a log table file */


#define	CF_DEBUG	1		/* switchable debug print-outs */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was adapted from other programs that
	do similar things.



*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine handles the processing for one lockfile.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#ifndef	LINELEN
#define	LINELEN		100
#endif


/* external subroutines */


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int process(pip,ofp,lockfname,timeout,to_remove)
struct proginfo	*pip ;
bfile		*ofp ;
char		lockfname[] ;
int		timeout, to_remove ;
{
	struct ustat	sb ;

	bfile	lkfile ;

	int	rs, i ;
	int	len ;

	cchar	*cp ;
	char	linebuf[LINELEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	timebuf1[TIMEBUFLEN + 1] ;
	char	timebuf2[TIMEBUFLEN + 1] ;


	if ((lockfname == NULL) || (lockfname[0] == '\0'))
	    return SR_FAULT ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("process: lockfname=\"%s\"\n",
	        lockfname) ;
#endif


	rs = bopen(&lkfile,lockfname,"rw",0666) ;

	if (rs < 0)
	    goto badopen ;

	rs = bcontrol(&lkfile,BC_LOCK,timeout) ;

	if (rs < 0)
	    goto badlock ;

	if (rs < 0) {

	    bprintf(ofp,"bad lock (%d)\n", rs) ;

	}

	rs = breadline(&lkfile,linebuf,LINELEN) ;

	len = rs ;
	if (rs >= 0) {

	    if (linebuf[len - 1] == '\n')
	        linebuf[--len] = '\0' ;

	    if (strncmp(linebuf,"GOOD",4) == 0) {

	        bseek(&lkfile,0L,SEEK_SET) ;

	        bprintf(&lkfile,"BAD    \n") ;

	        for (i = 0 ; i < 20 ; i += 1) {

	            sleep(1) ;

	            bseek(&lkfile,0L,SEEK_SET) ;

			breadline(&lkfile,linebuf,LINELEN) ;

	            if (strncmp(linebuf,"BAD",3) != 0) {

	                bprintf(ofp,"string changed> %s\n",
	                    linebuf) ;

	                bseek(&lkfile,0L,SEEK_SET) ;

	                bprintf(&lkfile,"BAD    \n") ;

	            }

	        } /* end for */

	        bseek(&lkfile,0L,SEEK_SET) ;

	        bprintf(&lkfile,"GOOD   \n") ;

	    } else
	        bprintf(ofp,"bad string> %s\n",
	            linebuf) ;

	} /* end if (was able to read a line) */

badlock:
	bclose(&lkfile) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("process: ret rs=%d\n",rs) ;
#endif

badopen:
	return rs ;
}
/* end subroutine (process) */



