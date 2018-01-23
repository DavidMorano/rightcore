/* ns_getcname */

/* subroutine to get a canonical hostname */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_LOG		1


/* revision history:

	= 1995-11-01, David A­D­ Morano
	This program was originally written.

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to get a canonical INET hostname for a supplied
        name. Note carefully that the returned hostname, if any, may NOT be a
        name which can be traslated into a good INET address. In other words,
        this subroutine defines its own definition of a "canonical" name and
        that definition does NOT necessarily include the fact that the resulting
        name can be translated into a good INET address. If you want a name
        which is guaranteed to be translatable into a valid INET address, then
        you want to investigate the subroutine GETEHOSTNAME (Get Entry
        HostName).

	Synopsis:

	int ns_getcname(name,hostname)
	char	name[] ;
	char	hostname[] ;

	Arguments:
	- name		name to lookup
	- hostname	if not NULL, a buffer to hold the resulting hostname

	Returns:
	>=0		<name> had a valid INET address
	<0		<name> did not have a valid address


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/utsname.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>

#if	CF_LOG
#include	<logfile.h>
#endif

#include	<localmisc.h>


/* local defines */

#define	HOSTBUFLEN	(4 * 1024)

#ifndef	LOGIDLEN
#define	LOGIDLEN	80
#endif

#define	SERIALFILE1	"/tmp/serial"
#define	SERIALFILE2	"/tmp/ns_getcname.serial"
#define	LOGFNAME	"/tmp/ns_getcname.log"
#define	DEFLOGSIZE	(10 * MAXHOSTNAMELEN)


/* external subroutines */

extern int	ns_gethe1() ;
extern int	ns_getheaddr(const void *,struct hostent *,char *,int) ;


/* forward references */

static int	nsi_getthing2() ;


/* external variables */


/* global variables */


/* local structures */


/* local variables */


/* exported subroutines */


int ns_getcname(name,hostname)
char	name[] ;
char	hostname[] ;
{

#if	CF_LOG
	bfile	sf, *sfp = &sf ;
#endif

	struct hostent	he, *hep ;

	struct utsname	uts ;

#if	CF_LOG
	logfile	lh ;
#endif

	unsigned long	addr ;

	int	rs = SR_OK ;
	int	i, l ;

#if	CF_LOG
	int	slen, brs, pid, serial ;
#endif

	char	localnode[NODENAMELEN + 1] ;
	char	localdomain[MAXHOSTNAMELEN + 1] ;
	char	newname[MAXHOSTNAMELEN + 1] ;
	char	altname[MAXHOSTNAMELEN + 1] ;
	char	hostbuf[HOSTBUFLEN + 1] ;
	char	*np, *cp ;

#if	CF_LOG
	char	logid[LOGIDLEN + 1] ;
#endif


#if	CF_DEBUGS
	debugprintf("ns_getcname: ent name=\"%s\"\n",name) ;
#endif

#if	CF_LOG
	pid = getpid() ;

	if ((serial = ns_getserial(SERIALFILE1)) < 0)
		serial = ns_getserial(SERIALFILE2) ;

#if	CF_DEBUGS
	debugprintf("ns_getcname: using serial=%d\n",serial) ;
#endif

	sprintf(logid,"%d.%d",pid,serial) ;

	logfile_open(&lh,LOGFNAME,0,0666,logid) ;

	logfile_printf(&lh,"name=%s\n",(name == NULL) ? STDNULLFNAME : name) ;

#endif /* CF_LOG */


/* are we "doing" ourselves? or is the name OK? */

	if (name == NULL) {

	    if (uname(&uts) < 0)
	        name = "localhost" ;

	    else
	        name = uts.nodename ;

	}

/* remove any trailing dots from the name */

	    l = strnlen(name,MAXHOSTNAMELEN) ;

	    if (name[l - 1] == '.') {

	        while (name[l - 1] == '.') l -= 1 ;

	        strwcpy(altname,name,l) ;

	        name = altname ;

	    } /* end if */


/* continue */

	if (hostname != NULL)
	    hostname[0] = '\0' ;

#if	CF_DEBUGS
	debugprintf("ns_getcname: calling 'nsi_getthing2' w/ name='%s'\n",
	    name) ;
#endif

	hep = &he ;
	if (nsi_getthing2(name,hostname,&he,hostbuf,HOSTBUFLEN) < 0)
	    hep = NULL ;

#if	CF_DEBUGS
	debugprintf("ns_getcname: returned from '2' HEP is %s\n",
	    (hep == NULL) ? "NULL" : "non-NULL") ;
#endif

	if (hep != NULL) {

	    rs = OK ;
	    if (hostname == NULL)
	        goto good ;

#if	CF_DEBUGS
	    debugprintf("ns_getcname: their hostname=%s\n",
	        hostname) ;
#endif

	    if ((cp = strchr(hostname,'.')) != NULL)
	        goto good ;

#if	CF_DEBUGS
	    debugprintf("ns_getcname: continuing to check local domain\n") ;
#endif

	    getnodedomain(localnode,localdomain) ;

	    strcat(hostname,".") ;

	    strcat(hostname,localdomain) ;

#if	CF_DEBUGS
	    debugprintf("ns_getcname: domain=%s\n",localdomain) ;
#endif

		strcpy(newname,hostname) ;

	    rs = nsi_getthing2(newname,hostname,&he,hostbuf,HOSTBUFLEN) ;

	    goto ret ;

	} /* end if */

/* no entry as it was, does it have a domain part already? */

#if	CF_DEBUGS
	debugprintf("ns_getcname: no initial entry\n") ;
#endif

	if ((addr = inet_addr(name)) != (~ 0))
		goto bad ;

	getnodedomain(localnode,localdomain) ;

#if	CF_DEBUGS
	debugprintf("ns_getcname: got local machine node-domain\n") ;
#endif

	if ((cp = strchr(name,'.')) != NULL) {

#if	CF_DEBUGS
	    debugprintf("ns_getcname: original had a domain\n") ;
#endif

/* it has a domain, is it the same as our local domain? */

	    if (strcmp(cp + 1,localdomain) != 0)
	        goto bad ;

/* it has our domain, check it without the domain qualification */

	    strwcpy(newname,name,cp - name) ;

	    hep = &he ;
	    if (nsi_getthing2(newname,hostname,
	        hep,hostbuf,HOSTBUFLEN) >= 0) {

	        if (hostname != NULL) {

	            if ((cp = strchr(hostname,'.')) != NULL)
	                goto bad ;

	            strcat(hostname,".") ;

	            strcat(hostname,localdomain) ;

	        }

	        goto bad ;

	    } /* end if (we had an entry) */

	    goto bad ;

	} /* end if (it has a domain name) */

#if	CF_DEBUGS
	debugprintf("ns_getcname: we're in final part\n") ;
#endif

/* do the best that we can with what is left over */

	strcpy(newname,name) ;

	strcat(newname,".") ;

	strcat(newname,localdomain) ;

#if	CF_DEBUGS
	debugprintf("ns_getcname: about to call '2', newname=%s\n",
	    newname) ;
#endif

	hep = &he ;
	rs = nsi_getthing2(newname,hostname,hep,hostbuf,HOSTBUFLEN) ;

	if ((rs >= 0) && (hostname != NULL) && (strchr(hostname,'.') == NULL))
	    strcpy(hostname,newname) ;

#if	CF_DEBUGS
	debugprintf("ns_getcname: called '2' rs=%d\n",rs) ;

	debugprintf("ns_getcname: result=%s\n",
	    (hostname == NULL) ? "*succeeded*" : hostname) ;
#endif


/* let's get out of here */
good:

/* everybody returns here */
ret:

#if	CF_LOG
	cp = (rs == 0) ? "*succeeded*" : "*failed*" ;
	logfile_printf(&lh,"rs=%d result=%s\n",
	    rs,(hostname == NULL) ? cp : hostname) ;
#endif

#if	CF_LOG
	logfile_checksize(&lh,DEFLOGSIZE) ;

	logfile_close(&lh) ;

#endif /* CF_LOG */

	return rs ;

/* return a failure (we could not find a translation) */
bad:
	rs = BAD ;
	goto ret ;

}
/* end subroutine (ns_getcname) */


