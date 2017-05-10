/* main (ecover) */

/* main subroutine (for the ECOVER program) */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory allocations */


/* revision history:

	= 1994-09-01, David A­D­ Morano

	This subroutine was originally written.


	= 1997-05-03, David A­D­ Morano

	This subroutine was enhanced to take multiple input files.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	This is the main subroutine for the 'ecover' program.

	Synopsis:

	$ ecover -d <infile> > <outfile>


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<varsub.h>
#include	<sbuf.h>
#include	<randomvar.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"ecmsg.h"
#include	"ecinfo.h"
#include	"config.h"
#include	"defs.h"
#include	"configfile.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + REALNAMELEN)
#endif

#ifndef	NOISEBUFLEN
#define	NOISEBUFLEN	100
#endif

#ifndef		FILEINFO
#define		FILEINFO	struct fileinfo
#endif


/* external subroutines */

extern uint	hashelf(const char *,int) ;

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	iceil(int,int) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,cchar *,cchar *) ;
extern int	isdigitlatin(int) ;
extern int	isalnumlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	proglogfname(PROGINFO *,char *,
			const char *,const char *) ;
extern int	encode(PROGINFO *,RANDOMVAR *,
			FILEINFO *,ECMSG *,int,int) ;
extern int	decode(PROGINFO *,RANDOMVAR *,
			FILEINFO *,ECMSG *,int,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*getourenv(cchar **,cchar *) ;


/* external variables */


/* forward references */

static int	usage(PROGINFO *) ;

static uint	lightnoise(PROGINFO *,USERINFO *,const char *) ;


/* local structures */


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"HELP",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"LOGFILE",
	"lf",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"mf",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_help,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_logfile,
	argopt_lf,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_msgfile,
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
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const char	*tmpdirs[] = {
	"/tmp",
	"/var/tmp",
	"/var/spool/uucppublic",
	".",
	NULL
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	FILEINFO	fi ;
	USERINFO	u ;
	RANDOMVAR	rv ;
	ECMSG		extra ;
	bfile		errfile ;
	uint		hv ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		i ;
	int		size ;
	int		loglen = -1 ;
	int		ifd, ofd ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*lfname = NULL ;
	const char	*helpfname = NULL ;
	const char	*jobid = NULL ;
	const char	*cp ;
	char		argpresent[MAXARGGROUPS] ;
	char		buf[BUFLEN + 1] ;
	char		userbuf[USERINFO_LEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		jobidbuf[LOGFILE_LOGIDLEN + 1] ;

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

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;

	rs = ecmsg_start(&extra) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badecinit ;
	}

/* start parsing the arguments */

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1) 
	    argpresent[ai] = 0 ;

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

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pr = avp ;
	                        } else {
	                            if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
				    } else
					rs = SR_INVALID ;
	                        }
	                        break ;

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

/* log file */
	                    case argopt_logfile:
	                    case argopt_lf:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                lfname = avp ;
	                        } else {
	                            if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lfname = argp ;
				    } else
					rs = SR_INVALID ;
	                        }
	                        break ;

	                    case argopt_msgfile:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->msgfname = avp ;
	                        } else {
	                            if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->msgfname = argp ;
				    } else
					rs = SR_INVALID ;
	                        }
	                        break ;

/* print out the help */
	                    case argopt_help:
	                        f_help = TRUE ;
	                        break ;

/* progaram search-name */
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

/* argument file */
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

/* output file */
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

/* input file */
	                    case argopt_if:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                ifname = avp ;
	                        } else {
	                            if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ifname = argp ;
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

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* unscramble */
	                        case 'u':
	                        case 'd':
	                            pip->f.unscramble = TRUE ;
	                            break ;

/* scramble */
	                        case 's':
	                        case 'e':
	                            pip->f.unscramble = FALSE ;
	                            break ;

/* user specified job ID */
	                        case 'j':
	                            if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                jobid = argp ;
				    } else
					rs = SR_INVALID ;
	                            break ;

/* message text */
	                        case 'm':
	                            if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = ecmsg_loadbuf(&extra,argp,argl) ;
				    }
				    } else
					rs = SR_INVALID ;
	                            break ;

				case 'q':
	                            pip->verboselevel = 0 ;
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

/* print usage summary */
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

	        } /* end if (digits as argument or not) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	}

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

/* get our program root */

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
	    printhelp(NULL,pip->pr,pip->searchname,helpfname) ;
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;

	ex = EX_OK ;

/* get some host/user information */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;
	if (rs < 0) {
		ex = EX_NOUSER ;
	        bprintf(pip->efp,"%s: no user (%d)\n",
	            pip->progname,rs) ;
		goto retearly ;
	}

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;
	pip->groupname = u.groupname ;

	pip->daytime = time(NULL) ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: mode=%s\n",
		pip->progname,
		((pip->f.unscramble) ? "decode" : "encode")) ;

/* create a good (?) starting seed value */

	hv = lightnoise(pip,&u,ofname) ;

/* initialize our random generator */

	rs = randomvar_start(&rv,FALSE,hv) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badnoise ;
	}

/* check program parameters */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checking program parameters\n") ;
#endif

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv(VARTMPDNAME) ;

	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0')) {
	    const int	am = (W_OK|R_OK|X_OK) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: for-access\n") ;
#endif

	    for (i = 0 ; tmpdirs[i] != NULL ; i += 1) {
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: access fn=%s\n",tmpdirs[i]) ;
#endif
	        if (u_access(tmpdirs[i],am) >= 0) break ;
	    } /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: for-out rs=%d\n",rs) ;
#endif

	    if (tmpdirs[i] != NULL) {
	        pip->tmpdname = tmpdirs[i] ;
	    }

	} /* end if */

	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0'))
	    pip->tmpdname = TMPDNAME ;

/* possibly clean up the user specified JOB ID */

	if (jobid != NULL) {
	    cp = jobid ;
	    i = 0 ;
	    while ((*cp != '\0') && (i < LOGFILE_LOGIDLEN)) {
		int ch = MKCHAR(*cp) ;
	        if (isalnumlatin(ch) || (*cp == '_')) {
	            jobidbuf[i++] = *cp ;
		}
	        cp += 1 ;
	    } /* end while */
	    jobid = jobidbuf ;
	} else
	    jobid = u.logid ;

/* do we have an activity log file? */

	rs = proglogfname(pip,tmpfname,LOGCNAME,lfname) ;
	if (rs > 0) lfname = tmpfname ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: lfname=%s\n",tmpfname) ;
#endif

	if ((rs >= 0) && (lfname != NULL)) {
	    rs1 = SR_NOENT ;
	    if ((lfname[0] != '\0') && (lfname[0] != '-'))
	        rs1 = logfile_open(&pip->lh,lfname,0,0666,jobid) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: logfile_open() rs=%d\n",rs) ;
#endif

	    if (rs1 >= 0) {
	        pip->open.log = TRUE ;
	        if (loglen < 0) loglen = LOGSIZE ;
	        logfile_checksize(&pip->lh,loglen) ;
	        logfile_userinfo(&pip->lh,&u,
		    pip->daytime,pip->progname,pip->version) ;
	    } /* end if (we have a log file or not) */

	} /* end if */
	if (rs < 0) goto retearly ;

/* initialize (calculate) the number of OPwords to store ECINFO */

	size = sizeof(struct ecinfo_data) ;
	pip->necinfo = iceil(size,sizeof(ULONG)) / sizeof(ULONG) ;

	if (pip->necinfo >= (NOPWORDS/2)) {
		ex = EX_SOFTWARE ;
		goto done ;
	}

/* process the positional arguments */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        cp = argv[ai] ;
	        switch (pan) {
	        case 0:
		    ifname = cp ;
		    break ;
	        } /* end switch */
	        pan += 1 ;
	    }

	    if (ifname != NULL) break ;
	} /* end for (processing positional arguments) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("main: ifname=%s\n",ifname) ;
#endif

/* open the input */

	ifd = FD_STDIN ;
	if ((ifname != NULL) && (ifname[0] != '\0') && (ifname[0] != '-')) {
	    rs = uc_open(ifname,O_RDONLY,0666) ;
	    ifd = rs ;
	}

	if (rs < 0) {
	ex = EX_NOINPUT ;
	bprintf(pip->efp,"%s: inaccessible input file (%d)\n",
	    pip->progname,rs) ;
	    goto badinopen ;
	}

/* open the output */

	ofd = FD_STDOUT ;
	if ((ofname != NULL) && (ofname[0] != '\0')) {
	    rs = u_open(ofname,O_CREAT | O_TRUNC | O_WRONLY,0666) ;
	    ofd = rs ;
	}

	if (rs < 0) {
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: inaccessible output file (%d)\n",
	    pip->progname,rs) ;
	    goto badoutopen ;
	}

