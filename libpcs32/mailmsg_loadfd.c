/* mailmsg_loadfd */

/* load a mail-message from a file-descriptor (FD) */


#define	CF_DEBUGS	0		/* switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	The subroutine was written from scratch.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine loads a mail-message (into the MAILMSG object) from a
        source that consists of a file-descriptor (FD).

        Note: At first we skip empty lines until we find a non-empty line;
        afterwards we do not ignore empty lines.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<filebuf.h>
#include	<mailmsg.h>
#include	<localmisc.h>


/* local defines */

#ifndef	MAILMSGLINEBUFLEN
#define	MAILMSGLINEBUFLEN	(LINEBUFLEN * 5)
#endif

#define	ISEND(c)	(((c) == '\n') || ((c) == '\r'))


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	nextfield(const char *,int,const char **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* global variables */


/* local variables */


/* exported subroutines */


int mailmsg_loadfd(MAILMSG *mmp,int mfd,offset_t fbo)
{
	FILEBUF		b ;
	const int	bsize = 2048 ;
	int		rs ;
	int		rs1 ;
	int		line = 0 ;
	int		tlen = 0 ;

	if ((rs = filebuf_start(&b,mfd,fbo,bsize,0)) >= 0) {
	    const int	llen = MAILMSGLINEBUFLEN ;
	    char	*lbuf ;
	    if ((rs = uc_malloc((llen+1),&lbuf)) >= 0) {
	        int	ll ;

	        while ((rs = filebuf_readline(&b,lbuf,llen,-1)) > 0) {
	            ll = rs ;

	            tlen += ll ;
	            while ((ll > 0) && ISEND(lbuf[0]))
	                ll -= 1 ;

	            if ((ll > 0) || (line > 0)) {
	                line += 1 ;
	                rs = mailmsg_loadline(mmp,lbuf,ll) ;
	            }

	            if (rs <= 0) break ;
	        } /* end while (reading lines) */

	        rs1 = uc_free(lbuf) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (memory-allocation) */
	    rs1 = filebuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (mailmsg_loadfd) */


