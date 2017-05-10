/* main */

/* generic (pretty much) front end program subroutine */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_DEBUGN	0		/* special debugging */
#define	CF_KSHLOGID	0		/* KSH-support log-ID */
#define	CF_TMPDNAME	0		/* do not think we need this */


/* revision history:

	= 1994-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine forms the front-end part of a generic PCS type of
	program.  This front-end is used in a variety of PCS programs.

	This subroutine was originally part of the Personal Communication
	Services (PCS) package but can also be used independently from it.
	Historically, this was developed as part of an effort to maintain high
	function (and reliable) email communications in the face of
	increasingly draconian security restrictions imposed on the computers
	in the DEFINITY development organization.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<limits.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<getax.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"proglog.h"
#include	"proguserlist.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + (12 * MAXPATHLEN))
#endif

#define	NDF		"/tmp/mkpwi.deb"


/* external subroutines */

extern int	snsd(char *,int,cchar *,uint) ;
extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	snwcpy(char *,int,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	mkdirs(cchar *,mode_t) ;
extern int	mkuibang(char *,int,USERINFO *) ;
extern int	mkuiname(char *,int,USERINFO *) ;
extern int	pcsuserfile(cchar *,cchar *,cchar *,cchar *,cchar *) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,cchar *,cchar *) ;
extern int	getfname(cchar *,cchar *,int,char *) ;
extern int	getclustername(cchar *,char *,int,cchar *) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	getmailgid(cchar *,gid_t) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(bfile *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	progdb(PROGINFO *,bfile *,cchar *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;

static int	procdbname(PROGINFO *pip) ;
static int	procvardname(PROGINFO *,char *) ;
static int	proclogresult(PROGINFO *,int) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;

#if	CF_KSHLOGID
static int	procuserinfo_logid(PROGINFO *) ;
#endif

static int	procout_begin(PROGINFO *,cchar *) ;
static int	procout_end(PROGINFO *) ;


/* local variables */

static cchar *argopts[] = {
	"ROOT",
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"LOGFILE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"db",
	"pwfile",
	"indexfile",
	"pwifile",
	"ipwfile",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_tmpdir,
	argopt_version,
	argopt_verbose,
	argopt_logfile,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_db,
	argopt_pwfile,
	argopt_indexfile,
	argopt_pwifile,
	argopt_ipwfile,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ 0, 0 }
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*helpfname = NULL ;
	cchar		*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

#if	CF_DEBUGN
	nprintf(NDF,"starting argz=%s\n",argv[0]) ;
	if ((cp = getenv(VARDBNAME)) != NULL) {
	    nprintf(NDF,"dbname=%s\n",cp) ;
	}
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->daytime = time(NULL) ;

	pip->f.logprog = OPT_LOGPROG ;

/* start parsing the arguments */

#if	CF_DEBUGN
	nprintf(NDF,"args-start rs=%d\n",rs) ;
#endif

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

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
	        const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp + 1) ;

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

	                case argopt_tmpdir:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->tmpdname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->tmpdname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose mode */
	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
	                            pip->verboselevel = rs ;
	                        }
	                    }
	                    break ;

/* program root */
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

/* log file */
	                case argopt_logfile:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            pip->lfname = avp ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)  {
	                                pip->lfname = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
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

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                efname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ofname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->pwfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->pwfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_pwfile:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->pwfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->pwfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_indexfile:
	                case argopt_pwifile:
	                case argopt_ipwfile:
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            pip->dbname = avp ;
				}
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->dbname = argp ;
				    }
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

/* configuration file */
	                    case 'C':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->cfname = argp ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* debug */
	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->debuglevel = rs ;
	                            }
	                        }
	                        break ;

/* mutex lock PID file */
	                    case 'P':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->pidfname = argp ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* program-root */
	                    case 'R':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pr = argp ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* testing */
	                    case 'n':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                pip->n = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'p':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            pip->pwfname = avp ; /* zero-length OK */
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* file root */
	                    case 'r':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->fileroot = argp ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* default target log file size */
	                    case 'l':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					int	v ;
					rs = cfdecmfi(argp,argl,&v) ;
	                                pip->logsize = v ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* output file type */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->typespec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->verboselevel = rs ;
	                            }
	                        }
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

#if	CF_DEBUGN
	nprintf(NDF,"args-end rs=%d\n",rs) ;
