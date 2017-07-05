/* UNFINISHED */
/* gethostaddr */

/* subroutine to get a host INET address */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int gethostaddr(name,hep,hostbuf,buflen)
	char		name[] ;
	struct hostent	*hep ;
	char		hostbuf[] ;
	int		buflen ;


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#undef	GETHOSTADDR_SYSV
#define	GETHOSTADDR_SYSV	SYSHAS_GETHOSTXXXR


/* external subroutines */

extern int	getnodename(char *,int) ;


/* forward references */


/* external variables */

#if	(! GETHOSTADDR_SYSV)
extern int	h_errno ;
#endif


/* global variables */


/* local structures */


/* exported subroutines */


int gethostaddr(name,hep,hostbuf,hostlen)
const char	name[] ;
struct hostent	*hep ;
char		hostbuf[] ;
int		hostlen ;
{
	struct hostent	he, *lp ;

	int	rs ;

#if	GETHOSTADDR_SYSV
	int	h_errno_local ;
#endif

	char	nnbuf[MAXHOSTNAMELEN+1] ;


	if (hep == NULL)
	    return SR_FAULT ;

	if (hostbuf == NULL)
	    return SR_FAULT ;

	if (name == NULL) {
	    name = nnbuf ;
	    rs = getnodename(nnbuf,MAXHOSTNAMELEN) ;
	}
	if (rs < 0) goto ret0 ;

#if	GETHOSTADDR_SYSV
	h_errno_local = 0 ;
	do {

	    lp = gethostbyname_r(name,
	        &he,hostbuf,hostlen,&h_errno_local) ;

	    if ((lp == NULL) && (h_errno_local == TRY_AGAIN)) 
		sleep(1) ;

	} while ((lp == NULL) && (h_errno_local == TRY_AGAIN)) ;
#else
	do {

	    lp = gethostbyname(name) ;

	    if ((lp == NULL) && (h_errno == TRY_AGAIN)) 
		sleep(1) ;

	} while ((lp == NULL) && (h_errno == TRY_AGAIN)) ;
#endif /* GETHOSTADDR_SYSV */

	if (lp != NULL) {
	    *hep = *lp ;
	} else
	    rs = SR_NOTFOUND ;

ret0:
	return rs ;
}
/* end subroutine (gethostaddr) */



