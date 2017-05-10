/* debugfork */

/* debug-fork stubs */


#define	CF_DEBUGS	1		/* compile-time debugforkging */
#define	CF_UCOPEN	0		/* use 'uc_openprog(3uc)' */


/* revision history:

	= 1998-08-15, David A­D­ Morano
	This was written to debugfork the REXEC program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This modeule provides debugging support for the 'uc_fork(3uc)'.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stropts.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */

#ifndef	COLUMNS
#define	COLUMNS		80
#endif

#define	RBUFLEN		100

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	bufprintf(char *,int,const char *,...) ;
extern int	nprintf(const char *,const char *,...) ;
extern int	debugforkprintf(const char *,...) ;
extern int	debugforkprint(const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward subroutines */


/* external variables */


/* local variables */


/* exported subroutines */


int debugfork(const char *s)
{
	const int	of = O_RDWR ;
	int		rs ;
	const char	*pf = "/home/dam/rje/proghello" ;

#if	CF_UCOPEN
	debugprintf("debugfork: uc_openprog() %s\n",s) ;
	rs = uc_openprog(pf,of,NULL,NULL) ;
	debugprintf("debugfork: uc_openprog() rs=%d\n",rs) ;
	if (rs >= 0) {
	    const int	rlen = RBUFLEN ;
	    const int	opts = 0 ;
	    const int	to = 4 ;
	    const int	fd = rs ;
	    char	rbuf[RBUFLEN+1] ;
	    while ((rs = uc_reade(fd,rbuf,rlen,to,opts)) > 0) {
	        int	len = rs ;
	        debugprintf("debugfork: l=>%t<\n",
	            rbuf,strlinelen(rbuf,len,40)) ;
	    } /* end while */
	    u_close(fd) ;
	}
#else /* CF_UCOPEN */
	debugprintf("debugfork: uc_fork() %s\n",s) ;
	rs = uc_fork() ;
	if (rs == 0) {
	    debugprintf("debugfork: child %s\n",s) ;
	    u_exit(0) ;
	}
#endif /* CF_UCOPEN */

	debugprintf("debugfork: ret rs=%d (parent)\n",rs) ;
	return rs ;
}
/* end subroutines (debugfork) */


