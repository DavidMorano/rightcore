/* opensvc_statmsg */

/* LOCAL facility open-service (statmsg) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGMASTER	0		/* pretend that we are master */
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

	int opensvc_statmsg(pr,prn,of,om,argv,envv,to)
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
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vechand.h>
#include	<statmsg.h>
#include	<localmisc.h>

#include	"opensvc_statmsg.h"
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

#define	VARMOTDADMIN	"MOTD_ADMIN"
#define	VARMOTDUSER	"MOTD_USERNAME"
#define	VARMOTDGROUP	"MOTD_GROUPNAME"
#define	VARMOTDUID	"MOTD_UID"
#define	VARMOTDGID	"MOTD_GID"
#define	VARMOTDADMINDNAME	"MOTD_ADMINDIR"

#define	VARISSUEADMIN	"ISSUE_ADMIN"
#define	VARKEYNAME	"ISSUE_KEYNAME"

#define	STRBUFLEN	GROUPNAMELEN

#define	DSTDERRFNAME	"opensvc_statmsg.err"
#define	NDEBFNAME	"opensvc_statmsg.deb"


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	ctdeci(char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	getnodename(char *,int) ;
extern int	getusername(char *,int,uid_t) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	attachso(cchar **,cchar *,cchar **,cchar **,int,void **) ;
extern int	nusers(const char *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	isdigitlatin(int) ;
extern int	isSpecialObject(void *) ;

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

static const char *argopts[] = {
	"ROOT",
	"sn",
	"ru",
	"pru",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_sn,
	argopt_ru,
	argopt_pru,
	argopt_overlast
} ;


/* exported subroutines */


int opensvc_statmsg(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	BITS		pargs ;
	KEYOPT		akopts ;
	VECHAND		adms ;
	STATMSG_ID	mid ;
	gid_t		gid = -1 ;
	uid_t		uid = -1 ;
	const int	am = (of & O_ACCMODE) ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs = SR_OK ;
	int		argc = 0 ;
	int		pan = 0 ;
	int		pipes[2] ;
	int		fd = -1 ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_akopts = FALSE ;
	int		f ;
	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*sn = "conslog" ;
	const char	*un = NULL ;
	const char	*gn = NULL ;
	const char	*pru = NULL ;
	const char	*kn = NULL ;
	const char	*cp ;
	char		ubuf[USERNAMELEN+1] ;
	char		gbuf[GROUPNAMELEN+1] ;
	char		admbuf[USERNAMELEN+1] ;
	char		keybuf[GROUPNAMELEN+1] ;

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
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl)
	                        pr = argp ;
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sn = argp ;
	                    }
	                    break ;

/* STAMSG program-root-user */
	                case argopt_ru:
	                case argopt_pru:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pru = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pru = argp ;
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

	                    case 'k':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            kn = argp ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'u':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            un = argp ;
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

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_statmsg: ai_pos=%u ai_max=%u\n",
		ai_pos,ai_max) ;
#endif

#if	CF_DEBUGS && CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_statmsg: ent\n") ;
#endif

#if	CF_DEBUGS && CF_DEBUGMASTER
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

/* check arguments */

	if ((am == O_WRONLY) || (am == O_RDWR)) {
	    rs = SR_BADF ;
	    goto badarg ;
	}

/* do we need a default argument? */

	if ((pru == NULL) || (pru[0] == '\0') || (pru[0] == '-')) {
	    pru = getourenv(envv,VARMOTDADMIN) ;
	    if ((pru == NULL) || (pru[0] == '\0'))
	        pru = getourenv(envv,VARISSUEADMIN) ;
	    if ((pru == NULL) || (pru[0] == '\0'))
		pru = prn ;
	} /* end if (pr-username) */
	if (rs < 0) goto ret0 ;

