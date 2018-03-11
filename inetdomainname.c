/* inetdomainname */

/* get the local node name and INET domain name */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0	/* non-switchable */
#define	CF_GUESS	1	/* try to guess domain names? */
#define	CF_GETHE	0	/* use 'gethe1()'? */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


	= 1999-04-18, David A­D­ Morano

	This was modified to search in the following order:

		1. LOCALDOMAIN environment variable
		2. reading the file '/etc/resolv.conf'


*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get the local host node name and INET domain name.


	Synopsis:

	int inetdomainname(nodename,domainname)
	char	nodename[] ;
	char	domainname[] ;


	Arguments:

	nodename	buffer to receive the local node name
	domainname	buffer to receive the local INET domain name


	Returns:

	- SR_OK		if OK
	- SR_NOTFOUND	if could not get something needed for correct operation


	The algorithm for finding the local nodename is:

	1. use the first component of the environment variable NODE

	2. use the first component of the nodename returned from the system


	NOTE: Searching for the "current" domain is not an easy task and
	never has been.  There is no easy way to find out the domain part
	of the hostname for the current machine node.

	We use the following algorithm for finding the local domain:

	1. use the environment variable DOMAIN

	2. use the first component of the LOCALDOMAIN environment variable

	3. see if the environment variable NODE has a domain

	4. get the node name from the system and see if it has a domain

	5. lookup the system node name with the system resolver functions
	   and grab the first name for the current node that has a domain
	   attached

	6. use the domainname given in the resolver configuration file
           (keyword 'domain') if it is not corrupted

	7. use the first component of the 'search' keyword from the
	   resolver configuration file (NOT YET IMPLEMENTED !)

	8. we try to guess what the domain name is from the given node name

	9. return that we couln't find a domain for the current node !!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<strings.h>		/* |strncasecmp(3c)| */

#include	<vsystem.h>
#include	<char.h>
#include	<filebuf.h>
#include	<localmisc.h>


/* local defines */

#define	BUFLEN		(MAXPATHLEN + (8 * 1024))
#define	RESOLVCONF	"/etc/resolv.conf"
#define	LOCALHOSTNAME	"localhost"
#define	VARNODE		"NODE"
#define	VARDOMAIN	"DOMAIN"

#define	FILEBUFLEN	1024
#define	TO_READ		30


/* external subroutines */

#if	defined(BSD) && (! defined(EXTERN_STRNCASECMP))
extern int	strncasecmp(const char *,const char *,int) ;
#endif

