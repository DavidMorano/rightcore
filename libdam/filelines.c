/* filelines */

/* determine the number of lines in a file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Given a file-name we determine the number of lines the file has.

	Synopsis:

	int filelines(cchar *fname,int cc)

	Arguments:

	fname		file-path to check
	cc		optional comment character

	Returns:

	<0		error
	>=0		number of lines in the file


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sfskipwhite(cchar *,int,cchar *) ;
extern int	siskipwhite(cchar *,int) ;
extern int	rmeol(cchar *,int) ;
extern int	isEOL(cchar *) ;


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int filelines(cchar *fname,int cc)
{
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;
	char		*lbuf ;

	if (fname == NULL) return SR_FAULT ;
	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = uc_malloc((llen+1),&lbuf)) >= 0) {
	    BFILE	ifile, *ifp = &ifile ;
	    if ((rs = bopen(ifp,fname,"r",0666)) >= 0) {
	        while ((rs = breadline(ifp,lbuf,llen)) > 0) {
#if	CF_DEBUGS
		    debugprintf("filelines: ll=%u l=>%t<\n",rs,lbuf,rs) ;
#endif
		    if (cc > 0) {
		        int	ll = rmeol(lbuf,rs) ;
			int	si ;
#if	CF_DEBUGS
			debugprintf("filelines: ll=%u l=>%t<\n",ll,lbuf,ll) ;
#endif
			if (ll > 0) {
			    if ((si = siskipwhite(lbuf,ll)) >= 0) {
				if (ll > si) {
				    if (lbuf[si] != cc) n += 1 ;
				}
			    }
			}
		    } else {
		        n += 1 ;
		    }
#if	CF_DEBUGS
		    debugprintf("filelines: n=%u\n",n) ;
#endif
	        } /* end while */
	        rs1 = bclose(ifp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (bfile) */
	    rs1 = uc_free(lbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (m-a-f) */

#if	CF_DEBUGS
	debugprintf("filelines: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (filelines) */


