/* localgetorgloc */

/* get the LOCAL organization location (ORGLOC) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-05-01, David A­D­ Morano
        This subroutine is originally written. This is a minimal implementation,
        but at least there is something here. ORGLOCs are not currently a part
        of the system-user-database system ('passwd', 'user_attr', et cetera) so
        currently this subroutine does not look anywhere in those places for an
        ORGLOC. I guess that an ORGLOC should be added to the 'user_attr'
        database but this practice has not yet begun, if it ever will be.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used to (try to) get the LOCAL software distribution
        organization location (ORGLOC). This particular value is not often set
        in the LOCAL software distribution and also this subroutine does not
        look hard in other places to find a possible location. So an ORGLOC is
        not often returned. But nonetheless there are places (pieces of code,
        middleware et cetera) that still call this subroutine rather regularly.

	Synopsis:

	int localgetorgloc(pr,rbuf,rlen,un)
	const char	pr[] ;
	char		rbuf[] ;
	char		rlen ;
	const char	*un ;

	Arguments:

	pr		program root
	rbuf		caller supplied buffer to place result in
	rlen		length of caller supplied buffer
	un		username

	Returns:

	>=0		length of returned ID
	<0		error in process of creating ID


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARORGLOC
#define	VARORGLOC	"ORGLOC"
#endif

#define	ETCCNAME	"etc"
#define	ORGLOCFNAME	"orgloc"


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


int localgetorgloc(pr,rbuf,rlen,un)
const char	pr[] ;
char		rbuf[] ;
int		rlen ;
const char	*un ;
{
	int		rs = SR_OK ;
	int		len = 0 ;
	const char	*etcdname = ETCCNAME ;
	const char	*orglocname = ORGLOCFNAME ;
	const char	*orgloc = getenv(VARORGLOC) ;
	char		tfname[MAXPATHLEN+1] ;

	if (pr == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	rbuf[0] = '\0' ;

/* user environment */

	if ((len <= 0) && ((rs >= 0) || isNotPresent(rs))) {
	    if ((orgloc != NULL) && (orgloc[0] != '\0')) {
	        rs = sncpy1(rbuf,rlen,orgloc) ;
	        len = rs ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("localgetorgloc: 0 rs=%d org=>%s<\n",rs,rbuf) ;
#endif

/* user configuration */

	if ((len <= 0) && ((rs >= 0) || isNotPresent(rs))) {
	    const int	hlen = MAXPATHLEN ;
	    char	hbuf[MAXPATHLEN+1] ;
	    if ((un == NULL) || (un[0] == '\0')) un = "-" ;
	    if ((rs = getuserhome(hbuf,hlen,un)) >= 0) {
	        if ((rs = mkpath3(tfname,hbuf,etcdname,orglocname)) >= 0) {
	            rs = readfileline(rbuf,rlen,tfname) ;
	            len = rs ;
	        }
	    }
	}

#if	CF_DEBUGS
	debugprintf("localgetorgloc: 1 rs=%d org=>%s<\n",rs,rbuf) ;
#endif

/* software facility (LOCAL) configuration */

	if ((len <= 0) && ((rs >= 0) || isNotPresent(rs))) {
	    if ((rs = mkpath3(tfname,pr,etcdname,orglocname)) >= 0) {
	        rs = readfileline(rbuf,rlen,tfname) ;
	        len = rs ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("localgetorgloc: 2 rs=%d org=>%s<\n",rs,rbuf) ;
#endif

/* any operating system configuration (in '/etc') */

	if ((len <= 0) && ((rs >= 0) || isNotPresent(rs))) {
	    if ((rs = mkpath3(tfname,"/",etcdname,orglocname)) >= 0) {
	        rs = readfileline(rbuf,rlen,tfname) ;
	        len = rs ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("localgetorgloc: 3 rs=%d org=>%s<\n",rs,rbuf) ;
#endif

/* get out (nicely as possible in our case) */

	if ((rs < 0) && isNotPresent(rs)) {
	    rs = SR_OK ;
	    len = 0 ;
	}

#if	CF_DEBUGS
	debugprintf("localgetorgloc: ret rs=%d org=>%s<\n",rs,rbuf) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (localgetorgloc) */


