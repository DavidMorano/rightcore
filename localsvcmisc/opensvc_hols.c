/* opensvc_hols */

/* LOCAL facility open-service (hols) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */


/* revision history:

	= 2003-11-04, David A­D­ Morano
	This code was started by taking the code from some other open-service.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a facility-open-service module.

	Synopsis:

	int opensvc_hols(pr,prn,of,om,argv,envv,to)
	const char	*pr ;
	const char	*prn ;
	int		of ;
	mode_t		om ;
	const char	**argv ;
	const char	**envv ;
	int		to ;

	Arguments:

	pr		program-root
	prn		facility name
	of		open-flags
	om		open-mode
	argv		argument array
	envv		environment array
	to		time-out

	Returns:

	>=0		file-descriptor
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<keyopt.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"opensvc_hols.h"
#include	"defs.h"


/* local defines */

#ifndef	STDNULLFNAME
#define	STDNULLFNAME	"*STDNULL*"
#endif

#ifndef	LKCMDNAME
#define	LKCMDNAME	"liblkcmd"
#endif

#ifndef	CALLSYMNAME
#define	CALLSYMNAME	"lib_callfunc"
#endif

#ifndef	CMDNAME
#define	CMDNAME		"holiday"
#endif

#define	DSTDERRFNAME	"opensvc_hols.err"
#define	NDEBFNAME	"opensvc_hols.deb"

#undef	NUMBUFLEN
#define	NUMBUFLEN	20


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	ctdeci(char *,int,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	attachso(cchar **,cchar *,cchar **,cchar **,int,void **) ;
extern int	hasalldig(const char *,int) ;
extern int	isSpecialObject(void *) ;

extern int	lib_callfunc(int(*)(),int,const char **,const char **,void *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif
#if	CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const char	*syslibs[] = {
	"/usr/extra/lib",
	"/usr/preroot/lib",
	NULL
} ;


/* exported subroutines */


int opensvc_hols(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	int		rs = SR_OK ;
	int		argc = 0 ;
	int		fd = -1 ;
	const char	*callsymname = CALLSYMNAME ;
	const char	*cmdname = CMDNAME ;
	char		cmdsymname[MAXNAMELEN+1] ;
	char		numbuf[NUMBUFLEN+1] ;
	const void	*symp = NULL ;
	const void	*callsymp = NULL ;
	void		*sop = RTLD_DEFAULT ;

#if	CF_DEBUGS 
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	numbuf[0] = '+' ;
	numbuf[1] = '\0' ;
	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	    if ((argc >= 2) && (argv[1] != NULL)) {
		if (hasalldig(argv[1],-1)) {
		    const int	ml = (NUMBUFLEN-1) ;
	            strwcpy((numbuf+1),argv[1],ml) ;
		}
	    }
	} 

#if	CF_DEBUGS
	debugprintf("opensvc_hols: numbuf=%s\n",numbuf) ;
#endif
#if	CF_DEBUGS && CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_hols: numbuf=%s\n",numbuf) ;
#endif

/* create the symbol name for the function (command) call */
/* find and load the shared-object we need (LIBLKCMD) */

	if ((rs = sncpy2(cmdsymname,MAXNAMELEN,"p_",cmdname)) >= 0) {
	    const char	*dnames[10] ; /* careful that the size is big enough */
	    const char	*syms[3] ; /* careful that the size is big enough */
	    int		i, j ;
	    int		dlmode = RTLD_LAZY ;
	    char	libdname[MAXPATHLEN+1] ;
	    rs = mkpath2(libdname,pr,"lib") ;
	    i = 0 ;
	    dnames[i++] = libdname ;
	    for (j = 0 ; syslibs[j] != NULL ; j += 1) 
		dnames[i++] = syslibs[j] ;
	    dnames[i] = NULL ;
	    i = 0 ;
	    syms[i++] = callsymname ;
	    syms[i++] = cmdsymname ;
	    syms[i] = NULL ;
	    if (rs >= 0) {
	        rs = attachso(dnames,LKCMDNAME,NULL,syms,dlmode,&sop) ;
	    }
	}

#if	CF_DEBUGS && CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_hols: attachso() rs=%d\n",rs) ;
#endif
#if	CF_DEBUGS
	debugprintf("opensvc_hols: attachso() rs=%d sop=%p\n",rs,sop) ;
#endif

/* find some symbols */

	if (rs >= 0) {
	    callsymp = dlsym(sop,callsymname) ;
	    if (callsymp == NULL) rs = SR_LIBACC ;
	}

	if (rs >= 0) {
	    symp = dlsym(sop,cmdsymname) ;
	    if (symp == NULL) rs = SR_LIBACC ;
	}

/* continue */

	if (rs >= 0) {
	if ((rs = opentmp(NULL,0,0664)) >= 0) {
	    int	ex ;
	    int (*callfunc)(int(*)(),int,cchar **,const char **,void *) ;
	    int	(*cmdfunc)(int,const char **,const char **,void *) ;
	    const char	*ef = STDNULLFNAME ;
	    char	ofname[MAXNAMELEN+1] ;
	    fd = rs ;

	    callfunc = 
		(int (*)(int(*)(),int,const char **,const char **,void *)) 
		callsymp ;

	    cmdfunc = (int (*)(int,cchar **,cchar **,void *)) symp ;

	    if ((rs = snsd(ofname,MAXNAMELEN,"*",fd)) >= 0) {
		    int		ac = 0 ;
	    	    const char	*av[10] ; /* careful */

	            av[ac++] = cmdname ;
	            av[ac++] = "-ef" ;
#if	CF_DEBUGS
	            av[ac++] = DSTDERRFNAME ;
		    av[ac++] = "-D=5" ;
#else
	            av[ac++] = ef ;
#endif
	            av[ac++] = "-of" ;
	            av[ac++] = ofname ;
		    av[ac++] = numbuf ;
	            av[ac] = NULL ;

	            ex = (*callfunc)(cmdfunc,ac,av,envv,NULL) ;
	            if (ex != EX_OK) rs = SR_IO ;

#if	CF_DEBUGS && CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_hols: cmd() ex=%u\n",ex) ;
#endif
#if	CF_DEBUGS
	debugprintf("opensvc_hols: cmd() ex=%u\n",ex) ;
#endif

	    } /* end if */

	    if (rs >= 0) u_rewind(fd) ;

	    if (rs < 0) {
		u_close(fd) ;
		fd = -1 ;
	    }
	} /* end if (opentmp) */
	} /* end if (ok) */

	if ((sop != NULL) && (! isSpecialObject(sop))) dlclose(sop) ;

#if	CF_DEBUGS && CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_hols: ret rs=%d\n",rs) ;
#endif
#if	CF_DEBUGS
	debugprintf("opensvc_hols: ret rs=%d fd=%u\n",rs,fd) ;
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_hols) */


