/* readfilestrs */

/* we read a string(s) from a file! */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine reads strings line from a file (the string
	of which means something to someone).

	Synopsis:

	int readfilestrs(rbuf,rlen,fname)
	char		rbuf[] ;
	int		rlen ;
	const char	*fname ;

	Arguments:

	rbuf		user supplied buffer to hold result
	rlen		length of user supplied buffer
	fname		file to read

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
extern int	nextfield(const char *,int,const char **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int readfilestrs(rbuf,rlen,fname)
char		*rbuf ;
int		rlen ;
const char	*fname ;
{
	FILEBUF		f ;

	const int	llen = LINEBUFLEN ;
	const int	to = -1 ;

	int	rs ;
	int	cl ;
	int	c = 0 ;
	int	bl = 0 ;

	const char	*tp, *cp ;

	char	lbuf[LINEBUFLEN+1] ;


	if ((rbuf == NULL) || (fname == NULL))
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("readfilestrs: fname=%s\n",fname) ;
#endif

	rbuf[0] = '\0' ;
	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = uc_open(fname,O_RDONLY,0666)) >= 0) {
	    int	fd = rs ;

	    if ((rs = filebuf_start(&f,fd,0L,512,0)) >= 0) {
	        int		ll ;
		const char	*lp ;

	        while ((rs = filebuf_readline(&f,lbuf,llen,to)) > 0) {
	            ll = rs ;

	            if (lbuf[ll - 1] == '\n') ll -= 1 ;
	            lbuf[ll] = '\0' ;

	            lp = lbuf ;
	            if ((tp = strnchr(lp,ll,'#')) != NULL) {
	                ll = (tp-lp) ;
	            }

		    while ((cl = nextfield(lp,ll,&cp)) > 0) {
			if ((rlen-bl) >= (cl+1)) {
			    if (c > 0) rs = rbuf[bl++] = ' ' ;
			    if (rs >= 0) {
	                        strwcpy((rbuf+bl),cp,cl) ;
			        bl += cl ;
			    }
			}
	            } /* end while */

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        filebuf_finish(&f) ;
	    } /* end if (filebuf) */

	    u_close(fd) ;
	} /* end if (open) */

#if	CF_DEBUGS
	debugprintf("readfilestrs: ret rs=%d len=%u\n",rs,bl) ;
#endif

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (readfilestrs) */



