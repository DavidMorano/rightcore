/* readfileline */

/* we read one line from a file! */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine reads a single line from a file (the string of which
	means something to someone).

	Synopsis:

	int readfileline(rbuf,rlen,fname)
	char		rbuf[] ;
	int		rlen ;
	const char	*fname ;

	Arguments:

	rbuf		user supplied buffer to hold result
	rlen		length of user supplied buffer
	fname		file to read

	Returns:

	>=0		length of returned string
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

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;

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


int readfileline(char *rbuf,int rlen,cchar *fname)
{
	const int	to = -1 ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;

	if (rbuf == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("readfileline: fname=%s\n",fname) ;
#endif

	rbuf[0] = '\0' ;
	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = uc_open(fname,O_RDONLY,0666)) >= 0) {
	    FILEBUF	b ;
	    int		fd = rs ;
	    if ((rs = filebuf_start(&b,fd,0L,512,0)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		ll ;
		int		cl ;
		cchar		*tp, *cp ;
		char		lbuf[LINEBUFLEN+1] ;

	        while ((rs = filebuf_readline(&b,lbuf,llen,to)) > 0) {
	            ll = rs ;

	            if (lbuf[ll - 1] == '\n') ll -= 1 ;
	            lbuf[ll] = '\0' ;

	            cp = lbuf ;
	            cl = ll ;

#if	CF_DEBUGS
	            debugprintf("readfileline: line=>%t<\n",cp,cl) ;
#endif

	            while ((cl > 0) && CHAR_ISWHITE(*cp)) {
	                cp += 1 ;
	                cl -= 1 ;
	            }

	            if ((tp = strnchr(cp,cl,'#')) != NULL) {
	                cl = (tp-cp) ;
	            }

	            while ((cl > 0) && CHAR_ISWHITE(cp[cl-1])) {
	                cl -= 1 ;
	            }

#if	CF_DEBUGS
	            debugprintf("readfileline: cl=%u reduced=>%t<\n",
				cl,cp,cl) ;
#endif

	            if (cl > 0) {
	                rs = sncpy1w(rbuf,rlen,cp,cl) ;
	                len = rs ;
	            }

	            if (len > 0) break ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = filebuf_finish(&b) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (filebuf) */
	    rs1 = u_close(fd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (uc_open) */

#if	CF_DEBUGS
	debugprintf("readfileline: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (readfileline) */


