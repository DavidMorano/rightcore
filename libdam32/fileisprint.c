/* fileisprint */

/* is the specified file totally printable? */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine checks if the specified file is
	totally printable.

	Synopsis:

	int fileisprint(fname)
	const char	*fname ;

	Arguments:

	fname		file to check

	Returns:

	>=0		length of return organization string
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<filebuf.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	hasprintbad(const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int fileisprint(fname)
const char	fname[] ;
{
	const int	to = -1 ;

	int	rs ;
	int	f = FALSE ;


	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("fileisprint: fname=%s\n",fname) ;
#endif

	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = uc_open(fname,O_RDONLY,0666)) >= 0) {
	    FILEBUF	b ;
	    int		fd = rs ;
	    if ((rs = filebuf_start(&b,fd,0L,512,0)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		ll ;
	        char		lbuf[LINEBUFLEN+1] ;

	        while ((rs = filebuf_readline(&b,lbuf,llen,to)) > 0) {
	            ll = rs ;

	            if (lbuf[ll - 1] == '\n') ll -= 1 ;
	            lbuf[ll] = '\0' ;

		    f = hasprintbad(lbuf,ll) ;
		    if (f) break ;

	            if (rs < 0) break ;
	        } /* end while (reading lines) */
	        filebuf_finish(&b) ;
	    } /* end if (filebuf) */
	    u_close(fd) ;
	} /* end if (open) */

#if	CF_DEBUGS
	debugprintf("fileisprint: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? (!f) : rs ;
}
/* end subroutine (fileisprint) */



