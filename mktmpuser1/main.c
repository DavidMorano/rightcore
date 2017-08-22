/* main */

/* front-end subroutine for the MKTMPUSER program */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* switchable */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ mktmpuser [<dir(s)>] [-V]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<bfile.h>
#include	<keyopt.h>
#include	<userinfo.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	BUFLEN		4096

#define	TMPUSERDNAME	"users"

#ifndef	TMPUSERDMODE
#define	TMPUSERDMODE	(S_IAMB | S_ISVTX)
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	isdigitlatin(int) ;

extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;
static int	proctmpuser(struct proginfo *) ;
static int	procname(struct proginfo *,bfile *,const char *) ;
static int	ensuremode(const char *,mode_t) ;

#ifdef	COMMENT
static int	dirok(const char *) ;
#endif


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
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


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	USERINFO	u ;

	BITS		pargs ;

	KEYOPT		akopts ;

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan = 0 ;
	int	rs = SR_OK ;
	int	rs1 ;
	int	v ;
	int	jobtime = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_version = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	userbuf[USERINFO_LEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*efname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

	pip->verboselevel = 1 ;
	pip->dmode = -1 ;

/* key options */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

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
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp + 1) ;

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

/* help */
	                case argopt_help:
	                    f_help = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* search-name */
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

/* argument-list filename */
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

/* output filename */
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

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

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

/* default action and user specified help */
	                default:
	                    f_usage = TRUE ;
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
			    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

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

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* file creation mode */
	                    case 'm':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfocti(argp,argl,&v) ;
	                            pip->dmode = v ;
				}
	                        break ;

/* options */
	                    case 'o':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = keyopt_loads(&akopts,argp,argl) ;
	                        break ;

/* quiet */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* remove the file (eventually) */
	                    case 'r':
	                        pip->f.remove = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdecti(avp,avl,&v) ;
	                                jobtime = v ;
				    }
	                        }
	                        break ;

/* timeout */
	                    case 't':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdeci(argp,argl,&v) ;
	                            jobtime = v ;
				}
	                        break ;

/* verbose (level) */
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

	if (efname == NULL) efname = getenv(VAREFNAME) ;
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

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_usage || f_version || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* check some arguments */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (afname == NULL) afname = getenv(VARAFNAME) ;

/* continue with other things */

	if ((jobtime < 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    jobtime = rs ;
	}

	if (jobtime < 0)
	    jobtime = JOBTIME ;

	if (pip->dmode > S_IAMB)
	    pip->dmode &= S_IAMB ;

	if (pip->dmode < 0)
	    pip->dmode = DMODE ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: tmpdir=%s\n",
	        pip->progname,pip->tmpdname) ;

	if (rs < 0)
	    goto retearly ;

/* get some user information (we need the 'username') */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;
	if (rs < 0) {
	    ex = EX_NOUSER ;
	    bprintf(pip->efp,
	        "%s: userinfo unavailable (%d)\n",
	        pip->progname,rs) ;

	    goto retearly ;
	}

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;
	pip->homedname = u.homedname ;

/* process the TMPUSER directory */

	rs = proctmpuser(pip) ;
	if (rs < 0) {
	    if (! pip->f.quiet)
		bprintf(pip->efp,"%s: TMPDIR inaccessible (%d)\n",rs) ;
	    goto badtmpuser ;
	}

/* open the output file */

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {

/* process names */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    rs = procname(pip,ofp,cp) ;

	    if (rs < 0) break ;
	} /* end for */

/* process any files in the argument filename list file */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afname,"-") == 0) afname = BFILE_STDIN ;

	    if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
		    lbuf[len] = '\0' ;

		    cp = lbuf ;
	            if (cp[0] == '\0') continue ;

	            pan += 1 ;
	            rs = procname(pip,ofp,cp) ;

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        bclose(afp) ;
	    } else {
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: inaccessible argument-list file (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",
	                pip->progname,afname) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	bclose(ofp) ;
	} else {
	    ex = EX_CANTCREAT ;
	    if (! pip->f.quiet)
	    bprintf(pip->efp,"%s: unavailable output (%d)\n",
	        pip->progname,rs) ;
	}

	if ((! pip->f.quiet) && (rs < 0))
	    bprintf(pip->efp,"%s: could not complete function (%d)\n",rs) ;

/* we are done! */
done:
badtmpuser:
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
		    const char	*pn = pip->progname ;
	            bprintf(pip->efp,"%s: invalid query (%d)\n",pn,rs) ;
		}
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

badoutopen:
badjobfile:
retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
		pip->progname,ex,rs) ;

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument (%d)\n",
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

	const char	*pn = pip->progname ;
	const char	*fmt ;


	fmt = "%s: USAGE> %s [<dir(s)>] [-af <afile>] \n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int proctmpuser(pip)
struct proginfo	*pip ;
{
	struct ustat	sb ;

	const int	tmpuserdmode = TMPUSERDMODE ;

	int	rs ;
	int	rs1 ;

	const char	*tmpuserdname = TMPUSERDNAME ;

	char	tmpdname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/proctmpuser: username=%s\n",pip->username) ;
#endif

	rs = mkpath2(tmpdname,pip->tmpdname,tmpuserdname) ;
	if (rs < 0)
	    goto ret0 ;

	rs1 = u_stat(tmpdname,&sb) ;
	if (rs1 == SR_NOEXIST) {
	    rs = u_mkdir(tmpdname,tmpuserdmode) ;
	    if (rs >= 0)
	        ensuremode(tmpdname,tmpuserdmode) ;
	} else
	    rs = rs1 ;

	if (rs < 0)
	    goto ret0 ;

	rs = mkpath3(tmpdname,pip->tmpdname,tmpuserdname,pip->username) ;
	if (rs < 0)
	    goto ret0 ;

	rs1 = u_stat(tmpdname,&sb) ;
	if (rs1 == SR_NOEXIST) {
	    rs = u_mkdir(tmpdname,pip->dmode) ;
	} else
	    rs = rs1 ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/proctmpuser: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (proctmpuser) */


static int procname(pip,ofp,name)
struct proginfo	*pip ;
bfile		*ofp ;
const char	name[] ;
{
	struct ustat	sb ;

	int	rs ;
	int	rs1 ;
	int	wlen = 0 ;

	const char	*td = TMPUSERDNAME ;

	char	tmpdname[MAXPATHLEN + 1] ;


	if (name == NULL)
	    return SR_FAULT ;

	if (name[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procname: name=%s\n",name) ;
#endif

	rs = mkpath4(tmpdname,pip->tmpdname,td,pip->username,name) ;

	if (rs >= 0) {
	    rs1 = u_stat(tmpdname,&sb) ;
	    if (rs1 >= 0) {
		if (! S_ISDIR(sb.st_mode))
		    rs = SR_NOTDIR ;
	    } else
	        rs = mkdirs(tmpdname,pip->dmode) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procname: mid rs=%d\n",rs) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: jobdir=%s (%d)\n",
		pip->progname,tmpdname,rs) ;

	if (rs >= 0) {
	    rs = bprintf(ofp,"%s\n",tmpdname) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procname: bprintf() rs=%d\n",rs) ;
#endif
	    wlen += rs ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procname: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procname) */


#ifdef	COMMENT

static int dirok(dname)
const char	dname[] ;
{
	struct ustat	sb ;

	int	rs ;
	int	preumask ;


	if ((rs = u_stat(dname,&sb)) < 0) {

	    if (rs == SR_NOEXIST) {

	        preumask = umask(0) ;
	        rs = mkdirs(dname,DMODE) ;
	        umask(preumask) ;

	        if (rs >= 0)
	            u_chmod(dname, (DMODE | S_ISVTX)) ;

	    }

	} else {

	    rs = SR_NOTDIR ;
	    if (S_ISDIR(sb.st_mode)) {

	        if ((sb.st_mode & 077) == 077)
	            rs = SR_OK ;

	        if (rs < 0)
	            rs = perm(dname,-1,-1,NULL,W_OK) ;

	    } /* end if */

	} /* end if */

	return rs ;
}
/* end subroutine (dirok) */

#endif /* COMMENT */


static int ensuremode(tmpdname,m)
const char	tmpdname[] ;
mode_t		m ;
{
	struct ustat	sb ;

	int	rs ;
	int	f = FALSE ;


	if (tmpdname == NULL)
	    return SR_FAULT ;

	if (tmpdname[0] == '\0')
	    return SR_INVALID ;

	if ((rs = u_open(tmpdname,O_RDONLY,0666)) >= 0) {
	    int	fd = rs ;
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
	        m = (m & (~ S_IFMT)) ;
	        if ((sb.st_mode & m) != m) {
		    f = TRUE ;
		    rs = u_fchmod(fd,m) ;
	        }
	    }
	    u_close(fd) ;
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (ensuremode) */



