/* geteaddrinfo */

/* subroutine to get a canonical hostname */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used to get a canonical INET hostname for a supplied
	name.  Note carefully that the returned hostname, if any, may NOT be a
	name that can be translated into a good INET address.  In other words,
	this subroutine uses its own definition of a "canonical" name and that
	definition does NOT necessarily include the fact that the resulting
	name can be translated into a good INET address.  If you want a name
	that is guaranteed to be translatable into a valid INET address then
	you would use the 'ehostname' name that is supplied as a result of
	calling this subroutine.

	Having said that the resuling name is not guaranteed to be
	translatable, a good translation facility will generally figure out
	that the given name is something that can be translated given the
	existing host information.

	Synopsis:

	int geteaddrinfo(hostname,svcname,hintp,ehostname,rpp)
	const char	hostname[] ;
	const char	svcname[] ;
	struct addrinfo	*hintp ;
	char		ehostname[] ;
	struct addrinfo	**rpp ;

	Arguments:

	hostname	name of host to lookup
	svcname		name of service to lookup
	hintp		pointer to 'addrinfo' structure
	ehostname	caller-supplied buffer to received "effective" name
	rpp		pointer to pointer to 'addrinfo' result

	Returns:

	>=0		<name> had a valid INET address
	<0		<name> did not have a valid address


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

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

#ifndef	LOCALHOST
#define	LOCALHOST	"localhost"
#endif

#ifndef	ANYHOST
#define	ANYHOST		"anyhost"
#endif

#ifndef	LOCALDOMAINNAME
#define	LOCALDOMAINNAME	"local"
#endif

#ifndef	ADDRINFO
#define	ADDRINFO	struct addrinfo
#endif

