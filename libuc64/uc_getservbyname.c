/* uc_getservbyname */

/* subroutine to get a single host entry by name (raw) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A.D. Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is a platform independent subroutine to get an INET
        service port entry, but does it dumbly on purpose.

	Synopsis:

	int uc_getservbyname(name,proto,sep,buf,buflen)
	const char	name[] ;
	const char	proto[] ;
	struct hostent	*sep ;
	char		buf[] ;
	int		buflen ;

	Arguments:

	- name		name to lookup
	- proto		service to lookup
	- sep		pointer to 'hostent' structure
	- buf		user supplied buffer to hold result
	- buflen	length of user supplied buffer

	Returns:

	0		host was found OK
	SR_FAULT	address fault
	SR_TIMEDOUT	request timed out (bad network someplace)
	SR_NOTFOUND	host could not be found


*******************************************************************************/


#define	LIBUC_MASTER	0

#undef	LOCAL_SOLARIS
#define	LOCAL_SOLARIS	\
	(defined(OSNAME_SunOS) && (OSNAME_SunOS > 0))

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>


/* local defines */

#define	TIMEOUT		3


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* exported subroutines */


int uc_getservbyname(name,proto,sep,buf,buflen)
const char	name[] ;
const char	proto[] ;
struct servent	*sep ;
char		buf[] ;
int		buflen ;
{
	struct servent	*lp ;
	int		rs = SR_OK ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("uc_getservbyname: name=%s proto=%s\n",
	    name,proto) ;
#endif

	if (name == NULL)
	    return SR_FAULT ;

/* do the real work */

#if	defined(SYSHAS_GETSERVXXXR) && SYSHAS_GETSERVXXXR

#if	CF_DEBUGS
	    debugprint("uc_getservbyname: on POSIX\n") ;
#endif

	for (i = 0 ; i < TIMEOUT ; i += 1) {

	    if (i > 0) sleep(1) ;

	    lp = getservbyname_r(name,proto,sep,buf,buflen) ;

	    if (lp != NULL)
	        break ;

	} /* end for */

#else /* ! defined(POSIX) */

#if	CF_DEBUGS
	    debugprint("uc_getservbyname: not on POSIX\n") ;
#endif

	for (i = 0 ; i < TIMEOUT ; i += 1) {

	    if (i > 0) sleep(1) ;

	    lp = getservbyname(name,proto) ;

	    if (lp != NULL)
		break ;

	} /* end for */

	if (lp != NULL)
	    memcpy(sep,lp,sizeof(struct servent)) ;

#endif /* POSIX or not */

	if (i >= TIMEOUT) {
	    rs = SR_TIMEDOUT ;

	} else if (lp == NULL)
	    rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("uc_getservbyname: return is %s\n",
	    (lp == NULL) ? "bad" : "good") ;
#endif

	return rs ;
}
/* end subroutine (uc_getservbyname) */


