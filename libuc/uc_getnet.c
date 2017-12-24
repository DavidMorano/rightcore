/* uc_getnet */

/* interface component for UNIX® library-3c */
/* subroutine to get a network entry (raw) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Name:

	uc_getnetbyname

	Description:

	This subroutine is a platform independent subroutine to get an
	INET 'network' (by name).

	Synopsis:

	int uc_getnetbyname(name,nep,rbuf,rlen)
	const char	name[] ;
	struct hostent	*nep ;
	char		rbuf[] ;
	int		rlen ;

	Arguments:

	- name		network name to lookup
	- nep		pointer to 'hostent' structure
	- rbuf		user supplied buffer to hold result
	- rlen		length of user supplied buffer

	Returns:

	0		host was found OK
	SR_FAULT	address fault
	SR_TIMEDOUT	request timed out (bad network someplace)
	SR_NOTFOUND	host could not be found
	SR_OVERFLOW	buffer overflow

	Name:

	uc_getnetbynumber

	Description:

	This subroutine is a platform independent subroutine to get an
	INET 'network' (by number).

	Synopsus:

	int uc_getnetbyaddr(addr,type,eep,rbuf,rlen)
	long		addr ;
	int		type ;
	struct hostent	*nep ;
	char		rbuf[] ;
	int		rlen ;

	Arguments:

	- addr		address to lookup
	- type		type of address to lookup
	- nep		pointer to 'hostent' structure
	- rbuf		user supplied buffer to hold result
	- rlen		length of user supplied buffer

	Returns:

	0		host was found OK
	SR_FAULT	address fault
	SR_TIMEDOUT	request timed out (bad network someplace)
	SR_NOTFOUND	host could not be found
	SR_OVERFLOW	buffer overflow

	Notes:

	- structure

	struct netent {
		char		*n_name;	# official name of net
		char		**n_aliases;	# alias list
		int		n_addrtype;	# net address type
		in_addr_t	n_net;		# network number
	} ;


*******************************************************************************/


#define	LIBUC_MASTER	0

#undef	LOCAL_SOLARIS
#define	LOCAL_SOLARIS	\
	(defined(OSNAME_SunOS) && (OSNAME_SunOS > 0))

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

#define	TO_AGAIN	10		/* seconds */

#define	RF_NETCOPY	0		/* flag to set need of 'netcopy()' */
#if	defined(SYSHAS_GETNETXXXR) && (SYSHAS_GETNETXXXR > 0)
#else
#undef	RF_NETCOPY
#define	RF_NETCOPY	1		/* flag to set need of 'netcopy()' */
#endif


/* external subroutines */

