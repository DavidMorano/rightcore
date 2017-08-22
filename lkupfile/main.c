/* main */

/* the front-end of the LKUPFILE program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	The program was written from scratch to do what
	the previous program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This subroutine is a full program. We access the PRINTER database and
        print out the requested values.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<ids.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<paramopt.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matstr3(const char **,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	isdigitlatin(int) ;

extern int	process(PROGINFO *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strshrink(char *) ;


/* forward references */

static int	usage(PROGINFO *) ;


/* external variables */


/* local variables */

static const char	*progmodes[] = {
	"lkupfile",
	NULL
} ;

enum progmodes {
	progmode_lkupfile,
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
	"option",
	"set",
	"follow",
	"db",
	"af",
	"if",
	"of",
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
	argopt_option,
	argopt_set,
	argopt_follow,
	argopt_db,
	argopt_af,
	argopt_if,
	argopt_of,
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


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	PROGINFO	pi, *pip = &pi ;
	struct ustat	sb ;

	PARAMOPT	aparams ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs ;
	int	opts ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	const char	*pr = NULL ;
	const char	*pmspec = NULL ;
	const char	*searchname = NULL ;
	const char	*dbfname = NULL ;
	const char	*afname = NULL ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*tp, *cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;

	pip->verboselevel = 1 ;

	ids_load(&pip->ids) ;

	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* early things to initialize */

	pip->ofp = ofp ;

	pip->tmpdname = NULL ;
	pip->namelen = MAXNAMELEN ;

	pip->progmode = -1 ;

	pip->f.nochange = FALSE ;
	pip->f.quiet = FALSE ;
	pip->f.follow = FALSE ;
	pip->f.suffix = FALSE ;

/* process program arguments */

	rs = paramopt_start(&aparams) ;
	pip->open.aparams = (rs >= 0) ;

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
	    argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	                rs = cfdecti((argp + 1),(argl - 1),&argvalue) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	                f_optequal = TRUE ;

	            } else {

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
	                            searchname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            searchname = argp ;

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
	                        if (avl > 0) {

	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

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

	                    if (argl)
	                        rs = paramopt_loads(&aparams,PO_OPTION,
	                            argp,argl) ;

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

/* DB file */
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

/* argument file */
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

/* input file */
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

/* follow symbolic links */
	                case argopt_follow:
	                    pip->f.follow = TRUE ;
	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;

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
				    if (akl)
	                            rs = cfdeci(avp,avl, &pip->debuglevel) ;

	                        }

	                        break ;

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

			    case 'f':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            dbfname = argp ;

				break ;

/* file name length restriction */
	                    case 'l':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
				    if (avl)
	                            rs = cfdeci(avp,avl, &pip->namelen) ;

	                        } else {

	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
	                                break ;
	                            }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
				    if (argl)
	                            rs = cfdeci(argp,argl,&pip->namelen) ;

	                        }

	                        break ;

/* no-change */
	                    case 'n':
	                        pip->f.nochange = TRUE ;
	                        break ;

/* require a suffix for file names */
	                    case 's':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = paramopt_loads(&aparams,PO_SUFFIX,
	                                    avp,avl) ;

	                        } else {

	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
	                                break ;
	                            }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                rs = paramopt_loads(&aparams,PO_SUFFIX,
	                                    argp,argl) ;

	                        }

	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                        }

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        bprintf(pip->efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                    } /* end switch */

	                    akp += 1 ;
	                    if (rs < 0)
	                        break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        }

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (rs < 0)
	    goto badarg ;

/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: finished parsing command line arguments\n") ;
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
	debugprintf("main: afname=%s\n",afname) ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,
	        "%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bcontrol(pip->efp,BC_LINEBUF,0) ;

	    bflush(pip->efp) ;

	}

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

/* get the program root */

	if (pr == NULL) {

	    pr = getenv(VARPROGRAMROOT1) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT2) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT3) ;

/* try to see if a path was given at invocation */

	    if ((pr == NULL) && (pip->progdname != NULL))
	        proginfo_rootprogdname(pip) ;

/* do the special thing */

#if	CF_GETEXECNAME && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	    if ((pr == NULL) && (pip->pr == NULL)) {

	        const char	*pp ;


	        pp = getexecname() ;

	        if (pp != NULL)
	            proginfo_execname(pip,pp) ;

	    }
#endif /* SOLARIS */

	} /* end if (getting a program root) */

	if (pip->pr == NULL) {

	    if (pr == NULL)
	        pr = PROGRAMROOT ;

	    proginfo_setprogroot(pip,pr,-1) ;

	}

/* program search name */

	if (searchname == NULL)
	    searchname = getenv(VARSEARCHNAME) ;

#ifdef	COMMENT
	if ((searchname == NULL) && (pmspec != NULL)) {

	    searchname = pmspec ;

	}
#endif /* COMMENT */

	if (searchname == NULL) {

	    searchname = pip->progname ;
	    if ((cp = strchr(pip->progname,'.')) != NULL) {

	        searchname = tmpfname ;
	        strwcpy(tmpfname,pip->progname,(cp - pip->progname)) ;

	    }
	}

	proginfo_setsearchname(pip,VARSEARCHNAME,searchname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pr=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    bprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	}

/* get our program mode */

	if (pmspec == NULL)
	    pmspec = pip->searchname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0)
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif

	if (pip->progmode < 0)
	    pip->progmode = progmode_lkupfile ;

/* help file */

	if (f_help)
	    goto help ;


/* check a few more things */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv("TMPDIR") ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;

/* get ready */

	if ((rs = paramopt_havekey(&aparams,PO_SUFFIX)) > 0) {
	    pip->f.suffix = TRUE ;
	} /* end if */

	if ((rs = paramopt_havekey(&aparams,PO_OPTION)) > 0) {
	    PARAMOPT_CUR	cur ;

	    paramopt_curbegin(&aparams,&cur) ;

	    while (paramopt_enumvalues(&aparams,PO_OPTION,&cur,&cp) >= 0) {

		if (cp == NULL) continue ;

	        if ((kwi = matstr(progopts,cp,-1)) >= 0) {

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

	} /* end if (progopts) */

/* open the printer-default database */

	if ((dbfname == NULL) && (ifname != NULL))
		dbfname = ifname ;

	if ((dbfname == NULL) && ((cp = getenv(VARDBFILE)) != NULL))
		dbfname = cp ;

	if (dbfname == NULL)
		dbfname = STDINFNAME ;

	rs = fdbinit(pip,dbfname) ;

	if (rs < 0)
		goto baddbfile ;

/* open the output */

	if ((ofname != NULL) && (ofname[0] != '\0'))
	    rs = bopen(ofp,ofname,"wct",0644) ;

	else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0644) ;

	if (rs < 0) {

	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: cannot open output file (%d)\n",
		pip->progname,rs) ;

	    goto done1 ;
	}

/* OK, we do it */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    pan += 1 ;
	    rs = process(pip,argv[ai]) ;

	    if (rs < 0)
	        break ;

	} /* end for (looping through requested circuits) */

/* process any files in the argument filename list file */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {

	    bfile	argfile ;


	    if (strcmp(afname,"-") != 0)
	        rs = bopen(&argfile,afname,"r",0666) ;

	    else
	        rs = bopen(&argfile,BFILE_STDIN,"dr",0666) ;

	    if (rs >= 0) {

	        int	len ;

	        char	linebuf[LINEBUFLEN + 1] ;


	        while ((rs = breadline(&argfile,linebuf,LINEBUFLEN)) > 0) {

	            len = rs ;
	            if (linebuf[len - 1] == '\n')
	                len -= 1 ;

	            linebuf[len] = '\0' ;
		    if ((tp = strnchr(linebuf,len,'#')) != NULL)
			*tp = '\0' ;

	            cp = strshrink(linebuf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            pan += 1 ;
	            rs = process(pip,cp) ;

	            if (rs < 0)
	                break ;

	        } /* end while (reading lines) */

	        bclose(&argfile) ;

	    } else {

	        if (! pip->f.quiet) {

	            bprintf(pip->efp,
	                "%s: could not open the argument list file (%d)\n",
	                pip->progname,rs) ;

	            bprintf(pip->efp,"%s: \targfile=%s\n",
	                pip->progname,afname) ;

	        }

	    }

	} /* end if (processing file argument file list) */

	bclose(ofp) ;

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

done1:
	fdbfree(pip) ;

done0:
	if ((pan == 0) && (afname == NULL))
	    goto badnokeys ;

#ifdef	COMMENT
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: files=%u processed=%u\n",
	        pip->progname,pip->c_files,pip->c_processed) ;
#endif /* COMMENT */

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	    debugprintf("main: exiting (%d) ex=%u\n",rs,ex) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: ex=%u\n",
	        pip->progname,ex) ;

/* we are out of here */
ret2:

retearly:
ret1:
	if (pip->open.aparams)
	    paramopt_finish(&aparams) ;

	if (pip->efp != NULL)
	    bclose(pip->efp) ;

	ids_release(&pip->ids) ;

ret0:
	proginfo_finish(pip) ;

	return ex ;

/* program usage */
usage:
	usage(pip) ;

	goto retearly ;

/* print out some help */
help:
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	goto retearly ;

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

baddbfile:
	ex = EX_NOINPUT ;
	bprintf(pip->efp,"%s: could not open DB file (%d)\n",
	    pip->progname,rs) ;

	goto ret2 ;

badnokeys:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: no keys were specified\n",
	    pip->progname) ;

	goto ret2 ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [-db dbfile] key(s) [-Vv]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s: \t[-f] [-af {argfile|-}]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