/* what program mode are we running in? */

	if (pip->f.unscramble) {

	    rs = decode(pip,&rv,&fi,&extra,ifd,ofd) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("main: decode() rs=%d\n",rs) ;
#endif

	} else {

	    if ((pip->msgfname != NULL) && (ecmsg_already(&extra) == 0)) {
		const char	*msgfname = pip->msgfname ;

	        if ((rs1 = uc_open(msgfname,O_RDONLY,0666)) >= 0) {
		    const int	mfd = rs1 ;

				size = MIN(BUFLEN,ECMSG_MAXBUFLEN) ;
				rs1 = uc_readn(mfd,buf,size) ;

				ecmsg_loadbuf(&extra,buf,rs1) ;

		    u_close(mfd) ;
		} /* end if (ec) */

	    } /* end if (message file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    debugprintf("main: extra.buf(%p) extra.buflen=%d\n",
		extra.buf,extra.buflen) ;
		if (extra.buf != NULL)
	    debugprintf("main: extra=>%t<\n",
		extra.buf,strnlen(extra.buf,extra.buflen)) ;
	}
#endif /* CF_DEBUG */

	if (rs >= 0) {

	    rs = encode(pip,&rv,&fi,&extra,ifd,ofd) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("main: encode() rs=%d\n",rs) ;
#endif

	}

	} /* end if (encode or decode) */

	if (rs < 0) {

	    logfile_printf(&pip->lh,"mode=%c",
	        ((pip->f.unscramble) ? 'd' : 'e')) ;

	    logfile_printf(&pip->lh,"sent cksum=\\x%08x (%u)",
	        fi.cksum,fi.cksum) ;

	    logfile_printf(&pip->lh,"failed (%d)",rs) ;

	    if ((pip->debuglevel > 0) || (! pip->f.quiet))
	        bprintf(pip->efp,"%s: failed (%d)\n",
	            pip->progname,rs) ;

	} else {
	    logfile_printf(&pip->lh,"mode=%c cksum=\\x%08x (%u)",
	        ((pip->f.unscramble) ? 'u' : 's'),
	        fi.cksum,fi.cksum) ;
	}

	u_close(ofd) ;

badoutopen:
	u_close(ifd) ;

badinopen:
done:
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    default:
	        ex = mapex(mapexs,rs) ;
		break ;
	    } /* end switch */
	} /* end if */

/* start the exit sequence */

	if (pip->open.log) {
	    pip->open.log = FALSE ;
	    logfile_close(&pip->lh) ;
	}

badnoise:
	randomvar_finish(&rv) ;

retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
		pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	ecmsg_finish(&extra) ;

badecinit:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* argument errors */
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
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<file>] \n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-s|-u] [-of <ofile>]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static uint lightnoise(PROGINFO *pip,USERINFO *uip,cchar *ofname)
{
	struct timeval	tod ;
	SBUF		hb ;
	uint		v = 0 ;
	uint		hv = 0 ;
	const int	dlen = BUFLEN ;
	int		rs ;
	int		i ;
	int		bl = 0 ;
	char		dbuf[BUFLEN + 1] ;

	if ((rs = sbuf_start(&hb,dbuf,dlen)) >= 0) {
	    pid_t	pid_parent = getppid() ;

/* get some miscellaneous stuff */

	    if (uip->username != NULL)
	        sbuf_strw(&hb,uip->username,-1) ;

	    if (uip->homedname != NULL)
	        sbuf_strw(&hb,uip->homedname,-1) ;

	    if (uip->nodename != NULL)
	        sbuf_strw(&hb,uip->nodename,-1) ;

	    if (uip->domainname != NULL)
	        sbuf_strw(&hb,uip->domainname,-1) ;

	    sbuf_deci(&hb,(int) uip->pid) ;

	    sbuf_decl(&hb,(long) pip->daytime) ;

	    if (ofname != NULL)
	        sbuf_strw(&hb,ofname,-1) ;

	    sbuf_deci(&hb,(int) pid_parent) ;

	    uc_gettimeofday(&tod,NULL) ;

	    sbuf_buf(&hb,(char *) &tod,sizeof(struct timeval)) ;

	    bl = sbuf_finish(&hb) ;
	    if (rs >= 0) rs = bl ;
	} /* end if (sbuf) */

	    hv = hashelf(dbuf,bl) ;

/* pop in our environment also! */

	for (i = 0 ; pip->envv[i] != NULL ; i += 1) {
	    v ^= hashelf(pip->envv[i],-1) ;
	}
	hv ^= v ;

#ifdef	COMMENT
	if ((cp = getenv("RANDOM")) != NULL) {
	    if (cfdecui(cp,-1,&v) >= 0)
	        hv ^= v ;
	}
#endif /* COMMENT */

	return hv ;
}
/* end subroutine (lightnoise) */


