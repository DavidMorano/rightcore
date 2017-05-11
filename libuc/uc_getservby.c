/* uc_getservby */

/* interface component for UNIX® library-3c */
/* subroutine to get a "service" entry (raw) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is a platform independent subroutine to get an INET
        'service' entry (by name).

	Synopsis:

	int uc_getservbyname(name,proto,sep,rbuf,rlen)
	const char	name[] ;
	const char	proto[] ;
	struct hostent	*sep ;
	char		rbuf[] ;
	int		rlen ;

	Arguments:

	- name		name to lookup
	- proto		service to lookup
	- sep		pointer to 'hostent' structure
	- rbuf		user supplied buffer to hold result
	- rlen		length of user supplied buffer

	Returns:

	0		host was found OK
	SR_FAULT	address fault
	SR_TIMEDOUT	request timed out (bad network someplace)
	SR_NOTFOUND	host could not be found


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

#define	TO_AGAIN	10

#define	RF_SERVCOPY	0		/* flag to set need of 'pwcopy()' */
#if	defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0)
#else
#undef	RF_SERVCOPY
#define	RF_SERVCOPY	1		/* flag to set need of 'pwcopy()' */
#endif


/* external subroutines */

extern int	msleep(int) ;


/* external variables */


/* local structures */

static int	servsize(struct servent *,const char *) ;

#if	RF_SERVCOPY
static int	servcopy(struct servent *,char *,int,struct servent *) ;
static int	si_copystr(STOREITEM *,char **,const char *) ;
#endif


/* forward references */


/* exported subroutines */


int uc_setservent(int stayopen)
{
	setservent(stayopen) ;
	return SR_OK ;
}

int uc_endservent()
{
	endservent() ;
	return SR_OK ;
}

int uc_getservent(sep,rbuf,rlen)
struct servent	*sep ;
char		rbuf[] ;
int		rlen ;
{
	struct servent	*rp ;
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		rc = 0 ;
	int		f_exit = FALSE ;

	if (sep == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

/* note (carefully) that there is NO POSIX standard version of this funtion */

	repeat {

#if	defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0)

	rs = SR_OK ;
	errno = 0 ;
	rp = getservent_r(sep,rbuf,rlen) ;
	if (rp == NULL) {
	    rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	}
	if (rs >= 0) {
	    rc = servsize(sep,rbuf) ;
	}
#else /* defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0) */

	rs = SR_OK ;
	errno = 0 ;
	rp = getservent() ;
	if (rp == NULL) {
	    rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	}
	if (rs >= 0) {
	    rc = servcopy(sep,rbuf,rlen,rp) ;
	}
#endif /* defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0) */

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

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (uc_getservent) */


int uc_getservbyname(name,proto,sep,rbuf,rlen)
const char	name[] ;
const char	proto[] ;
struct servent	*sep ;
char		rbuf[] ;
int		rlen ;
{
	struct servent	*rp ;
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		rc = 0 ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_getservbyname: name=%s proto=%s\n",
	    name,proto) ;
#endif

	if (sep == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	repeat {

#if	defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0)

	rs = SR_OK ;
	errno = 0 ;
	rp = getservbyname_r(name,proto,sep,rbuf,rlen) ;
	if (rp == NULL) {
	    rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	}
	if (rs >= 0) {
	    rc = servsize(sep,rbuf) ;
	}
#else /* defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0) */

	rs = SR_OK ;
	errno = 0 ;
	rp = getservbyname(name,proto) ;
	if (rp == NULL) {
	    rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	}
	if (rs >= 0) {
	    rc = servcopy(sep,rbuf,rlen,rp) ;
	}
#endif /* defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0) */

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
	debugprintf("uc_getservbyname: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (uc_getservbyname) */


int uc_getservbyport(port,proto,sep,rbuf,rlen)
int		port ;
const char	proto[] ;
struct servent	*sep ;
char		rbuf[] ;
int		rlen ;
{
	struct servent	*rp ;
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		rc = 0 ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_getservbyname: name=%s proto=%s\n",
	    name,proto) ;
#endif

	if (sep == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	repeat {

#if	defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0)

	rs = SR_OK ;
	errno = 0 ;
	rp = getservbyport_r(port,proto,sep,rbuf,rlen) ;
	if (rp == NULL) {
	    rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	}
	if (rs >= 0) {
	    rc = servsize(sep,rbuf) ;
	}
#else /* defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0) */

	rs = SR_OK ;
	errno = 0 ;
	rp = getservbyport(port,proto) ;
	if (rp == NULL) {
	    rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	}
	if (rs >= 0) {
	    rc = servcopy(sep,rbuf,rlen,rp) ;
	}
#endif /* defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0) */

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
	debugprintf("uc_getservbyname: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (uc_getservbyport) */


/* local subroutines */


static int servsize(sep,rbuf)
struct servent	*sep ;
const char	rbuf[] ;
{
	int		rs = 0 ;
	int		i ;
	const char	*ol = rbuf ;
	const char	*p ;

	p = sep->s_name ;
	if ((p != NULL) && (p > ol))
	    ol = p ;

	p = sep->s_proto ;
	if ((p != NULL) && (p > ol))
	    ol = p ;

	if (sep->s_aliases != NULL) {
	    for (i = 0 ; sep->s_aliases[i] != NULL ; i += 1) {
	        p = sep->s_aliases[i] ;
	        if ((p != NULL) && (p > ol)) {
	            ol = p ;
	        }
	    } /* end for */
	} /* end if (aliases) */

	if (ol > rbuf) {
	    rs = ol + strlen(ol) + 1 - rbuf ;
	}

	return rs ;
}
/* end subroutine (servsize) */

#if	RF_SERVCOPY

static int servcopy(sep,rbuf,rlen,rp)
struct servent	*sep, *rp ;
char		rbuf[] ;
int		rlen ;
{
	STOREITEM	ib ;
	int		rs ;
	int		rs1 ;

	if ((rs = storeitem_start(&ib,rbuf,rlen)) >= 0) {

	memcpy(sep,rp,sizeof(struct servent)) ;

	si_copystr(&ib,&sep->s_name,rp->s_name) ;

	si_copystr(&ib,&sep->s_proto,rp->s_proto) ;

	if (rp->s_aliases != NULL) {
	    int		n, i ;
	    void	**tab ;

	    for (n = 0 ; rp->s_aliases[n] != NULL ; n += 1) ;

	    if ((rs = storeitem_ptab(&ib,n,&tab)) >= 0) {

	        sep->s_aliases = (char **) tab ;
	        for (i = 0 ; rp->s_aliases[i] != NULL ; i += 1) {
	            rs = si_copystr(&ib,(sep->s_aliases + i),rp->s_aliases[i]) ;
	            if (rs < 0) break ;
		}

	    } /* end if */

	    if (rs >= 0)
	        sep->s_aliases[i] = NULL ;

	} else {
	    sep->s_aliases = NULL ;
	}

	rs1 = storeitem_finish(&ib) ;
	if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

	return rs ;
}
/* end subroutine (servcopy) */

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

#endif /* RF_SERVCOPY */


