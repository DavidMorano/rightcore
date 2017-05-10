/* opensvc_imail */

/* LOCAL facility open-service (imail) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */


/* revision history:

	= 2003-11-04, David A­D­ Morano
	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather I should
	have started this code by using the corresponding UUX dialer module.

	= 2014-02-19, David A­D­ Morano
        I rewrote this after all of these years, because ... the word is that on
        systems other than Solaris® many common lib-C 'man(3c)' subroutines --
        like 'malloc(3c)' for example -- are not fork-safe. Yes, friends, I have
        been very much spoiled by the often-times far superior OS environment
        provided by Solaris® as compared with other UNIXi® out there. So I have
        rewritten this whole thing to avoid any real computation after the
        "fork" and before the "exec" so that we can now be considered a happy
        playmate with the rest of the OS world. We now (so-called) spawn the
        IMAIL program rather than simply executing it (yes) as a subroutines.
        This has the (bad) overhead associated with a fork-exec combination, but
        it has the advantages that a new address-space environment is set up
        without the possibility of inheriting stale held locks from the parent
        address space.

	Don't hold your breathe that other UNIXi® besides Solaris® will be
	cleaning up their lib-C implementations to make them fork-safe any time
	soon.  It is just not going to happen.  If we want our code to run on
	platforms other than Solaris® then we have to lower ourselves to the
	lowest common denominator in OS technology (rather than the highest as
	was Solaris®).

*/

/* Copyright © 2003,2014 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

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
	shared-orject ourselves when we could have let the run-time linker do
	it for us?  The reason that we do it for ourselves is that in this way
	the shared-object that this subroutine is a part of does not need to
	specify the LIBLKCMD shared-object as a dependency.  By not making
	another dependency of the the shared-object that this subroutine is a
	part of we dramatically reduce the run-time linker work done whenever
	this (the current) shared-object is loaded.  Also, our own search for
	LIBLKCMD is a little bit faster than that of the run-time linker.  A
	local search of something like a LIBLKCMD shared-object should be
	performed whenever we have mixed subroutines made a part of the same
	shared-object.  That is: subroutines that need LIBLKCMD (or something
	like it) and those that do not.  In this way the subroutines that do
	not need complex object such as LIBLKCMD do not have to suffer the
	pretty great cost of loading it by the run-time linker.  Also, note
	that the fact that a shared-object such as LIBLKCMD is *already*
	attached to the parent object is *not* determined until after the
	run-time linker searches all of the myriad directories in the
	LD_LIBRARY_PATH.  This just adds to the unnecessary filesystem searches
	for subroutines that do not require something like LIBLKCMD.

	Besides the subtleties of using the run-time linker in an
	indiscriminate manner, trying to invoke a function like LOGINBLURB by
	loading the whole shell-builtin command by the same name seems to be a
	huge waste of time as compared with invoking some simple subroutine
	that mades the same output as the LOGINBLURB shell-builtin does!  What
	we sometimes do to use an existing piece of some code is often quite
	amazingly complex!


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
#include	<bits.h>
#include	<keyopt.h>
#include	<sigblock.h>
#include	<filebuf.h>
#include	<envhelp.h>
#include	<spawnproc.h>
#include	<localmisc.h>

#include	"opensvc_imail.h"
#include	"defs.h"


/* local defines */

#ifndef	STDNULLFNAME
#define	STDNULLFNAME	"*STDNULL*"
#endif

#ifndef	VARIMAILPR
#define	VARIMAILPR	"IMAIL_PROGRAMROOT"
#endif

#ifndef	VARIMAILSN
#define	VARIMAILSN	"IMAIL_SEARCHNAME"
#endif

#ifndef	VARIMAILOPTS
#define	VARIMAILOPTS	"IMAIL_OPTS"
#endif

#ifndef	VARIMAILAF
#define	VARIMAILAF	"IMAIL_AF"
#endif

#ifndef	VARIMAILEF
#define	VARIMAILEF	"IMAIL_EF"
#endif

#ifndef	VARIMAILIF
#define	VARIMAILIF	"IMAIL_IF"
#endif

#ifndef	CMDNAME
#define	CMDNAME		"IMAIL"
#endif

#define	SVCNAME		"imail"

#define	NDEBFNAME	"opensvc_imail.deb"

#define	DSTDERRFNAME	"imail.stderr"


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkfdfname(char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	ctdeci(char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	prgetprogpath(const char *,char *,const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isSpecialObject(void *) ;

#if	CF_DEBUGS || CF_DEBUGN
extern int	nprintf(const char *,const char *,...) ;
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	writeargs(int,BITS *,int,int,const char **) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"sn",
	"svc",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_sn,
	argopt_svc,
	argopt_overlast
} ;

static const char	*envbads[] = {
	VARIMAILPR,
	VARIMAILSN,
	VARIMAILOPTS,
	VARIMAILAF,
	VARIMAILEF,
	"PROGRAMROOT",
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
	SIGBLOCK	b ;
	BITS		pargs ;
	KEYOPT		akopts ;
	ENVHELP		eh ;
	const int	am = (of & O_ACCMODE) ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		argc = 0 ;
	int		fd = -1 ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_akopts = FALSE ;
	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*sn = NULL ;
	const char	*svc = NULL ;
	const char	*cmdname = CMDNAME ;
	const char	*execfname ;
	const char	**ev ;
	char		progfname[MAXPATHLEN+1] ;

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	}

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	f_akopts = (rs >= 0) ;

	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
	        const int ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp+1) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                f_optequal = TRUE ;
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	            } else {
	                avp = NULL ;
	                avl = 0 ;
	                akl = aol ;
	            }

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program-root */
	                case argopt_root:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sn = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* program service-name */
	                case argopt_svc:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            svc = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                svc = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

