/* uc_joinus */

/* join a UNIX®-Socket-Datagram */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1999-07-21, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine joins a UNIX® socket of the datagram variety.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<sockaddress.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	mkpath1(char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_joinus(int fd,SOCKADDR *sap,int sal,int of,mode_t om)
{
	int		rs = SR_OK ;
	const int	f_create = MKBOOL(of&O_CREAT) ;
#if	CF_DEBUGS
	debugprintf("sesnotes/uc_joinus: ent f_creat=%u\n",f_create) ;
#endif
	if (fd < 0) return SR_INVALID ;
	if (f_create) {
	    if ((rs = u_bind(fd,sap,sal)) >= 0) {
		if (of & O_MINMODE) {
		    SOCKADDRESS	*sa = (SOCKADDRESS *) sap ;
		    const int	af = AF_UNIX ;
		    if ((rs = sockaddress_getaf(sa)) == af) {
		        const int	slen = MAXPATHLEN ;
		        char		sbuf[MAXPATHLEN+1] ;
		        if ((rs = sockaddress_getaddr(sa,sbuf,slen)) >= 0) {
		            rs = u_chmod(sbuf,om) ;
		            if (rs < 0)
			        uc_unlink(sbuf) ;
		        } /* end if (sockaddress_getaddr) */
		    } /* end if (UNIX® address family) */
		} /* end if (mon-mod) */
	    } /* end if (u_bind) */
	} else {
	    rs = u_connect(fd,sap,sal) ;
	}
#if	CF_DEBUGS
	debugprintf("sesnotes/uc_joinus: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (uc_joinus) */


