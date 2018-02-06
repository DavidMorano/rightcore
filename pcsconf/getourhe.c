/* getourhe (Get Our Host Entry) */

/* get a host name that has an INET address */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SNSDS	1		/* use 'snsds(3dam)' */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get a host entry from the Name Server databases.

        This subroutine is used to get a host entry struct for a host name. It
        is not too fancy but will try to apply some standard defaults in order
        to get an entry back. Names given to lookup will assume the current
        domain if one is not supplied with the name. A NULL supplied name is
        assumed to refer to the current host. A name specified in the INET style
        dotted-decimal format is also acceptable.

        Remember that a design goal is to MINIMIZE the number of DNS lookups
        used. In general, DNS lookups are very slow.

	Synopsis:

	int getourhe(name,hostname,hep,hostbuf,hostlen)
	const char	name[] ;
	char		hostname[] ;
	struct hostent	*hep ;
	char		hostbuf[] ;
	int		hostlen ;

	Arguments:

	name		name to lookup an entry for
	hostname	optional buffer to hold name actually used for lookup
	hep		pointer to 'hostent' structure
	hostbuf		user specified storage area for returned data
	hostlen	length of user specified storage buffer

	Returns:

	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
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
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	gethename(const char *,struct hostent *,char *,int) ;
extern int	getheaddr(const void *,struct hostent *,char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */


/* local variables */

static const in_addr_t	anyaddr = 0 ;


/* exported subroutines */


int getourhe(name,hostname,hep,hostbuf,hostlen)
const char	name[] ;
char		hostname[] ;
struct hostent	*hep ;
char		hostbuf[] ;
int		hostlen ;
{
	struct utsname	uts ;
	in_addr_t	addr ;
	int		rs ;
	int		len ;
	char		localnode[NODENAMELEN + 1] ;
	char		localdomain[MAXHOSTNAMELEN + 1] ;
	char		newname[MAXHOSTNAMELEN + 1] ;
	char		altname[MAXHOSTNAMELEN + 1] ;
	char		*np = (char *) name ;
	char		*ap ;
	char		*cp ;

#if	CF_DEBUGS
	debugprintf("gethe: ent name=%s\n",name) ;
#endif

/* are we "doing" ourselves? */

	np = (char *) name ;
	if (name == NULL) {

	    if (u_uname(&uts) < 0) {
	        np = "localhost" ;
	    } else
	        np = uts.nodename ;

	} else {

/* strip trailing dots */

	    len = strnlen(np,MAXHOSTNAMELEN) ;

	    if (name[len - 1] == '.') {

	        while (len && (np[len - 1] == '.'))
			len -= 1 ;

	        snwcpy(altname,MAXHOSTNAMELEN,np,len) ;

	        np = altname ;

	    } /* end if */

	} /* end if (stuff on the end) */

/* is it a numeric address? */

	if ((addr = inet_addr(np)) != (~ 0)) {

#if	CF_DEBUGS
	debugprintf("gethe: address specified\n") ;
#endif

	    ap = (char *) &addr ;
	    rs = getheaddr(ap,hep,hostbuf,hostlen) ;

#ifdef	COMMENT
	     if ((rs < 0) && 
		(memcmp(&addr,&anyaddr,sizeof(struct in_addr_t)) == 0)) {


	    }
#endif /* COMMENT */

	} else {

#if	CF_DEBUGS
	debugprintf("gethe: name specified\n") ;
#endif

		rs = gethename(np,hep,hostbuf,hostlen) ;

	}

#if	CF_DEBUGS
	debugprintf("gethe: initial lookup returned rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    if (hostname != NULL)
	        sncpy1(hostname,MAXHOSTNAMELEN,np) ;

	    return SR_OK ;

	} else if (addr != (~ 0))
		return SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("gethe: continuing\n") ;
#endif /* CF_DEBUGS */

/* get our local domain name */

#if	CF_DEBUGS
	debugprintf("gethe: getnodedomain\n") ;
#endif

	localdomain[0] = '\0' ;
	getnodedomain(localnode,localdomain) ;

/* does the given name have a domain already? */

	if ((cp = strchr(np,'.')) != NULL) {

/* is the given domain our local domain name? */

	    if (strcmp((cp + 1),localdomain) == 0) {

/* yes */

	        snwcpy(newname,MAXHOSTNAMELEN,name,(cp - name)) ;

/* try it without our trailing local domain name */

#if	CF_DEBUGS
	debugprintf("gethe: gethename() newname=%s\n",newname) ;
#endif

	        rs = gethename(newname,hep,hostbuf,hostlen) ;

		if (rs >= 0) {

	            if (hostname != NULL)
	                sncpy1(hostname,MAXHOSTNAMELEN,hep->h_name) ;

	            return SR_OK ;
	        }

#if	CF_DEBUGS
	debugprintf("gethe: gethename() rs=%d\n",rs) ;
#endif

	    } /* end if (domains are the same) */

	} else {

/* it didn't have a domain so we try it with our local domain on it */

#if	CF_SNSDS
	    snsds(newname,MAXHOSTNAMELEN,name,localdomain) ;
#else /* CF_SNSDS */
	    sncpy3(newname,MAXHOSTNAMELEN,name,".",localdomain) ;
#endif /* CF_SNSDS */

#if	CF_DEBUGS
	debugprintf("gethe: gethename() 2 newname=%s\n",newname) ;
#endif

	    rs = gethename(newname,hep,hostbuf,hostlen) ;

	    if (rs >= 0) {

	        if (hostname != NULL)
	            sncpy1(hostname,MAXHOSTNAMELEN,hep->h_name) ;

	        return OK ;
	    }

#if	CF_DEBUGS
	debugprintf("gethe: gethename() 2 rs=%d\n",rs) ;
#endif

	} /* end if */

	if (hostname != NULL)
	    hostname[0] = '\0' ;

#if	CF_DEBUGS
	debugprintf("gethe: ret NOTFOUND\n") ;
#endif

	return SR_NOTFOUND ;
}
/* end subroutine (getourhe) */


