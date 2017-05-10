/* opensvc_issue */

/* LOCAL facility open-service (ISSUE) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */
#define	CF_SETENTRY	0		/* need |subinfo_setentry()| */
#define	CF_GETUSER	0		/* need |subinfo_getuser()| */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 2003-11-04, David A­D­ Morano

	This code was started by taking the corresponding code from the
	|opensvc_cotd(3opensvc)| module.


*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	This is an open-facility-service module.

	Filename:

		<pr>§issue[­-k­<keyname>][­<admin(s)>]

	Synopsis:

	int opensvc_issue(pr,prn,of,om,argv,envv,to)
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
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<getxusername.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<issue.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"opensvc_issue.h"
#include	"defs.h"


/* local defines */

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#ifndef	ARGINFO
#define	ARGINFO		struct arginfo
#endif

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#define	NDEBFNAME	"opensvc_issue.deb"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	localgetorg(const char *,char *,int,const char *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct subinfo_flags {
	uint		stores:1 ;
	uint		blocks:1 ;
} ;

struct subinfo {
	SUBINFO_FL	have, f, changed, final ;
	SUBINFO_FL	open ;
	vecstr		stores ;
	const char	*pr ;
	const char	*un ;
	const char	*groupname ;
	const char	*kn ;
	uid_t		uid ;
	gid_t		gid ;
	int		linelen ;
	int		to ;
} ;


/* forward references */

static int	npargs(ARGINFO *,BITS *) ;
static int	loadadmins(ARGINFO *,BITS *,const char **) ;

static int	subinfo_start(SUBINFO *,const char *) ;
static int	subinfo_finish(SUBINFO *) ;

#if	CF_SETENTRY
static int	subinfo_setentry(SUBINFO *,const char **,
			const char *,int) ;
#endif

#if	CF_GETUSER
static int	subinfo_getuser(SUBINFO *,const char *) ;
#endif

static int	subinfo_openissue(SUBINFO *,const char *,const char **) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"sn",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_sn,
	argopt_overlast
} ;


/* exported subroutines */


/* ARGSUSED */
int opensvc_issue(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	SUBINFO	si, *sip = &si ;
	ARGINFO	ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
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
	const char	*un = NULL ;
	const char	*kn = NULL ;

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	}

	rs = subinfo_start(sip,pr) ;
	if (rs < 0) goto badsubstart ;

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

/* key-name */
	                    case 'k':
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            kn = argp ;
	                        }
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

/* alternate username */
	                    case 'u':
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            un = argp ;
			    } else
	                        rs = SR_INVALID ;
	                        }
	                        break ;

/* line width */
	                    case 'w':
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = optvalue(argp,argl) ;
	                            sip->linelen = rs ;
	                        }
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

	if ((rs >= 0) && ((kn == NULL) || (kn[0] == '\0'))) kn = "-" ;

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = npargs(&ainfo,&pargs)) >= 0) {
	        const int	size = ((rs+1)*sizeof(const char **)) ;
	        void	*p ;
	        if ((rs = uc_malloc(size,&p)) >= 0) {
	            const char	**admins = p ;
	            if (admins[0] == NULL) admins = NULL ;
	            if ((rs = loadadmins(&ainfo,&pargs,admins)) >= 0) {
	                if ((rs = subinfo_openissue(sip,kn,admins)) >= 0) {
	                    fd = rs ;
	                }
	            } /* end if (loadadmins) */
	            uc_free(p) ;
	        } /* end if (memory-allocation) */
	    } /* end if (npargs) */
	} /* end if (ok) */

/* done */
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

	if ((rs < 0) && (fd >= 0)) u_close(fd) ;

badsubstart:
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_issue) */


/* local subroutines */


static int npargs(ARGINFO *aip,BITS *bop)
{
	int		rs = SR_OK ;
	int		ai ;
	int		n = 0 ;
	int		f ;
	for (ai = 1 ; ai < aip->argc ; ai += 1) {
	    f = (ai <= aip->ai_max) && ((rs = bits_test(bop,ai)) > 0) ;
	    f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	    if (f) {
	        const char	*cp = aip->argv[ai] ;
	        if (cp[0] != '\0') n += 1 ;
	    }
	    if (rs < 0) break ;
	} /* end for (counting positional arguments) */
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (npargs) */


static int loadadmins(ARGINFO *aip,BITS *bop,const char **admins)
{
	int		rs = SR_OK ;
	int		ai ;
	int		n = 0 ;
	int		f ;
	for (ai = 1 ; ai < aip->argc ; ai += 1) {
	    f = (ai <= aip->ai_max) && ((rs = bits_test(bop,ai)) > 0) ;
	    f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	    if (f) {
	        const char	*cp = aip->argv[ai] ;
	        if (cp[0] != '\0') admins[n++] = aip->argv[ai] ;
	    }
	    if (rs < 0) break ;
	} /* end for (loading positional arguments) */
	admins[n] = NULL ;
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (loadadmins) */


static int subinfo_start(SUBINFO *sip,cchar *pr)
{
	int		rs = SR_OK ;

	if (sip == NULL) return SR_FAULT ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->pr = pr ;
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
static int subinfo_setentry(SUBINFO *sip,cchar **epp,char *vp,int vl)
{
	int		rs = SR_OK ;
	int		len = 0 ;

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
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(&sip->stores,vp,len,epp) ;
	    } /* end if (added new entry) */
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(&sip->stores,oi) ;
	    }
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_setentry) */
#endif /* CF_SETENTRY */


#if	CF_GETUSER
static int subinfo_getuser(SUBINFO *sip,const char *un)
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	char		*pwbuf ;

	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	if ((un != NULL) && (un[0] != '\0')) {
	    sip->un = un ;
	    rs = GETPW_NAME(&pw,pwbuf,pwlen,un) ;
	} else {
	    rs = getpwusername(&pw,pwbuf,pwlen,-1) ;
	}
	if (rs >= 0) {
	    const char	**vpp ;
	    sip->uid = pw.pw_uid ;
	    sip->gid = pw.pw_gid ;
	    if (sip->un == NULL) {
	        vpp = &sip->un ;
	        rs = subinfo_setentry(sip,vpp,pw.pw_name,-1) ;
	    }
	    if (rs >= 0) {
	        const int	gnlen = GROUPNAMELEN ;
	        char		gnbuf[GROUPNAMELEN+1] ;
	        if ((rs = getgroupname(gnbuf,gnlen,pw.pw_gid)) >= 0) {
	            vpp = &sip->groupname ;
	            rs = subinfo_setentry(sip,vpp,gnbuf,rs) ;
	        } /* end if (getgroupname) */
	    } /* end if (ok) */
	} /* end if (got user PW information) */
	    uc_free(pwbuf) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (subinfo_getuser) */
#endif /* CF_GETUSER */


static int subinfo_openissue(SUBINFO *sip,cchar *kn,cchar **admins)
{
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;
	const char	*pr = sip->pr ;

	if ((rs = opentmp(NULL,0,0664)) >= 0) {
	    ISSUE	m ;
	    fd = rs ;

	    if ((rs = issue_open(&m,pr)) >= 0) {

	        rs = issue_process(&m,kn,admins,fd) ;

	        issue_close(&m) ;
	    } /* end if (issue) */

	    if (rs >= 0) u_rewind(fd) ;
	    if (rs < 0) u_close(fd) ;
	} /* end if (opentmp) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (subinfo_openissue) */


