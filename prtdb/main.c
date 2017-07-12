/* main */

/* the front-end of the PRTDB program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_GETPROGPATH	1		/* use 'getprogpath(3dam)' */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	This program was written from scratch to do what the previous
	program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is the front-end for a program that accesses
	several possible databases in order to find the value for
	specified keys.  We use the PRINTER environment variable to find
	the default printer to lookup keys for when an explicit printer
	is not specified.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<keyopt.h>
#include	<bits.h>
#include	<ids.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<field.h>
#include	<paramopt.h>
#include	<getax.h>
#include	<getxusername.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"pdb.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif

#ifndef	PRINTERLEN
#define	PRINTERLEN	MAXNAMELEN
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getprogpath(IDS *,vecstr *,char *,const char *,int) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	prgetprogpath(const char *,char *,const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,const char *,
			const struct pivars *) ;
extern int	lpgetout(PROGINFO *,const char *,
			char *,int,const char *) ;
extern int	progkey(PROGINFO *,const char *,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	loadpath(PROGINFO *,vecstr *,const char *) ;


/* external variables */


/* local structures */


/* local variables */

static const char	*progmodes[] = {
	"prtdb",
	NULL
} ;

enum progmodes {
	progmode_prtdb,
	progmode_overlast
} ;

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"pm",
	"sn",
	"un",
	"option",
	"set",
	"follow",
	"af",
	"ef",
	"of",
	"db",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_un,
	argopt_option,
	argopt_set,
	argopt_follow,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_db,
	argopt_overlast
} ;

static const char	*progopts[] = {
	"follow",
	"nofollow",
	NULL
} ;

enum progopts {
	progopt_follow,
	progopt_nofollow,
	progopt_overlast
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
	{ SR_ACCESS, EX_ACCESS },
	{ SR_REMOTE, EX_FORWARDED },
	{ SR_NOSPC, EX_NOSPACE },
	{ SR_INVALID, EX_USAGE },
	{ 0, 0 }
} ;

static const uchar	aterms[] = {
	0x00, 0x3E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	struct ustat	sb ;
	PARAMOPT	aparams ;
	BITS		pargs ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	const int	hlen = MAXPATHLEN ;
	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan = 0 ;
	int	rs, rs1 ;
	int	opts ;
	int	cl ;
	int	v ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*plpget = PROG_LPGET ;
	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		printer[PRINTERLEN + 1] ;
	char		unbuf[USERNAMELEN + 1] ;
	char		hbuf[MAXPATHLEN+1] ;
	const char	*pr = NULL ;
	const char	*pmspec = NULL ;
	const char	*sn = NULL ;
	const char	*ur = NULL ;
	const char	*un = NULL ;
	const char	*utilname = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*dbfname = NULL ;
	const char	*cp ;


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

	ids_load(&pip->ids) ;

/* early things to initialize */

	printer[0] = '\0' ;

	pip->ofp = ofp ;

	pip->progmode = -1 ;
	pip->verboselevel = 1 ;

/* process program arguments */

	rs = paramopt_start(&aparams) ;
	pip->open.aparams = (rs >= 0) ;

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

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

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

/* program mode */
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

/* search name */
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

/* utility name */
	                case argopt_un:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            utilname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            utilname = argp ;
	                    }
	                    break ;

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose */
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

/* temporary directory */
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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* the user specified some progopts */
	                case argopt_option:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl) {
				const char	*po = PO_OPTION ;
	                        rs = paramopt_loads(&aparams,po,argp,argl) ;
			    }
	                    break ;

/* the user specified some progopts */
	                case argopt_set:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl)
	                        rs = paramopt_loadu(&aparams,argp,argl) ;
	                    break ;

/* argument files */
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

/* output file */
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

/* printer database */
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            dbfname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            dbfname = argp ;
	                    }
	                    break ;

/* follow symbolic links */
	                case argopt_follow:
	                    pip->f.follow = TRUE ;
	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;
			    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
			    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

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

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* printer destination */
	                    case 'd':
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            rs = sncpy1(printer,PRINTERLEN,avp) ;
	                        } else {
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
	                                break ;
	                            }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
				    if (argl)
	                            rs = sncpy1(printer,PRINTERLEN,argp) ;
	                        }
	                        break ;

			    case 'q':
				pip->verboselevel = 0 ;
				break ;

/* username to use for DB query */
	                    case 'u':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        un = argp ;
	                        break ;

/* verbose output */
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

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	}

	if (rs < 0)
	    goto badarg ;

/* check arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
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
	    } else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif

	if (pip->progmode < 0)
	    pip->progmode = progmode_prtdb ;

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* get ready */

	if ((rs = paramopt_havekey(&aparams,PO_SUFFIX)) > 0) {
	    pip->f.suffix = TRUE ;
	} /* end if */

	if ((rs = paramopt_havekey(&aparams,PO_OPTION)) > 0) {
	    PARAMOPT_CUR	cur ;

	    paramopt_curbegin(&aparams,&cur) ;

	    while (paramopt_enumvalues(&aparams,PO_OPTION,&cur,&cp) >= 0) {

	        if ((kwi = matostr(progopts,2,cp,-1)) >= 0) {
	            switch (kwi) {
	            case progopt_follow:
	                pip->f.follow = TRUE ;
	                break ;
	            case progopt_nofollow:
	                pip->f.follow = FALSE ;
	                break ;
	            } /* end switch */
	        } /* end if (progopts) */

	    } /* end while */

	    paramopt_curend(&aparams,&cur) ;
	} /* end if (paramopt_havekey) */

/* check if we have a username for the query */

	if (un == NULL) {
	    rs = getusername(unbuf,USERNAMELEN,-1) ;
	    un = unbuf ;
	}

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: user=%s\n",pip->progname,un) ;

	if ((rs >= 0) && (ur == NULL)) {
	    if ((rs = getuserhome(hbuf,hlen,un)) >= 0) {
	        ur = hbuf ;
	    }
	}

/* check the utility */

	if (utilname == NULL) utilname = getenv(VARUTILNAME) ;
	if (utilname == NULL) utilname = DEFUTILITY ;

/* try to find a DB file */

	if (dbfname == NULL) dbfname = getenv(VARDBFNAME) ;
	if (dbfname == NULL) dbfname = PDBFNAME ;

/* get the path to the LPGET program */

	if ((rs >= 0) && (pip->prog_lpget == NULL)) {
	    if ((cp = getenv(VARLPGET)) != NULL) {
		cchar	**vpp = &pip->prog_lpget ;
	        rs = proginfo_setentry(pip,vpp,cp,-1) ;
	    }
	}

	if ((rs >= 0) && (pip->prog_lpget == NULL)) {

	    opts = VECSTR_OORDERED ;
	    rs = vecstr_start(&pip->path,10,opts) ;
	    pip->f.path = (rs >= 0) ? TRUE : FALSE ;
	    if (rs < 0)
	        goto badinitpath ;

	    if ((cp = getenv(VARPATH)) != NULL)
	        rs = loadpath(pip,&pip->path,cp) ;

	    cp = tmpfname ;
	    mkpath2(tmpfname,pip->pr,"bin") ;

	    if (vecstr_find(&pip->path,cp) < 0)
	        vecstr_add(&pip->path,cp,-1) ;

	    cp = tmpfname ;
	    mkpath2(tmpfname,pip->pr,"sbin") ;

	    if (vecstr_find(&pip->path,cp) < 0)
	        vecstr_add(&pip->path,cp,-1) ;

	    cp = "/usr/bin" ;
	    if (vecstr_find(&pip->path,cp) < 0)
	        vecstr_add(&pip->path,cp,-1) ;

	    cp = "/usr/sbin" ;
	    if (vecstr_find(&pip->path,cp) < 0)
	        vecstr_add(&pip->path,cp,-1) ;

#if	CF_GETPROGPATH
	    rs = getprogpath(&pip->ids,&pip->path,tmpfname,plpget,-1) ;
#else
	    rs = prgetprogpath(pip->pr,tmpfname,plpget,-1) ;
#endif /* CF_GETPROGPATH */

	    cl = rs ;
	    if (rs >= 0)
	        proginfo_setentry(pip,&pip->prog_lpget,tmpfname,cl) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("main: getprogpath() rs=%d\n",rs) ;
	        debugprintf("main: prog_lpget=%s\n",pip->prog_lpget) ;
	    }
#endif

	} /* end if (trying to find a LPGET program) */

	if ((rs >= 0) && (pip->prog_lpget == NULL))
	    rs = SR_NOENT ;

	if (rs >= 0)
	    rs = u_stat(pip->prog_lpget,&sb) ;

	if (rs < 0) {
	    ex = EX_NOPROG ;
	    bprintf(pip->efp,"%s: LPGET program not found (%d)\n",
	        pip->progname,rs) ;
	    goto badnoprog ;
	}

/* get a default printer if necessary */

	if (printer[0] == '\0') {

	    if ((cp = getenv(VARPRINTER)) != NULL) {
	        rs = sncpy1(printer,PRINTERLEN,cp) ;

	    } else if ((cp = getenv(VARLPDEST)) != NULL)
	        rs = sncpy1(printer,PRINTERLEN,cp) ;

	} /* end if */

	if ((rs >= 0) && (printer[0] == '\0')) {

	    rs = lpgetout(pip,DEFPRINTER,printer,PRINTERLEN,DEFPRINTERKEY) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("main: lpgetout() rs=%d\n",rs) ;
	        debugprintf("main: lpgetout() printer=%s\n",printer) ;
	    }