/* default user as necessary */

	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) {
	    un = getourenv(envv,VARMOTDUSER) ;
	    if ((un == NULL) || (un[0] == '\0')) {
	        const char	*uidp = getourenv(envv,VARMOTDUID) ;
	        if ((uidp != NULL) && (uidp[0] != '\0')) {
	            int		v ;
	            if ((rs = cfdeci(uidp,-1,&v)) >= 0) {
	                uid = v ;
	                un = ubuf ;
	                rs = getusername(ubuf,USERNAMELEN,uid) ;
	            }
	        }
	    }
	} /* end if (default username) */
	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) un = "-" ;

	if (uid < 0) uid = getuid() ;

/* default group as necessary */

	if ((rs >= 0) && ((gn == NULL) || (gn[0] == '\0') || (gn[0] == '-'))) {
	    gn = getourenv(envv,VARMOTDGROUP) ;
	    if ((gn == NULL) || (gn[0] == '\0')) {
		const char	*gidp = getourenv(envv,VARMOTDGID) ;
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
	} /* end if (default groupname) */
	if (rs < 0) goto ret0 ;

	if (gid < 0) gid = getgid() ;

/* default keyname as necessary */

	if ((rs >= 0) && (kn != NULL)) {
	    if (strcmp(kn,"%g") == 0) {
	        kn = gn ;
	    } else if (strcmp(kn,"%u") == 0)
	        kn = un ;
	}

	if ((rs >= 0) && ((kn == NULL) || (kn[0] == '\0') || (kn[0] == '-'))) {
	    if (kn == NULL) {
		kn = keybuf ;
	        keybuf[0] = '\0' ;
	    }
	} /* end if (keyname) */
	if (rs < 0) goto ret0 ;

/* what do we have so far? */

#if	CF_DEBUGS
	debugprintf("opensvc_statmsg: pru=%s\n",pru) ;
	debugprintf("opensvc_statmsg: un=%s\n",un) ;
	debugprintf("opensvc_statmsg: keyname=%s\n",kn) ;
#endif
#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_statmsg: pru=%s\n",pru) ;
	nprintf(NDEBFNAME,"opensvc_statmsg: un=%s\n",un) ;
	nprintf(NDEBFNAME,"opensvc_statmsg: keyname=%s\n",kn) ;
#endif

/* fill in our identification for STATMSG processing */

	memset(&mid,0,sizeof(STATMSG_ID)) ;
	mid.username = un ;
	mid.groupname = gn ;
	mid.uid = uid ;
	mid.gid = gid ;

/* continue */

	if (rs >= 0) {
	if ((rs = vechand_start(&adms,1,0)) >= 0) {

	    for (ai = 1 ; ai < argc ; ai += 1) {
	        f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	        f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
		    cp = argv[ai] ;
		    rs = vechand_add(&adms,cp) ;
		    pan += 1 ;
	        }
	        if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
	        const char	**av = NULL ;
	        if ((rs = vechand_getvec(&adms,&av)) >= 0) {

	            if ((rs = opentmp(NULL,0,0664)) >= 0) {
	                STATMSG	m ;
	                fd = rs ;

	                if ((rs = statmsg_open(&m,pru)) >= 0) {

	                    rs = statmsg_processid(&m,&mid,av,kn,fd) ;

		            statmsg_close(&m) ;
	                } /* end if (statmsg) */

	                if (rs >= 0) u_rewind(fd) ;
	                if (rs < 0) u_close(fd) ;
	            } /* end if */

	        } /* end if (vechand-getvec) */
	    } /* end if (good) */

	    vechand_finish(&adms) ;
	} /* end if (vechand-adms) */
	} /* end if (ok) */

badarg:
	if (f_akopts) {
	    f_akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
ret0:

#if	CF_DEBUGS
#if	CF_DEBUGN
	    nprintf(NDEBFNAME,"opensvc_statmsg: ret rs=%d fd=%u\n",rs,fd) ;
#endif
	debugprintf("opensvc_statmsg: ret rs=%d fd=%u\n",rs,fd) ;
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_statmsg) */


