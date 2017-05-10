/* address */

/* parse email route addresses into host and local parts */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2002-05-30, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This subroutine will parse email route addresses into hostname and
	localname parts.  The assumption is that only route addresses are given
	to us.  If this is wrong, the results are indeterminate.  The hostname
	part is just the first host in the address as if the "focus" (using
	SENDMAIL language) was on the first host.

	Synopsis:

	int addressparse(buf,buflen,parthost,partlocal)
	const char	buf[] ;
	int		buflen ;
	char		parthost[], partlocal[] ;

	Synopsis:

	int addressjoin(buf,buflen,parthost,partlocal,type)
	char		buf[] ;
	int		buflen ;
	const char	parthost[], partlocal[] ;
	int		type ;

	Synopsis:

	int addressarpa(buf,buflen,parthost,partlocal,type)
	char		buf[] ;
	int		buflen ;
	const char	parthost[], partlocal[] ;
	int		type ;

	Arguments:

	buf		string buffer containing route address
	buflen		length of string buffer
	parthost	supplied buffer to receive parthost
	partlocal	supplied buffer to receive partlocal
	type		type of address desired

	Returns:

	0		local address
	1		UUCP
	2		ARPAnet normal
	3		ARPAnet route address
	<0		bad address of some kind


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>

#include	"address.h"


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* local variables */


/* exported subroutines */


int addressparse(s,slen,parthost,partlocal)
const char	s[] ;
int		slen ;
char		parthost[], partlocal[] ;
{
	int		rs = SR_OK ;
	int		t ;
	int		mal = MAILADDRLEN ;
	const char	*localhostpart = ADDRESS_LOCALHOST ;
	const char	*cp ;
	const char	*cp1, *cp2 ;

	if (s == NULL) return SR_FAULT ;
	if (parthost == NULL) return SR_FAULT ;
	if (partlocal == NULL) return SR_FAULT ;

	parthost[0] = '\0' ;
	partlocal[0] = '\0' ;
	if (slen < 0)
	    slen = strlen(s) ;

/* what kind of address do we have? */

	if ((cp1 = strnchr(s,slen,'@')) != NULL) {

#if	CF_DEBUGS
	    debugprintf("addressparse: @\n") ;
#endif

	    if ((cp2 = strnchr(s,slen,':')) != NULL) {

#if	CF_DEBUGS
	        debugprintf("addressparse: :\n") ;
#endif

/* ARPAnet route address */

	        t = ADDRESSTYPE_ARPAROUTE ;
	        if ((cp = strnchr(s,slen,',')) != NULL) {

	            if (rs >= 0)
	                rs = snwcpy(parthost,mal,(cp1 + 1),(cp - (cp1 + 1))) ;

	            if (rs >= 0)
	                rs = sncpy1(partlocal,mal,(cp + 1)) ;

	        } else {

	            if (rs >= 0)
	                rs = snwcpy(parthost,mal,(cp1 + 1),cp2 - (cp1 + 1)) ;

	            if (rs >= 0)
	                rs = sncpy1(partlocal,mal,(cp2 + 1)) ;

	        } /* end if */

	    } else {

/* normal ARPAnet address */

	        t = ADDRESSTYPE_ARPA ;
	        if (rs >= 0)
	            rs = sncpy1(parthost,mal,(cp1 + 1)) ;

	        if (rs >= 0)
	            rs = snwcpy(partlocal,mal,s,(cp1 - s)) ;

	    } /* end if */

	} else if ((cp = strnrchr(s,slen,'!')) != NULL) {

#if	CF_DEBUGS
	    debugprintf("addressparse: !\n") ;
#endif

	    t = ADDRESSTYPE_UUCP ;
	    if (rs >= 0)
	        rs = snwcpy(parthost,mal,s,(cp - s)) ;

	    if (rs >= 0)
	        rs = sncpy1(partlocal,mal,(cp + 1)) ;

	} else {

#if	CF_DEBUGS
	    debugprintf("addressparse: NONE@\n") ;
#endif

/* local */

	    t = ADDRESSTYPE_LOCAL ;
	    if (rs >= 0)
	        rs = sncpy1(parthost,mal,localhostpart) ;

	    if (rs >= 0)
	        rs = sncpy1(partlocal,mal,s) ;

	} /* end if */

/* perform some cleanup on host-domain part (remove stupid trailing dots) */

	{
	    int	cl = strlen(parthost) ;
	    while ((cl > 0) && (parthost[cl - 1] == '.')) {
	        cl -= 1 ;
	    }
	    parthost[cl] = '\0' ;
	}

	return (rs >= 0) ? t : rs ;
}
/* end subroutine (addressparse) */


