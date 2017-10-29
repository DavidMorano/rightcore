/* uc_gethostby */

/* interface component for UNIX® library-3c */
/* subroutine to get a single "host" entry (raw) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_USEREENT	1		/* use reentrant API if available */


/* revision history:

	= 1998-11-01, David Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Name:

	uc_gethostbyname

	Description:

	This subroutine is a platform independent subroutine to get an
	INET host address entry, but does it dumbly on purpose.

	Synopsis:

	int uc_gethostbyname(name,hep,buf,buflen)
	const char	name[] ;
	struct hostent	*hep ;
	char		buf[] ;
	int		buflen ;

	Arguments:

	- name		name to lookup
	- hep		pointer to 'hostent' structure
	- buf		user supplied buffer to hold result
	- buflen	length of user supplied buffer

	Returns:

	0		host was found OK
	SR_FAULT	address fault
	SR_TIMEDOUT	request timed out (bad network someplace)
	SR_NOTFOUND	host could not be found


	Name:

	uc_gethostbyaddr

	Description:

	This subroutine is a platform independent subroutine to
	get an INET host address entry, but does it dumbly on purpose.

	Synopsis:

	int uc_gethostbyaddr(addr,addrlen,type,hep,buf,buflen)
	const char	*addr ;
	int		addrlen ;
	int		type ;
	struct hostent	*hep ;
	char		buf[] ;
	int		buflen ;

	Arguments:

	- addr		address to lookup
	- addrlen	length if the supplied address
	- type		type of address to look up
	- hep		pointer to 'hostent' structure
	- buf		user supplied buffer to hold result
	- buflen	length of user supplied buffer

	Returns:

	0		host was found OK
	SR_FAULT	address fault
	SR_TIMEDOUT	request timed out (bad network someplace)
	SR_NOTFOUND	host could not be found


*******************************************************************************/


#define	LIBUC_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<storeitem.h>
#include	<localmisc.h>


/* local defines */

#if	(! defined(SYSHAS_GETHOSTXXXR)) || (SYSHAS_GETHOSTXXXR == 0)
#undef	CF_USEREENT
#define	CF_USEREENT	0
#endif /* SYSHAS_GETHOSTXXXR */

#define	TIMEOUT		3


/* external subroutines */

extern int	msleep(int) ;


/* external variables */

#if	(CF_USEREENT == 0)
extern int	h_errno ;
#endif


/* local structures */

#if	CF_DEBUGS
struct herrno {
	int		herrno ;
	const char	*name ;
} ;
#endif /* CF_DEBUGS */


/* forward references */

#if	(CF_USEREENT == 0)
static int	hecopy(struct hostent *,char *,int,struct hostent *) ;
static int	si_copyaliases(STOREITEM *,struct hostent *,struct hostent *) ;
static int	si_copyaddrs(STOREITEM *,struct hostent *,struct hostent *) ;
static int	si_copystr(STOREITEM *,char **,const char *) ;
static int	si_copybuf(STOREITEM *,char **,char *,int) ;
#endif /* (CF_USEREENT == 0) */

#if	CF_DEBUGS
static const char	*strherrno(int) ;
#endif /* CF_DEBUGS */


/* local variables */

#if	CF_DEBUGS
static struct herrno	herrnos[] = {
	{ TRY_AGAIN, "try_again" },
	{ NO_RECOVERY, "no_recovery" },
 	{ NO_DATA, "no_data" },
	{ NO_ADDRESS, "no_address" },
	{ HOST_NOT_FOUND, "host_not_found" },
	{ NETDB_INTERNAL, "netdb_internal" },
	{ 0, "zero" }
} ;
#endif /* CF_DEBUGS */


/* exported subroutines */


int uc_gethostbyname(name,hep,buf,buflen)
const char	name[] ;
struct hostent	*hep ;
char		buf[] ;
int		buflen ;
{
	struct hostent	*lp ;

	int	rs = SR_OK ;
	int	i ;
#if	CF_USEREENT
	int	h_errno ;
#endif


#if	CF_DEBUGS
	debugprintf("uc_gethostbyname: name=>%s<\n", name) ;
#endif

	if (name == NULL)
	    return SR_FAULT ;

	if (hep == NULL)
	    return SR_FAULT ;

	if (buf == NULL)
	    return SR_FAULT ;

	if (buflen < 0)
	    return SR_INVALID ;

	errno = 0 ;			/* don't know if needed, but hey! */

/* do the real work */

#if	CF_USEREENT

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyname: USEREENT=1\n") ;
#endif

	for (i = 0 ; i < TIMEOUT ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyname: i=%u gethostbyname_r()\n",i) ;
#endif

	    h_errno = 0 ;
	    lp = gethostbyname_r(name,hep,buf,buflen,&h_errno) ;

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyname: gethostbyname_r() lp=%p\n",lp) ;
	    debugprintf("uc_gethostbyname: h_errno=%s\n",
		strherrno(h_errno)) ;
#endif 

	    if ((lp != NULL) || (h_errno != TRY_AGAIN)) break ;
	    msleep(1000) ;

	} /* end for */

#else /* CF_USEREENT */

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyname: POSIX=0\n") ;
#endif

	for (i = 0 ; i < TIMEOUT ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyname: gethostbyname()\n") ;
#endif

	    h_errno = 0 ;
	    lp = gethostbyname(name) ;

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyname: gethostbyname() lp=%p\n",lp) ;
	    debugprintf("uc_gethostbyname: h_errno=%s\n",
		strherrno(h_errno)) ;
#endif

	    if ((lp != NULL) || (h_errno != TRY_AGAIN)) break ;
	    msleep(1000) ;

	} /* end for */

#if	CF_DEBUGS
	debugprintf("uc_gethostbyname: hecopy()\n") ;
#endif

	if (lp != NULL)
	    rs = hecopy(hep,buf,buflen,lp) ;

#endif /* CF_USEREENT */

	if (rs < 0)
	    goto ret0 ;

	if (i >= TIMEOUT) {

	    rs = SR_TIMEDOUT ;

	} else if (lp == NULL) {

	    switch (h_errno) {

	    case TRY_AGAIN:
		rs = SR_AGAIN ;
		break ;

	    case NO_DATA:
		rs = SR_HOSTUNREACH ;
		break ;

	    case NO_RECOVERY:
		rs = SR_BADFMT ;
		break ;

	    case HOST_NOT_FOUND:
	        rs = SR_NOTFOUND ;
		break ;

#ifdef	NETDB_INTERNAL /* not everyone (MacOS) has this! */
	    case NETDB_INTERNAL:
		rs = (- errno) ;
		break ;
#endif /* NETDB_INTERNAL */

	    default:
		rs = (errno != 0) ? (- errno) : SR_PROTONOSUPPORT ;
		break ;

	    } /* end switch */

	} /* end if */

#if	CF_DEBUGS
	{
		int	i ;
		if (hep->h_aliases != NULL) {
		for (i = 0 ; hep->h_aliases[i] != NULL ; i += 1)
		debugprintf("uc_gethostbyname: alias[%u]=>%s<\n",
			i,hep->h_aliases[i]) ;
		}
	}
#endif /* CF_DEBUGS */

ret0:

#if	CF_DEBUGS
	debugprintf("uc_gethostbyname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_gethostbyname) */


int uc_gethostbyaddr(addr,addrlen,type,hep,buf,buflen)
const char	*addr ;
int		addrlen ;
int		type ;
struct hostent	*hep ;
char		buf[] ;
int		buflen ;
{
	struct hostent	*lp ;

	int	rs = SR_OK ;
	int	i ;
#if	CF_USEREENTRANT
	int	h_errno ;
#endif


#if	CF_DEBUGS
	debugprintf("uc_gethostbyaddr: entered\n") ;
#endif

	if (addr == NULL)
	    return SR_FAULT ;

	if (hep == NULL)
	    return SR_FAULT ;

	if (buf == NULL)
	    return SR_FAULT ;

	if (buflen < 0)
	    return SR_INVALID ;

	errno = 0 ;			/* don't know if needed, but hey! */

/* do the real work */

#if	CF_USEREENT

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyname: USEREENT=1\n") ;
#endif

	for (i = 0 ; i < TIMEOUT ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyaddr: i=%u gethostbyaddr_r()\n",i) ;
#endif

	    h_errno = 0 ;
	    lp = gethostbyaddr_r(addr,addrlen,type,
	        hep,buf,buflen,&h_errno) ;

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyaddr: gethostbyaddr_r() lp=%p\n",lp) ;
#endif 

	    if ((lp != NULL) || (h_errno != TRY_AGAIN)) break ;
	    msleep(1000) ;

	} /* end for */

#else /* CF_USEREENT */

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyaddr: USEREENT=0\n") ;
#endif

	for (i = 0 ; i < TIMEOUT ; i += 1) {

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyaddr: gethostbyaddr()\n") ;
#endif

	    h_errno = 0 ;
	    lp = gethostbyaddr(addr,addrlen,type) ;

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyaddr: gethostbyaddr() lp=%p\n",lp) ;
#endif

	    if ((lp != NULL) || (h_errno != TRY_AGAIN)) break ;
	    msleep(1000) ;

	} /* end for */

#if	CF_DEBUGS
	debugprintf("uc_gethostbyaddr: some memcpy\n") ;
#endif

	if (lp != NULL)
	    rs = hecopy(hep,buf,buflen,lp) ;

#endif /* CF_USEREENT */

	if (rs < 0)
	    goto ret0 ;

	if (i >= TIMEOUT) {

	    rs = SR_TIMEDOUT ;

	} else if (lp == NULL) {

	    switch (h_errno) {

	    case TRY_AGAIN:
		rs = SR_AGAIN ;
		break ;

	    case NO_DATA:
		rs = SR_HOSTUNREACH ;
		break ;

	    case NO_RECOVERY:
		rs = SR_BADFMT ;
		break ;

	    case HOST_NOT_FOUND:
	        rs = SR_NOTFOUND ;
		break ;

#ifdef	NETDB_INTERNAL /* not everyone (MacOS) has this! */
	    case NETDB_INTERNAL:
		rs = (- errno) ;
		break ;
#endif /* NETDB_INTERNAL */

	    default:
		rs = (errno != 0) ? (- errno) : SR_PROTONOSUPPORT ;
		break ;

	    } /* end switch */

	} /* end if */

ret0:

#if	CF_DEBUGS
	debugprintf("uc_gethostbyaddr: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_gethostbyaddr) */


/* local subroutines */


#if	(CF_USEREENT == 0)

#ifdef	COMMENT
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#endif /* COMMENT */

static int hecopy(hep,buf,buflen,lp)
struct hostent	*hep ;
char		buf[] ;
int		buflen ;
struct hostent	*lp ;
{
	STOREITEM	ib ;

	int	rs ;
	int	len = 0 ;


#if	CF_DEBUGS
	debugprintf("uc_gethostby/hwcopy: entered buflen=%u\n",buflen) ;
#endif

	rs = storeitem_start(&ib,buf,buflen) ;
	if (rs < 0) goto ret0 ;

	memcpy(hep,lp,sizeof(struct hostent)) ;

#if	CF_DEBUGS
	debugprintf("uc_gethostby/hwcopy: si_copyaliases\n") ;
#endif

	rs = si_copyaliases(&ib,hep,lp) ;

#if	CF_DEBUGS
	debugprintf("uc_gethostby/hwcopy: si_copyaliases() rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS
	debugprintf("uc_gethostby/hwcopy: si_copyaddrs\n") ;
#endif

	if (rs >= 0)
	    rs = si_copyaddrs(&ib,hep,lp) ;

#if	CF_DEBUGS
	{
	int	i ;
	debugprintf("uc_gethostby/hwcopy: si_copyaddrs() rs=%d\n",rs) ;
		if (hep->h_aliases != NULL) {
	for (i = 0 ; hep->h_aliases[i] != NULL ; i += 1) 
	debugprintf("uc_gethostby/hwcopy: alias[%u]=>%s<\n",
		i,hep->h_aliases[i]) ;
		}
	}
#endif /* CF_DEBUGS */

	if (rs >= 0)
	    rs = si_copystr(&ib,&hep->h_name,lp->h_name) ;

#if	CF_DEBUGS
	{
	int	i ;
	debugprintf("uc_gethostby/hwcopy: si_copystr() rs=%d\n",rs) ;
		if (hep->h_aliases != NULL) {
	for (i = 0 ; hep->h_aliases[i] != NULL ; i += 1) 
	debugprintf("uc_gethostby/hwcopy: alias[%u]=>%s<\n",
		i,hep->h_aliases[i]) ;
		}
	}
#endif /* CF_DEBUGS */

	if (rs >= 0)
	    len = storeitem_getlen(&ib) ;

	rs = storeitem_finish(&ib) ;

ret0:
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (hecopy) */

static int si_copyaliases(ibp,hep,lp)
STOREITEM	*ibp ;
struct hostent	*hep ;
struct hostent	*lp ;
{
	int	rs = SR_OK ;
	int	n, i ;

	void	**vpp ;

	char	**cpp ;


	if (lp->h_aliases != NULL) {

	    for (n = 0 ; lp->h_aliases[n] != NULL ; n += 1) ;

#if	CF_DEBUGS
	debugprintf("uc_gethostby/si_copyaliases: n=%u\n",n) ;
#endif

	    if ((rs = storeitem_ptab(ibp,n,&vpp)) >= 0) {

		cpp = (char **) vpp ;
		hep->h_aliases = cpp ;
		for (i = 0 ; lp->h_aliases[i] != NULL ; i += 1) {

#if	CF_DEBUGS
	debugprintf("uc_gethostby/si_copyaliases: alias=>%s<\n",
		lp->h_aliases[i]) ;
#endif

		    rs = si_copystr(ibp,(hep->h_aliases + i),
				lp->h_aliases[i]) ;

#if	CF_DEBUGS
		    debugprintf("uc_gethostby/si_copyaliases: "
			"si_copystr() rs=%d ds>%s<\n",
			rs,hep->h_aliases[i]) ;
#endif

		    if (rs < 0)
			    break ;

		} /* end for */

	        if (rs >= 0)
		    hep->h_aliases[i] = NULL ;

	    } /* end if */

	} else
	    hep->h_aliases = NULL ;

	return rs ;
}
/* end subroutine (si_copyaliases) */

static int si_copyaddrs(ibp,hep,lp)
STOREITEM	*ibp ;
struct hostent	*hep ;
struct hostent	*lp ;
{
	int	rs = SR_OK ;
	int	n, i ;

	void	**vpp ;

	char	**cpp ;


	if (lp->h_addr_list != NULL) {

	    for (n = 0 ; lp->h_addr_list[n] != NULL ; n += 1) ;

	    if ((rs = storeitem_ptab(ibp,n,&vpp)) >= 0) {

		cpp = (char **) vpp ;
	        hep->h_addr_list = cpp ;
	        for (i = 0 ; lp->h_addr_list[i] != NULL ; i += 1) {

		    rs = si_copybuf(ibp,(hep->h_addr_list + i),
				lp->h_addr_list[i],lp->h_length) ;

		    if (rs < 0)
		        break ;

	        } /* end for */

	        if (rs >= 0)
		    hep->h_addr_list[i] = NULL ;

	    } /* end if */

	} else
	    hep->h_addr_list = NULL ;

	return rs ;
}
/* end subroutine (si_copyaddrs) */

static int si_copystr(ibp,pp,s1)
STOREITEM	*ibp ;
char		**pp ;
const char	*s1 ;
{
	int	rs = SR_OK ;

	const char	**cpp = (const char **) pp ;


	*cpp = NULL ;
	if (s1 != NULL)
	    rs = storeitem_strw(ibp,s1,-1,cpp) ;

	return rs ;
}
/* end subroutine (si_copystr) */

static int si_copybuf(ibp,pp,b,blen)
STOREITEM	*ibp ;
char		**pp ;
char		b[] ;
int		blen ;
{
	int	rs = SR_OK ;

	const char	**cpp = (const char **) pp ;


	*cpp = NULL ;
	if (b != NULL)
	    rs = storeitem_buf(ibp,b,blen,cpp) ;

	return rs ;
}
/* end subroutine (si_copybuf) */

#endif /* (CF_USEREENT == 0) */


#if	CF_DEBUGS

static const char	*strherrno(n)
int	n ;
{
	int	i ;


	for (i = 0 ; herrnos[i].herrno != 0 ; i += 1) {

	    if (n == herrnos[i].herrno)
		break ;

	} /* end for */

	return herrnos[i].name ;
}
/* end subroutine (strherrno) */

#endif /* CF_DEBUGS */



