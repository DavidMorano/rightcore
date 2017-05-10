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
#include	<signal.h>
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

extern int	mkpath2(char *,const char *,const char *) ;


/* external variables */


/* exported subroutines */


int bbhosts_start(vsp,pcs,bbhostfile)
vecstr		*vsp ;
const char	pcs[] ;
const char	bbhostfile[] ;
{
	bfile	hostfile, *hfp = &hostfile ;

	int	rs ;

	const char	*cp, *cp1, *cp2 ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	fname[MAXPATHLEN + 1] ;


	if (vsp == NULL) return SR_FAULT ;

	rs = vecstr_start(vsp,10,VECSTR_PDEFAULT) ;
	if (rs < 0) goto ret0 ;

	rs = mkpath2(fname,pcs,bbhostfile) ;
	if (rs < 0) goto ret0 ;

	if (bopen(hfp,fname,"r",0666) >= 0) {
	    int	len ;

	    while ((rs = breadline(hfp,linebuf,LINEBUFLEN)) > 0) {
		len = rs ;

	        if (linebuf[len - 1] == '\n') len -= 1 ;
	        linebuf[len] = '\0' ;

	        if (len <= 0) 
			continue ;

	        cp = linebuf ;
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

	        vecstr_add(vsp,cp1,cp2 - cp1) ;

	    } /* end while */

	    bclose(hfp) ;
	} /* end if (open) */

ret0:
	return vecstr_count(vsp) ;
}
/* end subroutine (bbhosts_start) */


int bbhosts_finish(vsp)
vecstr		*vsp ;
{


	return vecstr_finish(vsp) ;
}
/* end subroutine (bbhosts_finish) */


int bbhosts_get(bhp,i,rpp)
BBHOSTS		*bhp ;
int		i ;
const char	**rpp ;
{


	return vecstr_get(bhp,i,rpp) ;
}
/* end subroutine (bbhosts_get) */


int bbhosts_find(bhp,s)
BBHOSTS		*bhp ;
const char	s[] ;
{


	return vecstr_find(bhp,s) ;
}
/* end subroutine (bbhosts_find) */


/* private subroutines */



