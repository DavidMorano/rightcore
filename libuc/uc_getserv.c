/* uc_getserv */

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
	struct srvent	*sep ;
	char		rbuf[] ;
	int		rlen ;

	Arguments:

	- name		name to lookup
	- proto		service to lookup
	- sep		pointer to 'srvent' structure
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

#define	RF_SERVCOPY	0		/* flag to set need of 'servcopy()' */
#if	defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0)
#else
#undef	RF_SERVCOPY
#define	RF_SERVCOPY	1		/* flag to set need of 'servcopy()' */
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

static int	servsize(struct servent *) ;

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

int uc_getservent(struct servent *sep,char *rbuf,int rlen)
{
	struct servent	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	if (sep == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

/* note (carefully) that there is NO POSIX® standard version of this funtion */

	repeat {

#if	defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0)
	{
	    errno = 0 ;
	    rp = getservent_r(sep,rbuf,rlen) ;
	    if (rp != NULL) {
	        rs = servsize(sep) ;
	    } else {
	        rs = (- errno) ;
	    }
	}
#else /* defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0) */
	{
	    errno = 0 ;
	    rp = getservent() ;
	    if (rp != NULL) {
	        rs = servcopy(sep,rbuf,rlen,rp) ;
	    } else {
	        rs = (- errno) ;
	    }
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

	return rs ;
}
/* end subroutine (uc_getservent) */


int uc_getservbyname(cchar *name,cchar *proto,
		struct servent *sep,char *rbuf,int rlen)
{
	struct servent	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
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
	{
	    errno = 0 ;
	    rp = getservbyname_r(name,proto,sep,rbuf,rlen) ;
	    if (rp != NULL) {
	        rs = servsize(sep) ;
	    } else {
	        rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	    }
	}
#else /* defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0) */
	{
	    errno = 0 ;
	    rp = getservbyname(name,proto) ;
	    if (rp != NULL) {
	        rs = servcopy(sep,rbuf,rlen,rp) ;
	    } else {
	        rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	    }
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

	return rs ;
}
/* end subroutine (uc_getservbyname) */


int uc_getservbyport(int port,cchar *proto,struct servent *sep,
		char *rbuf,int rlen)
{
	struct servent	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_getservbyname: name=%s proto=%s\n",
	    name,proto) ;
#endif

	if (sep == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	repeat {

#if	defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0)
	{
	    errno = 0 ;
	    rp = getservbyport_r(port,proto,sep,rbuf,rlen) ;
	    if (rp != NULL) {
	        rs = servsize(sep) ;
	    } else {
	        rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	    }
	}
#else /* defined(SYSHAS_GETSERVXXXR) && (SYSHAS_GETSERVXXXR > 0) */
	{
	    errno = 0 ;
	    rp = getservbyport(port,proto) ;
	    if (rp != NULL) {
	        rs = servcopy(sep,rbuf,rlen,rp) ;
	    } else {
	        rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	    }
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

	return rs ;
}
/* end subroutine (uc_getservbyport) */


/* local subroutines */


static int servsize(struct servent *sep)
{
	int		size = 0 ;
	if (sep->s_name != NULL) {
	    size += (strlen(sep->s_name)+1) ;
	}
	if (sep->s_proto != NULL) {
	    size += (strlen(sep->s_proto)+1) ;
	}
	if (sep->s_aliases != NULL) {
	    int		i ;
	    for (i = 0 ; sep->s_aliases[i] != NULL ; i += 1) {
	        size += (strlen(sep->s_aliases[i])+1) ;
	    } /* end for */
	    size += (i*sizeof(const char *)) ;
	} /* end if (aliases) */
	size = iceil(size,sizeof(const char *)) ;
	return size ;
}
/* end subroutine (servsize) */

#if	RF_SERVCOPY

static int servcopy(struct servent *sep,char *rbuf,int rlen,struct servent *rp)
{
	STOREITEM	ib ;
	int		rs ;
	int		rs1 ;

	memcpy(sep,rp,sizeof(struct servent)) ;

	if ((rs = storeitem_start(&ib,rbuf,rlen)) >= 0) {

	    if (rp->s_aliases != NULL) {
	        int	n ;
	        void	**tab ;

	        for (n = 0 ; rp->s_aliases[n] != NULL ; n += 1) ;

	        if ((rs = storeitem_ptab(&ib,n,&tab)) >= 0) {
		    int		i ;
		    cchar	*aliases = rp->s_aliases ;

	            sep->s_aliases = (char **) tab ;
	            for (i = 0 ; rp->s_aliases[i] != NULL ; i += 1) {
	                rs = si_copystr(&ib,(sep->s_aliases + i),aliases[i]) ;
	                if (rs < 0) break ;
	            } /* end for */
	            sep->s_aliases[i] = NULL ;

	        } /* end if (storeitem_ptab) */

	    } /* end if (aliases) */

	    si_copystr(&ib,&sep->s_name,rp->s_name) ;

	    si_copystr(&ib,&sep->s_proto,rp->s_proto) ;

	    rs1 = storeitem_finish(&ib) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (storeitem) */

	return rs ;
}
/* end subroutine (servcopy) */

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

#endif /* RF_SERVCOPY */