#define	ARGINFO		struct arginfo

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	tolc(int) ;
extern int	isinetaddr(const char *) ;
extern int	isindomain(const char *,const char *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */

struct arginfo {
	const char	*hostname ;
	const char	*svcname ;
	struct addrinfo	*hintp ;
	struct addrinfo	**rpp ;
} ;

struct subinfo_flags {
	uint		inetaddr:1 ;
	uint		inetaddr_start:1 ;
} ;

struct subinfo {
	ARGINFO		*aip ;		/* user argument (hint) */
	char		*ehostname ;	/* user argument (writable) */
	const char	*domainname ;	/* dynamically determined */
	SUBINFO_FL	f ;
	int		rs_last ;
} ;


/* forward references */

static int	arginfo_load(ARGINFO *,cchar *,cchar *,
			struct addrinfo *,struct addrinfo **) ;

static int	subinfo_start(SUBINFO *,char *,ARGINFO *) ;
static int	subinfo_domain(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *) ;

static int	try_straight(SUBINFO *) ;
static int	try_add(SUBINFO *) ;
static int	try_rem(SUBINFO *) ;
static int	try_remlocal(SUBINFO *) ;


/* external variables */


/* local variables */

static int	(*tries[])(SUBINFO *) = {
	try_straight,
	try_add,
	try_rem,
	try_remlocal,
	NULL
} ;


/* exported subroutines */


int geteaddrinfo(hostname,svcname,hintp,ehostname,rpp)
const char	hostname[] ;
const char	svcname[] ;
struct addrinfo	*hintp ;
char		ehostname[] ;
struct addrinfo	**rpp ;
{
	SUBINFO		mi ;
	ARGINFO		ai ;
	int		rs ;
	int		rs1 ;
	int		rs_last = SR_NOTFOUND ;
	int		c = 0 ;

	if (hostname == NULL) return SR_FAULT ;
	if (svcname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("geteaddrinfo: ent hn=%s svc=%s\n",hostname,svcname) ;
#endif

	arginfo_load(&ai,hostname,svcname,hintp,rpp) ;

	if ((rs = subinfo_start(&mi,ehostname,&ai)) >= 0) {
	    int		i ;

	    for (i = 0 ; tries[i] != NULL ; i += 1) {
	        rs = (*tries[i])(&mi) ;
		c = rs ;
	        if (rs != 0) break ;
	    } /* end for */

#if	CF_DEBUGS
	debugprintf("geteaddrinfo: fin1 rs=%d c=%u\n",rs,c) ;
#endif

	    rs_last = mi.rs_last ;
	    rs1 = subinfo_finish(&mi) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("geteaddrinfo: fin2 rs=%d c=%u\n",rs,c) ;
#endif

	if ((rs >= 0) && (c == 0)) rs = rs_last ;

#if	CF_DEBUGS
	debugprintf("geteaddrinfo: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end if (geteaddrinfo) */


/* local subroutines */


static int subinfo_start(SUBINFO *mip,char *ehostname,ARGINFO *aip)
{

	memset(mip,0,sizeof(SUBINFO)) ;
	mip->domainname = NULL ;
	mip->aip = aip ;
	mip->ehostname = ehostname ;
	mip->rs_last = SR_NOTFOUND ;

	return SR_OK ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *mip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mip->domainname != NULL) {
	    rs1 = uc_free(mip->domainname) ;
	    if (rs >= 0) rs = rs1 ;
	    mip->domainname = NULL ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_domain(SUBINFO *mip)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (mip->domainname == NULL) {
	    char	dbuf[MAXHOSTNAMELEN + 1] ;
	    if ((rs = getnodedomain(NULL,dbuf)) >= 0) {
	        cchar	*dp ;
	        len = strlen(dbuf) ;
	        if ((rs = uc_mallocstrw(dbuf,len,&dp)) >= 0) {
	            mip->domainname = dp ;
		}
	    }
	} else {
	    len = strlen(mip->domainname) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_domain) */


static int try_straight(SUBINFO *mip)
{
	ARGINFO		*aip = mip->aip ;
	int		rs ;
	int		c = 0 ;
	const char	*hn = aip->hostname ;

	if (hn != NULL) {
	    int f = FALSE ;
	    f = f || ((hn[0] == 'a') && (strcmp(hn,ANYHOST) == 0)) ;
	    f = f || (hn[0] == '*') ;
	    f = f || (hn[0] == '\0') ;
	    if (f)
	        hn = NULL ;
	}

#if	CF_DEBUGS
	if (aip->hintp != NULL) {
	    debugprintf("geteaddrinfo/try_straight: hint ai_family=%d\n",
	        aip->hintp->ai_family) ;
	    debugprintf("geteaddrinfo/try_straight: hint ai_protocol=%d\n",
	        aip->hintp->ai_protocol) ;
	}
#endif

	if ((rs = uc_getaddrinfo(hn,aip->svcname,aip->hintp,aip->rpp)) >= 0) {

#if	CF_DEBUGS
	debugprintf("geteaddrinfo/try_straight: rs=%d\n",rs) ;
	if (rs >= 0) {
	    debugprintf("geteaddrinfo/try_straight: hostname=%s\n",
	        aip->hostname) ;
	    if (aip->rpp != NULL) {
	        struct addrinfo	*ap = *(aip->rpp) ;
	        debugprintf("geteaddrinfo/try_straight: ai_family=%d\n",
	            ap->ai_family) ;
	        debugprintf("geteaddrinfo/try_straight: ai_socktype=%d\n",
	            ap->ai_socktype) ;
	        debugprintf("geteaddrinfo/try_straight: ai_protocol=%d\n",
	            ap->ai_protocol) ;
	    }
	}
#endif /* CF_DEBUGS */

	    if (mip->ehostname != NULL) {
		const int	hlen = MAXHOSTNAMELEN ;
	        mip->ehostname[0] = '\0' ;
	        if (aip->hostname != NULL) {
		    c = 1 ;
	            rs = sncpy1(mip->ehostname,hlen,aip->hostname) ;
	        }
	    }
	} else if (isNotPresent(rs)) {
	    mip->rs_last = rs ;
	    rs = SR_OK ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (try_straight) */


/* try adding our own domain on the end if it does not already have one */
static int try_add(SUBINFO *mip)
{
	ARGINFO		*aip = mip->aip ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (aip->hostname != NULL) {
	    if (strchr(aip->hostname,'.') == NULL) {
	        if (! isinetaddr(aip->hostname)) {
	            if ((rs = subinfo_domain(mip)) >= 0) {
	                const int	hlen = MAXHOSTNAMELEN ;
			cchar		*dn = mip->domainname ;
			cchar		*hn = aip->hostname ;
	                char		ehostname[MAXHOSTNAMELEN + 1] ;
	                char		*bp ;
			bp = ehostname ;
	                if (mip->ehostname != NULL) {
			    bp = mip->ehostname ;
			}
	                if ((rs = snsds(bp,hlen,hn,dn)) >= 0) {
			    ADDRINFO	*hp = aip->hintp ;
			    ADDRINFO	**rpp = aip->rpp ;
	                    cchar	*sn = aip->svcname ;
	                    if ((rs = uc_getaddrinfo(bp,sn,hp,rpp)) >= 0) {
				c = 1 ;
			    } else if (isNotPresent(rs)) {
	    			mip->rs_last = rs ;
				rs = SR_OK ;
			    }
	                }
	            } /* end if (subinfo_domain) */
	        }
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (try_add) */


/* try removing our own domain from the end if it is the same as us */
static int try_rem(SUBINFO *mip)
{
	ARGINFO		*aip = mip->aip ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (aip->hostname != NULL) {
	    if (! isinetaddr(aip->hostname)) {
		cchar	*tp ;
	        if ((tp = strchr(aip->hostname,'.')) != NULL) {
	            if ((rs = subinfo_domain(mip)) >= 0) {
	                rs = SR_NOTFOUND ;
	                if (isindomain(aip->hostname,mip->domainname)) {
			    const int	hlen = MAXHOSTNAMELEN ;
	                    int		hl = (tp - aip->hostname) ;
			    cchar	*hn = aip->hostname ;
	                    char	ehostname[MAXHOSTNAMELEN + 1] ;
	                    char	*bp ;
	    		    bp = ehostname ;
			    if (mip->ehostname != NULL) {
	                        bp = mip->ehostname ;
	                    }
	    		    if ((rs = snwcpy(bp,hlen,hn,hl)) >= 0) {
				ADDRINFO	*hp = aip->hintp ;
				ADDRINFO	**rpp = aip->rpp ;
				cchar		*sn = aip->svcname ;
	        		if ((rs = uc_getaddrinfo(bp,sn,hp,rpp)) >= 0) {
				    c = 1 ;
			        } else if (isNotPresent(rs)) {
	    			    mip->rs_last = rs ;
				    rs = SR_OK ;
			        }
			    }
			} /* end if (subinfo_domain) */
		    } /* end if (subinfo_domain) */
	        }
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (try_rem) */


/* try removing our a LOCAL domain */
static int try_remlocal(SUBINFO *mip)
{
	ARGINFO		*aip = mip->aip ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (aip->hostname != NULL) {
	    if (! isinetaddr(aip->hostname)) {
		cchar	*tp ;
	        if ((tp = strchr(aip->hostname,'.')) != NULL) {
	            if (isindomain(aip->hostname,LOCALDOMAINNAME)) {
		        const int	hlen = MAXHOSTNAMELEN ;
	                int		hl = (tp - aip->hostname) ;
	                char		ehostname[MAXHOSTNAMELEN + 1] ;
	                char		*bp ;
	    		bp = ehostname ;
	    		if (mip->ehostname != NULL) {
			    bp = mip->ehostname ;
	    		}
	    		if ((rs = snwcpy(bp,hlen,aip->hostname,hl)) >= 0) {
			    ADDRINFO	*hp = aip->hintp ;
			    ADDRINFO	**rpp = aip->rpp ;
			    cchar	*sn = aip->svcname ;
	                    if ((rs = uc_getaddrinfo(bp,sn,hp,rpp)) >= 0) {
				c = 1 ;
			    } else if (isNotPresent(rs)) {
	    			mip->rs_last = rs ;
				rs = SR_OK ;
			    }
	                }
	            } /* end if */
	        }
	    }
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (try_remlocal) */


static int arginfo_load(aip,hostname,svcname,hintp,rpp)
ARGINFO		*aip ;
const char	hostname[] ;
const char	svcname[] ;
struct addrinfo	*hintp ;
struct addrinfo	**rpp ;
{

	aip->hostname = hostname ;
	aip->svcname = svcname ;
	aip->hintp = hintp ;
	aip->rpp = rpp ;
	return SR_OK ;
}
/* end subroutine (arginfo_load) */