/* subroutine to search for a canonical hostname */

static int nsi_getthing2(name,hostname,hep,buf,buflen)
char		name[] ;
char		hostname[] ;
struct hostent	*hep ;
char		buf[] ;
int		buflen ;
{
	struct hostent	*lp ;
	unsigned long	addr ;
	int		rs ;
	int		i ;
	char		*cp ;

#if	CF_DEBUGS
	debugprintf("nsi_getthing2: ent name=%s\n",name) ;
#endif

	if (hostname != NULL)
	    hostname[0] = '\0' ;

#if	CF_DEBUGS
	debugprintf("nsi_getthing2: determining name or address\n") ;
#endif

	if ((addr = inet_addr(name)) != (~ 0)) {

#if	CF_DEBUGS
	debugprintf("nsi_getthing2: got address\n") ;
#endif

		rs = ns_getheaddr(&addr,hep,buf,buflen) ;

	} else {

#if	CF_DEBUGS
	debugprintf("nsi_getthing2: got name\n") ;
#endif

		rs = ns_gethe1(name,hep,buf,buflen) ;

	}

	if (rs < 0) {

#if	CF_DEBUGS
	    debugprintf("nsi_getthing2: returning BAD, rs=%d\n",rs) ;
#endif

	    return rs ;
	}

#if	CF_DEBUGS
	debugprintf("nsi_getthing2: continuing 1\n") ;
#endif

	if (hostname == NULL) return OK ;

#if	CF_DEBUGS
	debugprintf("nsi_getthing2: continuing 2\n") ;
#endif

/* can we get a domain name out of this */

	if ((cp = strchr(hep->h_name,'.')) != NULL) {

#if	CF_DEBUGS
	    debugprintf("nsi_getthing2: their hostname has a domain h=\"%s\"\n",
	        hep->h_name) ;
#endif

	    strcpy(hostname,hep->h_name) ;

	    return OK ;

	} /* end if */

#if	CF_DEBUGS
	    debugprintf("nsi_getthing2: supposed canonical name=%s\n",
	        hep->h_name) ;
#endif

	for (i = 0 ; hep->h_aliases[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("nsi_getthing2: alias%d a=\"%s\"\n",
	        i,hep->h_aliases[i]) ;
#endif

	    if ((cp = strchr(hep->h_aliases[i],'.')) != NULL) {

	        strcpy(hostname,hep->h_aliases[i]) ;

#if	CF_DEBUGS
	        debugprintf("nsi_getthing2: exiting w/ alias\n") ;
#endif

	        return OK ;

	    } /* end if */

	} /* end for */

#if	CF_DEBUGS
	debugprintf("nsi_getthing2: exiting w/ best canonical\n") ;
#endif

	if (strchr(name,'.') != NULL)
	    strcpy(hostname,name) ;

	else
	    strcpy(hostname,hep->h_name) ;

#if	CF_DEBUGS
	debugprintf("nsi_getthing2: exiting, result=%s\n",
	    hostname) ;
#endif

	return OK ;
}
/* end subroutine (nsi_getthing2) */



