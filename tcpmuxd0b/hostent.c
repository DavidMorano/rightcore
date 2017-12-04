/* hostent */

/* manipulate host entry structures */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-02-03, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These routines provide means to peruse the elements of a 'hostent'
	structure as returned by the system subroutines:

		gethostbyname
		gethostbyaddr

	and their kin.

	All, INET addresses (when in any sort of binary form) are in network
	byte order!  This is true of the above subroutines as well.

	Below is the structure that represents the object itself:

	struct	hostent {
		char	*h_name;
		char	**h_aliases;
		int	h_addrtype;
		int	h_length;
		char	**h_addr_list;
	#define	h_addr	h_addr_list[0]	
	} ;


*******************************************************************************/


#define	HOSTENT_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storeitem.h>
#include	<localmisc.h>

#include	"hostent.h"

#if	CF_DEBUGS
#include	"inetaddr.h"
#endif


/* local defines */

#ifndef	HOSTENT
#define	HOSTENT		struct hostent
#endif


/* external subroutines */

extern int	iceil(int,int) ;


/* forward references */

static int	si_copyaliases(STOREITEM *,struct hostent *,struct hostent *) ;
static int	si_copyaddrs(STOREITEM *,struct hostent *,struct hostent *) ;
static int	si_copystr(STOREITEM *,char **,cchar *) ;
static int	si_copybuf(STOREITEM *,char **,cchar *,int) ;


/* exported subroutines */


int hostent_getofficial(HOSTENT *hep,cchar **rpp)
{
	int		rs = SR_NOTFOUND ;
	int		nlen = 0 ;

	if (hep == NULL) return SR_FAULT ;

	if (hep->h_name != NULL) {

	    rs = SR_OK ;
	    nlen = strlen(hep->h_name) ;

	    if (rpp != NULL)
	        *rpp = hep->h_name ;

	} /* end if */

	if ((rs < 0) && (rpp != NULL))
	    *rpp = NULL ;

	return (rs >= 0) ? nlen : rs ;
}
/* end subroutine (hostent_getofficial) */


/* get address family type (assume it is in host byte order) */
int hostent_getaf(HOSTENT *hep)
{
	int		type ;

	if (hep == NULL) return SR_FAULT ;

	type = (hep->h_addrtype) ;
	return type ;
}
/* end subroutine (hostent_getaf) */


int hostent_getalen(HOSTENT *hep)
{

	if (hep == NULL) return SR_FAULT ;
	return hep->h_length ;
}
/* end subroutine (hostent_getalen) */


