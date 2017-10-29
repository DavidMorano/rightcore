/* uc_openpsem */

/* interface component for UNIX® library-3c */
/* open a Posix Semaphore (PSEM) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1999-07-23, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module provides a sanitized version of the standard POSIX semaphore
        facility provided with some new UNIX®i. Some operating system problems
        are managed within these routines for the common stuff that happens when
        a poorly configured OS gets overloaded!

	Enjoy!


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<semaphore.h>
#include	<errno.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"psem.h"


/* local defines */

#define	TO_NOSPC	5
#define	TO_MFILE	5
#define	TO_INTR		5


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	msleep(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */


/* local variables */


/* exported subroutines */


int uc_openpsem(name,oflag,operm,count,rpp)
const char	name[] ;
int		oflag ;
mode_t		operm ;
uint		count ;
PSEM		**rpp ;
{
	int		rs ;
	int		to_mfile = 0 ;
	int		to_nospc = 0 ;
	char		altname[PSEM_NAMELEN + 1] ;

	if (rpp == NULL) return SR_FAULT ;
	if (name == NULL) return SR_FAULT ;

	if (name[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("psem_open: name=%s\n",name) ;
#endif

	if (name[0] != '/') {
	    sncpy2(altname,PSEM_NAMELEN,"/",name) ;
	    name = altname ;
	}

#if	CF_DEBUGS
	debugprintf("psem_open: psem_name=%s\n",name) ;
#endif

	memset(*rpp,0,sizeof(PSEM)) ;

again:
	rs = SR_OK ;
	*rpp = sem_open(name,oflag,operm,count) ;

	if (*rpp == SEM_FAILED)
	    rs = (- errno) ;

	if (rs < 0) {
	    switch (rs) {
	    case SR_INTR:
	        goto again ;
	    case SR_MFILE:
	    case SR_NFILE:
	        if (to_mfile++ >= TO_MFILE) break ;
	        msleep(1000) ;
	        goto again ;
	    case SR_NOSPC:
	        if (to_nospc++ >= TO_NOSPC) break ;
	        msleep(1000) ;
	        goto again ;
	    } /* end switch */
	}

	return rs ;
}
/* end subroutine (uc_openpsem) */