/* program-root */
	                    case 'R':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                rs = keyopt_loads(&akopts,argp,argl) ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (rs < 0) goto badarg ;

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

#if	CF_DEBUGS
	{
	    int	i ;
	    debugprintf("opensvc_imail: entered argc=%u\n",argc) ;
	    for (i = 0 ; argv[i] != NULL ; i += 1)
	        debugprintf("opensvc_imail: argv[%u]=>%s<\n",i,argv[i]) ;
	}
#endif

	if ((am == O_RDWR) || (am == O_RDONLY)) {
	    rs = SR_BADF ;
	    goto badacc ;
	}

	if (sn == NULL) sn = SEARCHNAME ;

	if (svc == NULL) {
	    if (argc > 0) svc = argv[0] ;
	}
	if (svc == NULL) svc = SVCNAME ;

	execfname = progfname ;
	rs = prgetprogpath(pr,progfname,svc,-1) ;
	if (rs == 0) execfname = svc ;

#if	CF_DEBUGS
	debugprintf("opensvc_imail: prgetprogfname() rs=%d\n",rs) ;
	debugprintf("opensvc_imail: execfname=%s\n",execfname) ;
#endif

	if (rs >= 0) {
	    if ((rs = sigblock_start(&b,NULL)) >= 0) {
	        const char	*template = "/tmp/opensvcXXXXXXX" ;
	        char		afname[MAXPATHLEN+1] ;

	        if ((rs = opentmpfile(template,O_RDWR,0666,afname)) >= 0) {
	            const char	**eb = envbads ;
	            int		afd = rs ;

	            if ((rs = writeargs(afd,&pargs,ai_pos,argc,argv)) >= 0) {
	                if ((rs = envhelp_start(&eh,eb,envv)) >= 0) {

	                    if (rs >= 0) {
	                        const char	*kp = VARIMAILPR ;
	                        rs = envhelp_envset(&eh,kp,pr,-1) ;
	                    }
	                    if (rs >= 0) {
	                        const char	*kp = VARIMAILSN ;
	                        rs = envhelp_envset(&eh,kp,svc,-1) ;
	                    }
	                    if (rs >= 0) {
	                        const char	*kp = VARIMAILOPTS ;
	                        rs = envhelp_envset(&eh,kp,"ignore",-1) ;
	                    }
	                    if (rs >= 0) {
	                        const char	*kp = VARIMAILEF ;
#if	CF_DEBUGS
	                        const char	*vp = DSTDERRFNAME ;
#else
	                        const char	*vp = STDNULLFNAME ;
#endif
	                        rs = envhelp_envset(&eh,kp,vp,-1) ;
	                    }
	                    if (rs >= 0) {
	                        cchar	*kp = VARIMAILAF ;
	                        char	afbuf[USERNAMELEN+1] ;
	                        if ((rs = mkfdfname(afbuf,afd)) >= 0) {
	                            rs = envhelp_envset(&eh,kp,afbuf,-1) ;
	                        }
	                    }

	                    if (rs >= 0) {
	                        if ((rs = envhelp_getvec(&eh,&ev)) >= 0) {
	                            SPAWNPROC	ps ;
	                            int		ac = 0 ;
	                            const char	*av[4] ;

	                            av[ac++] = cmdname ;
#if	CF_DEBUGS
	                            av[ac++] = "-D=5" ;
#endif
	                            av[ac] = NULL ;

	                            memset(&ps,0,sizeof(SPAWNPROC)) ;
	                            ps.opts |= SPAWNPROC_OIGNINTR ;
	                            ps.opts |= SPAWNPROC_OSETPGRP ;
	                            ps.disp[0] = SPAWNPROC_DCREATE ;
	                            ps.disp[1] = SPAWNPROC_DCLOSE ;
	                            ps.disp[2] = SPAWNPROC_DCLOSE ;

	                            rs = spawnproc(&ps,execfname,av,ev) ;
	                            fd = ps.fd[0] ;

#if	CF_DEBUGS
	                            debugprintf("opensvc_imail: "
	                                "spawnproc() rs=%d\n", rs) ;
#endif

	                        } /* end if (envhelp_getvec) */
	                    } /* end if (ok) */

	                    rs1 = envhelp_finish(&eh) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (envhelp) */
	            } /* end if (writeargs) */

	            u_close(afd) ;
	            uc_unlink(afname) ;
	        } /* end if (opentmpfile) */

	        rs1 = sigblock_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sigblock) */
	} /* end if (ok) */

badacc:
badarg:
	if (f_akopts) {
	    f_akopts = FALSE ;
	    rs1 = keyopt_finish(&akopts) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = bits_finish(&pargs) ;
	if (rs >= 0) rs = rs1 ;

	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

badpargs:

#if	CF_DEBUGS
	debugprintf("opensvc_imail: ret rs=%d fd=%u\n",rs,fd) ;
	debugclose() ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_imail) */


/* local subroutines */


static int writeargs(int afd,BITS *blp,int ai_pos,int argc,cchar **argv)
{
	FILEBUF		b ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

	if ((rs = filebuf_start(&b,afd,0L,512,0)) >= 0) {
	    int		ai ;
	    int		f ;
	    const char	*ap ;

	    for (ai = 1 ; ai < argc ; ai += 1) {
	        f = (bits_test(blp,ai) > 0) ;
	        f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            ap = argv[ai] ;
	            if (ap[0] != '-') {
	                n += 1 ;
	                rs = filebuf_print(&b,ap,-1) ;
	            }
	        }
	    } /* end for */

	    rs1 = filebuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */

	if (rs >= 0) u_rewind(afd) ;

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (writeargs) */


