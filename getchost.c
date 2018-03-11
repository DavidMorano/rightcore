/* getchost */

/* get INET host names or addresses */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module contains a family of subroutines. These subroutines form a
        compatible replacement for:

		gethostbyname
		gethostbyaddr

        when the canonical name is supposed to be given in the first returned
        name slot for 'gethostbyname' and when a canonical name is passed to
        'gethostbyaddr' that is not in the database.


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
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	HOSTBUFLEN	(5 * MAXHOSTNAMELEN)
#define	NNAMES		10
#define	NADDRESSES	10

#ifndef	MAXADDRESSLEN
#define	MAXADDRESSLEN	32
#endif


/* external subroutines */

extern int	getourhe(const char *,char *,struct hostent *,char *,int) ;
extern int	getcname(const char *,char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */


/* external variables */


/* local variables */

static char	hostbuf[HOSTBUFLEN + 1] ;

static struct hostent	he ;

static char	*aname_list[NNAMES + 1] ;
static char	cname[MAXHOSTNAMELEN + 1] ;


/* exported subroutines */


struct hostent *getchostbyname(name)
const char	name[] ;
{
	struct utsname	uts ;

	unsigned long	addr ;

	int	rs ;
	int	len, i ;

	struct hostent	*hep = &he ;

	char	localnode[NODENAMELEN + 1] ;
	char	localdomain[MAXHOSTNAMELEN + 1] ;
	char	newname[MAXHOSTNAMELEN + 1] ;
	char	ename[MAXHOSTNAMELEN + 1] ;
	char	altname[MAXHOSTNAMELEN + 1] ;
	char	*cp ;


#if	CF_DEBUGS
	debugprintf("gethe: ent name=%s\n",name) ;
#endif

/* are we "doing" ourselves? */

	if (name == NULL) {

	    if (uname(&uts) < 0) {
	        name = "localhost" ;

	    } else
	        name = uts.nodename ;

	} else {

/* strip trailing dots */

	    len = strnlen(name,MAXHOSTNAMELEN) ;

	    if (name[len - 1] == '.') {

	        while (name[len - 1] == '.')
	            len -= 1 ;

	        strwcpy(altname,name,len) ;

	        name = altname ;

	    } /* end if */

	} /* end if (stuff on the end) */

/* do it ! */

	rs = getourhe(name,ename,hep,hostbuf,HOSTBUFLEN) ;
	if (rs < 0)
	    return NULL ;

/* does the first entry have a dot in its name? */

	if ((cp = strchr(hep->h_name,'.')) != NULL)
	    return hep ;


/* find if subsequent entries have a dot in them ! */

	for (i = 0 ; hep->h_aliases[i] != NULL ; i += 1) {

	    if ((cp = strchr(hep->h_aliases[i],'.')) != NULL)
	        break ;

	} /* end for */


/* is there a subsequent entry with a dot in its name? */

	if (hep->h_aliases[i] != NULL) {

/* swap this entry with the first and we're done ! */

	    cp = hep->h_name ;
	    hep->h_name = hep->h_aliases[i] ;
	    hep->h_aliases[i] = cp ;

	    return hep ;
	}


/* move the the names to the new name list array */

	aname_list[0] = hep->h_name ;
	for (i = 1 ; (hep->h_aliases[i - 1] != NULL) && (i < NNAMES) ; i += 1) {
	    aname_list[i] = hep->h_aliases[i - 1] ;
	}

	aname_list[i] = NULL ;
	hep->h_aliases = aname_list ;

/* get the canonical name */

	getcname(hep->h_name,cname) ;

	hep->h_name = cname ;
	return hep ;
}
/* end subroutine (getchostbyname) */


