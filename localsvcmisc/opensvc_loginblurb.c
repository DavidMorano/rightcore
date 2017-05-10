/* opensvc_loginblurb */

/* LOCAL facility open-service (loginblurb) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */


/* revision history:

	= 2003-11-04, David A­D­ Morano
        This code was started by taking the corresponding code from the
        TCP-family module. In retrospect, that was a mistake. Rather I should
        have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an open-facility-service module.

	Filename:

		local§loginblurb[­<strspec>]

	Synopsis:

	int opensvc_loginblurb(pr,prn,of,om,argv,envv,to)
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


	= Implementation notes:

        Why do we go through so much trouble to find and load the LIBLKCMD
        shared-orject ourselves when we could have let the run-time linker do it
        for us? The reason that we do it for ourselves is that in this way the
        shared-object that this subroutine is a part of does not need to specify
        the LIBLKCMD shared-object as a dependency. By not making another
        dependency of the the shared-object that this subroutine is a part of we
        dramatically reduce the run-time linker work done whenever this (the
        current) shared-object is loaded. Also, our own search for LIBLKCMD is a
        little bit faster than that of the run-time linker. A local search of
        something like a LIBLKCMD shared-object should be performed whenever we
        have mixed subroutines made a part of the same shared-object. That is:
        subroutines that need LIBLKCMD (or something like it) and those that do
        not. In this way the subroutines that do not need a complex object such
        as LIBLKCMD do not have to suffer the pretty great cost of loading it by
        the run-time linker. Also, note the fact that a shared-object such as
        LIBLKCMD is *already* attached to the parent object is *not* determined
        until after the run-time linker searches all of the myriad directories
        in the LD_LIBRARY_PATH. This just adds to the unnecessary filesystem
        searches for subroutines that do not require something like LIBLKCMD.

        Besides the subtleties of using the run-time linker in an indiscriminate
        manner, trying to invoke a function like LOGINBLURB by loading the whole
        shell-builtin command by the same name seems to be a huge waste of time
        as compared with invoking some simple subroutine that mades the same
        output as the LOGINBLURB shell-builtin does! What we sometimes do to use
        an existing piece of some code is often quite amazingly complex!


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

#include	"opensvc_loginblurb.h"
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
#define	CMDNAME		"loginblurb"
#endif

#define	VARMOTDUSER	"MOTD_USERNAME"
#define	VARMOTDGROUP	"MOTD_GROUPNAME"
#define	VARMOTDUID	"MOTD_UID"
#define	VARMOTDGID	"MOTD_GID"

#define	STRBUFLEN	GROUPNAMELEN

#define	DSTDERRFNAME	"opensvc_loginblurb.err"
#define	NDEBFNAME	"opensvc_loginblurb.deb"


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	ctdeci(char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	attachso(cchar **,cchar *,cchar **,cchar **,int,void **) ;
extern int	isSpecialObject(void *) ;

extern int	lib_callfunc(int(*)(),int,const char **,const char **,void *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
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


int opensvc_loginblurb(pr,prn,of,om,argv,envv,to)
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
	int		pipes[2] ;
	int		fd = -1 ;
	const char	*callsymname = CALLSYMNAME ;
	const char	*cmdname = CMDNAME ;
	const char	*strspec = NULL ;
	char		strbuf[STRBUFLEN+1] ;
	char		cmdsymname[MAXNAMELEN+1] ;
	const void	*symp = NULL ;
	const void	*callsymp = NULL ;
	void		*sop = NULL ;

#if	CF_DEBUGS && CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_loginblurb: entered\n") ;
#endif

#if	CF_DEBUGS 
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	debugprintf("opensvc_loginblurb: argv=%p\n",argv) ;
#endif

	if (argv == NULL) {
	    rs = SR_FAULT ;
	    goto ret0 ;
	}

	for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;

#if	CF_DEBUGS
	{
		int	i ;
	debugprintf("opensvc_loginblurb: entered argc=%u\n",argc) ;
		for (i = 0 ; argv[i] != NULL ; i += 1)
	debugprintf("opensvc_loginblurb: argv[%u]=>%s<\n",i,argv[i]) ;
	}
#endif

/* fingure some arguments */

	if ((argc >= 2) && (argv[1] != NULL)) {
	   strspec = argv[1] ;
	}

/* do we need a default argument? */

	if ((strspec == NULL) || (strspec[0] == '\0')) {
	    const char	*gn = getourenv(envv,VARMOTDGROUP) ;
	    char	gbuf[GROUPNAMELEN+1] ;
	    if ((gn == NULL) || (gn[0] == '\0')) {
		const char	*gidp = getourenv(envv,VARMOTDGID) ;
		if ((gidp != NULL) && (gidp[0] != '\0')) {
		    int		v ;
		    if ((rs = cfdeci(gidp,-1,&v)) >= 0) {
		        gid_t	 gid = v ;
			gn = gbuf ;
		        rs = getgroupname(gbuf,GROUPNAMELEN,gid) ;
		    }
		}
	    } /* end if (GID?) */
	    if ((rs >= 0) && (gn != NULL) && (gn[0] != '\0')) {
		strspec = strbuf ;
		rs = sncpyuc(strbuf,STRBUFLEN,gn) ;
	    }
	} /* end if (strspec) */

/* create the symbol for the function (command) call */
/* find and load the shared-object we need (LIBLKCMD) */

	if (rs >= 0) {
	if ((rs = sncpy2(cmdsymname,MAXNAMELEN,"p_",cmdname)) >= 0) {
	    const char	*dnames[10] ; /* careful size is big enough */
	    const char	*syms[3] ; /* careful size is big enough */
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
	} /* end if (ok) */

#if	CF_DEBUGS && CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_loginblurb: attachso() rs=%d\n",rs) ;
#endif
#if	CF_DEBUGS
	debugprintf("opensvc_loginblurb: attachso() rs=%d sop=%p\n",rs,sop) ;
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
	if ((rs = uc_piper(pipes,3)) >= 0) {
	    int	wfd = pipes[1] ;
	    int	ex ;
	    int (*callfunc)(int(*)(),int,cchar **,cchar **,void *) ;
	    int	(*cmdfunc)(int,const char **,void *) ;
	    const char	*nfn = STDNULLFNAME ;
	    char	ofname[MAXNAMELEN+1] ;
	    fd = pipes[0] ;

	    callfunc = 
		(int (*)(int(*)(),int,cchar **,cchar **,void *)) 
		callsymp ;

	    cmdfunc = (int (*)(int,const char **,void *)) symp ;

	    if ((rs = snsd(ofname,MAXNAMELEN,"*",wfd)) >= 0) {
		    int		ac = 0 ;
	    	    const char	*av[11] ; /* careful on size */

	            av[ac++] = cmdname ;
	            av[ac++] = "-ef" ;
#if	CF_DEBUGS
	            av[ac++] = DSTDERRFNAME ;
		    av[ac++] = "-D=5" ;
#else
	            av[ac++] = nfn ;
#endif
	            av[ac++] = "-of" ;
	            av[ac++] = ofname ;
		    if (strspec != NULL) {
	                av[ac++] = "-s" ;
	                av[ac++] = strspec ;
		    }
		    av[ac] = NULL ;

	            ex = (*callfunc)(cmdfunc,ac,av,envv,NULL) ;
	            if (ex != EX_OK) rs = SR_IO ;

#if	CF_DEBUGS
	debugprintf("opensvc_loginblurb: callfun() ex=%u\n",ex) ;
#endif

	    } /* end if */

	    u_close(wfd) ;
	    if (rs < 0) {
		u_close(fd) ;
		fd = -1 ;
	    }
	} /* end if (piper) */
	} /* end if (ok) */

	if ((sop != NULL) && (! isSpecialObject(sop))) dlclose(sop) ;

ret0:

#if	CF_DEBUGS
	debugprintf("opensvc_loginblurb: ret rs=%d fd=%u\n",rs,fd) ;
#endif

#if	CF_DEBUGS
	debugclose() ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_loginblurb) */


