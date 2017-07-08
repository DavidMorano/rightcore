/* progexec */

/* execute a server daemon program */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGSTDOUT	0		/* debug STDOUT */
#define	CF_CHDIR	0		/* 'chdir(2)' */


/* revision history:

	= 2008-07-14, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine just 'exec(2)'s a daemon server program.


*******************************************************************************/


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
#include	<vecstr.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#ifndef	NOFILE
#define	NOFILE		20
#endif


/* external subroutines */

extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strnnlen(const char *,int,int) ;
#endif


/* external variables */


/* forward references */


/* local variables */


/* exported subroutines */


int progexec(PROGINFO *pip,cchar *progfname,cchar *argz,VECSTR *alp)
{
	VECSTR		*elp = &pip->exports ;
	int		rs = SR_OK ;
	int		i ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    int	i ;
	    char	*cp ;
	    debugprintf("progexec: progfname=%s\n",progfname) ;
	    debugprintf("progexec: argz=%s\n",argz) ;
	    for (i = 0 ; vecstr_get(alp,i,&cp) >= 0 ; i += 1) {
	        debugprintf("progexec: arg%u=>%t<\n",i,
			cp,strnnlen(cp,-1,40)) ;
	    }
	    for (i = 0 ; vecstr_get(elp,i,&cp) >= 0 ; i += 1) {
	        debugprintf("progexec: env%u=>%t<\n",i,
			cp,strnnlen(cp,-1,40)) ;
	    }
	}
#endif /* CF_DEBUG */

#if	CF_CHDIR
	proginfo_pwd(pip) ;
	if ((pip->workdname != NULL) && (pip->workdname[0] != '\0')) {
	    if (pip->debuglevel == 0) {
	    rs = u_chdir(pip->workdname) ;
	    if (rs >= 0)
		rs = vecstr_envset(elp,VARPWD,pip->workdname,-1) ;
	    }
	}
#endif /* CF_CHDIR */

	if (rs >= 0)
	    rs = vecstr_envset(elp,"_",progfname,-1) ;

	if (rs >= 0)
	    rs = vecstr_envset(elp,"_EF",progfname,-1) ;

	if (rs >= 0)
	    rs = vecstr_envset(elp,"_A0",argz,-1) ;

	if (rs >= 0) {
	    cchar	**av ;
	    if ((rs = vecstr_getvec(alp,&av)) >= 0) {
		cchar	**ev ;
	        if ((rs = vecstr_getvec(elp,&ev)) >= 0) {
	            for (i = 3 ; i < NOFILE ; i += 1) {
		        u_close(i) ;
	            }
	            rs = uc_execve(progfname,av,ev) ;
		}
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progexec: u_execve() rs=%s\n",rs) ;
#endif /* CF_DEBUG */

#if	CF_CHDIR
	u_chdir(pip->pwd) ;
#endif

	return rs ;
}
/* end subroutine (progexec) */


