/* main */

/* main subroutine for the GROPEDICT directory update program */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-09-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is part of the program to rebuilt the dictionary files
	for the EXPTOOLS GROPE program.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<time.h>
#include	<dirent.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>
#include	<ftw.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<varsub.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"dictfiles.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,cchar *,cchar *) ;
extern int	isprintlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	proglogfname(PROGINFO *,char *,cchar *,cchar *) ;

extern char	*timestr_log(time_t,char *) ;


/* externals variables */


/* forward references */

static int	usage(PROGINFO *) ;

static int	process(PROGINFO *,DICTFILES *,const char *) ;


/* local global variabes */


/* local structures */


/* local variables */

static const char *argopts[] = {
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"ROOT",
	"LOGFILE",
	"CONFIG",
	"GETDICTDIR",
	"HELP",
	"sn",
	"af",
	"of",
	"ef",
	NULL
} ;

enum argopts {
	argopt_tmpdir,
	argopt_version,
	argopt_verbose,
	argopt_root,
	argopt_logfile,
	argopt_config,
	argopt_getdict,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_of,
	argopt_ef,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
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


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	USERINFO	u ;
	DICTFILES	dics ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan = 0 ;
	int	rs, rs1 ;
	int	v ;
	int	loglen = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_getdict = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*efname = NULL ;
	const char	*cfname = NULL ;
	const char	*lfname = NULL ;
	const char	*dictdname = NULL ;
	const char	*cp ;
	char	argpresent[MAXARGGROUPS] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;

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
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->pr = avp ;
	                        } else {
	                            if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) 
					pip->pr = argp ;
				    } else
					rs = SR_INVALID ;
	                        }
	                        break ;

/* configuration file */
	                    case argopt_config:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) 
					cfname = avp ;
	                        } else {
	                            if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) 
					cfname = argp ;
				    } else
					rs = SR_INVALID ;
	                        }
	                        break ;

/* log file */
	                    case argopt_logfile:
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

/* get the dictionary directory */
	                    case argopt_getdict:
	                        f_getdict = TRUE ;
	                        break ;

/* print out the help */
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

/* argument file-name */
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

/* output file-name */
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

/* handle all keyword defaults */
	                    default:
				rs = SR_INVALID ;
				f_usage = TRUE ;
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

/* configuration file */
	                        case 'C':
	                            if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) 
					cfname = argp ;
				    } else
					rs = SR_INVALID ;
	                            break ;

/* dictionary directory */
	                        case 'd':
	                        case 'l':
	                            if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) 
					dictdname = argp ;
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
				    f_usage = TRUE ;
				    break ;

	                        } /* end switch */
	                        akp += 1 ;

				if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits or progopts) */

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

#if	CF_DEBUGS
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: finished parsing arguments\n") ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: pr=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

/* get our program mode */

#ifdef	COMMENT
	if (pmspec == NULL)
	    pmspec = pip->searchname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;

	if (pip->progmode < 0)
	    pip->progmode = progmode_filesize ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0) {
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    } else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif /* CF_DEBUG */
#endif /* COMMENT */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_help || f_version || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* get some host/user information */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;
	if (rs < 0) {
	    ex = EX_NOUSER ;
	    goto retearly ;
	}

	pip->pid = u.pid ;
	pip->username = u.username ;
	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->logid = u.logid ;

/* check program parameters */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
		debugprintf("main: checking program parameters\n") ;
#endif

/* clean up the dictionary path as we may have it */

	if (dictdname == NULL) dictdname = getenv(VARDICTDNAME) ;

	if ((dictdname == NULL) || (dictdname[0] == '\0'))
	    dictdname = DICTDNAME ;

	rs = u_access(dictdname,(W_OK | X_OK | R_OK)) ;

	if ((rs < 0) && (dictdname[0] != '/')) {

	    mkpath2(tmpfname,pip->pr,dictdname) ;

	    if ((rs = u_access(tmpfname,(W_OK | X_OK | R_OK))) >= 0)
	        dictdname = tmpfname ;

	} /* end if */

	if (rs < 0) 
		goto baddictionary ;

	mkpath1(pip->dictdname,dictdname) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: dictionary=%s\n",pip->dictdname) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: dictionary=%s\n",
	        pip->progname,pip->dictdname) ;
	}

/* do we have a prefix string for the dictionary files? */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: 0 prefix=%s\n",pip->prefix) ;
#endif

	if ((pip->prefix == NULL) || (pip->prefix[0] == '\0'))
	    pip->prefix = PREFIX ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: 1 prefix=%s\n",pip->prefix) ;
#endif

/* do we have an activity log file? */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: 0 lfname=%s\n",lfname) ;
#endif

	rs = proglogfname(pip,tmpfname,LOGCNAME,lfname) ;
	if (rs > 0) lfname = tmpfname ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: lfname=%s\n",lfname) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: logfile=%s\n",pip->progname,lfname) ;

	if ((rs >= 0) && (lfname != NULL)) {
	    rs1 = SR_NOENT ;
	    if ((lfname[0] != '\0') && (lfname[0] != '-'))
	        rs1 = logfile_open(&pip->lh,lfname,0,0666,pip->logid) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: logfile_open() rs=%d\n",rs1) ;
#endif

/* open the log file (if we can) */

	    if (rs1 >= 0) {
	        pip->open.log = TRUE ;
	        if (loglen < 0) loglen = LOGSIZE ;
	        logfile_checksize(&pip->lh,loglen) ;
	        logfile_userinfo(&pip->lh,&u,pip->daytime,
		    pip->searchname,pip->version) ;
	        logfile_printf(&pip->lh,"dictionary dir=%s\n",
	            pip->dictdname) ;
	    } /* end if (we have a log file or not) */

	} /* end if */
	if (rs < 0) goto retearly ;

