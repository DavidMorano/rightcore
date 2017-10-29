/* process */

/* process the input file into the dictionary files */


#define	CF_DEBUG 	0		/* run-time debug print-outs */


/* revision history:

	= 1996-03-01, David A­D­ Morano
	This subroutine was originally written.

	= 1999-02-23, David A­D­ Morano
	I enhanced the 'process' subroutine to place all words
	that start with the same letter (upper or lowercase) into
	the SAME output dictionary file.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/***********************************************************************

	This subroutine processes ... something ! :-)

	Synopsis:

	int process(gp,dp,fname)
	struct proginfo	*pip ;
	DICTFILES	*dp ;
	const char	fname[] ;

	Arguments:
	- pip		program information pointer
	- dp		pointer to DICTFILES object
	- fname		file to process

	Returns:
	>=0		OK
	<0		error code


***********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<limits.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"dictfiles.h"


/* local defines */


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	isprintlatin(int) ;


/* external variables */


/* forward references */


/* exported subroutines */


int process(pip,dp,fname)
struct proginfo	*pip ;
DICTFILES	*dp ;
const char	fname[] ;
{
	bfile	infile, *ifp = &infile ;

	int	rs ;
	int	ch ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: fname=%s\n",fname) ;
#endif

	if (pip == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: starting\n") ;
#endif

	if (fname[0] == '-') fname = BFILE_STDIN ;

	if ((rs = bopen(ifp,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	        int	len = rs ;

#if	CF_DEBUG && 0
	        if (DEBUGLEVEL(2))
	            debugprintf("process: line> %t",lbuf,len) ;
#endif

	        ch = (lbuf[0] & 0xff) ;
	        if (ch == '\n')
	            continue ;

	        if (lbuf[len - 1] != '\n') {
	            while ((ch = bgetc(ifp)),((ch != SR_EOF) && (ch != '\n'))) ;
	            continue ;
	        }

	        if (len > MAXWORDLEN) continue ;
	        if (! isprintlatin(ch)) continue ;

	        rs = dictfiles_write(dp,lbuf,len) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: dictfiles_write() rs=%d\n",rs) ;
#endif

	        if (rs < 0) break ;
	    } /* end while (looping reading lines) */

	    bclose(ifp) ;
	} /* end if (open) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */


