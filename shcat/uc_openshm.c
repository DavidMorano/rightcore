/* uc_openshm */

/* interface component for UNIX® library-3c */
/* open POSIX share-memory */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine opens a POSIX shared-memory ('sem(3rt)') object.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snopenflags(char *,int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


#if	defined(SYSHAS_PSHM) && (SYSHAS_PSHM > 0)

int uc_openshm(cchar *shmname,int of,mode_t om)
{
	int		rs = SR_OK ;
	char		*nbuf = NULL ;

	if (shmname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("uc_openshm: shmname=%s\n",shmname) ;
	{
	    char	obuf[100+1] ;
	    snopenflags(obuf,100,of) ;
	    debugprintf("uc_openshm: of=%s om=0%3o\n",obuf,om) ;
	}
#endif

	if (shmname[0] != '/') {
	    const int	ns = (strlen(shmname)+2) ;
	    char	*bp ;
	    if ((rs = uc_libmalloc(ns,&bp)) >= 0) {
		const char	*a = bp ;
		*bp++ = '/' ;
		strwcpy(bp,shmname,-1) ;
		shmname = a ;
	    }
	}

	if (rs >= 0) {
	    repeat {
	        errno = 0 ;
	        if ((rs = shm_open(shmname,of,om)) < 0) rs = (- errno) ;
	    } until (rs != SR_INTR) ;
	    if (nbuf != NULL) uc_libfree(nbuf) ;
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (uc_openshm) */

#else /* defined(SYSHAS_PSHM) && (SYSHAS_PSHM > 0) */

int uc_openshm(fname,oflags,operm)
const char	fname[] ;
int		oflags ;
mode_t		operm ;
{
	return SR_NOSYS ;
}
/* end subroutine (uc_openshm) */

#endif /* defined(SYSHAS_PSHM) && (SYSHAS_PSHM > 0) */


