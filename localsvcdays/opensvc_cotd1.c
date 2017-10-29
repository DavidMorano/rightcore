/* opensvc_cotd */

/* LOCAL facility open-service (cotd) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-11-04, David Morano

	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather
	I should have started this code by using the corresponding UUX
	dialer module.


*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a facility-open-service module.

	Synopsis:

	int opensvc_cotd(pr,prn,of,om,argv,envv,to)
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
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<dlfcn.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<dayspec.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"opensvc_cotd.h"
#include	"defs.h"


/* local defines */

#ifndef	STDNULLFNAME
#define	STDNULLFNAME	"*STDNULL*"
#endif

#ifndef	LKCMDNAME
#define	LKCMDNAME	"liblkcmd"
#endif

#ifndef	CALLSYMNAME
#define	CALLSYMNAME	"lib_callcmd"
#endif

#ifndef	CMDNAME
#define	CMDNAME		"commandment"
#endif

#define	DSTDERRFNAME	"opensvc_cotd.err"


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkfdfname(char *,int) ;
extern int	ctdeci(char *,int,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
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


/* exported subroutines */


int opensvc_cotd(pr,prn,of,om,argv,envv,to)
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
	int	fd = -1 ;
	int	cmdnum = -1 ;

	const char	*callsymname = CALLSYMNAME ;
	const char	*cmdname = CMDNAME ;

	char	cmdsymname[MAXNAMELEN+1] ;
	char	daybuf[DIGBUFLEN+2] ;

	const void	*symp = NULL ;
	const void	*callsymp = NULL ;
	void		*sop = RTLD_DEFAULT ;


#if	CF_DEBUGS 
	{
	    const char	*cp ;
	    cp = getourenv(envv,VARDEBUGFNAME) ;
	    if (cp != NULL) {
	        int dfd = debugopen(cp) ;
	        debugprintf("opensvc_cotd: starting DFD=%u\n",dfd) ;
	    }
	} /* end block */
#endif /* CF_DEBUGS */

	if (argv != NULL)
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;

/* get a command-number */

	if ((argc >= 2) && (argv[1] != NULL)) {
	    DAYSPEC	d ;
	    rs = dayspec_load(&d,argv[1],-1) ;
	    if (rs >= 0) {
	        cmdnum = (d.d % 10) ;
	        if (cmdnum == 0) cmdnum = 10 ;
	    }
	}
#if	CF_DEBUGS
	debugprintf("opensvc_cotd: spec rs=%d\n",rs) ;
#endif
	if (rs < 0) goto ret0 ;

/* formulate the argument to the called command */

	if (cmdnum >= 0) {
	    rs = ctdeci(daybuf,DIGBUFLEN,cmdnum) ;
	} else {
	    daybuf[0] = '+' ;
	    daybuf[1] = '\0' ;
	}
#if	CF_DEBUGS
	debugprintf("opensvc_cotd: ctdeci() rs=%d\n",rs) ;
#endif
	if (rs < 0) goto ret0 ;

/* create the symbol name for the function call */

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
	nprintf(NDEBFNAME,"opensvc_loginblurb: attachso() rs=%d\n",rs) ;
#endif
#if	CF_DEBUGS
	debugprintf("opensvc_loginblurb: attachso() rs=%d sop=%p\n",rs,sop) ;
#endif

	if (rs < 0) goto ret0 ;

/* find some symbols */

	if (rs >= 0) {
	    callsymp = dlsym(sop,callsymname) ;
	    if (callsymp == NULL) rs = SR_LIBACC ;
	}

	if (rs >= 0) {
	    symp = dlsym(sop,cmdsymname) ;
	    if (symp == NULL) rs = SR_LIBACC ;
	}

	if (rs < 0) goto ret1 ;

/* continue */

	rs = opentmp(NULL,O_RDWR,0664) ;
	fd = rs ;

#if	CF_DEBUGS
	debugprintf("opensvc_cotd: opentmp() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    int	ex ;
	    int (*callfunc)(int(*)(),int,const char **,const char **,void *) ;
	    int	(*cmdfunc)(int,const char **,void *) ;
	    const char	*ef = STDNULLFNAME ;
	    char	ofname[MAXNAMELEN+1] ;

	    callfunc = 
		(int (*)(int(*)(),int,const char **,const char **,void *)) 
		callsymp ;

	    cmdfunc = (int (*)(int,const char **,void *)) symp ;

	    if ((rs = mkfdfname(ofname,fd)) >= 0) {
		    int		ac = 0 ;
	    	    const char	*av[8] ; /* careful! */

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
	            av[ac++] = daybuf ;
	            av[ac] = NULL ;

#if	CF_DEBUGS
	debugprintf("opensvc_cotd: callfunc() \n") ;
#endif

	            ex = (*callfunc)(cmdfunc,ac,av,envv,NULL) ;
	            if (ex != EX_OK) rs = SR_IO ;

#if	CF_DEBUGS
	debugprintf("opensvc_cotd: callfunc() ex=%u\n",ex) ;
#endif

	    } /* end if */

	    if (rs >= 0) u_rewind(fd) ;

	    if (rs < 0) u_close(fd) ;
	} /* end if */

ret1:
	if ((sop != NULL) && (! isSpecialObject(sop))) dlclose(sop) ;

ret0:
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_cotd) */