int hostent_curbegin(HOSTENT *hep,HOSTENT_CUR *curp)
{

	if (hep == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (hostent_curbegin) */


int hostent_curend(HOSTENT *hep,HOSTENT_CUR *curp)
{

	if (hep == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (hostent_curend) */


/* enumerate the next hostname */
int hostent_enumname(HOSTENT *hep,HOSTENT_CUR *curp,cchar **rpp)
{
	int		rs = SR_NOTFOUND ;
	int		nlen = 0 ;

	if (hep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("hostent_enumname: rpp=%08X\n",(int) rpp) ;
#endif

	if (curp == NULL) {

	    if (hep->h_name != NULL) {

		rs = SR_OK ;
	        nlen = strlen(hep->h_name) ;

	        if (rpp != NULL)
	            *rpp = hep->h_name ;

	    }

	} else {

	    if (curp->i < 0) {

	        if (hep->h_name != NULL) {

		    rs = SR_OK ;
	            nlen = strlen(hep->h_name) ;

	            if (rpp != NULL)
	                *rpp = hep->h_name ;

	            curp->i = 0 ;
	        }

	    } else {

	        if ((hep->h_aliases != NULL) &&
	            (hep->h_aliases[curp->i] != NULL)) {

		    rs = SR_OK ;
	            nlen = strlen(hep->h_aliases[curp->i]) ;

	            if (rpp != NULL)
	                *rpp = hep->h_aliases[curp->i] ;

	            curp->i += 1 ;
	        }

	    } /* end if */

	} /* end if */

	if ((rs < 0) && (rpp != NULL))
	    *rpp = NULL ;

#if	CF_DEBUGS
	debugprintf("hostent_enumname: ret rs=%d nlen=%u\n",rs,nlen) ;
#endif

	return (rs >= 0) ? nlen : rs ;
}
/* end subroutine (hostent_enumname) */


/* enumerate the next host address */
int hostent_enumaddr(HOSTENT *hep,HOSTENT_CUR *curp,const uchar **rpp)
{
	int		rs = SR_NOTFOUND ;
	int		alen ;
	int		ci ;

	if (hep == NULL) return SR_FAULT ;

	alen = hep->h_length ;
	if (hep->h_addr_list != NULL) {

	    if (curp == NULL) {

	        if (hep->h_addr_list[0] != NULL) {

	            rs = SR_OK ;
	            if (rpp != NULL)
	                *rpp = (uchar *) hep->h_addr_list[0] ;

	        }

	    } else {

	        ci = (curp->i >= 0) ? (curp->i + 1) : 0 ;
		if (hep->h_addr_list[ci] != NULL) {

	            rs = SR_OK ;
	            if (rpp != NULL)
	                *rpp = (uchar *) hep->h_addr_list[ci] ;

	        }

	        if (rs >= 0)
		    curp->i = ci ;

	    } /* end if */

	} /* end if */

	if ((rs < 0) && (rpp != NULL))
	    *rpp = NULL ;

	return (rs >= 0) ? alen : rs ;
}
/* end subroutine (hostent_enumaddr) */


int hostent_getcanonical(HOSTENT *hep,cchar **rpp)
{
	int		rs = SR_NOTFOUND ;
	int		nlen = 0 ;
	int		i ;

	if (hep == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;

	if ((hep->h_name != NULL) &&
	    (strchr(hep->h_name,'.') != NULL)) {

	    rs = SR_OK ;
	    nlen = strlen(hep->h_name) ;

	    *rpp = hep->h_name ;

	} /* end if */

	if ((rs == SR_NOTFOUND) && (hep->h_aliases != NULL)) {

	    for (i = 0 ; hep->h_aliases[i] != NULL ; i += 1) {

	        if (strchr(hep->h_aliases[i],'.') != NULL) {

		    rs = SR_OK ;
	    	    nlen = strlen(hep->h_aliases[i]) ;

	            *rpp = hep->h_aliases[i] ;
	            break ;

	        } /* end if */

	     } /* end for */

	} /* end if */

	if ((rs == SR_NOTFOUND) && (hep->h_name != NULL)) {

	    rs = SR_OK ;
	    nlen = strlen(hep->h_name) ;

	    *rpp = hep->h_name ;

	} /* end if */

	return (rs >= 0) ? nlen : rs ;
}
/* end subroutine (hostent_getcanonical) */


/* get a fully qualified domain name */
int hostent_getfqdn(HOSTENT *hep,cchar **rpp)
{

	return hostent_getcanonical(hep,rpp) ;
}
/* end subroutine (hostent_getfqdn) */


int hostent_size(HOSTENT *hep)
{
	int		i ;
	int		size = 1 ;
	if (hep->h_name != NULL) {
	    size += (strlen(hep->h_name)+1) ;
	}
	if (hep->h_aliases != NULL) {
	    for (i = 0 ; hep->h_aliases[i] != NULL ; i += 1) {
		size += (strlen(hep->h_aliases[i])+1) ;
	    }
	    size += (i*sizeof(const char *)) ;
	}
	if (hep->h_addr_list != NULL) {
	    for (i = 0 ; hep->h_addr_list[i] != NULL ; i += 1) {
		size += (hep->h_length+1) ;
	    }
	}
	size = iceil(size,sizeof(const char *)) ;
	return size ;
}
/* end subroutine (hostent_size) */


int hostent_load(HOSTENT *hep,char *hebuf,int helen,HOSTENT *lp)
{
	STOREITEM	ib ;
	int		rs ;

	memcpy(hep,lp,sizeof(HOSTENT)) ;

	if ((rs = storeitem_start(&ib,hebuf,helen)) >= 0) {
	    int	len ;

	    if (rs >= 0) rs = si_copyaliases(&ib,hep,lp) ;

	    if (rs >= 0) rs = si_copyaddrs(&ib,hep,lp) ;

	    if (rs >= 0) rs = si_copystr(&ib,&hep->h_name,lp->h_name) ;

	    len = storeitem_finish(&ib) ;
	    if (rs >= 0) rs = len ;
	} /* end if (storeitem) */

	return rs ;
}
/* end subroutine (hostent_load) */


/* private subroutines */


static int si_copyaliases(STOREITEM *ibp,struct hostent *hep,struct hostent *lp)
{
	int		rs = SR_OK ;

	if (lp->h_aliases != NULL) {
	    int		n ;
	    void	**vpp ;

	    for (n = 0 ; lp->h_aliases[n] != NULL ; n += 1) ;

#if	CF_DEBUGS
	debugprintf("uc_gethostby/si_copyaliases: n=%u\n",n) ;
#endif

	    if ((rs = storeitem_ptab(ibp,n,&vpp)) >= 0) {
	        int	i ;
	        char	**cpp ;

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

		    if (rs < 0) break ;
		} /* end for */
		hep->h_aliases[i] = NULL ;

	    } /* end if ( storeitem_ptab) */

	} else
	    hep->h_aliases = NULL ;

	return rs ;
}
/* end subroutine (si_copyaliases) */

static int si_copyaddrs(STOREITEM *ibp,struct hostent *hep,struct hostent *lp)
{
	int		rs = SR_OK ;

	if (lp->h_addr_list != NULL) {
	    int		n ;
	    void	**vpp ;

	    for (n = 0 ; lp->h_addr_list[n] != NULL ; n += 1) ;

	    if ((rs = storeitem_ptab(ibp,n,&vpp)) >= 0) {
	        int	i ;
	        char	**cpp ;

		cpp = (char **) vpp ;
	        hep->h_addr_list = cpp ;
	        for (i = 0 ; lp->h_addr_list[i] != NULL ; i += 1) {

		    rs = si_copybuf(ibp,(hep->h_addr_list + i),
				lp->h_addr_list[i],lp->h_length) ;

		    if (rs < 0) break ;
	        } /* end for */
		hep->h_addr_list[i] = NULL ;

	    } /* end if */

	} else
	    hep->h_addr_list = NULL ;

	return rs ;
}
/* end subroutine (si_copyaddrs) */

static int si_copystr(STOREITEM *ibp,char **pp,cchar *s1)
{
	int		rs = SR_OK ;
	const char	**cpp = (const char **) pp ;

	*cpp = NULL ;
	if (s1 != NULL) {
	    rs = storeitem_strw(ibp,s1,-1,cpp) ;
	}

	return rs ;
}
/* end subroutine (si_copystr) */

static int si_copybuf(STOREITEM *ibp,char **pp,cchar bp[],int bl)
{
	int		rs = SR_OK ;
	cchar		**cpp = (cchar **) pp ;

	*cpp = NULL ;
	if (bp != NULL) {
	    rs = storeitem_buf(ibp,bp,bl,cpp) ;
	}

	return rs ;
}
/* end subroutine (si_copybuf) */


