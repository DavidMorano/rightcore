/* findfile */

/* find a file within a given list of directories */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The subroutine was written from scratch. It is modeled after many other
        variations that I have written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutines just searches a list of directories for a specified
        file.

	Synopsis:

	int findfile(idp,plp,mode,pbuf,fbuf,flen)
	IDS		*idp ;
	vecstr		*plp ;
	int		mode ;
	char		pbuf[] ;
	const char	fbuf[] ;
	int		flen ;

	Arguments:

	idp		pointer to IDS object
	plp		pointer to VECSTR object of directories
	mode		mode of file to search for
	pbuf		buffer to receive result
	fbuf		buffer containing filename string to search for
	flen		length of filename string to search for

	Returns:

	>=0		file was found and this is the resulting plen
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<ids.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	sperm(IDS *,USTAT *,int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* local structures */


/* forward references */

static int	mkfpathw(char *,cchar *,cchar *,int) ;
static int	fileperm(IDS *,cchar *,mode_t) ;


/* local variables */


/* exported subroutines */


int findfile(idp,plp,am,pbuf,fbuf,flen)
IDS		*idp ;
vecstr		*plp ;
mode_t		am ;
char		pbuf[] ;
const char	fbuf[] ;
int		flen ;
{
	int		rs = SR_OK ;
	int		pl = 0 ;

	if (idp == NULL) return SR_FAULT ;
	if (plp == NULL) return SR_FAULT ;
	if (pbuf == NULL) return SR_FAULT ;
	if (fbuf == NULL) return SR_FAULT ;

	if (flen < 0)
	    flen = strlen(fbuf) ;

	while ((flen > 0) && (fbuf[flen - 1] == '/')) {
	    flen -= 1 ;
	}

	pbuf[0] = '\0' ;
	if (flen != 0) {
	    int	f_done = FALSE ;

	    if (strnchr(fbuf,flen,'/') != NULL) {
	        if ((rs = mkpath1w(pbuf,fbuf,flen)) >= 0) {
	            pl = rs ;
		    if ((rs = fileperm(idp,pbuf,am)) >= 0) {
		        f_done = TRUE ;
		    } else if (isNotAccess(rs)) {
		        rs = SR_OK ;
		    }
	        }
	    } /* end if (file was already absolute) */

	     if ((rs >= 0) && f_done) {
		int	i ;
		cchar	*pp ;
		for (i = 0 ; (rs = vecstr_get(plp,i,&pp)) >= 0 ; i += 1) {
	            if (pp != NULL) {
	                if ((rs = mkfpathw(pbuf,pp,fbuf,flen)) >= 0) {
	                    pl = rs ;
	                    if ((rs = fileperm(idp,pbuf,am)) >= 0) {
				f_done = TRUE ;
			    } else if (isNotAccess(rs)) {
				rs = SR_OK ;
			    }
	                }
		    } /* end if */
		    if (f_done) break ;
	            if (rs < 0) break ;
	        } /* end for */
	    } /* end for (looping through paths) */

	} /* end if (non-zero) */

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (findfile) */


/* local subroutines */


static int mkfpathw(pbuf,pp,fbuf,flen)
char		pbuf[] ;
const char	pp[] ;
const char	fbuf[] ;
int		flen ;
{
	const int	plen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		pl = 0 ;

	if (pp[0] != '\0') {

	    if (rs >= 0) {
	        rs = storebuf_strw(pbuf,plen,pl,pp,-1) ;
	        pl += rs ;
	    }

	    if ((rs >= 0) && (pl > 0) && (pbuf[pl - 1] != '/')) {
	        rs = storebuf_char(pbuf,plen,pl,'/') ;
	        pl += rs ;
	    }

	} /* end if */

	if ((rs >= 0) && (flen != 0) && (fbuf[0] != '\0')) {
	    rs = storebuf_strw(pbuf,plen,pl,fbuf,flen) ;
	    pl += rs ;
	}

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (mkfpathw) */


/* is it a file and are the permissions what we want? */
static int fileperm(IDS *idp,cchar *fname,mode_t am)
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = u_stat(fname,&sb)) >= 0) {
	    rs = SR_NOTFOUND ;
	    if (S_ISREG(sb.st_mode)) {
	        rs = sperm(idp,&sb,am) ;
	    }
	}

	return rs ;
}
/* end subroutine (fileperm) */