/* print out the dictionary directory if requested */

	if (f_getdict) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: getdict\n") ;
#endif

	    if (ofname == NULL) ofname = BFILE_STDOUT ;
	    if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {

	        bprintf(ofp,"%s\n",pip->dictdname) ;

	        bclose(ofp) ;
	    } /* end if (output) */

	} /* end if (printing out the dictionary directory path) */

/* initialize the output files */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: dictfiles_open() \n") ;
#endif

	if (rs >= 0) {
	    const int	n = NOOUTFILES ;
	    const char	*dd = pip->dictdname ;
	    const char	*prefix = pip->prefix ;
	    if ((rs = dictfiles_open(&dics,n,dd,prefix)) >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
		debugprintf(
	    "main: checking for positional arguments\n") ;
#endif

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        cp = argv[ai] ;
		if (cp[0] != '\0') {
	            pan += 1 ;
	            rs = process(pip,&dics,cp) ;
		}
	    }

		if (rs < 0) {
	            if (*cp == '-') cp = "*stdinput*" ;
	            bprintf(pip->efp,"%s: error processing input file (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: file=%s\n",
	                pip->progname,cp) ;
	        } /* end if */

	    if (rs < 0) break ;
	} /* end for (processing positional arguments) */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    BFILE	afile, *afp = &afile ;

	    if (afname[0] == '-') afname = BFILE_STDIN ;

	    if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            int	len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

		    rs = process(pip,&dics,lbuf) ;

		    if (rs < 0) break ;
	        } /* end while (reading lines) */

	        bclose(afp) ;
	    } else {
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: argument file inaccessible (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",
	                pip->progname,afname) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pan == 0)) {

	    cp = "-" ;
	    pan += 1 ;
	        rs = process(pip,&dics,cp) ;

		if (rs < 0) {
	            if (*cp == '-') 
			cp = "*stdinput*" ;
	            bprintf(pip->efp,"%s: error processing input file (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: file=%s\n",
	                pip->progname,cp) ;
	        } /* end if */

	} /* end if */

/* close the output files */

	rs1 = dictfiles_close(&dics) ;
	if (rs >= 0) rs = rs1 ;
	} else {
		switch (rs) {
		case SR_INPROGRESS:
		ex = EX_UNAVAILABLE ;
		break ;
		default:
		ex = EX_SOFTWARE ;
		} /* end switch */
	}
	} /* end if (ok) */

	if (pip->open.log) {
	    pip->open.log = FALSE ;
	    logfile_close(&pip->lh) ;
	}

#ifdef	COMMENT
	    bprintf(pip->efp,"%s: %d file(s) used simultaneously\n",
	        pip->progname,rs1) ;
#endif

#ifdef	COMMENT
	    j = 0 ;
	    for (i = 0 ; i < 256 ; i += 1) {
	        if (outtable[i]->letter != '\0') j += 1 ;
	    } /* end for */
	    bprintf(pip->efp,"%s: %u files written\n",
	        pip->progname,j) ;
#endif /* COMMENT */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
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

/* return */
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u\n",
		pip->progname,ex) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("main: exiting rs=%d ex=%u\n",rs,ex) ;
#endif

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

baddictionary:
	bprintf(pip->efp,
	    "%s: could not access dictionary, dir=%s (%d)\n",
	    pip->progname,pip->dictdname,rs) ;

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

	fmt = "%s: USAGE> %s [-C <conf>] [<file(s)> ...] [-d <dictdir>] \n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int process(PROGINFO *pip,DICTFILES *dp,cchar *fname)
{
	bfile		infile, *ifp = &infile ;
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: fname=%s\n",fname) ;
#endif

	if (pip == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: starting\n") ;
#endif

	if (fname[0] == '-') fname = BFILE_STDIN ;

	if ((rs = bopen(ifp,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		ch ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	        int	len = rs ;

#if	CF_DEBUG && 0
	        if (DEBUGLEVEL(2))
	            debugprintf("process: line> %t",lbuf,len) ;
#endif

	        ch = (lbuf[0] & 0xff) ;
	        if (ch == '\n')
	            continue ;

	        if (lbuf[len - 1] != '\n') {
	            while ((ch = bgetc(ifp)),((ch != SR_EOF) && (ch != '\n'))) ;
	            continue ;
	        }

	        if (len > MAXWORDLEN) continue ;
	        if (! isprintlatin(ch)) continue ;

	        rs = dictfiles_write(dp,lbuf,len) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: dictfiles_write() rs=%d\n",rs) ;
#endif

	        if (rs < 0) break ;
	    } /* end while (looping reading lines) */

	    bclose(ifp) ;
	} /* end if (open) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */


