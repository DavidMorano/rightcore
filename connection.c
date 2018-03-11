/* connection */

/* manipulate INET connection information */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_LOCALFIFO	1		/* identify local FIFOs? */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will take an INET socket and a local domain name and
	find the hostname of the remote end of the socket.


*******************************************************************************/


#define	CONNECTION_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<vecstr.h>
#include	<hostent.h>
#include	<sockaddress.h>
#include	<hostinfo.h>
#include	<inetaddr.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"connection.h"


/* local defines */

#ifndef	VARPRLOCAL
#define	VARPRLOCAL	"LOCAL"
#endif

#undef	LOCALHOST
#define	LOCALHOST	"localhost"

#ifndef	INET4ADDRLEN
#define	INET4ADDRLEN	sizeof(struct in_addr)
#endif

#undef	HOSTBUFLEN
#define	HOSTBUFLEN	MAX(MAXHOSTNAMELEN,NI_MAXHOST)

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy1w(char *,int,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	isindomain(const char *,const char *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

int		connection_peername(CONNECTION *,SOCKADDRESS *,int,char *) ;

static int	connection_addname(CONNECTION *,vecstr *,cchar *,char *,int *) ;
static int	connection_ip4lookup(CONNECTION *,char *,int) ;

static int	isin4mapped(ushort *) ;
static int	isin6loopback(ushort *) ;


/* local variables */


/* exported subroutines */


int connection_start(CONNECTION *cnp,cchar *domainname)
{

	if (cnp == NULL) return SR_FAULT ;

	memset(cnp,0,sizeof(CONNECTION)) ;
	cnp->domainname = (char *) domainname ;
	cnp->f.inet = FALSE ;
	cnp->s = -1 ;

	return SR_OK ;
}
/* end subroutine (connection_start) */


int connection_finish(CONNECTION *cnp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (cnp == NULL) return SR_FAULT ;

	if (cnp->f.domainname && (cnp->domainname != NULL)) {
	    cnp->f.domainname = FALSE ;
	    rs1 = uc_free(cnp->domainname) ;
	    if (rs >= 0) rs = rs1 ;
	    cnp->domainname = NULL ;
	}

	if (cnp->peername != NULL) {
	    rs1 = uc_free(cnp->peername) ;
	    if (rs >= 0) rs = rs1 ;
	    cnp->peername = NULL ;
	}

	return rs ;
}
/* end subroutine (connection_finish) */


int connection_socksrcname(CONNECTION *cnp,char *peerbuf,int s)
{
	struct ustat	sb ;
	SOCKADDRESS	*sap ;
	const int	peerlen = CONNECTION_PEERNAMELEN ;
	int		sal = sizeof(SOCKADDRESS) ;
	int		rs ;
	int		len = 0 ;

	if (cnp == NULL) return SR_FAULT ;

	if (s < 0) return SR_BADF ;

	cnp->s = s ;
	sap = &cnp->sa ;
	if ((rs = u_getsockname(s,sap,&sal)) >= 0) {
	    cnp->f.sa = TRUE ;
	    rs = connection_peername(cnp,sap,sal,peerbuf) ;
	    len = rs ;
	}

#if	CF_LOCALFIFO
	if ((peerbuf[0] == '\0') && (u_fstat(s,&sb) >= 0)) {

#if	CF_DEBUGS
	    debugprintf("connection_sockpeername: mode=%10o\n",
	        sb.st_mode) ;
#endif

	    if (S_ISFIFO(sb.st_mode)) {
	        const char	*lh = LOCALHOST ;
	        rs = sncpy1(peerbuf,peerlen,lh) ;
	        len = rs ;
	    }

	} /* end if (local pipe) */
#endif /* CF_LOCALFIFO */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (connection_socksrcname) */


/* get the peername from the socket */
int connection_sockpeername(CONNECTION *cnp,char *peerbuf,int s)
{
	struct ustat	sb ;
	SOCKADDRESS	*sap ;
	const int	peerlen = CONNECTION_PEERNAMELEN ;
	int		sal = sizeof(SOCKADDRESS) ;
	int		rs ;
	int		len = 0 ;

	if (cnp == NULL) return SR_FAULT ;

	if (s < 0) return SR_BADF ;

	cnp->s = s ;
	sap = &cnp->sa ;
	if ((rs = u_getpeername(s,sap,&sal)) >= 0) {
	    cnp->f.sa = TRUE ;
	    rs = connection_peername(cnp,sap,sal,peerbuf) ;
	    len = rs ;
	}

#if	CF_LOCALFIFO
	if ((peerbuf[0] == '\0') && (u_fstat(s,&sb) >= 0)) {

#if	CF_DEBUGS
	    debugprintf("connection_sockpeername: mode=%10o\n",
	        sb.st_mode) ;
#endif

	    if (S_ISFIFO(sb.st_mode)) {
	        const char	*lh = LOCALHOST ;
	        rs = sncpy1(peerbuf,peerlen,lh) ;
	        len = rs ;
	    }

	} /* end if (local pipe) */
#endif /* CF_LOCALFIFO */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (connection_sockpeername) */


/* get the peername from the peer address */
int connection_peername(CONNECTION *cnp,SOCKADDRESS *sap,int sal,
	char *peerbuf)
{
	const int	peerlen = CONNECTION_PEERNAMELEN ;
	int		rs = SR_OK ;
	int		af ;
	int		len = 0 ;
	const char	*lh = LOCALHOST ;
	char		tmpnamebuf[HOSTBUFLEN + 1] ;
	char		namebuf[HOSTBUFLEN + 1] ;
	char		svcname[NI_MAXSERV + 1] ;

#if	CF_DEBUGS
	debugprintf("connection_peername: ent\n") ;
#endif

	if (cnp == NULL) return SR_FAULT ;
	if (sap == NULL) return SR_FAULT ;
	if (peerbuf == NULL) peerbuf = namebuf ;

	peerbuf[0] = '\0' ;

/* is the peername cached? */

	if (cnp->peername != NULL) {
	    if (peerbuf != NULL) {
	        rs = sncpy1(peerbuf,peerlen,cnp->peername) ;
	        len = rs ;
	    } else {
	        rs = strlen(cnp->peername) ;
	        len = rs ;
	    }
	} /* end if (cached peername) */

/* we have to do it the hard way */

#if	CF_DEBUGS
	debugprintf("connection_peername: not cached\n") ;
#endif

	if (rs >= 0) {
	    int		alen = 0 ;
	    cnp->f.sa = TRUE ;
	    cnp->f.addr = FALSE ;
	    memcpy(&cnp->sa,sap,sizeof(SOCKADDRESS)) ;

	    af = sockaddress_getaf(sap) ;

#if	CF_DEBUGS
	    debugprintf("connection_peername: af=%u\n",af) ;
#endif

	    switch (af) {
	    case AF_UNIX:
		alen = MAXPATHLEN ;
	        if ((rs = sockaddress_getaddr(sap,peerbuf,alen)) >= 0) {
	            len = rs ;
	            if (len == 0) {
	                rs = sncpy1(peerbuf,peerlen,lh) ;
	                len = rs ;
		    }
	        }
	        break ;
	    case AF_INET4:
		alen = INET4ADDRLEN ;
	        cnp->f.inet = TRUE ;
	        cnp->f.addr = TRUE ;
	        if ((rs = sockaddress_getaddr(sap,&cnp->netipaddr,alen)) >= 0) {
	            rs = connection_ip4lookup(cnp,peerbuf,peerlen) ;
	            len = rs ;
	        }
	        break ;
	    case AF_INET6:
	        cnp->f.inet = TRUE ;
	        {
	            const struct sockaddr	*ssap ;
	            int		flags = NI_NOFQDN ;
	            ushort	in6addr[8] ;
	            ssap = (struct sockaddr *) sap ;
	            rs = uc_getnameinfo(ssap,sal,tmpnamebuf,NI_MAXHOST,
	                svcname,NI_MAXSERV,flags) ;
	            if (rs >= 0) {
	                rs = sncpy1(peerbuf,MAXHOSTNAMELEN,tmpnamebuf) ;
	                len = rs ;
	            } else if (isNotPresent(rs)) {
	                sockaddress_getaddr(sap,in6addr,16) ;
	                if (isin4mapped(in6addr)) {
			    alen = INET6ADDRLEN ;
	                    cnp->f.addr = TRUE ;
	                    memcpy(&cnp->netipaddr,(in6addr + 6),alen) ;
	                    rs = connection_ip4lookup(cnp,peerbuf,peerlen) ;
	                    len = rs ;
	                } else if (isin6loopback(in6addr)) {
	                    rs = sncpy1(peerbuf,peerlen,lh) ;
	                    len = rs ;
	                }
	            } /* end if (getnameinfo) */
	        } /* end if (AF_INET6) */
	        break ;
	    default:
	        break ;
	    } /* end if (got an INET host entry) */

#if	CF_DEBUGS
	    debugprintf("connection_peername: saving peername=%s\n",peerbuf) ;
#endif

	    if (rs >= 0) {
	        if (peerbuf[0] != '\0') {
	            if (cnp->peername != NULL) {
	                uc_free(cnp->peername) ;
	                cnp->peername = NULL ;
	            }
	            cnp->peername = mallocstrw(peerbuf,len) ;
	            if (cnp->peername == NULL) rs = SR_NOMEM ;
	        } else if (af != AF_UNIX) {
	            rs = SR_NOTFOUND ;
		}
	    } /* end if */

	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("connection_peername: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (connection_peername) */


/* create a list (vector list) of the names for this connection */
int connection_mknames(CONNECTION *cnp,vecstr *nlp)
{
	HOSTINFO	hi ;
	HOSTINFO_CUR	hicur ;
	inetaddr	ia ;
	uint		af ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		al ;
	int		n = 0 ;
	const uchar	*ap ;
	const char	*hp ;
	char		namebuf[MAXHOSTNAMELEN + 1] ;

	if (cnp == NULL) return SR_FAULT ;
	if (nlp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("connection_mknames: ent peername=%s\n", cnp->peername) ;
#endif

	if ((cnp->peername != NULL) && (cnp->peername[0] != '\0')) {

#if	CF_DEBUGS
	    debugprintf("connection_mknames: adding pn=%s\n",
	        cnp->peername) ;
#endif

	    rs = connection_addname(cnp,nlp,cnp->peername,namebuf,NULL) ;
	    n += rs ;
#if	CF_DEBUGS
	    debugprintf("connection_mknames: _addname() rs=%d\n",rs) ;
#endif
	}

	if (rs < 0)
	    goto ret0 ;

	if (! cnp->f.sa) {
	    rs = SR_DESTADDRREQ ;
	    goto ret0 ;
	}

#if	CF_DEBUGS
	debugprintf("connection_mknames: mid0 rs=%d n=%u\n",rs,n) ;
#endif

	rs = sockaddress_getaf(&cnp->sa) ;
	af = rs ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("connection_mknames: sockaddress_getaf() rs=%d\n",rs) ;
#endif

/* UNIX family */

	if (af == AF_UNIX) {

#if	CF_DEBUGS
	    debugprintf("connection_mknames: AF_UNIX -> 'localhost'\n") ;
#endif

	    n = 1 ;
	    rs = connection_addname(cnp,nlp,LOCALHOST,namebuf,NULL) ;
	    goto ret0 ;
	}

#if	CF_DEBUGS
	debugprintf("connection_mknames: mid1 rs=%d n=%u\n",rs,n) ;
#endif

/* prepare for INET hostname lookups */

	if (cnp->peername == NULL)
	    goto ret0 ;

	if (rs == 0)
	    goto ret0 ;

/* INET families? */

#if	CF_DEBUGS
	debugprintf("connection_mknames: mid2 rs=%d n=%u\n",rs,n) ;
#endif

	if ((rs = hostinfo_start(&hi,af,cnp->peername)) >= 0) {

	    if (rs >= 0) {
	        rs = hostinfo_geteffective(&hi,&hp) ;
	        if (rs >= 0) {
	            rs = connection_addname(cnp,nlp,hp,namebuf,NULL) ;
	            n += rs ;
	        }
	    }

#if	CF_DEBUGS
	debugprintf("connection_mknames: mid3 rs=%d n=%u\n",rs,n) ;
#endif

	    if (rs >= 0) {
	        if ((rs = hostinfo_getcanonical(&hi,&hp)) >= 0) {
#if	CF_DEBUGS
	debugprintf("connection_mknames: addname\n") ;
#endif
	            rs = connection_addname(cnp,nlp,hp,namebuf,NULL) ;
	            n += rs ;
#if	CF_DEBUGS
	debugprintf("connection_mknames: out1 rs=%d n=%u\n",rs,n) ;
#endif
	        }
#if	CF_DEBUGS
	debugprintf("connection_mknames: out2 rs=%d n=%u\n",rs,n) ;
#endif
	    }

#if	CF_DEBUGS
	debugprintf("connection_mknames: mid4 rs=%d n=%u\n",rs,n) ;
#endif

/* all other names */

	    if (rs >= 0) {
	        if ((hostinfo_curbegin(&hi,&hicur)) >= 0) {

	            while (hostinfo_enumname(&hi,&hicur,&hp) >= 0) {

	                rs = connection_addname(cnp,nlp,hp,namebuf,NULL) ;
	                n += rs ;
	                if (rs < 0)
	                    break ;

	            } /* end while (names) */

	            hostinfo_curend(&hi,&hicur) ;
	        } /* end if (hostinfo-cur) */
	    } /* end if (names) */

#if	CF_DEBUGS
	debugprintf("connection_mknames: mid5 rs=%d n=%u\n",rs,n) ;
#endif

/* addresses as names */

	    if (rs >= 0) {
		const int	nlen = MAXHOSTNAMELEN ;

	        if ((rs = hostinfo_curbegin(&hi,&hicur)) >= 0) {

	        while ((al = hostinfo_enumaddr(&hi,&hicur,&ap)) >= 0) {
	            if (al != INET4ADDRLEN) continue ;

	            if ((rs = inetaddr_start(&ia,ap)) >= 0) {

	                inetaddr_getdotaddr(&ia,namebuf,nlen) ;

	                rs = vecstr_adduniq(nlp,namebuf,-1) ;
	                if (rs < INT_MAX) n += 1 ;

	                inetaddr_finish(&ia) ;
	            } /* end if (inet4addr) */

	            if (rs < 0) break ;
	        } /* end while (addresses) */

	        hostinfo_curend(&hi,&hicur) ;
		} /* end if (hostinfo-cur) */
	    } /* end if (addresses) */

	    rs1 = hostinfo_finish(&hi) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (hostinfo) */

#if	CF_DEBUGS
	debugprintf("connection_mknames: mid6 rs=%d n=%u\n",rs,n) ;
#endif

/* store the INET "dot" address also! */

	if ((rs >= 0) && (af == AF_INET4) && cnp->f.inet && cnp->f.addr) {
	    if ((rs = inetaddr_start(&ia,&cnp->netipaddr)) >= 0) {
	        inetaddr_getdotaddr(&ia,namebuf,MAXHOSTNAMELEN) ;
	        rs = vecstr_adduniq(nlp,namebuf,-1) ;
	        if (rs < INT_MAX) n += 1 ;
	        inetaddr_finish(&ia) ;
	    } /* end if (inetaddr) */
	} /* end if (go) */

ret0:

#if	CF_DEBUGS
	debugprintf("connection_mknames: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (connection_mknames) */


/* private subroutines */


static int connection_addname(cnp,nlp,name,namebuf,idp)
CONNECTION	*cnp ;
vecstr		*nlp ;
const char	name[] ;
char		namebuf[] ;
int		*idp ;
{
	int		rs = SR_OK ;
	int		nl ;
	int		n = 0 ;
	const char	*tp ;

	if (vecstr_find(nlp,name) < 0) {
	    n += 1 ;
	    rs = vecstr_add(nlp,name,-1) ;
	}

	if ((rs >= 0) && (namebuf != NULL)) {
	    if ((cnp->domainname != NULL) && isindomain(name,cnp->domainname)) {

	        if (idp != NULL)
	            *idp = TRUE ;

#ifdef	COMMENT
	        nl = strwcpy(namebuf,name,MAXHOSTNAMELEN) - namebuf ;

	        if ((tp = strchr(namebuf,'.')) != NULL) {
	            nl = (tp - namebuf) ;
	            namebuf[nl] = '\0' ;
	        }
#else
	        nl = MAXHOSTNAMELEN ;
	        if ((tp = strchr(name,'.')) != NULL) nl = (tp - name) ;
	        nl = strwcpy(namebuf,name,nl) - namebuf ;
#endif /* COMMENT */

	        if (vecstr_findn(nlp,namebuf,nl) < 0) {
	            n += 1 ;
	            rs = vecstr_add(nlp,namebuf,nl) ;
	        }

	    } /* end if (it's in our domain) */
	} /* end if (non-null) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (connection_addname) */


/* lookup this IP (INET4) address */
static int connection_ip4lookup(cnp,peerbuf,peerlen)
CONNECTION	*cnp ;
char		peerbuf[] ;
int		peerlen ;
{
	struct hostent	he ;
	const int	helen = getbufsize(getbufsize_he) ;
	int		rs = SR_OK ;
	int		rs1 ;
	char		*hebuf ;

	if ((rs = uc_malloc((helen+1),&hebuf)) >= 0) {
	    const int	af = AF_INET4 ;
	    const int	al = INET4ADDRLEN ;
	    cchar	*ap  = (const char *) &cnp->netipaddr ;
	    cchar	*tp ;

	    if ((rs1 = uc_gethostbyaddr(ap,al,af,&he,hebuf,helen)) >= 0) {
	        hostent_cur	hc ;
	        const char	*np ;

	        if (cnp->domainname != NULL) {
	            if ((rs = hostent_curbegin(&he,&hc)) >= 0) {
	                while ((rs1 = hostent_enumname(&he,&hc,&np)) >= 0) {
	                    if (isindomain(np,cnp->domainname)) break ;
	                } /* end while */
	                hostent_curend(&he,&hc) ;
	            } /* end if (hostent-cur) */
	        } else
	            rs1 = SR_INVALID ;

		if (rs >= 0) {
	        if (rs1 >= 0) {

/* it is in our domain */

#ifdef	COMMENT
	            rs = sncpy1(peerbuf,peerlen,np) ;
	            if ((tp = strchr(peerbuf,'.')) != NULL) {
	                *tp = '\0' ;
	            }
#else
	            {
	                int	nl = -1 ;
	                if ((tp = strchr(np,'.')) != NULL) nl = (tp-np) ;
	                rs = sncpy1w(peerbuf,peerlen,np,nl) ;
	            }
#endif /* COMMENT */

	        } else {

/* it is NOT in our domain */

	            rs = hostent_getcanonical(&he,&np) ;

	            if (rs >= 0)
	                rs = sncpy1(peerbuf,peerlen,np) ;

	        } /* end if */
		} /* end if (ok) */

	    } else {
	        inetaddr	ia ;

	        if ((rs = inetaddr_start(&ia,&cnp->netipaddr)) >= 0) {

	            rs = inetaddr_getdotaddr(&ia,peerbuf,peerlen) ;

	            inetaddr_finish(&ia) ;
	        } /* end if (inetadd) */

	    } /* end if */

	    uc_free(hebuf) ;
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (connection_ip4lookup) */


static int isin4mapped(ushort in6addr[])
{
	ushort		v ;
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; i < 5 ; i += 1) {
	    if (in6addr[i] != 0) break ;
	}

	if (i == 5) {
	    v = in6addr[5] ;
	    f = ((v == 0) || (v == 0xFFFF)) ;
	}

	return f ;
}
/* end subroutine (isin4mapped) */


static int isin6loopback(ushort in6addr[])
{
	ushort		v ;
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; i < 7 ; i += 1) {
	    if (in6addr[i] != 0) break ;
	}

	if (i == 7) {
	    v = ntohs(in6addr[7]) ;
	    f = (v == 0x0001) ;
	}

	return f ;
}
/* end subroutine (isin6loopback) */


