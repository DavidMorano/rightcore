/* process */

/* process a file (I really have no idea what this was supposed to do!) */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The program was written from scratch to do what the previous
	program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine processes a file for SpamAssassin bugs.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<bfile.h>
#include	<localmisc.h>

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


/* exported subroutines */


int process(pip,ofp,fname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	fname[] ;
{
	bfile	ifile, *ifp = &ifile ;

	int	rs ;
	int	i ;
	int	vi ;
	int	f_inheader = TRUE ;
	int	f_bol, f_eol ;
	int	f ;


	if (fname == NULL)
	    return SR_FAULT ;

	if ((fname[0] == '\0') || (fname[0] == '-')) fname = BFILE_STDIN ;

	if ((rs = bopen(ifp,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    vi = -1 ;
	    f_bol = TRUE ;
	    while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	        len = rs ;

		f_eol = (lbuf[len - 1] == '\n') ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
		debugprintf("process: line> %t\n",
		lbuf,(f_eol) ? (len - 1) : len) ;
#endif

	            if (lbuf[0] == '\n')
	                break ;

/* check for a spam header key */

		if (f_bol) {

		    for (i = 0 ; spamkeys[i] != NULL ; i += 1) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
		debugprintf("process: trying >%s<\n",spamkeys[i]) ;
#endif

	                vi = headkeymat(spamkeys[i],lbuf,len) ;

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

		if (rs < 0) break ;
	    } /* end while (reading lines) */

	    bclose(ifp) ;
	} /* end if (opened file) */

	f = (vi > 0) ;
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (process) */


/* local subroutines */


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


