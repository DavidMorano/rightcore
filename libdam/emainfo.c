/* emainfo */

/* parse mail route-address host and local parts */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will parse email route addresses into hostname and
	localname parts.  The assumption is that only route addresses are given
	to us.  If this is wrong, the results are indeterminate.  The hostname
	part is just the first host in the address as if the "focus" (using
	SENDMAIL language) was on the first host.

	Synopsis:

	int emainfo(eip,sp,sl)
	EMAINFO		*eip ;
	const char	sp[] ;
	int		sl ;

	Arguments:

	eip		pointer to 'emainfo' structure
	sp		string buffer containing route emainfo
	sl		length of string buffer

	Returns:

	type
	0		local emainfo
	1		UUCP
	2		ARPAnet normal
	3		ARPAnet route emainfo
	<0		bad emainfo of some kind


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<storebuf.h>
#include	<localmisc.h>

#include	"emainfo.h"


/* external subroutines */

extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*strwcpy(char *,const char *,int) ;


/* local variables */


/* exported subroutines */


int emainfo(EMAINFO *eip,cchar *sp,int sl)
{
	const char	*cp ;
	const char	*cp1, *cp2 ;

	memset(eip,0,sizeof(EMAINFO)) ;

	if (sl < 0)
	    sl = strlen(sp) ;

/* what kind of address do we have? */

	if ((cp1 = strnchr(sp,sl,'@')) != NULL) {

	    if ((cp2 = strnchr(sp,sl,':')) != NULL) {

/* ARPAnet route address */

	        eip->type = EMAINFO_TARPAROUTE ;
	        if ((cp = strnchr(sp,sl,',')) != NULL) {

	            eip->host = (cp1 + 1) ;
	            eip->hlen = (cp - (cp1 + 1)) ;

	            eip->local = (cp + 1) ;
	            eip->llen = (sp + sl) - (cp + 1) ;

	        } else {

	            eip->host = (cp1 + 1) ;
	            eip->hlen = (cp2 - (cp1 + 1)) ;

	            eip->local = (cp2 + 1) ;
	            eip->llen = (sp + sl) - (cp2 + 1) ;

	        } /* end if */

	    } else {

/* normal ARPAnet address */

	        eip->type = EMAINFO_TARPA ;

	        eip->host = (cp1 + 1) ;
	        eip->hlen = (sp + sl) - (cp1 + 1) ;

	        eip->local = sp ;
	        eip->llen = (cp1 - sp) ;

	    } /* end if */

	} else if ((cp = strnrchr(sp,sl,'!')) != NULL) {

	    eip->type = EMAINFO_TUUCP ;

	    eip->host = sp ;
	    eip->hlen = (cp - sp) ;

	    eip->local = (cp + 1) ;
	    eip->llen = (sp + sl) - (cp + 1) ;
	    eip->hlen = (cp - sp) ;

	} else {

/* local */

	    eip->type = EMAINFO_TLOCAL ;

	    eip->host = NULL ;
	    eip->hlen = 0 ;

	    eip->local = sp ;
	    eip->llen = sl ;

	} /* end if */

	return eip->type ;
}
/* end subroutine (emainfo) */


int emainfo_mktype(EMAINFO *eip,int type,char *rbuf,int rlen)
{
	int		rs = 0 ;
	int		i = 0 ;

	if (type >= 0) {
	    switch (type) {

	    case EMAINFO_TLOCAL:
	        rs = strwcpy(rbuf,eip->local,MIN(eip->llen,rlen)) - rbuf ;
	        i += rs ;
	        break ;

	    case EMAINFO_TUUCP:
	        if ((eip->host != NULL) && (eip->hlen >= 0)) {
	            rs = storebuf_strw(rbuf,rlen,i,eip->host,eip->hlen) ;
	            i += rs ;
		    if (rs >= 0) {
	                rs = storebuf_char(rbuf,rlen,i,'!') ;
	                i += rs ;
		    }
	        } /* end if (had a host part) */
		if (rs >= 0) {
	            rs = storebuf_strw(rbuf,rlen,i,eip->local,eip->llen) ;
	            i += rs ;
		}
	        break ;

	    case EMAINFO_TARPA:
	        rs = storebuf_strw(rbuf,rlen,i,eip->local,eip->llen) ;
	        i += rs ;
	        if ((rs >= 0) && (eip->host != NULL) && (eip->hlen >= 0)) {
	            rs = storebuf_char(rbuf,rlen,i,'@') ;
	            i += rs ;
	            if (rs >= 0) {
	                rs = storebuf_strw(rbuf,rlen,i,eip->host,eip->hlen) ;
	                i += rs ;
		    }
	        } /* end if */
	        break ;

	    case EMAINFO_TARPAROUTE:
	        if ((eip->host != NULL) && (eip->hlen >= 0)) {
	            rs = storebuf_char(rbuf,rlen,i,'@') ;
	            i += rs ;
		    if (rs >= 0) {
	                rs = storebuf_strw(rbuf,rlen,i,eip->host,eip->hlen) ;
	                i += rs ;
		    }
		    if (rs >= 0) {
	                if (strnchr(eip->local,eip->llen,':') != NULL) {
	                    rs = storebuf_char(rbuf,rlen,i,',') ;
			    i += rs ;
	                } else {
	                    rs = storebuf_char(rbuf,rlen,i,':') ;
	                    i += rs ;
		        }
		    }
	        } /* end if (had a host part) */
		if (rs >= 0) {
	            rs = storebuf_strw(rbuf,rlen,i,eip->local,eip->llen) ;
	            i += rs ;
		}
	        break ;

	    } /* end switch */
	} else {
	    i = strwcpy(rbuf,eip->local,MIN(eip->llen,rlen)) - rbuf ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (emainfo_mktype) */


