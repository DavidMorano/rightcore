/* opensvc_motdblurb */

/* LOCAL facility open-service (motdblurb) */


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

	Synopsis:

	int opensvc_motdblurb(pr,prn,of,om,argv,envv,to)
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

#include	<vsystem.h>
#include	<keyopt.h>
#include	<sysmemutil.h>
#include	<localmisc.h>

#include	"opensvc_motdblurb.h"
#include	"defs.h"


/* local defines */

#ifndef	LKCMDNAME
#define	LKCMDNAME	"liblkcmd"
#endif

#ifndef	CALLSYMNAME
#define	CALLSYMNAME	"lib_callfunc"
#endif

#ifndef	CMDNAME
#define	CMDNAME		"motdblurb"
#endif

#define	VARMOTDUSER	"MOTD_USERNAME"
#define	VARMOTDGROUP	"MOTD_GROUPNAME"
#define	VARMOTDUID	"MOTD_UID"
#define	VARMOTDGID	"MOTD_GID"

#define	STRBUFLEN	GROUPNAMELEN

#define	DSTDERRFNAME	"opensvc_motdblurb.err"
#define	NDF	"opensvc_motdblurb.deb"


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
extern int	getnodename(char *,int) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	attachso(cchar **,cchar *,cchar **,cchar **,int,void **) ;
extern int	nusers(const char *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	isSpecialObject(void *) ;
extern int	ndigmax(double *,int,int) ;

#if	CF_DEBUGS || CF_DEBUGN
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
extern int	nprintf(const char *,const char *,...) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int opensvc_motdblurb(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		argc = 0 ;
	int		ll = 0 ;
	int		pipes[2] ;
	int		fd = -1 ;
	const char	*strspec = NULL ;
	char		nodename[NODENAMELEN+1] ;
	char		strbuf[STRBUFLEN+1] ;
	char		lbuf[LINEBUFLEN+1] ;

#if	CF_DEBUGS && CF_DEBUGN
	nprintf(NDF,"opensvc_motdblurb: ent\n") ;
#endif

#if	CF_DEBUGS 
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	debugprintf("opensvc_motdblurb: argv=%p\n",argv) ;
#endif

	if (argv == NULL) {
	    rs = SR_FAULT ;
	    goto ret0 ;
	}

	for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;

#if	CF_DEBUGS
	{
		int	i ;
	debugprintf("opensvc_motdblurb: ent argc=%u\n",argc) ;
		for (i = 0 ; argv[i] != NULL ; i += 1)
	debugprintf("opensvc_motdblurb: argv[%u]=>%s<\n",i,argv[i]) ;
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
		gid_t	gid = 0 ;
		if ((gidp != NULL) && (gidp[0] != '\0')) {
		    int		v ;
		    rs = cfdeci(gidp,-1,&v) ;
		    gid = v ;
		} else
		    gid = getgid() ;
		if (rs >= 0) {
		    gn = gbuf ;
		    rs = getgroupname(gbuf,GROUPNAMELEN,gid) ;
		}
	    } /* end if (GID?) */
	    if ((rs >= 0) && (gn != NULL) && (gn[0] != '\0')) {
		strspec = strbuf ;
		rs = sncpyuc(strbuf,STRBUFLEN,gn) ;
	    }
	} /* end if (strspec) */

#if	CF_DEBUGS && CF_DEBUGN
	nprintf(NDF,"opensvc_motdblurb: strspec=%s\n",strspec) ;
#endif

	nodename[0] = '\0' ;
	if (rs >= 0) {
	    double	fla[3] ;
	    int		nuser ;
	    int		nproc = 0 ;
	    int		mu = 0 ;
	    const char	*fmt ;

	    if (rs >= 0) {
		rs = getnodename(nodename,NODENAMELEN) ;
	    }

#if	CF_DEBUGS && CF_DEBUGN
	    nprintf(NDF,"opensvc_motdblurb: getnodename() rs=%d nn=%s\n",
		rs,nodename) ;
#endif

	    if (rs >= 0) {
		rs = nusers(NULL) ;
		nuser = rs ;
	    }

#if	CF_DEBUGS && CF_DEBUGN
	    nprintf(NDF,"opensvc_motdblurb: nusers() rs=%d\n", rs) ;
#endif

	    if (rs >= 0) {
		if ((rs = uc_nprocs(0)) >= 0) {
		    nproc = rs ;
		} else if (isNotPresent(rs) || (rs == SR_NOSYS))
		    rs = SR_OK ;
	    }

#if	CF_DEBUGS && CF_DEBUGN
	    nprintf(NDF,"opensvc_motdblurb: nprocs() rs=%d\n", rs) ;
#endif

	    if (rs >= 0) {
	        if ((rs = sysmemutil(NULL)) >= 0) {
	            mu = rs ;
		} else if (rs == SR_NOSYS) {
		    rs = SR_OK ;
		    mu = 99 ;
		}
	    }

#if	CF_DEBUGS && CF_DEBUGN
	    nprintf(NDF,"opensvc_motdblurb: sysmemutil() rs=%d\n", rs) ;
#endif

	    if (rs >= 0) {
		rs = uc_getloadavg(fla,3) ;
	        ndigmax(fla,3,2) ;
	    }

#if	CF_DEBUGS && CF_DEBUGN
	    nprintf(NDF,"opensvc_motdblurb: uc_getloadavg() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        fmt = "%s %s users=%u procs=%u mem=%u%% "
		    "la=(%4.1f %4.1f %4.1f)\n" ;
	        rs = bufprintf(lbuf,llen,fmt,
		    strspec,nodename,nuser,nproc,mu,fla[0],fla[1],fla[2]) ;
	        ll = rs ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS && CF_DEBUGN
	    nprintf(NDF,"opensvc_motdblurb: mid  rs=%d ll=%d\n",rs,ll) ;
#endif

/* continue */

	if ((rs >= 0) && (ll > 0)) {
	    if ((rs = u_pipe(pipes)) >= 0) {
	        const int	wfd = pipes[1] ;
		{
	            fd = pipes[0] ;
	            rs = u_write(wfd,lbuf,ll) ;
		}
	        u_close(wfd) ;
	        if (rs < 0) u_close(fd) ;
	    } /* end if (pipe) */
	} /* end if (ok) */

ret0:

#if	CF_DEBUGS
#if	CF_DEBUGN
	nprintf(NDF,"opensvc_motdblurb: ret rs=%d fd=%u\n",rs,fd) ;
#endif
	debugprintf("opensvc_motdblurb: ret rs=%d fd=%u\n",rs,fd) ;
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_motdblurb) */


