/* netgroupcheck */

/* check names against the system netgroups */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time print-outs */


/* revision history:

	= 2008-07-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subrotuine checks some names against the UNIX® system set of
        netgroups (if there are any).

	Synopsis:

	int netgroupcheck(dname,glp,nlp)
	const char	dname[] ;
	vecstr		*glp ;
	vecstr		*nlp ;

	Arguments:

	dname		domain-name
	glp		list of system netgroups
	nlp		list of machine names

	Returns:

	0		no match (not-allowed)
	>0		match (allowed)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"config.h"


/* local defines */

#ifndef	PEERNAMELEN
#ifdef	CONNECTION_PEERNAMELEN
#define	PEERNAMELEN	CONNECTION_PEERNAMELEN
#else
#define	PEERNAMELEN	MAX(MAXHOSTNAMELEN,MAXPATHLEN)
#endif
#endif

#ifndef	INETXADDRLEN
#define	INETXADDRLEN	sizeof(struct in_addr)
#endif

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
#endif

#ifndef	STEBUFLEN
#define	STEBUFLEN	MAXNAMELEN
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	isdigitlatin(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int netgroupcheck(dname,glp,nlp)
const char	dname[] ;
vecstr		*glp ;
vecstr		*nlp ;
{
	int		rs = SR_OK ;
	int		i, j ;
	int		ch ;
	int		f = FALSE ;
	const char	*ngp, *mnp ;

	if (dname == NULL) return SR_FAULT ;

	if (dname[0] == '\0') return SR_INVALID ;

	for (i = 0 ; vecstr_get(glp,i,&ngp) >= 0 ; i += 1) {
	    if (ngp != NULL) {
	        for (j = 0 ; vecstr_get(nlp,j,&mnp) >= 0 ; j += 1) {
	            if (mnp != NULL) {
			ch = MKCHAR(mnp[0]) ;
	                if (! isdigitlatin(ch)) {
	                    f = innetgr(ngp,mnp,NULL,dname) ;
	 	            if (f) break ;
			}
		    }
	        } /* end for (machine names) */
	     }
	    if (f || (rs < 0)) break ;
	} /* end for (netgroups) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (netgroupcheck) */