#endif

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUGN
	nprintf(NDF,"file-err rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* set the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	}

	if (f_usage)
	    usage(pip) ;

/* help */

	if (f_help) {
	    if ((helpfname == NULL) || (helpfname[0] == '\0')) {
	        helpfname = HELPFNAME ;
	    }
	    printhelp(NULL,pip->pr,SEARCHNAME,helpfname) ;
	} /* end if (help) */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialize */

#if	CF_DEBUGN
	nprintf(NDF,"initialize rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (pip->dbname == NULL) pip->dbname = getenv(VARDBNAME) ;

#if	CF_DEBUGN
	nprintf(NDF,"dbname=%s\n",pip->dbname) ;
#endif

	if (pip->pidfname == NULL) pip->pidfname = getenv(VARPFNAME) ;
	if (pip->pidfname == NULL) pip->pidfname = getenv(VARPIDFNAME) ;

/* get some host-user information */

#if	CF_TMPDNAME
	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0')) {
	    if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	    if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;
	} /* end if (tmpdname) */
#endif /* CF_TMPDNAME */

/* continue */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
		    if ((rs = proglog_begin(pip,&u)) >= 0) {
			if ((rs = proguserlist_begin(pip)) >= 0) {
			    if ((rs = procout_begin(pip,ofname)) >= 0) {
				if ((rs = procdbname(pip)) >= 0) {
				    bfile	*ofp = pip->outfile ;
				    cchar	*dbn = pip->dbname ;
	    			    if ((rs = progdb(pip,ofp,dbn)) >= 0) {
					rs = proclogresult(pip,rs) ;
				    }
				}
			        rs1 = procout_end(pip) ;
			        if (rs >= 0) rs = rs1 ;
			    } /* end if (procout) */
			    rs1 = proguserlist_end(pip) ;
			    if (rs >= 0) rs = rs1 ;
			} /* end if (proguserlist) */
			rs1 = proglog_end(pip) ;
			if (rs >= 0) rs = rs1 ;
		    } /* end if (proglog) */
	            rs1 = procuserinfo_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: userinfo failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	    } /* end if (userinfo) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_rest: finishing rs=%d\n",rs) ;
#endif

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_INTR:
	        ex = EX_INTR ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} /* end if (error) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: done ex=%u (%d)\n",ex,rs) ;
#endif

retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo_finish ;
	    uc_mallout(&mo_finish) ;
	    debugprintf("main: final mallout=%u\n",(mo_finish-mo_start)) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad argument usage */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [-if <passwdfile>] [-db <dbfile>] [-ROOT <pr>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procdbname(PROGINFO *pip)
{
	int		rs = SR_OK ;
	cchar		*pn = pip->progname ;
	if (pip->dbname == NULL) {
	    const int	clen = NODENAMELEN ;
	    cchar	*nn = pip->nodename ;
	    char	cbuf[NODENAMELEN + 1] ;
	    if ((rs = getclustername(pip->pr,cbuf,clen,nn)) >= 0) {
		char	vbuf[MAXPATHLEN+1] ;
		if ((rs = procvardname(pip,vbuf)) >= 0) {
		    char	tbuf[MAXPATHLEN+1] ;
	    	    if ((rs = mkpath2(tbuf,vbuf,cbuf)) >= 0) {
			cchar	**vpp = &pip->dbname ;
			rs = proginfo_setentry(pip,vpp,tbuf,rs) ;
		    }
		} /* end if (procvardname) */
	    } /* end if (getclustername) */
	} /* end if (needed) */
	proglog_printf(pip,"dbname=%s (%d)",pip->dbname,rs) ;
	if (pip->debuglevel > 0) {
	    cchar	*fmt = "%s: dbname=%s\n" ;
	    bprintf(pip->efp,fmt,pn,pip->dbname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procdbname: ret rs=%d\n",rs) ;
	    debugprintf("main/procdbname: ret dbname=%s\n",pip->dbname) ;
	}
#endif

	return rs ;
}
/* end subroutine (procdbname) */


static int procvardname(PROGINFO *pip,char *dbuf)
{
	int		rs ;
	cchar		*pwi = PWIDNAME ;
	if ((rs = mkpath3(dbuf,pip->pr,PRVDNAME,pwi)) >= 0) {
	    USTAT	sb ;
	    if ((rs = uc_stat(dbuf,&sb)) >= 0) {
		const gid_t	gid = pip->gid_tools ;
		if ((pip->euid == sb.st_uid) && (sb.st_gid != gid)) {
		    const int	n = _PC_CHOWN_RESTRICTED ;
		    if ((rs = uc_pathconf(dbuf,n,NULL)) == 0) {
			rs = uc_chown(dbuf,-1,gid) ;
		    }
		}
	    } else if (isNotPresent(rs)) {
	        const mode_t	dm = 0775 ;
	        if ((rs = mkdirs(dbuf,dm)) >= 0) {
		    if ((rs = uc_minmod(dbuf,dm)) >= 0) {
		        const gid_t	gid = pip->gid_tools ;
		        const int	n = _PC_CHOWN_RESTRICTED ;
		        if ((rs = uc_pathconf(dbuf,n,NULL)) == 0) {
			    rs = uc_chown(dbuf,-1,gid) ;
		        }
		    }
		} /* end if (mkdirs) */
	    } /* end if (uc_stat) */
	} /* end if (mkpath) */
	return rs ;
}
/* end subroutine (procvardname) */


static int proclogresult(PROGINFO *pip,int res)
{
	int		rs = SR_OK ;
	cchar		*pn = pip->progname ;
	if ((res < 0) && (! pip->f.quiet)) {
	    bprintf(pip->efp,"%s: indexing failed (%d)\n",pn,res) ;
	}
	if ((pip->debuglevel > 0) && (rs >= 0)) {
	    bprintf(pip->efp,"%s: records processed=%u\n",pn,res) ;
	}
	if (pip->open.logprog) {
	    if (res >= 0) {
	        proglog_printf(pip,"records processed=%u\n",res) ;
	    } else {
	        proglog_printf(pip,"failed (%d)\n",res) ;
	    }
	}
	return rs ;
}
/* end subroutine (proclogresult) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->gecosname = uip->gecosname ;
	pip->realname = uip->realname ;
	pip->name = uip->name ;
	pip->fullname = uip->fullname ;
	pip->mailname = uip->mailname ;
	pip->org = uip->organization ;
	pip->logid = uip->logid ;
	pip->pid = uip->pid ;
	pip->uid = uip->uid ;
	pip->euid = uip->euid ;
	pip->gid = uip->gid ;
	pip->egid = uip->egid ;

	if (rs >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    cchar	*nn = pip->nodename ;
	    cchar	*dn = pip->domainname ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        cchar	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}

	if (rs >= 0) {
	    cchar	*gn = GROUPNAME_TOOLS ;
	    if ((rs = getmailgid(gn,GID_TOOLS)) >= 0) {
		pip->gid_tools = rs ;
	    }
	}

#if	CF_KSHLOGID
	if (rs >= 0) {
	    rs = procuserinfo_logid(pip) ;
	} /* end if (ok) */
#endif

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (procuserinfo_end) */


#if	CF_KSHLOGID
static int procuserinfo_logid(PROGINFO *pip)
{
	int		rs ;
	if ((rs = lib_runmode()) >= 0) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("procuserinfo_logid: rm=%08ß\n",rs) ;
#endif
	    if (rs & KSHLIB_RMKSH) {
	        if ((rs = lib_serial()) >= 0) {
	            const int	s = rs ;
	            const int	plen = LOGIDLEN ;
	            const int	pv = pip->pid ;
	            cchar	*nn = pip->nodename ;
	            char	pbuf[LOGIDLEN+1] ;
	            if ((rs = mkplogid(pbuf,plen,nn,pv)) >= 0) {
	                const int	slen = LOGIDLEN ;
	                char		sbuf[LOGIDLEN+1] ;
	                if ((rs = mksublogid(sbuf,slen,pbuf,s)) >= 0) {
	                    cchar	**vpp = &pip->logid ;
	                    rs = proginfo_setentry(pip,vpp,sbuf,rs) ;
	                }
	            }
	        } /* end if (lib_serial) */
	    } /* end if (runmode-KSH) */
	} /* end if (lib_runmode) */
	return rs ;
}
/* end subroutine (procuserinfo_logid) */
#endif /* CF_KSHLOGID */


static int procout_begin(PROGINFO *pip,cchar *ofn)
{
	int		rs = SR_OK ;
	if (pip->verboselevel > 1) {
	    const int	osize = sizeof(bfile) ;
	    void	*p ;
	    if ((ofn == NULL) || (ofn[0] == '-')) ofn = BFILE_STDOUT ;
	    if ((rs = uc_malloc(osize,&p)) >= 0) {
		bfile	*ofp = p ;
	        if ((rs = bopen(ofp,ofn,"wct",0666)) >= 0) {
	    	    pip->open.outfile = TRUE ;
		}
		if (rs < 0) {
		    uc_free(pip->outfile) ;
		    pip->outfile = NULL ;
		}
	    } /* end if (m-a) */
	} /* end if (verbosity) */
	return rs ;
}
/* end subroutine (procout_begin) */


static int procout_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if ((pip->outfile != NULL) && pip->f.outfile) {
	    bfile	*ofp = pip->outfile ;
	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->open.outfile = FALSE ;
	    rs1 = uc_free(pip->outfile) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->outfile = NULL ;
	}
	return rs ;
}
/* end subroutine (procout_end) */