extern int	iceil(int,int) ;
extern int	msleep(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */

static int	netsize(struct netent *) ;

#if	RF_NETCOPY
static int	netcopy(struct netent *,char *,int,struct netent *) ;
static int	si_copystr(STOREITEM *,char **,const char *) ;
#endif


/* forward references */

#if	CF_DEBUGS
static int netdebug(struct netent *,char *) ;
#endif


/* local variables */


/* exported subroutines */


int uc_setnetent(int stayopen)
{
	setnetent(stayopen) ;
	return SR_OK ;
}

int uc_endnetent()
{
	endnetent() ;
	return SR_OK ;
}

int uc_getnetent(struct netent *nep,char *rbuf,int rlen)
{
	struct netent	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	if (nep == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

/* note (carefully) that there is NO POSIX standard version of this funtion */

	repeat {

#if	defined(SYSHAS_GETNETXXXR) && (SYSHAS_GETNETXXXR > 0)
	    {
	        errno = 0 ;
	        rp = getnetent_r(nep,rbuf,rlen) ;
	        if (rp != NULL) {
#if	CF_DEBUGS
	            netdebug(nep,rbuf) ;
#endif
	            rs = netsize(nep) ;
	        } else {
	            rs = (- errno) ;
	        }
#if	CF_DEBUGS
	        debugprintf("uc_getnetent: rp=%p rs=%d\n",rp,rs) ;
#endif
	    }
#else /* defined(SYSHAS_GETNETXXXR) && (SYSHAS_GETNETXXXR > 0) */
	    {
	        errno = 0 ;
	        rp = getnetent() ;
	        if (rp != NULL) {
	            rs = netcopy(nep,rbuf,rlen,rp) ;
	        } else {
	            rs = (- errno) ;
	        }
	    }
#endif /* defined(SYSHAS_GETNETXXXR) && (SYSHAS_GETNETXXXR > 0) */

	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
	                msleep(1000) ;
	            } else {
	                rs = SR_TIMEDOUT ;
	                f_exit = TRUE ;
	            }
	            break ;
	        case SR_INTR:
	            break ;
	        default:
	            f_exit = TRUE ;
	            break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (uc_getnetent) */


int uc_getnetbyname(cchar *name,struct netent *nep,char *rbuf,int rlen)
{
	struct netent	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_getnetbyname: ent name=%s\n",name) ;
#endif

	if (nep == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

/* do the real work */

	repeat {

#if	defined(SYSHAS_GETNETXXXR) && (SYSHAS_GETNETXXXR > 0)
	    {
	        errno = 0 ;
	        rp = getnetbyname_r(name,nep,rbuf,rlen) ;
	        if (rp != NULL) {
	            rs = netsize(nep) ;
	        } else {
	            rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	        }
	    }
#else /* defined(SYSHAS_GETNETXXXR) && (SYSHAS_GETNETXXXR > 0) */
	    {
	        errno = 0 ;
	        rp = getnetbyname(name) ;
	        if (rp != NULL) {
	            rs = netcopy(nep,rbuf,rlen,rp) ;
	        } else {
	            rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	        }
	    }
#endif /* defined(SYSHAS_GETNETXXXR) && (SYSHAS_GETNETXXXR > 0) */

	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
	                msleep(1000) ;
	            } else {
	                rs = SR_TIMEDOUT ;
	                f_exit = TRUE ;
	            }
	            break ;
	        case SR_INTR:
	            break ;
	        default:
	            f_exit = TRUE ;
	            break ;
	        } /* end switch */
	    } /* end if */
	} until ((rs >= 0) || f_exit) ;

#if	CF_DEBUGS
	debugprintf("uc_getnetbyname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_getnetbyname) */


int uc_getnetbyaddr(long addr,int type,struct netent *nep,char *rbuf,int rlen)
{
	struct netent	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_getnetbyaddr: addr=%08lu type=%u\n",addr,type) ;
#endif

	if (nep == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

/* do the real work */

	repeat {

#if	defined(SYSHAS_GETNETXXXR) && (SYSHAS_GETNETXXXR > 0)
	    {
	        errno = 0 ;
	        rp = getnetbyaddr_r(addr,type,nep,rbuf,rlen) ;
	        if (rp != NULL) {
	            rs = netsize(nep) ;
	        } else {
	            rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	        }
	    }
#else /* defined(SYSHAS_GETNETXXXR) && (SYSHAS_GETNETXXXR > 0) */
	    {
	        errno = 0 ;
	        rp = getnetbyaddr(addr,type) ;
	        if (rp != NULL) {
	            rs = netcopy(nep,rbuf,rlen,rp) ;
	        } else {
	            rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	        }
	    }
#endif /* defined(SYSHAS_GETNETXXXR) && (SYSHAS_GETNETXXXR > 0) */

	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
	                msleep(1000) ;
	            } else {
	                rs = SR_TIMEDOUT ;
	                f_exit = TRUE ;
	            }
	            break ;
	        case SR_INTR:
	            break ;
	        default:
	            f_exit = TRUE ;
	            break ;
	        } /* end switch */
	    } /* end if */
	} until ((rs >= 0) || f_exit) ;

#if	CF_DEBUGS
	debugprintf("uc_getnetbyaddr: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_getnetaddr) */


/* local subroutines */


static int netsize(struct netent *nep)
{
	int		size = 0 ;
	if (nep->n_name != NULL) {
	    size += (strlen(nep->n_name)+1) ;
	}
	if (nep->n_aliases != NULL) {
	    int		i ;
	    for (i = 0 ; nep->n_aliases[i] != NULL ; i += 1) {
	        size += (strlen(nep->n_aliases[i])+1) ;
	    } /* end for */
	    size += (i*sizeof(const char *)) ;
	} /* end if (group members) */
	size = iceil(size,sizeof(const char *)) ;
	return size ;
}
/* end subroutine (netsize) */

#if	RF_NETCOPY

static int netcopy(nep,rbuf,rlen,rp)
struct netent	*nep, *rp ;
char		rbuf[] ;
int		rlen ;
{
	STOREITEM	ib ;
	int		rs ;
	int		rs1 ;

	memcpy(nep,rp,sizeof(struct netent)) ;

	if ((rs = storeitem_start(&ib,rbuf,rlen)) >= 0) {

	    if (rp->n_aliases != NULL) {
	        int	n ;
	        void	**tab ;

	        for (n = 0 ; rp->n_aliases[n] != NULL ; n += 1) ;

	        if ((rs = storeitem_ptab(&ib,n,&tab)) >= 0) {
	            int		i ;
	            const char	**aliases = rp->n_aliases ;

	            nep->n_aliases = (char **) tab ;
	            for (i = 0 ; rp->n_aliases[i] != NULL ; i += 1) {
	                rs = si_copystr(&ib,(nep->n_aliases + i),aliases[i]) ;
	                if (rs < 0) break ;
	            }
	            nep->n_aliases[i] = NULL ;

	        } /* end if */

	    } /* end if (aliases) */

	    si_copystr(&ib,&nep->n_name,rp->n_name) ;

	    rs1 = storeitem_finish(&ib) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

	return rs ;
}
/* end subroutine (netcopy) */

static int si_copystr(ibp,pp,p1)
STOREITEM	*ibp ;
const char	*p1 ;
char		**pp ;
{
	int		rs = SR_OK ;
	const char	**cpp = (const char **) pp ;

	*cpp = NULL ;
	if (p1 != NULL) {
	    rs = storeitem_strw(ibp,p1,-1,cpp) ;
	}

	return rs ;
}
/* end subroutine (si_copystr) */

#endif /* RF_NETCOPY */

#if	CF_DEBUGS
static int netdebug(struct netent *nep,char *rbuf)
{
	debugprintf("uc_getnet/netdebug: rbuf=%p\n",rbuf) ;
	debugprintf("uc_getnet/netdebug: name{%p}\n",nep->n_name) ;
	debugprintf("uc_getnet/netdebug: name=%s\n",nep->n_name) ;
	if (nep->n_aliases != NULL) {
	    int	i ;
	    for (i = 0 ; nep->n_aliases[i] != NULL ; i += 1) {
	        const char	*a = nep->n_aliases[i] ;
	        debugprintf("uc_getnet/netdebug: alias=%s\n",a) ;
	    } /* end for */
	} /* end if (group members) */
	return 0 ;
}
/* end subroutine (netdebug) */
#endif /* CF_DEBUGS */


