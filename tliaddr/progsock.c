/* progsock */

/* subroutine to process socket interface type addresses */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1999-08-17, David A­D­ Morano
        This subroutine (and whole program) is replacing serveral that did
        similar things in the past. One program that did something similar to
        this was 'hostrfs(1)' from the old days. There was also 'rfsaddr(1).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine tries to make a socket-style TLI address out of the
        given arguments.


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
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<char.h>
#include	<bfile.h>
#include	<hostinfo.h>
#include	<sockaddress.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	INET4_ADDRSTRLEN
#define	INET4_ADDRSTRLEN	16
#endif

#ifndef	INET6_ADDRSTRLEN
#define	INET6_ADDRSTRLEN	46	/* Solaris® says that this is 46! */
#endif

#ifndef	INETX_ADDRSTRLEN
#define	INETX_ADDRSTRLEN	MAX(INET4_ADDRSTRLEN,INET6_ADDRSTRLEN)
#endif

#ifndef	PORTNAMELEN
#define	PORTNAMELEN		32
#endif

#ifndef	SOCKADDRESSLEN
#define	SOCKADDRESSLEN		sizeof(SOCKADDRESS)
#endif

#ifndef	PROTONAME
#define	PROTONAME		"tcp"
#endif

#ifndef	ANYHOST
#define	ANYHOST			"anyhost"
#endif

#define	HEXBUFLEN_ADDR		(2*SOCKADDRESSLEN)
#define	HEXBUFLEN_PATH		(2*(MAXPATHLEN+MAXNAMELEN))
#define	HEXBUFLEN		MIN(HEXBUFLEN_ADDR,HEXBUFLEN_PATH)


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getportnum(const char *,const char *) ;
extern int	getaf(const char *,int) ;
extern int	getaflen(int) ;
extern int	cfnumi(const char *,int,int *) ;
extern int	progout_printf(PROGINFO *,const char *,...) ;
extern int	tolower(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* forward references */

int		progout_open(PROGINFO *) ;
int		progout_close(PROGINFO *) ;

static int	procsocker(PROGINFO *,int,cchar *,int) ;
static int	gethostinfo(PROGINFO *,char *,int,const char *,int *) ;


/* local structures */


/* local variables */


/* exported subroutines */


int progsock(pip,familyname,netaddr1,netaddr2)
PROGINFO	*pip ;
const char	familyname[] ;
const char	netaddr1[] ;
const char	netaddr2[] ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		af = AF_UNSPEC ;
	int		port = -1 ;
	int		aport = -1 ;
	const char	*proto = PROTONAME ;
	char		netaddr[SOCKADDRESSLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progsock: familyname=%s\n",familyname) ;
#endif

	if (netaddr1 == NULL) return SR_FAULT ;

	if ((netaddr1[0] == '\0') || (netaddr1[0] == '-')) {
	    netaddr1 = ANYHOST ;
	}

	if (familyname != NULL) {
	    rs = getaf(familyname,-1) ;
	    af = rs ;
	} else {
	    af = AF_INET ;
	    if (netaddr1[0] == '/') {
	        af = AF_UNIX ;
	    }
	} /* end if (getting address family code) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progsock: rs=%d af=%d\n",rs,af) ;
#endif

	if (rs >= 0) {

	if ((pip->debuglevel > 0) && (af >= 0)) {
	    bprintf(pip->efp,
	        "%s: address family=%s (%u)\n",
	        pip->progname,familyname,af) ;
	}

/* process thje given address family */

	switch (af) {
	case AF_UNIX:
	    strwcpy(netaddr,netaddr1,SOCKADDRESSLEN) ;
	    break ;
	case AF_INET4:
	case AF_INET6:
	    rs = gethostinfo(pip,netaddr,af,netaddr1,&aport) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: gethostinfo() rs=%d port=%d\n",
		rs,aport) ;
#endif
	    if (port < 0) port = aport ;
	    rs1 = SR_NOTFOUND ;
	    if ((rs >= 0) && (netaddr2 != NULL)) {
	        rs1 = getportnum(proto,netaddr2) ;
	        port = rs1 ;
	    }
	    if (rs1 < 0) {
	        rs1 = getportnum(proto,INETPORT_LISTEN) ;
	        port = rs1 ;
	        if (rs1 < 0)
	            port = INETPORTNUM_LISTEN ;
	    }
	    break ;
	default:
	    rs = SR_AFNOSUPPORT ;
	    break ;
	} /* end switch */

/* OK, we have everything we might need, do it! */

	if (rs >= 0) {
	    rs = procsocker(pip,af,netaddr,port) ;
	} else {
	    bprintf(pip->efp,"%s: error with network address >%s<\n",
	        pip->progname,netaddr1) ;
	}

	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progsock: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progsock) */


