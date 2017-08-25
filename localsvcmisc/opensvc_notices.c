/* opensvc_notices */

/* LOCAL facility service (notices) */


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

	int opensvc_notices(pr,prn,of,om,argv,envv,to)
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


	= Output:

	mail u=dam n=3 note=>this is the day<


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<getxusername.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"opensvc_notices.h"
#include	"defs.h"


/* local defines */

#define	VARMOTDUSER	"MOTD_USERNAME"
#define	VARMOTDUID	"MOTD_UID"

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#define	NDEBFNAME	"opensvcnotices.deb"

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	statvfsdir(const char *,struct statvfs *) ;
extern int	getusername(char *,int,uid_t) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	localnoticecheck(cchar *,char *,int,cchar *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	isdigitlatin(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
extern int	nprintf(const char *,const char *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct subinfo_flags {
	uint		stores:1 ;
	uint		summary:1 ;
} ;

struct subinfo {
	SUBINFO_FL	f ;
	SUBINFO_FL	open ;
	vecstr		stores ;
	int		to ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *) ;
#if	CF_SETENTRY
static int	subinfo_setentry(SUBINFO *,cchar **,cchar *,int) ;
#endif

static int	isNoMail(int) ;


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"sn",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_sn,
	argopt_overlast
} ;

static const int	rschecks[] = {
	SR_OVERFLOW,
	SR_NOENT,
	SR_ACCESS,
	0
} ;


/* exported subroutines */


/* ARGSUSED */
int opensvc_notices(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	SUBINFO		si, *sip = &si ;
	BITS		pargs ;
	KEYOPT		akopts ;
	const int	ulen = USERNAMELEN ;
	const int	llen = LINEBUFLEN ;
	const int	flen = MAILADDRLEN ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		cols = COLUMNS ;
	int		argc = 0 ;
	int		pan = 0 ;
	int		pipes[2] ;
	int		fd = -1 ;
	int		ll = -1 ;
	int		nmsgs ;
	int		period = DEFPERIOD ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_akopts = FALSE ;
	int		f ;
	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*varcols ;
	const char	*un = NULL ;
	char		ubuf[USERNAMELEN+1] ;
	char		lbuf[LINEBUFLEN+1] ;
	char		fbuf[MAILADDRLEN+1] = { 0 } ;

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	}

	rs = subinfo_start(sip) ;
	if (rs < 0) goto badsubstart ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	f_akopts = (rs >= 0) ;

	ai = 0 ;
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
	                            prn = avp ;
	                    } else {
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            prn = argp ;
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

	                    case 'p':
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
				    int	v ;
	                            rs = cfdecti(argp,argl,&v) ;
				    period = v ;
				}
			    } else
	                        rs = SR_INVALID ;
	                        break ;


/* summary mode */
	                    case 's':
	                        sip->f.summary = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
					sip->f.summary = (rs > 0) ;
				    }
	                        }
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
	nprintf(NDEBFNAME,"opensvc_notices: ent\n") ;
#endif

#if	CF_DEBUGS 
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

/* find the number of allowable columns (for line-length limiting) */

	if ((varcols = getourenv(envv,VARCOLUMNS)) != NULL) {
	    int	rs1 ;
	    int	v ;
	    rs1 = cfdeci(varcols,-1,&v) ;
	    if (rs1 >= 0) cols = v ;
	}

/* find the username to act upon */

	if (argv != NULL) {
	    for (ai = 1 ; ai < argc ; ai += 1) {

	        f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	        f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	        if (! f) continue ;
    
	        switch (pan) {
	        case 0:
	            un = argv[ai] ;
		    break ;
	        } /* end switch */
	        pan += 1 ;
    
	    } /* end for (handling positional arguments) */
	} /* end if (have arguments) */

/* default user as necessary */

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_notices: 0 un=%s\n",un) ;
#endif

	if ((un == NULL) || (un[0] == '\0')) {
	     un = getourenv(envv,VARMOTDUSER) ;
	     if ((un == NULL) || (un[0] == '\0')) {
		const char	*uidp = getourenv(envv,VARMOTDUID) ;
		if ((uidp != NULL) && (uidp[0] != '\0')) {
		    int		v ;
		    uid_t	uid ;
		    if ((rs = cfdeci(uidp,-1,&v)) >= 0) {
		        uid = v ;
			un = ubuf ;
		        rs = getusername(ubuf,ulen,uid) ;
		    }
		}
	     }
	}
	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) un = "-" ;

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_notices: 1 rs=%d un=%s\n",rs,un) ;
#endif

/* extra step to get a real username (if we do not have one) */

	if ((rs >= 0) && (un[0] == '-')) {
	    un = ubuf ;
	    rs = getusername(ubuf,ulen,-1) ;
	}

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_notices: 2 rs=%d un=%s\n",rs,un) ;
#endif

/* process this user */

	if (rs >= 0) {
	    rs = localnoticecheck(pr,fbuf,flen,un,period) ;
	    nmsgs = rs ;
	}

#if	CF_DEBUGN
	nprintf(NDEBFNAME,"opensvc_notices: localnoticecheck() rs=%d\n",rs) ;
#endif

	if (isNoMail(rs)) {
	    nmsgs = 0 ;
	    fbuf[0] = '\0' ;
	}

/* assemble the line to print */

	if (rs >= 0) {
		const int	ul = strlen(un) ;
		const int	ph = (period / 3600) ;
	        const char	*fmt ;
	        if ((nmsgs > 0) && (! sip->f.summary)) {
	            fmt = "notifications u=%s p=%uh n=%u note=>%s<" ;
	        } else {
	            fmt = "notifications u=%s p=%uh n=%u" ;
		}
		fbuf[MIN((COLUMNS-27-ul),flen)] = '\0' ;
	        rs = bufprintf(lbuf,llen,fmt,un,ph,nmsgs,fbuf) ;
	        ll = rs ;
	} /* end if */

/* finish off line to print */

	if (rs >= 0) {
	    if (ll > cols) ll = cols ;
	    if (ll > 0) {
	        lbuf[ll++] = '\n' ;
	        lbuf[ll] = '\0' ;
	    }
	}

/* write it out */

	if (rs >= 0) {
	    if ((rs = u_pipe(pipes)) >= 0) {
	        int	wfd = pipes[1] ;
	        fd = pipes[0] ;

	        rs = u_write(wfd,lbuf,ll) ;

	        u_close(wfd) ;
	        if (rs < 0) u_close(fd) ;
	    } /* end if (u_pipe) */
	} /* end if (ok) */

badarg:
	if (f_akopts) {
	    f_akopts = FALSE ;
	    rs1 = keyopt_finish(&akopts) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = bits_finish(&pargs) ;
	if (rs >= 0) rs = rs1 ;

badpargs:
	rs1 = subinfo_finish(sip) ;
	if (rs >= 0) rs = rs1 ;

badsubstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_notices) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip)
{
	int		rs = SR_OK ;

	if (sip == NULL) return SR_FAULT ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->to = -1 ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip == NULL) return SR_FAULT ;

	if (sip->open.stores) {
	    sip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&sip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


#if	CF_SETENTRY
int subinfo_setentry(SUBINFO *sip,cchar **epp,cchar vp[],int vl)
{
	int		rs = SR_OK ;
	int		vnlen = 0 ;

	if (sip == NULL) return SR_FAULT ;

	if (epp == NULL) return SR_INVALID ;

	if (! sip->open.stores) {
	    rs = vecstr_start(&sip->stores,0,0) ;
	    sip->open.stores = (rs >= 0) ;
	}

/* find existing entry for later deletion */

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
	        oi = vecstr_findaddr(&sip->stores,*epp) ;
	    }
	    if (vp != NULL) {
	        vnlen = strnlen(vp,vl) ;
	        if ((rs = vecstr_add(&sip->stores,vp,vnlen)) >= 0) {
	            rs = vecstr_get(&sip->stores,rs,epp) ;
	        } /* end if (added new entry) */
	    } /* end if (had a new entry) */
	} /* end if */

/* delete the old entry if we had one */

	if ((rs >= 0) && (oi >= 0)) {
	    vecstr_del(&sip->stores,oi) ;
	}

	return (rs >= 0) ? vnlen : rs ;
}
/* end subroutine (subinfo_setentry) */
#endif /* CF_SETENTRY */


static int isNoMail(int rs)
{
	return isOneOf(rschecks,rs) ;
}
/* end subroutine (isNoMail) */


