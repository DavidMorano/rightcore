/* clock */

/* the new(er) POSIX® clock services for UNIX® */


#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a version of |clock_gettime(3rt)| that is preloaded to over-ride
	the standard UNIX® system version.

	Note:

	This subroutine was from a set of subroutines that were originally
	branded under the name 'posix4' (if I remember).

	More notes:

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
#include	<dlfcn.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<vsystem.h>
#include	<localmisc.h>
#include	"pretime.h"


/* local defines */

#define	CLOCK		struct clock_head

#ifndef	CLOCK_REALTIME0
#define	CLOCK_REALTIME0	0		/* this is a special Solaris® thing */
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	NDF		"libpretime.nd"


/* typedefs */

typedef	int (*func_clock)(clockid_t,struct timespec *) ;


/* external subroutines */

extern int	cfdecui(const char *,int,uint *) ;
extern int	cfnumui(const char *,int,uint *) ;
extern int	cfhexui(const char *,int,uint *) ;
extern int	cfhexul(const char *,int,ulong *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* local structures */

struct clock_head {
	func_clock	func ;
} ;

#if	CF_DEBUGN
struct clockstr {
	int		cid ;
	const char	*name ;
} ;
#endif /* CF_DEBUGN */


/* forward references */

static int	clock_next(clockid_t,struct timespec *) ;

#if	CF_DEBUGN
static const char	*strclock(int) ;
#endif /* CF_DEBUGN */


/* local variables */

static CLOCK	clock_data ; /* zero-initialized */

#if	CF_DEBUGN
struct clockstr		strs[] = {
	{ CLOCK_REALTIME0, "realtime0" },	/* special to Solaris® */
	{ CLOCK_REALTIME, "realtime3" },
	{ -1, NULL }
} ;
#endif /* CF_DEBUGN */


/* exported subroutines */


int clock_gettime(clockid_t cid,struct timespec *tsp)
{
	int		rs ;
	int		rc = 0 ;
	if ((rs = pretime_init()) >= 0) {
#if	CF_DEBUGN
	    {
	        int	sn = pretime_serial() ;
	        nprintf(NDF,"libpretime/clock_gettime: ent sn=%u\n",sn) ;
	        nprintf(NDF,"libpretime/clock_gettime: cid=%s(%u)\n",
			strclock(cid),cid) ;
	    }
#endif
	    switch (cid) {
	    case CLOCK_REALTIME0:
	    case CLOCK_REALTIME:
		{
	            int		rs ;
	            if ((rs = pretime_modts(tsp)) < 0) {
		        errno = (-rs) ;
		        rc = -1 ;
	            }
		}
		break ;
	    default:
	        rc = clock_next(cid,tsp) ;
		break ;
	    } /* end switch (clock-id) */
	} else {
	    errno = (-rs) ;
	    rc = -1 ;
	}
#if	CF_DEBUGN
	{
	    char	tbuf[TIMEBUFLEN+1] ;
	    timestr_logz(tsp->tv_sec,tbuf) ;
	    nprintf(NDF,"libpretime/clock_gettime: time=%s\n",tbuf) ;
	    nprintf(NDF,"libpretime/clock_gettime: ret rc=%d\n",rc) ;
	}
#endif
	return rc ;
}
/* end subroutine (clock_gettime) */


/* local subroutines */


static int clock_next(clockid_t cid,struct timespec *tsp)
{
	CLOCK		*sip = &clock_data ;
	int		rc = -1 ;

	if (sip->func == NULL) {
	    void	*sp = dlsym(RTLD_NEXT,"clock_gettime") ;
	    sip->func = (func_clock) sp ;
	}

	if (sip->func != NULL) {
	    func_clock	func = (func_clock) sip->func ;
	    rc = (*func)(cid,tsp) ;
	} else {
	    errno = (- SR_LIBACC) ;
	}

	return rc ;
}
/* end subroutine (clock_next) */


#if	CF_DEBUGN
static const char	*strclock(int cid) {
	int	i ;
	int	f = FALSE ;
	cchar	*rp = "unknown" ;
	for (i = 0 ; strs[i].cid >= 0 ; i += 1) {
	    f = (cid == strs[i].cid) ;
	    if (f) break ;
	}
	if (f) rp = strs[i].name ;
	return rp ;
}
#endif /* CF_DEBUGN */

