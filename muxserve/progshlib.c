/* progshlib */

/* execute a server program from a shared-library */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_CHDIR	0		/* change directory? */


/* revision history:

	= 2008-07-14, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine finds the specified library entry point and calles it.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<dlfcn.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#undef	LIBCALLFUNC
#define	LIBCALLFUNC	"lib_callfunc"


/* external subroutines */

extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strnnlen(cchar *,int,int) ;
#endif

extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strdcpy2w(char *,int,const char *,const char *,int) ;
extern char	*strdcpy3w(char *,int,cchar *,cchar *,cchar *,int) ;


/* external variables */

extern char	**environ ;


/* local structures */

typedef int (*libent)(int,cchar **,cchar **,void *) ;
typedef int (*libcaller)(libent,int,cchar **,cchar **,void *) ;


/* forward references */


/* local variables */


/* exported subroutines */


int progshlib(pip,shlibfname,argz,alp,enp,enl)
PROGINFO	*pip ;
const char	shlibfname[] ;
const char	argz[] ;
VECSTR		*alp ;
const char	*enp ;
int		enl ;
{
	VECSTR		*elp = &pip->exports ;
	const char	**av ;
	const char	**ev ;
	const int	dlmode = RTLD_LAZY ;
	int		rs = SR_OK ;
	int		size ;
	int		entrylen ;
	int		nargs ;
	int		ex ;
	int		(*soentry)(int,cchar **,cchar **,void *) ;
	int		(*socaller)(libent,int,cchar **,cchar **,void *) ;
	const char	*callername = LIBCALLFUNC ;
	char		*entrybuf = NULL ;
	char		*p ;
	void		*sop ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    int		i ;
	    cchar	*cp ;
	    debugprintf("progshlib: shlibfname=%s\n",shlibfname) ;
	    debugprintf("progshlib: en=>%t<\n",enp,enl) ;
	    debugprintf("progshlib: argz=%s\n",argz) ;
	    debugprintf("progshlib: &environ=%p\n",&environ) ;
	    for (i = 0 ; vecstr_get(alp,i,&cp) >= 0 ; i += 1) {
	        debugprintf("progshlib: arg%u=>%t<\n",i,
			cp,strnnlen(cp,-1,40)) ;
	    }
	    for (i = 0 ; vecstr_get(elp,i,&cp) >= 0 ; i += 1) {
	        debugprintf("progshlib: env%u=>%t<\n",i,
			cp,strnnlen(cp,-1,40)) ;
	    }
	}
#endif /* CF_DEBUG */

	sop = dlopen(shlibfname,dlmode) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) 
	    debugprintf("progshlib: dlopen() sop=%p\n",sop) ;
#endif

	if (sop == NULL) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
		const char *es = dlerror() ;
	        debugprintf("progshlib: dlerror() >%s<\n",es) ;
	    }
#endif
	    rs = SR_NOENT ;
	    goto badnoprog ;
	}

	if (enl < 0) enl = strlen(enp) ;

	entrylen = (enl + 2) ;
	size = (entrylen + 1) ;
	rs = uc_malloc(size,&p) ;
	if (rs < 0)
	    goto badalloc ;

	entrybuf = p ;
	entrylen = strdcpy2w(entrybuf,entrylen,"p_",enp,enl) - entrybuf ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progshlib: entry=>%s<\n",entrybuf) ;
#endif

	soentry = (libent) dlsym(sop,entrybuf) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	debugprintf("progshlib: dlsym() >%s<\n",
		((soentry != NULL) ? "ok" : dlerror() )) ;
	}
#endif

	if (soentry == NULL) {
	    rs = SR_LIBACC ;
	    goto badnoexec ;
	}

	socaller = (libcaller) dlsym(sop,callername) ;
	if (socaller == NULL) {
	    rs = SR_LIBACC ;
	    goto badnoexec ;
	}

/* set up some final environment */

#if	CF_CHDIR && 0
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
	    rs = vecstr_envset(elp,"_",shlibfname,-1) ;

	if (rs >= 0)
	    rs = vecstr_envset(elp,"_EF",shlibfname,-1) ;

	if (rs >= 0)
	    rs = vecstr_envset(elp,"_A0",argz,-1) ;

	if (rs >= 0)
	    nargs = vecstr_getvec(alp,&av) ;

	if (rs >= 0)
	    vecstr_getvec(elp,&ev) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progshlib: call nargs=%d\n",nargs) ;
#endif /* CF_DEBUG */

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(5)) {
	    int		i ;
	    cchar	*ep ;
	    debugprintf("progshlib: env¬\n") ;
	    for (i = 0 ; ev[i] != NULL ; i += 1) {
	      ep = ev[i] ;
	      debugprintf("progshlib: e%03u=>%t<\n",i,
		    ep,strlinelen(ep,-1,50)) ;
	    }
	    debugprintf("progshlib: nenv=%u\n",i) ;
	}
#endif /* CF_DEBUGS */

	if (rs >= 0) {
	    ex = (*socaller)(soentry,nargs,av,ev,NULL) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progshlib: call ex=%u\n",ex) ;
#endif

#if	CF_CHDIR
	u_chdir(pip->pwd) ;
#endif

done:
	if (ex != EX_OK) {
	    switch (ex) {
	    case EX_NOPROG:
		rs = SR_NOENT ;
		break ;
	    case EX_NOEXEC:
		rs = SR_NOEXEC ;
		break ;
	    case EX_MUTEX:
		rs = SR_TIMEDOUT ;
		break ;
	    case EX_USAGE:
		rs = SR_INVALID ;
		break ;
	    case EX_INFO:
		rs = SR_OK ;
		break ;
	    default:
		rs = SR_NOPROTOOPT ;
	    } /* end switch */
	} /* end if */

badnoexec:

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progshlib: badnoexec rs=%d\n",rs) ;
#endif /* CF_DEBUG */

	if (entrybuf != NULL) {
	    uc_free(entrybuf) ;
	}

badalloc:

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progshlib: badalloc rs=%d\n",rs) ;
#endif /* CF_DEBUG */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) 
	    debugprintf("progshlib: sop=%p\n",sop) ;
#endif

	if (sop != NULL) {
	    dlclose(sop) ;
	}

badnoprog:
ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("progshlib: ret rs=%d\n",rs) ;
#endif /* CF_DEBUG */

	return rs ;
}
/* end subroutine (progshlib) */


