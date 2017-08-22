/* main */

/* main subroutine for the MKTAGPRINT program */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	1		/* run-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This is the 'main' module for the MKTAGPRINT program.  This
	program reads tags and prints out the associated original text
	with possibly some minor formatting.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<varsub.h>
#include	<field.h>
#include	<ascii.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"configfile.h"


/* local defines */


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int,int *) ;
extern int	mkuibang(char *,int,USERINFO *) ;
extern int	mkuiname(char *,int,USERINFO *) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			const char *,const char *) ;
extern int	isprintlatin(int) ;

extern int	expander() ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	mktagprint(struct proginfo *,struct arginfo *,
			const char *,const char *,const char *) ;

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* externals variables */

extern char	makedate[] ;


/* forward references */

static int	usage(struct proginfo *) ;


/* local structures */


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"CONFIG",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"LOGFILE",
	"HELP",
	"pm",
	"sn",
	"db",
	"af",
	"ef",
	"of",
	"if",
	"lf",
	"dn",
	"fn",
	"basedir",
	"tablen",
	"cookie",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_config,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_logfile,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_db,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_lf,
	argopt_dn,
	argopt_fn,
	argopt_basedir,
	argopt_tablen,
	argopt_cookie,
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

static const char	*progmodes[] = {
	"mkkey",
	"mkinv",
	"mkquery",
	"mkanalysis",
	NULL
} ;

enum progmodes {
	progmode_mkkey,
	progmode_mkinv,
	progmode_mkquery,
	progmode_mkanalysis,
	progmode_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	struct arginfo	ainfo ;

	USERINFO	u ;

	bfile		errfile ;
	bfile		eigenfile, *ifp = &eigenfile ;

	varsub	vsh_e, vsh_d ;

	time_t	daytime ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai ;
	int	pan ;
	int	rs, rs1 ;
	int	i, len ;
	int	c ;
	int	v ;
	int	loglen = -1 ;
	int	len1, len2 ;
	int	sl, cl ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*logdname = LOGDNAME ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*pmspec = NULL ;
	const char	*configfname = NULL ;
	const char	*helpfname = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*lfname = NULL ;
	const char	*basedname = NULL ;
	const char	*dbname = NULL ;
	const char	*outfmt = NULL ;
	const char	*ignorechars = IGNORECHARS ;
	const char	*delimiter = " " ;
	const char	*dn = NULL ;
	const char	*fn = NULL ;
	const char	*sp, *cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

	memset(&ainfo,0,sizeof(struct arginfo)) ;

	ainfo.argv = argv ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	} 

/* initialize */

	pip->verboselevel = 1 ;
	pip->minwordlen = -2 ;
	pip->maxwordlen = -2 ;
	pip->eigenwords = -2 ;
	pip->maxkeys = -2 ;

	pip->f.optoutcookie = TRUE ;	/* default */

/* start parsing the arguments */

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1) 
		ainfo.argpresent[ai] = 0 ;

	ai = 0 ;
	ainfo.ai_max = 0 ;
	ainfo.ai_pos = 0 ;
	ainfo.argc = argc ;
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

	        if (isdigit(argp[1])) {

		    argval = (argp + 1) ;

	        } else if (argp[1] == '-') {

	            ainfo.ai_pos = ai ;
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

/* do we have a keyword match or should we assume only key letters? */

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                    switch (kwi) {

/* configuration file */
	                    case argopt_config:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                configfname = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                configfname = argp ;
	                        }
	                        break ;

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pr = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        }
	                        break ;

	                    case argopt_tmpdir:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->tmpdname = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->tmpdname = argp ;
	                        }
	                        break ;

/* version */
	                    case argopt_version:
				f_makedate = f_version ;
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

/* log file */
	                    case argopt_lf:
	                    case argopt_logfile:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                lfname = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lfname = avp ;
	                        }
	                        break ;

/* print out the help */
	                    case argopt_help:
	                        f_help = TRUE ;
	                        break ;

	                    case argopt_pm:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pmspec = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pmspec = argp ;
	                        }
	                        break ;

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

	                    case argopt_db:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                dbname = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                dbname = argp ;
	                        }
	                        break ;

	                    case argopt_af:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                afname = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                afname = argp ;
	                        }
	                        break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            efname = argp ;
	                    }
	                    break ;

	                    case argopt_if:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                ifname = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ifname = argp ;
	                        }
	                        break ;

	                    case argopt_of:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                ofname = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ofname = argp ;
	                        }
	                        break ;

	                    case argopt_basedir:
	                    case argopt_dn:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                basedname = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                basedname = argp ;
	                        }
	                        break ;

	                    case argopt_fn:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                fn = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                fn = argp ;
	                        }
	                        break ;

	                    case argopt_tablen:
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdeci(argp,argl,&v) ;
	                                pip->tablen = v ;
				    }
	                        break ;

	                    case argopt_cookie:
				pip->f.optoutcookie = TRUE ;
	 			if (f_optequal) {
	         		    f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
					pip->f.optoutcookie = (rs > 0) ;
				    }
				}
	                        break ;

