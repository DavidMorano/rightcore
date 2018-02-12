/* bbhosts */

/* get the list of BB hosts from a PCS site BB hosts file */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was adapted from a prior version.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We just store node (host) names for use later in broadcasting the
        message around.


*******************************************************************************/

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<char.h>
#include	<localmisc.h>

#include	"bbhosts.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 1),2048)
#endif


/* external subroutines */

extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	isNotPresent(int) ;


/* external variables */


/* exported subroutines */


int bbhosts_start(BBHOSTS *vsp,cchar *pcs,cchar *bbhostfile)
{
	const int	vo = VECSTR_PDEFAULT ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (vsp == NULL) return SR_FAULT ;

	if ((rs = vecstr_start(vsp,10,vo)) >= 0) {
	    char	fname[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(fname,pcs,bbhostfile)) >= 0) {
	        bfile	hostfile, *hfp = &hostfile ;
	        if ((rs = bopen(hfp,fname,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            const char	*cp, *cp1, *cp2 ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = breadline(hfp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if (len <= 0)
	                    continue ;

	                cp = lbuf ;
	                while (CHAR_ISWHITE(*cp))
	                    cp += 1 ;

	                if (cp[0] == '#')
	                    continue ;

	                cp1 = cp ;
	                while (*cp && (! CHAR_ISWHITE(*cp)))
	                    cp += 1 ;

	                cp2 = cp ;
	                if (cp1 == cp2)
	                    continue ;

			c += 1 ;
	                rs = vecstr_add(vsp,cp1,cp2 - cp1) ;

			if (rs < 0) break ;
	            } /* end while */

	            rs1 = bclose(hfp) ;
	            if (rs >= 0) rs = rs1 ;
	        } else if (isNotPresent(rs)) {
	            rs = SR_OK ;
	        } /* end if (file) */
	    } /* end if */
	} /* end if (vecstr-start) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bbhosts_start) */


int bbhosts_finish(BBHOSTS *vsp)
{

	return vecstr_finish(vsp) ;
}
/* end subroutine (bbhosts_finish) */


int bbhosts_get(BBHOSTS *bhp,int i,cchar **rpp)
{

	return vecstr_get(bhp,i,rpp) ;
}
/* end subroutine (bbhosts_get) */


int bbhosts_find(BBHOSTS *bhp,cchar *s)
{

	return vecstr_find(bhp,s) ;
}
/* end subroutine (bbhosts_find) */


/* private subroutines */



