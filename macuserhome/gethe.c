/* gethe (Get Host Entry) */

/* get a host entry from the system database */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get a host entry from the System Name Server databases.

	This subroutine is used to get a host entry struct for a host name.  It
	is not too fancy but will try to apply some standard defaults in order
	to get an entry back.  Names given to lookup will assume the current
	domain if one is not supplied with the name.  A NULL supplied name is
	assumed to refer to the current host.  A name specified in the INET
	style dotted-decimal format is also acceptable.

	Remember that a design goal is to MINIMIZE the number of DNS lookups
	used.  In general, DNS lookups are very slow.

	Synopsis:

	int gethe_name(hep,hebuf,helen,name)
	struct hostent	*hep ;
	char		hebuf[] ;
	int		helen ;
	const char	name[] ;

	Arguments:

	hep		pointer to 'hostent' structure
	hebuf		user specified storage area for returned data
	helen		length of user specified storage buffer
	name		name to lookup an entry for

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
#include	<stdlib.h>
#include	<string.h>
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
extern int	getnodename(char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local typedefs */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int gethe_begin(int stayopen)
{
	return uc_sethostent(stayopen) ;
}
/* end subroutine (gethe_begin) */


int gethe_end()
{
	return uc_endhostent() ;
}
/* end subroutine (gethe_end) */


int gethe_ent(struct hostent *hep,char *hebuf,int helen)
{
	return uc_gethostent(hep,hebuf,helen) ;
}
/* end subroutine (gethe_ent) */


int gethe_name(struct hostent *hep,char *hebuf,int helen,cchar *name)
{
	const int	nlen = NODENAMELEN ;
	int		rs = SR_OK ;
	char		nbuf[NODENAMELEN+1] ;
	if ((name == NULL) || (name[0] == '\0')) {
	    rs = getnodename(nbuf,nlen) ;
	    name = nbuf ;
	}
	if (rs >= 0) rs = uc_gethostbyname(name,hep,hebuf,helen) ;
	return rs ;
}
/* end subroutine (gethe_name) */


int gethe_addr(struct hostent *hep,char *hebuf,int helen,int type,
		const void *ap,int al)
{
	return uc_gethostbyaddr(ap,al,type,hep,hebuf,helen) ;
}
/* end subroutine (gethe_addr) */


