/* process */

/* process a file */


#define	CF_DEBUG	0


/* revision history:

	= 96/03/01, David A­D­ Morano

	The program was written from scratch to do what
	the previous program by the same name did.


*/


/******************************************************************************

	This subroutine processes a file for SpamAssassin bugs.


******************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<bfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif /* LINEBUFLEN */

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	SPAMHEADER	"x-spam-level"



/* external subroutines */

extern int	headkeymat(const char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	headkey(const char *,int) ;


/* local variables */






int process(pip,ofp,fname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	fname[] ;
{
	bfile	infile ;

	int	rs, len ;
	int	vi, hi ;
	int	cl ;
	int	f_inheader = TRUE ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	*cp ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (strcmp(fname,"-") == 0)
	    rs = bopen(&infile,BFILE_STDIN,"dr",0666) ;

	else
	    rs = bopen(&infile,fname,"r",0666) ;

	if (rs >= 0) {

	    while ((rs = breadline(&infile,linebuf,LINEBUFLEN)) > 0) {

	        len = rs ;
	        if (f_inheader) {

	            if ((vi = headkeymat(SPAMHEADER,linebuf,len)) > 0) {

	                cp = linebuf + vi ;
	                cl = len - vi ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("process: spamheader value=%t\n",
				cp,(cl - 1)) ;
#endif

	                if ((hi = headkey(cp,cl)) >= 0) {

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("process: found screw-up hi=%u\n",hi) ;
#endif

	                    rs = bwrite(ofp,(cp + hi),(cl - hi)) ;

	                    len = 0 ;
	                }

	            } else if (linebuf[0] == '\0')
	                f_inheader = FALSE ;

	        } /* end if (inside header area) */

	        if (len > 0)
	            rs = bwrite(ofp,linebuf,len) ;

		if (rs < 0)
			break ;

	    } /* end while (reading lines) */

	    bclose(&infile) ;

	} /* end if (opened file) */

	return rs ;
}
/* end subroutine (process) */



/* LOCAL SUBROUTINES */



static int headkey(sp,sl)
const char	sp[] ;
int		sl ;
{
	int	hi = 0 ;


	while ((sl > 0) && CHAR_ISWHITE(*sp)) {
	    sp += 1 ;
	    sl -= 1 ;
	    hi += 1 ;
	}

	while ((sl > 0) && (! CHAR_ISWHITE(*sp)) && (*sp != ':')) {
	    sp += 1 ;
	    sl -= 1 ;
	}

	return (*sp == ':') ? hi : -1 ;
}
/* end subroutine (headkey) */



