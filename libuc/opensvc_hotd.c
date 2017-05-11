/* opensvc_hotd */

/* LOCAL facility open-service (hotd) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_DEBUGOUT	0		/* get debug output from program */
#define	CF_DEBUGEX	0		/* special debugging */


/* revision history:

	= 2003-11-04, David A­D­ Morano
	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather I should
	have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a facility-open-service module.

	Synopsis:

	int opensvc_hotd(pr,prn,of,om,argv,envv,to)
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

#include	<vsystem.h>
#include	<keyopt.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"opensvc_hotd.h"
#include	"defs.h"


/* local defines */

#ifndef	STDNULLFNAME
#define	STDNULLFNAME	"*STDNULL*"
#endif

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#ifndef	LKCMDNAME
#define	LKCMDNAME	"liblkcmd"
#endif

#ifndef	CMDNAME
#define	CMDNAME		"calyear"
#endif

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	DSTDERRFNAME	"opensvc_hotd.err"
#define	NDF		"opensvc_hotd.deb"
#define	NEX		"opensvc.deb"


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkfdfname(char *,int) ;
extern int	ctdeci(char *,int,int) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	attachso(cchar **,cchar *,cchar **,cchar **,int,void **) ;
extern int	isSpecialObject(void *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
extern int	nprintf(const char *,const char *,...) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
#endif

typedef int (*subhave_t)(const char *) ;
typedef int (*subcall_t)(const char *,int,const char **,const char **,void *) ;
typedef int (*subcalla_t)(const void *,int,const char **,const char **,void *) ;

struct subinfo {
	const char	*pr ;
	const char	**envv ;
	const char	*cmdname ;
	const char	*dayspec ;
	void		*sop ;
	subhave_t	subhave ;
	subcall_t	subcall ;
	subcalla_t	subcalla ;
	const void	*subcmd ;
} ;


/* forward references */

static int subinfo_start(SUBINFO *,cchar *,cchar **,cchar *,cchar *) ;
static int subinfo_attbegin(SUBINFO *) ;
static int subinfo_callprog(SUBINFO *) ;
static int subinfo_attend(SUBINFO *) ;
static int subinfo_finish(SUBINFO *) ;


/* local variables */

static const char	*syslibs[] = {
	"/usr/extra/lib",
	"/usr/preroot/lib",
	NULL
} ;

static const char	*attsyms[] = {
	"lib_proghave",
	"lib_progcall",
	"lib_progcalla",
	NULL
} ;


/* exported subroutines */


/* ARGSUSED */
int opensvc_hotd(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	SUBINFO		oi ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		argc = 0 ;
	int		fd = -1 ;
	const char	*cmdname = CMDNAME ;
	const char	*dayspec = NULL ;

#if	CF_DEBUGN
	nprintf(NDF,"opensvc_hotd: ent\n") ;
#endif

#if	CF_DEBUGS 
	debugprintf("opensvc_hotd: ent\n") ;
#endif /* CF_DEBUGS */

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	}

#if	CF_DEBUGS
	debugprintf("opensvc_hotd: argc=%u\n",argc) ;
#endif

/* figure out a day-specification (if there is one) */

	if ((argc >= 2) && (argv[1] != NULL)) {
	    dayspec = argv[1] ;
	}

	if (dayspec == NULL) dayspec = "+" ;

#if	CF_DEBUGS
	debugprintf("opensvc_hotd: dayspec=%s\n",dayspec) ;
#endif

	if ((rs = subinfo_start(&oi,pr,envv,cmdname,dayspec)) >= 0) {
	    if ((rs = subinfo_attbegin(&oi)) >= 0) {
		{
		    rs = subinfo_callprog(&oi) ;
		    fd = rs ;
		}
	    	rs1 = subinfo_attend(&oi) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (subinfo-att) */
	    rs1 = subinfo_finish(&oi) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */
	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

#if	CF_DEBUGS
	debugprintf("opensvc_hotd: ret rs=%d fd=%u\n",rs,fd) ;
#endif
#if	CF_DEBUGEX
	nprintf(NEX,"opensvc_hotd: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_hotd) */


/* local subroutines */


static int subinfo_start(SUBINFO *oip,cchar *pr,cchar **envv,
		cchar *cmdname,cchar *dayspec)
{
	memset(oip,0,sizeof(SUBINFO)) ;
	oip->pr = pr ;
	oip->envv = envv ;
	oip->cmdname = cmdname ;
	oip->dayspec = dayspec ;
	return SR_OK ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *oip)
{
	if (oip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_attbegin(SUBINFO *oip)
{
	const int	cslen = SYMNAMELEN ;
	int		rs ;
	const char	*pr = oip->pr ;
	const char	*cmdname = oip->cmdname ;
	char		csbuf[SYMNAMELEN+1] ;
	if ((rs = sncpy2(csbuf,cslen,"p_",cmdname)) >= 0) {
	    const int	dlmode = RTLD_LAZY ;
	    char	libdname[MAXPATHLEN+1] ;
	    if ((rs = mkpath2(libdname,pr,"lib")) >= 0) {
		const char	*ln = LKCMDNAME ;
	    	const char	*dnames[10] ; /* careful */
	    	const char	*syms[5] ; /* careful */
	        int		i = 0 ;
		int		j ;
		void		*n = NULL ;
		void		*sop ;
	        dnames[i++] = libdname ;
	        for (j = 0 ; syslibs[j] != NULL ; j += 1) {
		    dnames[i++] = syslibs[j] ;
	        }
	        dnames[i] = NULL ;
		for (i = 0 ; attsyms[i] != NULL ; i += 1) {
		    syms[i] = attsyms[i] ;
		}
	        syms[i++] = csbuf ;
	        syms[i] = NULL ;
#if	CF_DEBUGS
		debugprintf("opensvc_hotd/attbegin: ln=%s\n",ln) ;
		for (i = 0 ; dnames[i] != NULL ; i += 1) {
		    debugprintf("opensvc_hotd/attbegin: dname[%u]=%s\n",
			i,dnames[i]) ;
		}
		for (i = 0 ; syms[i] != NULL ; i += 1) {
		    debugprintf("opensvc_hotd/attbegin: sym[%u]=%s\n",
			i,syms[i]) ;
		}
#endif /* CF_DEBUGS */
	        if ((rs = attachso(dnames,ln,n,syms,dlmode,&sop)) >= 0) {
		    oip->sop = sop ;
	    	    oip->subhave = (subhave_t) dlsym(sop,attsyms[0]) ;
	    	    oip->subcall = (subcall_t) dlsym(sop,attsyms[1]) ;
	    	    oip->subcalla = (subcalla_t) dlsym(sop,attsyms[2]) ;
	    	    oip->subcmd = dlsym(sop,csbuf) ;
#if	CF_DEBUGS
	debugprintf("opensvc_hotd/attbegin: subcmd{%p}\n",oip->subcmd) ;
#endif
	        } /* end if (attachso) */
	    } /* end if (mkpath) */
	} /* end if (sncpy) */
#if	CF_DEBUGS
	debugprintf("opensvc_hotd/attbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (subinfo_attbegin) */


static int subinfo_attend(SUBINFO *oip)
{
	void		*sop = oip ->sop ;
	if ((sop != NULL) && (! isSpecialObject(sop))) dlclose(sop) ;
	return SR_OK ;
}
/* end subroutine (subinfo_attend) */


static int subinfo_callprog(SUBINFO *oip)
{
	int		rs ;
	int		fd = -1 ;
	if ((rs = opentmp(NULL,0,0664)) >= 0) {
	    const char	*ef = STDNULLFNAME ;
	    char	ofname[MAXNAMELEN+1] ;
	    fd = rs ;
	    if ((rs = mkfdfname(ofname,fd)) >= 0) {
		subcalla_t	subcalla = oip->subcalla ;
	    	int		ex ;
		int		ac = 0 ;
		const char	**envv = oip->envv ;
		const char	*cmdname = oip->cmdname ;
	    	    const char	*av[10] ; /* careful */
	            av[ac++] = cmdname ;
	            av[ac++] = "-ef" ;
#if	CF_DEBUGOUT
	            av[ac++] = DSTDERRFNAME ;
		    av[ac++] = "-D=5" ;
#else
	            av[ac++] = ef ;
#endif
	            av[ac++] = "-of" ;
	            av[ac++] = ofname ;
	            av[ac++] = "-o" ;
	            av[ac++] = "default" ;
		    av[ac++] = oip->dayspec ;
	            av[ac] = NULL ;
		    while (ac && (av[ac-1] == NULL)) ac -= 1 ;
#if	CF_DEBUGS
	debugprintf("opensvc_hotd/callprog: subcall() cmdname=%s\n",cmdname) ;
#endif
	            ex = (*subcalla)(oip->subcmd,ac,av,envv,NULL) ;
#if	CF_DEBUGS
	debugprintf("opensvc_hotd/callprog: subcall() ex=%u\n",ex) ;
#endif
#if	CF_DEBUGEX
		    nprintf(NEX,"opensvc_hotd: ex=%u\n",ex) ;
#endif
	            if (ex != EX_OK) rs = SR_IO ;
	    } /* end if */
	    if (rs >= 0) u_rewind(fd) ;
	    if (rs < 0) {
		u_close(fd) ;
		fd = -1 ;
	    }
	} /* end if (opentmp) */
#if	CF_DEBUGS
	debugprintf("opensvc_hotd/callprog: ret rs=%d\n",rs) ;
#endif
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (subinfo_callprog) */