/* handle all keyword defaults */
	                    default:
				f_usage = TRUE ;
				rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {
			        const int	kc = MKCHAR(*akp) ;

	                        switch (kc) {

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

/* configuration file */
	                        case 'C':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                configfname = argp ;
	                            break ;

/* quiet mode */
	                        case 'Q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* program-root */
	                    case 'R':
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

/* version */
	                        case 'V':
				    f_makedate = f_version ;
	                            f_version = TRUE ;
	                            break ;

/* append to the key file */
	                        case 'a':
	                            pip->f.append = TRUE ;
	                            break ;

				case 'b':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                basedname = argp ;
	                            break ;

/* entry delimiter string */
	                        case 'd':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
/* we want to store the pointer to zero length string if given */
	                            delimiter = argp ;
	                            break ;

/* output format */
	                        case 'f':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
/* we allow a zero length string as a valid argument */
	                            outfmt = argp ;
	                            break ;

	                        case 'i':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ignorechars = argp ;
	                            break ;

/* maximum number of keys written out */
	                        case 'k':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdeci(argp,argl,&v) ;
					pip->maxkeys = v ;
				    }
	                            break ;

	                        case 'l':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdeci(argp,argl,&v) ;
	                                pip->minwordlen = v ;
				    }
	                            break ;

	                        case 'm':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdeci(argp,argl,&v) ;
	                                pip->maxwordlen = v ;
				    }
	                            break ;

/* number of eigenwords to consider */
	                        case 'n':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdeci(argp,argl,&v) ;
	                                pip->eigenwords = v ;
				    }
	                            break ;

	                        case 'o':
	                            break ;

/* prefix match */
	                        case 'p':
	                            pip->f.prefix = TRUE ;
	                            break ;

	                        case 'q':
	                            pip->verboselevel = 0 ;
	                            break ;

/* remove labels */
	                        case 's':
	                            pip->f.removelabel = TRUE ;
	                            break ;

/* index whole files */
	                        case 'w':
	                            pip->f.wholefile = TRUE ;
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
				    f_usage = TRUE ;
	                            rs = SR_INVALID ;
	                            break ;

	                        } /* end switch */

	                        akp += 1 ;
				if (rs < 0)
					break ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(ainfo.argpresent,ai) ;
	        ainfo.ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ainfo.ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
		pip->progname,pip->debuglevel) ;
	    bflush(pip->efp) ;
	}

	if (f_version) {

	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	    if (f_makedate) {

	    if ((cp = strchr(makedate,CH_RPAREN)) != NULL) {

	        cp += 1 ;
	        while (*cp && isspace(*cp))
	            cp += 1 ;

	    } else
	        cp = makedate ;

	    bprintf(pip->efp,"%s: makedate %s\n",
	        pip->progname,cp) ;

	    }
	}

	if (f_usage) 
	    usage(pip) ;

/* get our program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

/* get our program mode */

	if (pmspec == NULL)
	    pmspec = pip->progname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0) {
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    }else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif

	if (pip->progmode < 0)
		pip->progmode = progmode_mkkey ;

/* print out the help file if requested */

	if (f_help) {

	    if ((helpfname == NULL) || (helpfname[0] == '\0'))
	        helpfname = HELPFNAME ;

	    printhelp(NULL,pip->pr,pip->searchname,helpfname) ;

	} /* end if (helpfname) */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check program parameters */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (pip->linelen <= 0) {
	    cp = argval ;
	    
	    if (cp == NULL)
		cp = getenv(VARLINELEN) ;

		if (cp == NULL)
		    cp = getenv(VARCOLUMNS) ;

	        if (cp != NULL) {
		    if (cfdeci(cp,-1,&v) >= 0)
		        pip->linelen = v ;
		}

	    if (pip->linelen <= 0)
		pip->linelen = COLUMNS ;

	} /* end if (line-fold length) */

/* get some host/user information */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	if (rs < 0) {
	    ex = EX_NOUSER ;
	    goto retearly ;
	}

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;
	pip->logid = u.logid ;

/* do we have an activity log file? */

	if (lfname == NULL) {
	    lfname = tmpfname ;
	    mkpath3(tmpfname,pip->pr,logdname,pip->searchname) ;
	}

	rs1 = logfile_open(&pip->lh,lfname,0,0666,pip->logid) ;

	if (rs1 >= 0) {
	    pip->open.log = TRUE ;

	    if (loglen < 0) loglen = LOGSIZE ;
	    logfile_checksize(&pip->lh,loglen) ;

	    logfile_userinfo(&pip->lh,&u,
		pip->daytime,pip->progname,pip->version) ;

	} /* end if (we have a log file or not) */

	if ((afname != NULL) && (afname[0] != '\0'))
		ainfo.afname = afname ;

#ifdef	COMMENT

/* open output */

	if ((ofname != NULL) && (ofname[0] != '\0')) {
	    rs = bopen(ofp,ofname,"wct",0644) ;
	} else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0644) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: output unavailable(%d)\n",
	        pip->progname,rs) ;
	    goto badoutopen ;
	}
#endif /* COMMENT */

	rs = mktagprint(pip,&ainfo,basedname,outfmt,ofname) ;

#ifdef	COMMENT
	bclose(ofp) ;
#endif

done:
badoutopen:
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {

	    case SR_INVALID:
	        ex = EX_USAGE ;
	        break ;

	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;

	    case SR_AGAIN:
	        ex = EX_TEMPFAIL ;
	        break ;

	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;

	    } /* end switch */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%d\n",ex) ;
#endif

/* we are done */
ret3:
	if (pip->open.log) {
	    pip->open.log = FALSE ;
	    logfile_close(&pip->lh) ;
	}

retearly:
ret2:
	bclose(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

ret0:
badprogstart:

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


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [<tagfile(s)> ...] [-af <argfile>]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
		"%s:  [-b <basedir>] [-f <outfmt>]\n",
		pip->progname) ;

	wlen += rs ;
	rs = shio_printf(pip->efp,
	    "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */



