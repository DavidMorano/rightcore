/* uc_getproto */

/* interface component for UNIX® library-3c */
/* subroutine to get a protocol entry (raw) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Name:

	uc_getprotobyname

	Description:

	This subroutine is a platform independent subroutine to get an
	INET 'protocol' (by name).

	Synopsis:

	int uc_getprotobyname(proto,pep,rbuf,rlen)
	const char	proto[] ;
	struct hostent	*pep ;
	char		rbuf[] ;
	int		rlen ;

	Arguments:

	- proto		protocol name to lookup
	- pep		pointer to 'hostent' structure
	- rbuf		user supplied buffer to hold result
	- rlen		length of user supplied buffer

	Returns:

	0		host was found OK
	SR_FAULT	address fault
	SR_TIMEDOUT	request timed out (bad network someplace)
	SR_NOTFOUND	host could not be found
	SR_OVERFLOW	buffer overflow

	Name:

	uc_getprotobynumber

	Description:

	This subroutine is a platform independent subroutine to get an
	INET 'protocol' (by number).

	Synopsus:

	int uc_getprotobynumber(name,proto,pep,rbuf,rlen)
	const char	name[] ;
	const char	proto[] ;
	struct hostent	*pep ;
	char		rbuf[] ;
	int		rlen ;

	Arguments:

	- name		name to lookup
	- proto		service to lookup
	- pep		pointer to 'hostent' structure
	- rbuf		user supplied buffer to hold result
	- rlen		length of user supplied buffer

	Returns:

	0		host was found OK
	SR_FAULT	address fault
	SR_TIMEDOUT	request timed out (bad network someplace)
	SR_NOTFOUND	host could not be found
	SR_OVERFLOW	buffer overflow


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

#define	RF_PROTOCOPY	0		/* flag to set need of 'protocopy()' */
#if	defined(SYSHAS_GETPROTOXXXR) && (SYSHAS_GETPROTOXXXR > 0)
#else
#undef	RF_PROTOCOPY
#define	RF_PROTOCOPY	1		/* flag to set need of 'protocopy()' */
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

static int	protosize(struct protoent *) ;

#if	RF_PROTOCOPY
static int	protocopy(struct protoent *,char *,int,struct protoent *) ;
static int	si_copystr(STOREITEM *,char **,const char *) ;
#endif


/* forward references */


/* local variables */


/* exported subroutines */


int uc_setprotoent(int stayopen)
{
	setprotoent(stayopen) ;
	return SR_OK ;
}

int uc_endprotoent()
{
	endprotoent() ;
	return SR_OK ;
}

int uc_getprotoent(struct protoent *pep,char *rbuf,int rlen)
{
	struct protoent	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	if (pep == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (rlen <= 0) return SR_OVERFLOW ;

/* note (carefully) that there is NO POSIX® standard version of this funtion */

	repeat {

#if	defined(SYSHAS_GETPROTOXXXR) && (SYSHAS_GETPROTOXXXR > 0)
	{
	    errno = 0 ;
	    rp = getprotoent_r(pep,rbuf,rlen) ;
	    if (rp != NULL) {
	        rs = protosize(pep) ;
	    } else {
	        rs = (- errno) ;
	    }
	}
#else /* defined(SYSHAS_GETPROTOXXXR) && (SYSHAS_GETPROTOXXXR > 0) */
	{
	    errno = 0 ;
	    rp = getprotoent() ;
	    if (rp != NULL) {
	        rs = protocopy(pep,rbuf,rlen,rp) ;
	    } else {
	        rs = (- errno) ;
	    }
	}
#endif /* defined(SYSHAS_GETPROTOXXXR) && (SYSHAS_GETPROTOXXXR > 0) */

	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
		    if (to_again-- > 0) {
	                msleep(1000) ;
	 	    } else {
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
/* end subroutine (uc_getprotoent) */


int uc_getprotobyname(cchar *name,struct protoent *pep,char *rbuf,int rlen)
{
	struct protoent	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_getprotobyname: name=%s proto=%s\n",
	    name,proto) ;
#endif

	if (pep == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	if (rlen <= 0) return SR_OVERFLOW ;

	repeat {

#if	defined(SYSHAS_GETPROTOXXXR) && (SYSHAS_GETPROTOXXXR > 0)
	{
	    errno = 0 ;
	    rp = getprotobyname_r(name,pep,rbuf,rlen) ;
	    if (rp != NULL) {
	        rs = protosize(pep) ;
	    } else {
	        rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	    }
	}
#else /* defined(SYSHAS_GETPROTOXXXR) && (SYSHAS_GETPROTOXXXR > 0) */
	{
	    errno = 0 ;
	    rp = getprotobyname(name,proto) ;
	    if (rp != NULL) {
	        rs = protocopy(pep,rbuf,rlen,rp) ;
	    } else {
	        rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	    }
	}
#endif /* defined(SYSHAS_GETPROTOXXXR) && (SYSHAS_GETPROTOXXXR > 0) */

	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
		    if (to_again-- > 0) {
	                msleep(1000) ;
	 	    } else {
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

#if	CF_DEBUGS
	debugprintf("uc_getprotobyname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_getprotobyname) */


int uc_getprotobynumber(int proto,struct protoent *pep,char *rbuf,int rlen)
{
	struct protoent	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_getprotobynumber: proto=%u\n",proto) ;
#endif

	if (pep == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (rlen <= 0) return SR_OVERFLOW ;

	repeat {

#if	defined(SYSHAS_GETPROTOXXXR) && (SYSHAS_GETPROTOXXXR > 0)
	{
	    errno = 0 ;
	    rp = getprotobynumber_r(proto,pep,rbuf,rlen) ;
	    if (rp != NULL) {
	        rs = protosize(pep) ;
	    } else {
	        rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	    }
	}
#else /* defined(SYSHAS_GETPROTOXXXR) && (SYSHAS_GETPROTOXXXR > 0) */
	{
	    errno = 0 ;
	    rp = getprotobynumber(proto) ;
	    if (rp != NULL) {
	        rs = protocopy(pep,rbuf,rlen,rp) ;
	    } else {
	        rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	    }
	}
#endif /* defined(SYSHAS_GETPROTOXXXR) && (SYSHAS_GETPROTOXXXR > 0) */

	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
		    if (to_again-- > 0) {
	                msleep(1000) ;
	 	    } else {
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

#if	CF_DEBUGS
	debugprintf("uc_getprotobynumber: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_getprotobynumber) */


/* local subroutines */


static int protosize(struct protoent *pep)
{
	int		size = 0 ;
	if (pep->p_name != NULL) {
	    size += (strlen(pep->p_name)+1) ;
	}
	if (pep->p_aliases != NULL) {
	    int		i ;
	    for (i = 0 ; pep->p_aliases[i] != NULL ; i += 1) {
	        size += (strlen(pep->p_aliases[i])+1) ;
	    } /* end for */
	    size += (i*sizeof(const char *)) ;
	} /* end if (group members) */
	size = iceil(size,sizeof(const char *)) ;
	return size ;
}
/* end subroutine (protosize) */

#if	RF_PROTOCOPY

static int protocopy(struct protoent *pep,char *rbuf,int rlen,
		struct protoent *rp)
{
	STOREITEM	ib ;
	int		rs ;
	int		rs1 ;

	memcpy(pep,rp,sizeof(struct protoent)) ;

	if ((rs = storeitem_start(&ib,rbuf,rlen)) >= 0) {

	    if (rp->p_aliases != NULL) {
	        int	n ;
	        void	**tab ;

	        for (n = 0 ; rp->p_aliases[n] != NULL ; n += 1) ;

	        if ((rs = storeitem_ptab(&ib,n,&tab)) >= 0) {
		    const char	**aliases = rp->p_aliases ;
		    int		i ;

	            pep->p_aliases = (char **) tab ;
	            for (i = 0 ; rp->p_aliases[i] != NULL ; i += 1) {
	                rs = si_copystr(&ib,(pep->s_aliases + i),aliases[i]) ;
	                if (rs < 0) break ;
	            } /* end for */
	            pep->p_aliases[i] = NULL ;

	        } /* end if (storeitem_ptab) */

	    } /* end if (aliases) */

	    si_copystr(&ib,&pep->p_name,rp->p_name) ;

	    rs1 = storeitem_finish(&ib) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

	return rs ;
}
/* end subroutine (protocopy) */

static int si_copystr(STOREITEM *ibp,cchar *p1,char **pp)
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

#endif /* RF_PROTOCOPY */


