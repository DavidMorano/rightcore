/* opensvc_pcsinfo */

/* PCS facility service (pcsinfo) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */


/* revision history:

	= 2003-11-04, David A­D­ Morano
	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather I should
	have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an open-facility-service module.  This subroutine (module)
	does tripple duty.  It serves the following service codes:
		- name
		- projinfo
		- pcsorg


	Synopsis:

	int opensvc_pcsinfo(pr,prn,of,om,argv,envv,to)
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
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<field.h>
#include	<sysusernames.h>
#include	<getxusername.h>
#include	<pcsns.h>
#include	<filebuf.h>
#include	<nulstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"opensvc_pcsinfo.h"
#include	"defs.h"
#include	"opensvcpcs.h"


/* local defines */

#define	VARMOTDUSER	"MOTD_USERNAME"
#define	VARMOTDUID	"MOTD_UID"

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	NDF		"opensvcname.deb"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	openshmtmp(char *,int,mode_t) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	getusername(char *,int,uid_t) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
extern int	nprintf(const char *,const char *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct subinfo_flags {
	uint		full:1 ;
	uint		all:1 ;
} ;

struct subinfo {
	SUBINFO_FL	f ;
	cchar		*pr ;
	cchar		**envv ;
	int		n ;
	int		pm ;
} ;


/* forward references */

static int opensvc_pcsinfo(cchar *,cchar *,int,mode_t,cchar **,cchar **,int) ;

static int subinfo_start(SUBINFO *,cchar *,cchar **) ;
static int subinfo_finish(SUBINFO *) ;


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"sn",
	"af",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_sn,
	argopt_af,
	argopt_overlast
} ;

static cchar	*submodes[] = {
	"name",
	"projinfo",
	"pcsorg",
	NULL
} ;

enum submodes {
	submode_name,
	submode_projinfo,
	submode_pcsorg,
	submode_overlast
} ;


/* exported subroutines */


int opensvc_name(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	return opensvc_pcsinfo(pr,prn,of,om,argv,envv,to) ;
}
/* end subroutine (opensvc_name) */


int opensvc_projinfo(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	return opensvc_pcsinfo(pr,prn,of,om,argv,envv,to) ;
}
/* end subroutine (opensvc_projinfo) */


int opensvc_pcsorg(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	return opensvc_pcsinfo(pr,prn,of,om,argv,envv,to) ;
}
/* end subroutine (opensvc_pcsorg) */


/* local subroutines */


/* ARGSUSED */
static int opensvc_pcsinfo(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	SUBINFO		si, *sip = &si ;
	ARGINFO		ainfo ;
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

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argz = NULL ;
	cchar		*argval = NULL ;
	cchar		*afname = NULL ;

#if	CF_DEBUGN
	nprintf(NDF,"opensvc_pcsinfo: ent\n") ;
#endif

#if	CF_DEBUGS 
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	if (argv != NULL) {
	    argz = argv[0] ;
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	}

	if (rs >= 0) rs = subinfo_start(sip,pr,envv) ;
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

/* argument-list name */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                afname = argp ;
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

/* call users */
	                    case 'a':
	                        sip->f.all = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                sip->f.all = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* type of name (regular or full) */
	                    case 'f':
	                        sip->f.full = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                sip->f.full = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					KEYOPT	*kop = &akopts ;
	                                rs = keyopt_loads(kop,argp,argl) ;
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

#if	CF_DEBUGN
	nprintf(NDF,"opensvc_logwelcome: ai_pos=%u ai_max=%u\n",
	    ai_pos,ai_max) ;
#endif

	if ((rs >= 0) && (argz != NULL)) {
	    int	mi ;
	    if ((mi = matstr(submodes,argz,-1)) >= 0) {
		sip->pm = mi ;
	    }
	}

	if ((rs >= 0) && (sip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    sip->n = rs ;
	}

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = openshmtmp(NULL,0,0664)) >= 0) {
	        SUBPCS		si ;
	        int		w = 0 ;
	        fd = rs ;
		switch (sip->pm) {
		case submode_name:
		    w = (sip->f.full) ? pcsnsreq_fullname : pcsnsreq_pcsname ;
		    break ;
		case submode_projinfo:
		    w = pcsnsreq_projinfo ;
		    break ;
		case submode_pcsorg:
		    w = pcsnsreq_pcsorg;
		    break ;
		} /* end switch */
	        if ((rs = subpcs_start(&si,pr,envv,w)) >= 0) {
		    FILEBUF	b ;
		    if ((rs = filebuf_start(&b,fd,0L,0,0)) >= 0) {
		        if (sip->f.all) {
			    rs = subpcs_all(&si,&b) ;
		        } else {
			    cchar	*afn = afname ;
			    rs = subpcs_args(&si,&b,&ainfo,&pargs,afn) ;
		        }
		        rs1 = filebuf_finish(&b) ;
		        if (rs >= 0) rs = rs1 ;
		    } /* end if (filebuf) */
	            rs1 = subpcs_finish(&si) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (subpcs) */
	        if (rs >= 0) {
		    rs = u_rewind(fd) ;
	        } else
	            u_close(fd) ;
	    } /* end if (tmp-file) */
	} /* end if (ok) */

	if (f_akopts) {
	    f_akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	subinfo_finish(sip) ;

badsubstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_pcsinfo) */


int subinfo_start(SUBINFO *sip,cchar *pr,cchar **ev)
{
	int		rs = SR_OK ;
	memset(sip,0,sizeof(SUBINFO)) ;
	sip->pr = pr ;
	sip->envv = ev ;
	return rs ;
}
/* end subroutine (subinfo_start) */


int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	return rs ;
}
/* end subroutine (subinfo_finish) */


