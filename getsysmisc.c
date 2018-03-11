/* getsysmisc */

/* get the SYMISC information from the kernel */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-12-18, David A­D­ Morano
        This little subroutine was put together so that for those situations
        where only the number of CPUs is desired (not often the case, but
        sometimes) we do not have to go through the process (hassle) of using
        the KINFO object directly (oh like that is a huge problem).

	= 2010-12-09, David A­D­ Morano
        I enhanced this subroutine to get the number of CPUs without using the
        KINFO object. That KINFO object (as it is and has been) is NOT
        reentrant. This is no fault of my own (since I wrote that KINFO code
        also) but rather from Sun-Solaris. The KINFO object uses the underlying
        Solaris KSTAT facility -- which is not reentrant (and therefore not
        thread-safe). I needed a thread-safe way of getting the number of CPUs
        so I had to add some sort of mechanism to do that. We have (basically)
        cheap and cheaper ways to do it. I tried regular 'cheap' and got tired,
        so I switched to 'cheaper'. The 'cheaper' version is the shared-memory
        thing I added below. The regular 'cheap' one was to query the MSINFO or
        MSU facility. The latter is left unfinished due to time constraints.
        Also, it (naturally) took longer than desired to even to the 'cheaper'
        solution.

*/

/* Copyright © 1998,2010 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine returns the number of CPUs available from the current
        node. We do note that the number of CPUs can change dynamically as some
        may be added or removed during the course of live machine operation. We
        allow the number of CPUs returned to the caller to be zero (0) even
        though it is not clear how this might happen. This sort of assumes that
        the caller understands (believes) that at least one CPU is available at
        any time -- otherwise how would we be able to execute in the first
        place!

	Notes:

	= Load-averages

        Although load-averages are available when retrieving SYSMISC
        (miscellaneous system) information from the kernel, we don't bother with
        it at all since the general introduction of the 'getloadavg(3c)'
        subroutine in the workd. If that subroutine was not available,
        load-averages would have to be treated as being as difficult to retrieve
        as the number of CPUs is.

	= NOT thread-safe!

        This subroutine is not thread-safe! This is because the KINFO object is
        not thread-safe.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<dlfcn.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<kinfo.h>
#include	<localmisc.h>

#include	"getsysmisc.h"


/* local defines */

#ifndef	ENDIANSTR
#ifdef	ENDIAN
#if	(ENDIAN == 0)
#define	ENDIANSTR	"0"
#else
#define	ENDIANSTR	"1"
#endif
#else
#define	ENDIANSTR	"1"
#endif
#endif

#ifndef	VARLIBPATH
#define	VARLIBPATH	"LD_LIBRARY_PATH"
#endif

#ifndef	VARPR
#define	VARPR		"LOCAL"
#endif

#ifndef	VARNCPU
#define	VARNCPU		"NCPU"
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,const char *,
			const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,const char *,const char *,const char *,
			const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


/* this subroutine is NOT thread-safe (KINFO object is the problem) */
int getsysmisc(gsp,daytime)
GETSYSMISC	*gsp ;
time_t		daytime ;
{
	KINFO		ki ;
	KINFO_DATA	kd ;
	int		rs ;
	int		i ;
	int		n = 0 ;

	if (daytime == 0) daytime = time(NULL) ;

	memset(gsp,0,sizeof(GETSYSMISC)) ;

	if ((rs = kinfo_open(&ki,daytime)) >= 0) {

	    if ((rs = kinfo_sysmisc(&ki,daytime,&kd)) >= 0) {
		gsp->btime = kd.boottime ;
		gsp->ncpu = kd.ncpu ;
		gsp->nproc = kd.nproc ;
		for (i = 0 ; i < 3 ; i += 1)
		    gsp->la[i] = kd.la[i] ;
		n = gsp->ncpu ;
	    }

	    kinfo_close(&ki) ;
	} /* end if (opened KINFO) */

#if	CF_DEBUGS
	debugprintf("getsysmisc: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (getsysmisc) */


