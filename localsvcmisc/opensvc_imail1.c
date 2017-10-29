/* opensvc_imail */

/* LOCAL facility open-service (imail) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */


/* revision history:

	= 2003-11-04, David Morano

	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather
	I should have started this code by using the corresponding UUX
	dialer module.


*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is an open-facility-service module.

	Synopsis:

	int opensvc_imail(pr,prn,of,om,argv,envv,to)
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
	shared-orject ourselves when we could have let the run-time linker
	do it for us?  The reason that we do it for ourselves is that in
	this way the shared-object that this subroutine is a part of does
	not need to specify the LIBLKCMD shared-object as a dependency.
	By not making another dependency of the the shared-object that
	this subroutine is a part of we dramatically reduce the run-time
	linker work done whenever this (the current) shared-object is
	loaded.  Also, our own search for LIBLKCMD is a little bit faster
	than that of the run-time linker.  A local search of something
	like a LIBLKCMD shared-object should be performed whenever we
	have mixed subroutines made a part of the same shared-object.
	That is: subroutines that need LIBLKCMD (or something like it)
	and those that do not.	In this way the subroutines that do
	not need complex object such as LIBLKCMD do not have to suffer
	the pretty great cost of loading it by the run-time linker.
	Also, note that the fact that a shared-object such as LIBLKCMD
	is *already* attached to the parent object is *not* determined
	until after the run-time linker searches all of the myriad
	directories in the LD_LIBRARY_PATH.  This just adds to the
	unnecessary filesystem searches for subroutines that do not
	require something like LIBLKCMD.

	Besides the subtleties of using the run-time linker in an
	indiscriminate manner, trying to invoke a function like LOGINBLURB
	by loading the whole shell-builtin command by the same name
	seems to be a huge waste of time as compared with invoking some
	simple subroutine that mades the same output as the LOGINBLURB
	shell-builtin does!  What we sometimes do to use an existing
	piece of some code is often quite amazingly complex!


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<keyopt.h>
#include	<bits.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<sbuf.h>
#include	<buffer.h>
#include	<paramfile.h>
#include	<nulstr.h>
#include	<logfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"opensvc_imail.h"
#include	"defs.h"


/* local defines */

#ifndef	STDNULLFNAME
#define	STDNULLFNAME	"*STDNULL*"
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	LKCMDNAME
#define	LKCMDNAME	"liblkcmd"
#endif

#ifndef	CALLSYMNAME
#define	CALLSYMNAME	"lib_callcmd"
#endif

#ifndef	CMDNAME
#define	CMDNAME		"imail"
#endif

#define	DSTDERRFNAME	"opensvc_imail.err"
#define	NDEBFNAME	"opensvc_imail.deb"


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
extern int	attachso(const char **,const char *,const char **,
			const char **,int,void **) ;
extern int	isSpecialObject(void *) ;

extern int	lib_callcmd(int(*)(),int,const char **,const char **,void *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
extern int	nprintf(const char *,const char *,...) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */

static const char	*syslibs[] = {
	"/usr/extra/lib",
	"/usr/preroot/lib",
	NULL
} ;


/* external variables */


/* exported subroutines */


int opensvc_imail(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	int	rs = SR_OK ;
	int	argc = 0 ;
	int	am = (of & O_ACCMODE) ;
	int	pipes[2] ;
	int	fd = -1 ;

	const char	*callsymname = CALLSYMNAME ;
	const char	*cmdname = CMDNAME ;

	char		cmdsymname[MAXNAMELEN+1] ;

	const void	*symp = NULL ;
	const void	*callsymp = NULL ;
	void		*sop = NULL ;


#if	CF_DEBUGS && CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_imail: entered\n") ;
#endif

#if	CF_DEBUGS 
	{
	    const char	*cp ;
	    cp = getourenv(envv,VARDEBUGFNAME) ;
	    if (cp != NULL) {
	        int dfd = debugopen(cp) ;
	        debugprintf("opensvc_imail: starting DFD=%u\n",dfd) ;
	    }
	} /* end block */
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	debugprintf("opensvc_imail: argv=%p\n",argv) ;
#endif

	if (argv == NULL) {
	    rs = SR_FAULT ;
	    goto ret0 ;
	}

	for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("opensvc_imail: entered argc=%u\n",argc) ;
	    for (i = 0 ; argv[i] != NULL ; i += 1)
	        debugprintf("opensvc_imail: argv[%u]=>%s<\n",i,argv[i]) ;
	}
#endif

	if ((am == O_RDWR) || (am == O_RDONLY)) return SR_BADF ;

/* create the symbol for the function (command) call */

	rs = sncpy2(cmdsymname,MAXNAMELEN,"p_",cmdname) ;
	if (rs < 0) goto ret0 ;

/* find and load the shared-object we need (LIBLKCMD) */

	{
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
	    if (rs >= 0)
	        rs = attachso(dnames,LKCMDNAME,NULL,syms,dlmode,&sop) ;
	}

#if	CF_DEBUGS && CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_imail: attachso() rs=%d\n",rs) ;
#endif
#if	CF_DEBUGS
	debugprintf("opensvc_imail: attachso() rs=%d sop=%p\n",rs,sop) ;
#endif

	if (rs < 0) goto ret0 ;

/* find some symbols */

	callsymp = dlsym(sop,callsymname) ;
	if (callsymp == NULL) {
	    rs = SR_LIBACC ;
#if	CF_DEBUGS
	    debugprintf("opensvc_imail: dlsym() s=%s rs=%d\n",
	        callsymname,rs) ;
#endif
	    goto ret1 ; /* goto close the already loaded shared-object */
	}

	symp = dlsym(sop,cmdsymname) ;
	if (symp == NULL) {
	    rs = SR_LIBACC ;
#if	CF_DEBUGS
	    debugprintf("opensvc_imail: dlsym() s=%s rs=%d\n",
	        cmdsymname,rs) ;
#endif
	    goto ret1 ; /* goto close the already loaded shared-object */
	}

/* continue */

	rs = uc_piper(pipes,3) ;
	fd = pipes[1] ;
	if (rs >= 0) {
	    int	rfd = pipes[0] ;
	    int (*callfunc)(int(*)(),int,const char **,const char **,void *) ;
	    int	(*cmdfunc)(int,const char **,void *) ;
	    const char	*stdnull = STDNULLFNAME ;
	    char	ifname[MAXNAMELEN+1] ;

	    callfunc = 
	        (int (*)(int(*)(),int,const char **,const char **,void *))
	        callsymp ;

	    cmdfunc = (int (*)(int,const char **,void *)) symp ;

	    rs = snsd(ifname,MAXNAMELEN,"*",rfd) ;

	    if (rs >= 0) rs = uc_fork() ;
	    if (rs == 0) { /* child */
	        int	ex ;
	        int	i ;

	        u_close(fd) ;

#ifdef	COMMENT
	        u_setsid() ;
#endif
	        uc_sigignore(SIGHUP) ;
	        uc_sigignore(SIGPIPE) ;

	        for (i = 0 ; i < 3 ; i += 1) u_close(i) ;
	        if ((rs = u_dup(rfd)) >= 0) {
	            int		size ;
	            const char	*nulldev = NULLFNAME ;
	            void	*p ;

	            uc_open(nulldev,O_WRONLY,0666) ;
	            uc_open(nulldev,O_WRONLY,0666) ;

	            size = (argc + 11) * sizeof(char *) ;
	            if ((rs = uc_malloc(size,&p)) >= 0) {
	                int		ac = 0 ;
	                const char	**av = (const char **) p ;

	                av[ac++] = cmdname ;
	                av[ac++] = "-if" ;
	                av[ac++] = ifname ;
	                av[ac++] = "-of" ;
	                av[ac++] = stdnull ;
	                av[ac++] = "-ef" ;
#if	CF_DEBUGS
	                av[ac++] = DSTDERRFNAME ;
	                av[ac++] = "-D=5" ;
#else
	                av[ac++] = stdnull ;
#endif

	                for (i = 1 ; i < argc ; i += 1)
	                    av[ac++] = argv[i] ;
	                av[ac] = NULL ;

	                ex = (*callfunc)(cmdfunc,ac,av,envv,NULL) ;
	                if (ex != EX_OK) rs = SR_IO ;

#if	CF_DEBUGS
	                debugprintf("opensvc_imail: callfun() ex=%u\n",ex) ;
#endif

			uc_free(p) ;
	            } else
	                ex = EX_TEMPFAIL ;

	        } else
	            ex = EX_NOINPUT ;

	        u_exit(ex) ;
	    } /* end if (child) */

	    u_close(rfd) ;
	    if (rs < 0) u_close(fd) ;
	} /* end if */

ret1:
	if ((sop != NULL) && (! isSpecialObject(sop))) dlclose(sop) ;

ret0:

#if	CF_DEBUGS
	debugprintf("opensvc_imail: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_imail) */


/* local subroutines */