extern int	sncpy1(char *,int,const char *) ;
extern int	sfshrink(char *,int,const char **) ;
extern int	gethe1(char *,struct hostent *,char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strshrink(char *) ;


/* external variables */


/* local structures */

struct guess {
	const char	*node ;
	const char	*domain ;
} ;


/* forward references */

static char	*findguess(char *) ;


/* local static variables */

static const struct guess	ga[] = {
	{ "hammett", "ece.neu.com" },
	{ "gilmore", "ece.neu.com" },
	{ "dr", "dr.lucent.com" },
	{ "ho", "ho.lucent.com" },
	{ "mh", "mh.lucent.com" },
	{ "mt", "mt.lucent.com" },
	{ "cb", "cb.lucent.com" },
	{ NULL, NULL }
} ;


/* exported subroutines */


int inetdomainname(dbuf,dlen,un,nodename)
char		dbuf[] ;
int		dlen ;
const char	un[] ;
const char	nodename[] ;
{
	struct utsname	uu ;

	struct hostent	he, *hep ;

	int	rs, rs1 ;
	int	i, len ;
	int	f_badresolv = FALSE ;
	int	f ;

	const char	*dn = domainname ;
	const char	*nn0, *nn1 ;
	const char	*sp, *cp ;
	const char	*cp1 ;

	char	buf[BUFLEN + 1] ;
	char	nodenamebuf[NODENAMELEN + 1], *nn = nodenamebuf ;


#if	CF_DEBUGS
	debugprintf("inetdomainname: ent\n") ;
#endif

	if (nodename != NULL)
	    nn = nodename ;

	nn[0] = '\0' ;
	un.nodename[0] = '\0' ;

/* try to get a nodename */

	nn0 = nn1 = getenv(VARNODE) ;

	if ((nn1 == NULL) || (nn1[0] == '\0')) {

#if	CF_DEBUGS
	    debugprintf("inetdomainname: u_uname()\n") ;
#endif

	    nn1 = uu.nodename ;
	    if (u_uname(&uu) < 0)
	        nn1[0] = '\0' ;

	} /* end if */

	if ((nn1 != NULL) && (nn1[0] != '\0')) {

	    if ((cp = strchr(nn1,'.')) != NULL) {
	        strwcpy(nn,nn1,(cp - nn1)) ;
	    } else
	        strcpy(nn,nn1) ;

	} /* end if */

	if (nn[0] == '\0')
	    strcpy(nn,LOCALHOSTNAME) ;

	if (domainname == NULL)
	    return SR_OK ;

/* try to get a domainname */

	dn[0] = '\0' ;

/* use the environment variable DOMAIN, if it exists */

#if	CF_DEBUGS
	debugprintf("inetdomainname: environment variable DOMAIN\n") ;
#endif

	if ((dn[0] == '\0') &&
	    ((cp = getenv(VARDOMAIN)) != NULL)) {

	    len = sfshrink(cp,-1,&cp1) ;

	    if (len <= MAXHOSTNAMELEN)
	        strwcpy(dn,cp1,len) ;

	} /* end if */

/* OK, we try LOCALDOMAIN */

	if ((dn[0] == '\0') &&
	    ((sp = getenv("LOCALDOMAIN")) != NULL)) {

/* get the first domain name in the variable (there can be several !) */

	    while (CHAR_ISWHITE(*sp))
	        sp += 1 ;

	    cp = sp ;
	    while (*cp && (! CHAR_ISWHITE(*cp)) && (*cp != ':'))
	        cp += 1 ;

	    if ((cp - sp) <= MAXHOSTNAMELEN)
	        strwcpy(dn,sp,(cp - sp)) ;

	} /* end if (localdomain) */

/* use the environment variable NODE, if it exists */

	if ((dn[0] == '\0') &&
	    (nn0 != NULL) && ((cp = strchr(nn0,'.')) != NULL)) {

	    if (cp[1] != '\0')
	        strcpy(dn,(cp + 1)) ;

	} /* end if */

/* sse if the system nodename has a domain on it */

	if (dn[0] == '\0') {

	    if (nn1 == NULL) {

	        nn1 = un.nodename ;
	        if (u_uname(&un) < 0)
	            nn1[0] = '\0' ;

	    }

	    if ((nn1[0] != '\0') && ((cp = strchr(nn1,'.')) != NULL)) {

	        if (cp[1] != '\0')
	            strcpy(dn,(cp + 1)) ;

	    } /* end if */

	} /* end if */

/* OK, we do not have a domain name yet, try 'gethostbyname()' */

#if	CF_DEBUGS
	debugprintf("inetdomainname: system resolver\n") ;
	debugprintf("inetdomainname: gethe1()\n") ;
#endif

	f = (dn[0] == '\0') ;

	if (f) {

#if	CF_GETHE
	    rs = gethe1(nn,&he,buf,BUFLEN) ;
#else
	    rs = uc_gethostbyname(nn,&he,buf,BUFLEN) ;
#endif

	    f = (rs >= 0) ;
	}

	if (f) {

	    hep = &he ;

#if	CF_DEBUGS
	    debugprintf("inetdomainname: RESOLVER their hostname h=%s\n",
	        hep->h_name) ;
#endif

	    if ((cp = strchr(hep->h_name,'.')) != NULL) {

#if	CF_DEBUGS
	        debugprintf("inetdomainname: hostname has domain h=%s\n",
	            hep->h_name) ;
#endif

	        strcpy(dn,(cp + 1)) ;

#if	CF_DEBUGS
	        debugprintf("inetdomainname: NS supplied d=>%s<\n",dn) ;
#endif

	        return SR_OK ;

	    } /* end if */

	    for (i = 0 ; hep->h_aliases[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	        debugprintf("inetdomainname: alias%d a=%s\n",
	            i,hep->h_aliases[i]) ;
#endif

	        if ((cp = strchr(hep->h_aliases[i],'.')) != NULL) {

	            strcpy(dn,(cp + 1)) ;

#if	CF_DEBUGS
	            debugprintf("inetdomainname: NS supplied d=>%s<\n",dn) ;
#endif

	            return SR_OK ;

	        } /* end if */

	    } /* end for */

	} /* end if (use system resolver) */

/* resort to searching the RESOLVER configuration file ! */

#if	CF_DEBUGS
	debugprintf("inetdomainname: resolver configuration file\n") ;
#endif

	if ((dn[0] == '\0') && ((rs = u_open(RESOLVCONF,O_RDONLY,0666)) >= 0)) {
	    FILEBUF	b ;
	    int		fd = rs ;

	    rs = filebuf_start(&b,fd,0L,FILEBUFLEN,0) ;
	    if (rs >= 0) {

	        while ((rs = filebuf_readline(&b,buf,BUFLEN,TO_READ)) > 0) {

	            len = rs ;
		    if (buf[len - 1] == '\n')
			len -= 1 ;

	            buf[len] = '\0' ;
	            cp = strshrink(buf) ;

	            if ((strncasecmp(cp,"domain",6) != 0) ||
	                (! CHAR_ISWHITE(cp[6]))) continue ;

	            cp += 7 ;
	            while (CHAR_ISWHITE(*cp))
	                cp += 1 ;

	            cp1 = cp ;
	            while ((*cp != '\0') && (! CHAR_ISWHITE(*cp)))
	                cp += 1 ;

	            if ((cp - cp1) <= 0) continue ;

#if	CF_DEBUGS
	            debugprintf("inetdomainname: RESOLV? supplied d=>%W<\n",
	                cp1,(cp - cp1)) ;
#endif

	            if (strncmp(cp1,"..",2) != 0) {

	                strwcpy(dn,cp1,(cp - cp1)) ;

#if	CF_DEBUGS
	                debugprintf("inetdomainname: RESOLV supplied d=>%s<\n",
	                    dn) ;
#endif

	                break ;

	            } else
	                f_badresolv = TRUE ;

	        } /* end while (reading lines) */

	        filebuf_finish(&b) ;
	    } /* end if (filebuf) */

	    u_close(fd) ;
	} /* end if (opened RESOLV file) */

/* OK, we try even harder (we guess) */

#if	CF_DEBUGS
	debugprintf("inetdomainname: do we want to guess?\n") ;
#endif

#if	CF_GUESS

#if	CF_DEBUGS
	debugprintf("inetdomainname: guessing\n") ;
#endif

	if (dn[0] == '\0') {

#if	CF_DEBUGS
	    debugprintf("inetdomainname: trying to GUESS\n") ;
#endif

	    if ((cp = findguess(nn)) != NULL) {

	        rs1 = sncpy1(dn,MAXHOSTNAMELEN,cp) ;

		if (rs1 < 0)
			dn[0] = '\0' ;

#if	CF_DEBUGS
	        debugprintf("inetdomainname: GUESS supplied d=>%s<\n",dn) ;
#endif

	    }

	} /* end if */
#endif /* CF_GUESS */


/* remove any stupid trailing dots from the domain name if any */

	if (dn[0] != '\0') {

	    len = strlen(dn) ;

	    while ((len > 1) && (dn[len - 1] == '.'))
	        dn[--len] = '\0' ;

	} /* end if (trailing dot removal) */

#if	CF_DEBUGS
	debugprintf("inetdomainname: ret dn=%s\n",dn) ;
#endif

	return ((dn[0] != '\0') ? SR_OK : SR_NOTFOUND) ;
}
/* end subroutine (inetdomainname) */


/* local subroutines */


/* try guessing with possible leading string matches on nodename */
static char *findguess(name)
char	name[] ;
{
	int	i = 0 ;

	char	*np, *gnp ;


#if	CF_DEBUGS
	debugprintf("inetdomainname/findguess: ent name=%s\n",
	    name) ;
#endif

	for (i = 0 ; ga[i].node != NULL ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("inetdomainname/findguess: node=%s\n",
	        ga[i].node) ;
#endif

	    np = name ;
	    gnp = ga[i].node ;
	    while (*gnp && (*gnp == *np)) {

#if	CF_DEBUGS
	        debugprintf("inetdomainname/findguess: gn=%c n=%c\n",
	            *gnp,*np) ;
#endif

	        gnp += 1 ;
	        np += 1 ;

	    } /* end while */

	    if (*gnp == '\0')
	        break ;

	} /* end for */

	return ((ga[i].node != NULL) ? ga[i].domain : NULL) ;
}
/* end subroutine (findguess) */