/* local subroutine */


static int procsocker(PROGINFO *pip,int af,cchar *netaddr,int port)
{
	SOCKADDRESS	sa ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = sockaddress_start(&sa,af,netaddr,port,0)) >= 0) {
	    const int	hlen = HEXBUFLEN ;
	    char	hbuf[HEXBUFLEN + 1] ;

	    if ((rs = sockaddress_gethex(&sa,hbuf,hlen)) >= 0) {
	        int	hl = rs ;
		int	i ;

/* lower case */

	        for (i = 0 ; i < hl ; i += 1) {
	            if (CHAR_ISUC(hbuf[i])) {
	                hbuf[i] = CHAR_TOLC(hbuf[i]) ;
		    }
	        } /* end for */

	        if ((rs = progout_open(pip)) >= 0) {
		    {
	                rs = progout_printf(pip,"\\x%t\n",hbuf,hl) ;
		        wlen += rs ;
		    }
	            rs1 = progout_close(pip) ;
		    if (rs >= 0) rs = rs1 ;
	        } /* end if (progout) */

	    } /* end if (gethex) */

	    sockaddress_finish(&sa) ;
	} /* end if (sockaddress) */

	return rs ;
}
/* end subroutine (procsocker) */


static int gethostinfo(pip,netaddr,af,namespec,pp)
PROGINFO	*pip ;
char		netaddr[] ;
int		af ;
const char	namespec[] ;
int		*pp ;
{
	HOSTINFO	hi ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		sl ;
	int		al = 0 ;
	const char	*pr = pip->pr ;
	const char	*tp ;
	char		hostname[MAXHOSTNAMELEN + 1] ;
	char		portname[PORTNAMELEN + 1] ;

	if (namespec == NULL) return SR_INVALID ;
	if (netaddr == NULL) return SR_FAULT ;

	if (namespec[0] == '\0') return SR_INVALID ;

	netaddr[0] = '\0' ;

	hostname[0] = '\0' ;
	portname[0] = '\0' ;
	if ((af == AF_INET4) && ((tp = strchr(namespec,':')) != NULL)) {

	    sl = (tp - namespec) ;
	    snwcpy(hostname,MAXHOSTNAMELEN,namespec,sl) ;

	    sncpy1(portname,PORTNAMELEN,(tp + 1)) ;

	} else {
	    sncpy1(hostname,MAXHOSTNAMELEN,namespec) ;
	}

	if (pp != NULL)
	    *pp = -1 ;

/* resolve the host name first if possible */

	if ((rs = hostinfo_start(&hi,af,hostname)) >= 0) {
	    HOSTINFO_CUR	hc ;

	    if ((rs = hostinfo_curbegin(&hi,&hc)) >= 0) {
		const int	aflen = getaflen(af) ;
		const uchar	*ap ;

	        while ((rs = hostinfo_enumaddr(&hi,&hc,&ap)) >= 0) {
		    al = rs ;

	            if (al == aflen) {
			memcpy(netaddr,ap,aflen) ;
			break ;
	            }

		} /* end while */

	        rs1 = hostinfo_curend(&hi,&hc) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */

	    rs1 = hostinfo_finish(&hi) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (hostinfo) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("gethostinfo: mid rs=%d\n",rs) ;
#endif

/* try to do something with the port spec */

	rs1 = SR_OK ;
	if ((rs >= 0) && (portname[0] != '\0')) {
	    rs1 = cfnumi(portname,-1,pp) ;
	}

	if (rs1 < 0)
	    *pp = -1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("gethostinfo: ret rs=%d af=%u\n",rs,al) ;
#endif

	return (rs >= 0) ? al : rs ;
}
/* end subroutine (gethostinfo) */