#endif

	    if (rs < 0)
	        printer[0] = '\0' ;

	} /* end if (trying to get default printer) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: printer=>%s<\n",printer) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: printer=%s\n",pip->progname,printer) ;

/* open the printer-default database */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: ur=%s\n",ur) ;
#endif

	rs = pdb_open(&pip->db,pip->pr,ur,utilname,dbfname) ;
	pip->f.pdbopen = (rs >= 0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: pdb_open() rs=%d\n",rs) ;
#endif

	if (pip->debuglevel > 0) {
	    cp = (rs >= 0) ? "" : "not " ;
	    bprintf(pip->efp,"%s: default PRT DB %sfound\n",
	        pip->progname,cp) ;
	}

/* open the output */

	if ((ofname != NULL) && (ofname[0] != '\0')) {
	    rs = bopen(ofp,ofname,"wct",0644) ;
	} else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0644) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: could not open output (%d)\n",
	        pip->progname,rs) ;
	    goto badopenout ;
	}

/* OK, we do it */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        cp = argv[ai] ;
		if (cp[0] != '\0') {
	    	    pan += 1 ;
	    	    rs = progkey(pip,printer,cp,-1) ;
		}
	    }

	    if (rs < 0) break ;
	} /* end for (looping through requested circuits) */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afname,"-") == 0) afname = BFILE_STDIN ;

	    if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
	        FIELD	fsb ;
		const int	llen = LINEBUFLEN ;
	        int	len ;
	        int	fl ;
	        const char	*fp ;
	        char	lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(&afile,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            cl = sfshrink(lbuf,len,&cp) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            if ((rs = field_start(&fsb,cp,cl)) >= 0) {

	                while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	                    if (fl > 0) {
	                    pan += 1 ;
	                    rs = progkey(pip,printer,fp,fl) ;
			    }
	                    if (fsb.term == '#') break ;
	                    if (rs < 0) break ;
	                } /* end while */

			field_finish(&fsb) ;
	            } /* end if (field) */

	        } /* end while (reading lines) */

	        bclose(&afile) ;
	    } else {
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: inaccessible argument-list (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",
	                pip->progname,afname) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pan == 0) && (afname == NULL)) {

	    rs = SR_INVALID ;
	    bprintf(pip->efp,"%s: no keys specified\n",
	        pip->progname) ;

	}

	bclose(ofp) ;

	if ((rs > 0) && (pip->debuglevel > 0))
	    bprintf(pip->efp,"%s: keys processed=%u\n",
	        pip->progname,pan) ;

badopenout:
	if (pip->f.pdbopen) {
	    pip->f.pdbopen = FALSE ;
	    pdb_close(&pip->db) ;
	}

badnoprog:
	if (pip->f.path) {
	    pip->f.path = FALSE ;
	    vecstr_finish(&pip->path) ;
	}

done:
badinitpath:
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	} /* end if */

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
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	bits_finish(&pargs) ;

badpargs:
	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&aparams) ;
	}

	ids_release(&pip->ids) ;

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

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
PROGINFO	*pip ;
{
	int		rs ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s <key(s)> [...] [-af {<afile>|-}]\n",
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-u <username>] [-db <dbfile>]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-un <utility>] [-d <printer>]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int loadpath(pip,lp,pp)
PROGINFO	*pip ;
vecstr		*lp ;
const char	*pp ;
{
	const int	nrs = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		pathlen ;
	int		c = 0 ;
	const char	*tp ;
	char		pathdname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/loadpath: entered\n") ;
#endif

	while ((tp = strchr(pp,':')) != NULL) {

	    pathlen = pathclean(pathdname,pp,(tp - pp)) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("main/loadpath: pathname=%t\n",pathdname,pathlen) ;
#endif

	    if ((rs = vecstr_findn(lp,pathdname,pathlen)) == nrs) {
	        c += 1 ;
	        rs = vecstr_add(lp,pathdname,pathlen) ;
	    }

	    pp = (tp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (pp[0] != '\0')) {

	    pathlen = pathclean(pathdname,pp,-1) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("main/loadpath: pathname=%t\n",pathdname,pathlen) ;
#endif

	    if ((rs = vecstr_findn(lp,pathdname,pathlen)) == nrs) {
	        c += 1 ;
	        rs = vecstr_add(lp,pathdname,pathlen) ;
	    }

	} /* end if (trailing one) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpath) */


