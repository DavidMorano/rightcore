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



/* external subroutines */

extern int	headkeymat(const char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	headkey(const char *,int) ;


/* local variables */

static const char	*spamkeys[] = {
	"x-spam-level",
	"x-spam-status",
	"x-spam-flag",
	"x-spam-report",
	"x-spam-checker-version",
	NULL
} ;





int process(pip,ofp,fname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	fname[] ;
{
	bfile	infile ;

	int	rs, i, len ;
	int	vi ;
	int	f_inheader = TRUE ;
	int	f_bol, f_eol ;
	int	f ;

	char	linebuf[LINEBUFLEN + 1] ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (strcmp(fname,"-") == 0)
	    rs = bopen(&infile,BFILE_STDIN,"dr",0666) ;

	else
	    rs = bopen(&infile,fname,"r",0666) ;

	vi = -1 ;
	if (rs >= 0) {

		f_bol = TRUE ;
	    while ((rs = breadline(&infile,linebuf,LINEBUFLEN)) > 0) {

	        len = rs ;
		f_eol = (linebuf[len - 1] == '\n') ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
		debugprintf("process: line> %t\n",
		linebuf,(f_eol) ? (len - 1) : len) ;
#endif

	            if (linebuf[0] == '\n')
	                break ;

/* check for a spam header key */

		if (f_bol) {

		    for (i = 0 ; spamkeys[i] != NULL ; i += 1) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
		debugprintf("process: trying >%s<\n",spamkeys[i]) ;
#endif

	                vi = headkeymat(spamkeys[i],linebuf,len) ;

			if (vi > 0)
				break ;

		    } /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
		debugprintf("process: vi=%d\n",vi) ;
#endif

		    if (vi > 0)
			break ;

		}

		f_bol = f_eol ;

	    } /* end while (reading lines) */

	} /* end if (opened file) */

	f = (vi > 0) ;
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (process) */



/* LOCAL SUBROUTINES */



#ifdef	COMMENT

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

#endif /* COMMENT */