/* put an address back together as it was ORIGINALLY */
int addressjoin(buf,buflen,parthost,partlocal,type)
char		buf[] ;
int		buflen ;
const char	parthost[], partlocal[] ;
int		type ;
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (type >= 0) {

	    switch (type) {

	    case ADDRESSTYPE_LOCAL:
	  	{
	            rs = storebuf_strw(buf,buflen,i,partlocal,-1) ;
	            i += rs ;
		}
	        break ;

	    case ADDRESSTYPE_UUCP:
		{
	            rs = storebuf_strw(buf,buflen,i, parthost,-1) ;
	            i += rs ;
		}
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, "!",1) ;
	            i += rs ;
		}
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, partlocal,-1) ;
	            i += rs ;
		}
	        break ;

	    case ADDRESSTYPE_ARPA:
		{
	            rs = storebuf_strw(buf,buflen,i, partlocal,-1) ;
	            i += rs ;
		}
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, "@",1) ;
	            i += rs ;
		}
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, parthost,-1) ;
	            i += rs ;
		}
	        break ;

	    case ADDRESSTYPE_ARPAROUTE:
		{
	            rs = storebuf_strw(buf,buflen,0, "@",1) ;
	            i += rs ;
		}
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, parthost,-1) ;
	            i += rs ;
		}
	        if (rs >= 0) {
	            if (strchr(partlocal,':') != NULL) {
	                rs = storebuf_strw(buf,buflen,i, ",",1) ;
	            } else
	                rs = storebuf_strw(buf,buflen,i, ":",1) ;
	            i += rs ;
	        }
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, partlocal,-1) ;
	            i += rs ;
		}
	        break ;

	    } /* end switch */

	} else {

	    rs = storebuf_strw(buf,buflen,i, partlocal,-1) ;
	    i += rs ;

	} /* end if */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (addressjoin) */


/* put an address back together as its ARPA form only */
int addressarpa(buf,buflen,parthost,partlocal,type)
char		buf[] ;
int		buflen ;
const char	parthost[], partlocal[] ;
int		type ;
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (type >= 0) {

	    switch (type) {

	    case ADDRESSTYPE_LOCAL:
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, partlocal,-1) ;
	            i += rs ;
		}
	        break ;

	    case ADDRESSTYPE_UUCP:
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, partlocal,-1) ;
	            i += rs ;
		}
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, "@",1) ;
	            i += rs ;
		}
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, parthost,-1) ;
	            i += rs ;
		}
	        break ;

	    case ADDRESSTYPE_ARPA:
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, partlocal,-1) ;
	            i += rs ;
		}
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, "@",1) ;
	            i += rs ;
		}
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, parthost,-1) ;
	            i += rs ;
		}
	        break ;

	    case ADDRESSTYPE_ARPAROUTE:
		if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, "@",1) ;
	            i += rs ;
		}
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, parthost,-1) ;
	            i += rs ;
		}
	        if (rs >= 0) {
	            if (strchr(partlocal,':') != NULL) {
	                rs = storebuf_strw(buf,buflen,i, ",",1) ;
	            } else
	                rs = storebuf_strw(buf,buflen,i, ":",1) ;
	            i += rs ;
	        }
	        if (rs >= 0) {
	            rs = storebuf_strw(buf,buflen,i, partlocal,-1) ;
	            i += rs ;
		}

	        break ;

	    } /* end switch */

	} else {
	    rs = storebuf_strw(buf,buflen,i, partlocal,-1) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (addressarpa) */


