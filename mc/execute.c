/* progexec */

/* execute a server daemon program */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* debug print-outs */
#define	CF_ISAEXEC	1		/* try 'isaexec(3c)' */


/* revision history:

	= 1998-07-01, David A­D­ Morano

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
#include	<vecstr.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#undef	VARUNDER
#define	VARUNDER	"_"



/* external subroutines */

extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;


/* external variables */


/* forward references */

static int	vecstr_loadenames(vecstr *,const char **,const char *) ;
static int	vecstr_loadename(vecstr *,const char *,const char *) ;


/* local variables */

static const char	*enames[] = {
	VARUNDER,
	"_EF",
	NULL
} ;


/* exported subroutines */


int progexec(pip,progfname,alp,elp)
struct proginfo	*pip ;
const char	progfname[] ;
VECSTR		*alp ;
VECSTR		*elp ;
{
	int	rs ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    int	i ;
	    char	*sp ;
	    debugprintf("execute: program=%s\n",progfname) ;
	    for (i = 0 ; vecstr_get(alp,i,&sp) >= 0 ; i += 1)
	        debugprintf("execute: arg%d> %s\n",i,sp) ;
	}
#endif /* CF_DEBUG */

	rs = vecstr_loadenames(elp,enames,progfname) ;
	if (rs < 0)
	    goto ret0 ;

	if ((pip->workdname != NULL) && (pip->workdname[0] != '\0')) {
	    rs = u_chdir(pip->workdname) ;
	    if (rs >= 0)
		rs = vecstr_envset(elp,VARPWD,pip->workdname,-1) ;
	}

	if (rs >= 0) {
	    const char	**av = (const char **) alp->va ;
	    const char	**ev = (const char **) elp->va ;

#if	CF_ISAEXEC && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	    uc_isaexecve(progfname,av,ev);
#endif

	    rs = uc_execve(progfname,av,ev) ;

	} /* end if */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("execute: ret rs=%s\n",rs) ;
#endif /* CF_DEBUG */

	return rs ;
}
/* end subroutine (progexec) */


/* local subroutines */


static int vecstr_loadenames(elp,names,fname)
vecstr		*elp ;
const char	*names[] ;
const char	fname[] ;
{
	int	rs = SR_OK ;
	int	i ;


	for (i = 0 ; names[i] != NULL ; i += 1) {

	    rs = vecstr_loadename(elp,names[i],fname) ;
	    if (rs < 0)
		break ;

	} /* end for */

	return rs ;
}
/* end subroutines (vecstr_loadenames) */


static int vecstr_loadename(elp,name,fname)
vecstr		*elp ;
const char	name[] ;
const char	fname[] ;
{
	int	rs = SR_OK ;
	int	i ;


	if ((i = vecstr_search(elp,name,vstrkeycmp,NULL)) >= 0)
	    rs = vecstr_del(elp,i) ;

	if (rs >= 0)
	    rs = vecstr_envadd(elp,name,fname,-1) ;

	return rs ;
}
/* end subroutines (vecstr_loadename) */



