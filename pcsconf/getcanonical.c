/* getcanonical */

/* get a canonical host name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	int getcanonical()


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */

#ifndef	HOSTBUFLEN
#define	HOSTBUFLEN		MAXPATHLEN
#endif

#undef	GETCANONICAL_SYSV
#define	GETCANONICAL_SYSV	SYSHAS_GETHOSTXXXR


/* external subroutines */

extern char	*strwcpy(char *,char *,int) ;


/* forward references */


/* external variables */


#if	(! GETCANONICAL_SYSV)
extern int	h_errno ;
#endif


/* global variables */


/* local structures */


/* exported subroutines */


int getcanonical(name,hostname,nodename,domainname)
char	name[] ;
char	hostname[] ;
char	nodename[], domainname[] ;
{
	struct hostent	he, *hep ;

	struct utsname	uts ;

	int	i ;

#if	GETCANONICAL_SYSV
	int	h_errno_local ;
#endif

	char	hostbuf[HOSTBUFLEN + 1] ;
	char	*cp, *cp1 ;


/* are we "doing" ourselves? */

	if (name == NULL) {

	    if (uname(&uts) >= 0) {
	        name = uts.nodename ;

	    } else
	        name = "localhost" ;

	}

#if	GETCANONICAL_SYSV
	h_errno_local = 0 ;
	do {
	    hep = gethostbyname_r(name,
	        &he,hostbuf,HOSTBUFLEN,&h_errno_local) ;
	    if ((hep == NULL) && (h_errno_local == TRY_AGAIN)) sleep(1) ;
	} while ((hep == NULL) && (h_errno_local == TRY_AGAIN)) ;
#else
	do {
	    hep = gethostbyname(name) ;
	    if ((hep == NULL) && (h_errno == TRY_AGAIN)) sleep(1) ;
	} while ((hep == NULL) && (h_errno == TRY_AGAIN)) ;
#endif /* GETCANONICAL_SYSV */

	if (hep != NULL) {

#if	CF_DEBUGS
	    debugprintf("getcanonical: their hostname h=\"%s\"\n",
	        hep->h_name) ;
#endif

	    if ((cp = strchr(hep->h_name,'.')) != NULL) {

#if	CF_DEBUGS
	        debugprintf("getcanonical: hostname has domain h=\"%s\"\n",
	            hep->h_name) ;
#endif

	        strcpy(hostname,hep->h_name) ;

	        *cp++ = '\0' ;
	        if (nodename != NULL)
	            strcpy(nodename,hep->h_name) ;

	        if (domainname != NULL)
	            strcpy(domainname,cp) ;

	        return OK ;

	    } /* end if */

	    for (i = 0 ; hep->h_aliases[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	        debugprintf("getcanonical: alias%d a=\"%s\"\n",
	            i,hep->h_aliases[i]) ;
#endif

	        if ((cp = strchr(hep->h_aliases[i],'.')) != NULL) {

	            strcpy(hostname,hep->h_aliases[i]) ;

	            *cp++ = '\0' ;
	            if (nodename != NULL)
	                strcpy(nodename,hep->h_aliases[i]) ;

	            if (domainname != NULL)
	                strcpy(domainname,cp) ;

	            return OK ;

	        } /* end if */

	    } /* end for */

	} /* end if */

/* assume that any dots in the original name means that it is fully qualled */

	if ((cp = strchr(name,'.')) != NULL) {

#if	CF_DEBUGS
	    debugprintf("getcanonical: using original name n=\"%s\"\n",
	        name) ;
#endif

	    strcpy(hostname,name) ;

	    if (nodename != NULL)
	        strwcpy(nodename,name,cp - name) ;

	    if (domainname != NULL)
	        strcpy(domainname,cp + 1) ;

	    return OK ;

	} /* end if */

/* try to attach our local domain to it */

	if ((cp = getenv("LOCALDOMAIN")) != NULL) {
		char	*bp ;

		while (CHAR_ISWHITE(*cp))
			cp += 1 ;

		cp1 = cp ;
		while (*cp1 && (! CHAR_ISWHITE(*cp1)))
			cp1 += 1 ;

		bp = strwcpy(hostname,name,-1) ;

		*bp++ = '.' ;
		strwcpy(bp,cp,(cp1 - cp)) ;

	    if (nodename != NULL)
	        strcpy(nodename,name) ;

	    if (domainname != NULL)
	        strcpy(domainname,cp) ;

	    return OK ;
	}

	return BAD ;
}
/* end subroutine (getcanonical) */


