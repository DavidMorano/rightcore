/* gethostid */

/* Get-Host-ID UNIX® System interposer */


#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a version of |gethostid(3c)| that is preloaded to over-ride
	the standard UNIX® system version.

	Q. Is this multi-thread safe?
	A. Since it is a knock-off of an existing UNIX® system LIBC (3c)
	   subroutine that is already multi-thread safe -- then of course
	   it is!

	Q. Is this much slower than the default system version?
	A. No, not really.

	Q. How are we smarter than the default system version?
	A. Let me count the ways:
		+ value optionally from environment
		+ value optionally from a configuration file
		+ customizable built-in compiled default
		+ value is cached!

	Q. Why are you so smart?
	A. I do not know.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/systeminfo.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARHOSTID
#define	VARHOSTID	"HOSTID"
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int64_t in decimal */
#endif

#define	GETHOSTID	struct gethostid_head

#define	NDF		"gethostid.deb"


/* external subroutines */

extern int	cfdecui(const char *,int,uint *) ;
extern int	cfnumui(const char *,int,uint *) ;
extern int	cfhexui(const char *,int,uint *) ;
extern int	cfhexul(const char *,int,ulong *) ;
extern int	ctdecui(char *,int,uint) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* local structures */

struct gethostid_head {
	long		hostid ;
} ;


/* forward references */


/* local variables */

static GETHOSTID	gethostid_data ; /* zero-initialized */


/* exported subroutines */


long gethostid(void)
{
	GETHOSTID	*gip = &gethostid_data ;
	long		rc = 0 ;
	int		rs = SR_OK ;
	if (gip->hostid == 0) {
	    const int	dlen = DIGBUFLEN ;
	    int		vl = -1 ;
	    const char	*var = VARHOSTID ;
	    const char	*vp ;
	    char	dbuf[DIGBUFLEN+1] ;
	    if (((vp = getenv(var)) == NULL) || (vp[0] == '\0')) {
	        if ((rs = u_sysinfo(SI_HW_SERIAL,dbuf,dlen)) >= 0) {
	            vp = dbuf ;
	            vl = rs ;
	        } /* end if (u_sysinfo) */
	    } /* end if (getenv) */
	    if (rs >= 0) {
	        uint	uv ;
	        if ((rs = cfnumui(vp,vl,&uv)) >= 0) {
		    gip->hostid = (long) uv ;
		    rc = (long) uv ;
	        }
	    } /* end if (ok) */
	    if (rs < 0) errno = (- rs) ;
	} else {
	    rc = gip->hostid ;
	}
	return rc ;
}
/* end subroutine (gethostid) */


