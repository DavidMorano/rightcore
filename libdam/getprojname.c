/* getprojname */

/* get the default project for a given username (UNIX® Solaris® thing) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine is an extraction of code from elsewhere.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine retrieves the default project for a given username.

	Synopsis:

	int getprojname(rbuf,rlen,un)
	char		rbuf[] ;
	int		rlen ;
	const char	*un ;

	Arguments:

	rbuf		buffer to receive the nodename 
	rlen		length of supplied buffer (should be NODENAMELEN)
	un		username to lookup

	Returns:

	<0		could not get the nodename (should be pretty rare!)
	>=0		length of retrieved nodename


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<getxusername.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy4(char *,int, cchar *,cchar *,cchar *,cchar *) ;
extern int	snwcpylc(const char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;

extern char	*strnchr(const char *,int,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getprojname(char *nbuf,int nlen,cchar *un)
{
	const int	ulen = USERNAMELEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		nl = 0 ;
	char		ubuf[USERNAMELEN+1] ;

	if (nbuf == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if ((un[0] == '-') || (un[0] == '\0')) {
	    un = ubuf ;
	    rs = getusername(ubuf,ulen,-1) ;
	}

	if (rs >= 0) {
	    struct project	pj ;
	    const int		pjlen = getbufsize(getbufsize_pj) ;
	    char		*pjbuf ;
	    if ((rs = uc_malloc((pjlen+1),&pjbuf)) >= 0) {
	        if ((rs = uc_getdefaultproj(un,&pj,pjbuf,pjlen)) >= 0) {
	            rs = sncpy1(nbuf,nlen,pj.pj_name) ;
		    nl = rs ;
		}
		rs1 = uc_free(pjbuf) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (memory-allocation) */
	} /* end if (ok) */

	return (rs >= 0) ? nl : rs ;
}
/* end subroutine (getprojname) */


