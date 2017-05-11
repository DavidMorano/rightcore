/* localgetnetload */

/* get the LOCAL network-load (NETLOAD) */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_UPROGDATA	1		/* use |uprogdata_xxx(3uc)| */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine retrieves the LOCAL system network-load.

	Synopsis:

	int localgetnetload(pr,rbuf,rlen)
	const char	pr[] ;
	char		rbuf[] ;
	char		rlen ;

	Arguments:

	pr		program root
	rbuf		caller supplied buffer to place result in
	rlen		length of caller supplied buffer

	Returns:

	>=0		length of returned value
	<0		error


	Notes:

	Q. Why the program-cache?
	A. Because this subroutine, and a couple others like it, get called
	   everytime certain pseudo-"files" are read out.  We want some of
	   those files to read out very quickly, so caching away an extra
	   real-file read in this routine (and others like it) really speeds
	   things up.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<uprogdata.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARNETLOAD
#define	VARNETLOAD	"NETLOAD"
#endif

#define	ETCDNAME	"etc"
#define	VARDNAME	"var"
#define	NETLOADFNAME	"netload"
#define	TO_TTL		(5*60)


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	getnodedomain(char *,char *) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	localgetorg(const char *,char *,int,const char *) ;
extern int	readfileline(char *,int,const char *) ;
extern int	isNotPresent(int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int localgetnetload(cchar *pr,char *rbuf,int rlen)
{
	const int	di = UPROGDATA_DNETLOAD ;
	const int	ttl = TO_TTL ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (pr == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	rbuf[0] = '\0' ;

/* user environment */

	if ((rs >= 0) && (len == 0)) {
	    cchar	*netload = getenv(VARNETLOAD) ;
	    if ((netload != NULL) && (netload[0] != '\0')) {
	        rs = sncpy1(rbuf,rlen,netload) ;
	        len = rs ;
	    }
	} /* end if (needed) */

#if	CF_DEBUGS
	debugprintf("localgetnetload: 0 rs=%d org=>%s<\n",rs,rbuf) ;
#endif

/* program cache */

#if	CF_UPROGDATA
	if ((rs >= 0) && (len == 0)) {
	    if ((rs = uprogdata_get(di,rbuf,rlen)) > 0) {
	        len = rs ;
	    }
	}
#endif /* CF_UPROGDATA */

/* software facility (LOCAL) configuration */

	if ((rs >= 0) && (len == 0)) {
	    cchar	*vardname = VARDNAME ;
	    cchar	*netloadname = NETLOADFNAME ;
	    char	tfname[MAXPATHLEN+1] ;
	    if ((rs = mkpath3(tfname,pr,vardname,netloadname)) >= 0) {
	        if ((rs = readfileline(rbuf,rlen,tfname)) > 0) {
	            len = rs ;
#if	CF_UPROGDATA
		    rs = uprogdata_set(di,rbuf,len,ttl) ;
#endif /* CF_UPROGDATA */
		} else if (isNotPresent(rs)) {
		    rs = SR_OK ;
		}
	    }
	} /* end if (needed) */

#if	CF_DEBUGS
	debugprintf("localgetnetload: ret rs=%d org=>%s<\n",rs,rbuf) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (localgetnetload) */


