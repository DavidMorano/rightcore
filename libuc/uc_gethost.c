/* uc_gethost */

/* interface component for UNIX® library-3c */
/* subroutine to get a single "host" entry (raw) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_USEREENT	1		/* use reentrant API if available */
#define	CF_GETHOSTENT	1		/* compile |uc_gethostent(3uc)| */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Name:

	uc_gethostbyname

	Description:

        This subroutine is a platform independent subroutine to get an INET host
        address entry, but does it dumbly on purpose.

	Synopsis:

	int uc_gethostbyname(name,hep,hebuf,helen)
	const char	name[] ;
	struct hostent	*hep ;
	char		hebuf[] ;
	int		helen ;

	Arguments:

	- name		name to lookup
	- hep		pointer to 'hostent' structure
	- hebuf		user supplied buffer to hold result
	- helen		length of user supplied buffer

	Returns:

	0		host was found OK
	SR_FAULT	address fault
	SR_TIMEDOUT	request timed out (bad network someplace)
	SR_NOTFOUND	host could not be found


	Name:

	uc_gethostbyaddr

	Description:

        This subroutine is a platform independent subroutine to get an INET host
        address entry, but does it dumbly on purpose.

	Synopsis:

	int uc_gethostbyaddr(addr,addrlen,type,hep,hebuf,helen)
	const char	*addr ;
	int		addrlen ;
	int		type ;
	struct hostent	*hep ;
	char		hebuf[] ;
	int		helen ;

	Arguments:

	- addr		address to lookup
	- addrlen	length if the supplied address
	- type		type of address to look up
	- hep		pointer to 'hostent' structure
	- hebuf		user supplied buffer to hold result
	- helen		length of user supplied buffer

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
#include	<hostent.h>
#include	<localmisc.h>


/* local defines */

#if	(! defined(SYSHAS_GETHOSTXXXR)) || (SYSHAS_GETHOSTXXXR == 0)
#undef	CF_USEREENT
#define	CF_USEREENT	0
#endif /* SYSHAS_GETHOSTXXXR */

#define	NLOOPS		3


/* external subroutines */

extern int	msleep(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif


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

static int	gethosterr(int) ;

#if	CF_DEBUGS
static cchar	*strherrno(int) ;
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


int uc_sethostent(int stayopen)
{
	int		rs ;
	errno = 0 ;
	if ((rs = sethostent(stayopen)) != 0) {
	    rs = (errno == 0) ? SR_IO : (- errno) ;
	}
	return rs ;
}
/* end subrouttine (uc_sethostent) */


int uc_endhostent()
{
	int		rs ;
	errno = 0 ;
	if ((rs = endhostent()) != 0) {
	    rs = (errno == 0) ? SR_IO : (- errno) ;
	}
	return rs ;
}
/* end subrouttine (uc_endhostent) */


#if	CF_GETHOSTENT
int uc_gethostent(struct hostent *hep,char *hebuf,int helen)
{
	struct hostent	*lp ;
	int		rs = SR_OK ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("uc_gethostent: ent helen=%d\n",helen) ;
#endif

	if (hep == NULL) return SR_FAULT ;
	if (hebuf == NULL) return SR_FAULT ;

	if (helen <= 0) return SR_OVERFLOW ;

/* do the real work */

#if	CF_USEREENT
	{
	    int		h_errno ;

#if	CF_DEBUGS
	    debugprintf("uc_gethostent: USEREENT=1\n") ;
#endif

	    for (i = 0 ; i < NLOOPS ; i += 1) {
	        if (i > 0) msleep(1000) ;

#if	CF_DEBUGS
	        debugprintf("uc_gethostent: i=%u gethostent_r()\n",i) ;
#endif

	        rs = SR_OK ;
	        errno = 0 ;
	        h_errno = 0 ;
	        lp = gethostent_r(hep,hebuf,helen,&h_errno) ;

	        if (lp != NULL) break ;
	        rs = gethosterr(h_errno) ;
	        if (rs != SR_AGAIN) break ;

#if	CF_DEBUGS
	        debugprintf("uc_gethostent: gethostent_r() lp=%p\n",lp) ;
	        debugprintf("uc_gethostent: h_errno=%s\n",
	            strherrno(h_errno)) ;
#endif 

	    } /* end for */

	    if (rs >= 0) {
	        rs = hostent_size(hep) ;
	    } else if (rs == SR_AGAIN) {
	        rs = SR_TIMEDOUT ;
	    }

	}
#else /* CF_USEREENT */
	{

#if	CF_DEBUGS
	    debugprintf("uc_gethostent: POSIX=0\n") ;
#endif

	    for (i = 0 ; i < NLOOPS ; i += 1) {
	        if (i > 0) msleep(1000) ;

#if	CF_DEBUGS
	        debugprintf("uc_gethostent: gethostent()\n") ;
#endif

	        rs = SR_OK ;
	        errno = 0 ;
	        h_errno = 0 ;
	        lp = gethostent() ;
	        if (lp != NULL) break ;

	        rs = gethosterr(h_errno) ;
	        if (rs != SR_AGAIN) break ;

#if	CF_DEBUGS
	        debugprintf("uc_gethostent: gethostent() lp=%p\n",lp) ;
	        debugprintf("uc_gethostent: h_errno=%s\n",
	            strherrno(h_errno)) ;
#endif

	    } /* end for */

	    if (rs >= 0) {
	        rs = hostent_load(hep,hebuf,helen,lp) ;
	    } else if (rs == SR_AGAIN) {
	        rs = SR_TIMEDOUT ;
	    }

	}
#endif /* CF_USEREENT */

	if (rs == SR_PROTONOSUPPORT) rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("uc_gethostent: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_gethostent) */
#endif /* CF_GETHOSTENT */


int uc_gethostbyname(cchar *name,struct hostent *hep,char *hebuf,int helen)
{
	struct hostent	*lp ;
	int		rs = SR_OK ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("uc_gethostbyname: name=>%s<\n", name) ;
	debugprintf("uc_gethostbyname: helen=%d\n",helen) ;
#endif

	if (name == NULL) return SR_FAULT ;
	if (hep == NULL) return SR_FAULT ;
	if (hebuf == NULL) return SR_FAULT ;

	if (helen <= 0) return SR_OVERFLOW ;

/* do the real work */

#if	CF_USEREENT
	{
	    int	h_errno ;

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyname: USEREENT=1\n") ;
#endif

	    for (i = 0 ; i < NLOOPS ; i += 1) {
	        if (i > 0) msleep(1000) ;

#if	CF_DEBUGS
	        debugprintf("uc_gethostbyname: i=%u gethostbyname_r()\n",i) ;
#endif

	        rs = SR_OK ;
	        errno = 0 ;
	        h_errno = 0 ;
	        lp = gethostbyname_r(name,hep,hebuf,helen,&h_errno) ;

	        if (lp != NULL) break ;
	        rs = gethosterr(h_errno) ;
	        if (rs != SR_AGAIN) break ;

#if	CF_DEBUGS
	        debugprintf("uc_gethostbyname: gethostbyname_r() lp=%p\n",lp) ;
	        debugprintf("uc_gethostbyname: h_errno=%s\n",
	            strherrno(h_errno)) ;
#endif 

	    } /* end for */

	    if (rs >= 0) {
	        rs = hostent_size(hep) ;
	    } else if (rs == SR_AGAIN) {
	        rs = SR_TIMEDOUT ;
	    }

	}
#else /* CF_USEREENT */
	{

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyname: POSIX=0\n") ;
#endif

	    for (i = 0 ; i < NLOOPS ; i += 1) {
	        if (i > 0) msleep(1000) ;

#if	CF_DEBUGS
	        debugprintf("uc_gethostbyname: gethostbyname()\n") ;
#endif

	        rs = SR_OK ;
	        errno = 0 ;
	        h_errno = 0 ;
	        lp = gethostbyname(name) ;

	        if (lp != NULL) break ;
	        rs = gethosterr(h_errno) ;
	        if (rs != SR_AGAIN) break ;

#if	CF_DEBUGS
	        debugprintf("uc_gethostbyname: gethostbyname() lp=%p\n",lp) ;
	        debugprintf("uc_gethostbyname: h_errno=%s\n",
	            strherrno(h_errno)) ;
#endif

	    } /* end for */

	    if (rs >= 0) {
	        rs = hostent_load(hep,hebuf,helen,lp) ;
	    } else if (rs == SR_AGAIN) {
	        rs = SR_TIMEDOUT ;
	    }

	}
#endif /* CF_USEREENT */

#if	CF_DEBUGS
	if (rs >= 0) {
	    int	i ;
	    if (hep->h_aliases != NULL) {
	        for (i = 0 ; hep->h_aliases[i] != NULL ; i += 1) {
	            debugprintf("uc_gethostbyname: alias[%u]=>%s<\n",
	                i,hep->h_aliases[i]) ;
		}
	    }
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	debugprintf("uc_gethostbyname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_gethostbyname) */


int uc_gethostbyaddr(cchar *addr,int addrlen,int type,struct hostent *hep,
		char *hebuf,int helen)
{
	struct hostent	*lp ;
	int		rs = SR_OK ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("uc_gethostbyaddr: ent\n") ;
#endif

	if (addr == NULL) return SR_FAULT ;
	if (hep == NULL) return SR_FAULT ;
	if (hebuf == NULL) return SR_FAULT ;

	if (helen <= 0) return SR_OVERFLOW ;

/* do the real work */

#if	CF_USEREENT
	{
	    int	h_errno ;

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyname: USEREENT=1\n") ;
#endif

	    for (i = 0 ; i < NLOOPS ; i += 1) {
	        if (i > 0) msleep(1000) ;

#if	CF_DEBUGS
	        debugprintf("uc_gethostbyaddr: i=%u gethostbyaddr_r()\n",i) ;
#endif

	        rs = SR_OK ;
	        errno = 0 ;
	        h_errno = 0 ;
	        lp = gethostbyaddr_r(addr,addrlen,type,
	            hep,hebuf,helen,&h_errno) ;

	        if (lp != NULL) break ;

	        rs = gethosterr(h_errno) ;
	        if (rs != SR_AGAIN) break ;

#if	CF_DEBUGS
	        debugprintf("uc_gethostbyaddr: gethostbyaddr_r() lp=%p\n",lp) ;
#endif 

	    } /* end for */

	    if (rs >= 0) {
	        rs = hostent_size(hep) ;
	    } else if (rs == SR_AGAIN) {
	        rs = SR_TIMEDOUT ;
	    }

	}
#else /* CF_USEREENT */
	{

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyaddr: USEREENT=0\n") ;
#endif

	    for (i = 0 ; i < NLOOPS ; i += 1) {
	        if (i > 0) msleep(1000) ;

#if	CF_DEBUGS
	        debugprintf("uc_gethostbyaddr: gethostbyaddr()\n") ;
#endif

	        rs = SR_OK ;
	        errno = 0 ;
	        h_errno = 0 ;
	        lp = gethostbyaddr(addr,addrlen,type) ;
	        if (lp != NULL) break ;

	        rs = gethosterr(h_errno) ;
	        if (rs != SR_AGAIN) break ;

#if	CF_DEBUGS
	        debugprintf("uc_gethostbyaddr: gethostbyaddr() lp=%p\n",lp) ;
#endif

	    } /* end for */

#if	CF_DEBUGS
	    debugprintf("uc_gethostbyaddr: some memcpy\n") ;
#endif

	    if (rs >= 0) {
	        rs = hostent_load(hep,hebuf,helen,lp) ;
	    } else if (rs == SR_AGAIN) {
	        rs = SR_TIMEDOUT ;
	    }

	}
#endif /* CF_USEREENT */

#if	CF_DEBUGS
	debugprintf("uc_gethostbyaddr: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_gethostbyaddr) */


/* local subroutines */


static int gethosterr(int h_errno)
{
	int		rs = SR_OK ;
	if (errno != 0) {
	    rs = (- errno) ;
	} else {
	    switch (h_errno) {
	    case TRY_AGAIN:
	        rs = SR_AGAIN ;
	        break ;
	    case NO_DATA:
	        rs = SR_HOSTUNREACH ;
	        break ;
	    case NO_RECOVERY:
	        rs = SR_IO ;
	        break ;
	    case HOST_NOT_FOUND:
	        rs = SR_NOTFOUND ;
	        break ;
#ifdef	NETDB_INTERNAL /* not everyone (MacOS) has this! */
	    case NETDB_INTERNAL:
	        rs = SR_NOANODE ;
	        break ;
#endif /* NETDB_INTERNAL */
	    default:
	        rs = SR_PROTONOSUPPORT ;
	        break ;
	    } /* end switch */
	} /* end if (errno) */
	return rs ;
}
/* end subroutine (gethosterr) */


#if	CF_DEBUGS
static const char	*strherrno(int n)
{
	int		i ;
	for (i = 0 ; herrnos[i].herrno != 0 ; i += 1) {
	    if (n == herrnos[i].herrno) break ;
	} /* end for */
	return herrnos[i].name ;
}
/* end subroutine (strherrno) */
#endif /* CF_DEBUGS */


