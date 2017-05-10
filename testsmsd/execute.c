/* progexec */

/* execute a server daemon program */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGSTDOUT	0		/* debug STDOUT */


/* revision history:

	= 1998-07-14, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This subroutine just 'exec(2)'s a daemon server program.


***************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecstr.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	NOFILE
#define	NOFILE		20
#endif



/* external subroutines */


/* external variables */


/* forward references */


/* local variables */






int progexec(pip,cip,progfname,argz,alp,elp)
struct proginfo		*pip ;
struct clientinfo	*cip ;
const char		progfname[], argz[] ;
VECSTR		*alp ;
VECSTR		*elp ;
{
	int	rs = SR_OK ;
	int	i ;

	char	*oldargz ;


#if	CF_DEBUG && CF_DEBUGSTDOUT
	if (DEBUGLEVEL(4)) {
		int	cl ;
		char	*cp = "What hath God wrought?\n" ;
		cl = strlen(cp) ;
		uc_writen(FD_STDOUT,cp,cl) ;
	}
#endif

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(4)) {
	    int	i ;
	    char	*sp ;
	    char	*cp ;
	    debugprintf("progexec: progfname=%s\n",progfname) ;
	    debugprintf("progexec: argz=%s\n",argz) ;
	    for (i = 0 ; vecstr_get(alp,i,&sp) >= 0 ; i += 1)
	        debugprintf("progexec: arg%d> %s\n",i,sp) ;
	    for (i = 0 ; vecstr_get(elp,i,&cp) >= 0 ; i += 1)
	        debugprintf("progexec: env> %s\n",cp) ;
	}
#endif /* CF_DEBUG */

	if ((pip->workdname != NULL) && (pip->workdname[0] != '\0'))
	    rs = u_chdir(pip->workdname) ;

	oldargz = alp->va[0] ;
	alp->va[0] = (char *) argz ;
	if (rs >= 0) {
	    for (i = 3 ; i < NOFILE ; i += 1) {
		u_close(i) ;
	    }
	    {
	        const char	**eav = (const char **) alp->va ;
	        const char	**eev = (const char **) elp->va ;
	        rs = u_execve(progfname,eav,eev) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progexec: u_execve() rs=%s\n",rs) ;
#endif /* CF_DEBUG */

	alp->va[1] = oldargz ;
	return rs ;
}
/* end subroutine (execute) */



