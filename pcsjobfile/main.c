/* main */

/* front-end subroutine for the PCSJOBFILE program */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* switchable */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ pcsjobfile [-D] [-r] [-t <timeout>] [<tmpdir>] [-V]


**************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<keyopt.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	(1000000 / 20)
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	BUFLEN		4096

#ifndef	JOBDMODE
#define	JOBDMODE	0777
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfocti(const char *,int,int *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	mkjobfile(const char *,int,char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	unlinkd(const char *,int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;

extern cchar	*getourenv(cchar **,cchar *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;
static int	dirok(const char *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"of",
	"mtg",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_sn,
	argopt_of,
	argopt_mtg,
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

static const char	*jobdirs[] = {
	MKJOB1DNAME,
	MKJOB2DNAME,
	MKJOB3DNAME,
	NULL
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	KEYOPT	akopts ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs = SR_OK ;
	int	rs1 = SR_OK ;
	int	i ;
	int	jobtime = -1 ;
	int	jobfilemode = JOBFILEMODE ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_version = FALSE ;
	int	f_meeting = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*pr = NULL ;
	const char	*searchname = NULL ;
	const char	*ofname = NULL ;
	const char	*jobdname = NULL ;
	const char	*mtgdate = NULL ;
	const char	*cp ;
	char	argpresent[MAXARGGROUPS + 1] ;
	char	jobdnamebuf[MAXPATHLEN + 1] ;
	char	jobfname[MAXPATHLEN + 1] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

	pip->verboselevel = 1 ;

/* miscellaneous early stuff */

	jobdnamebuf[0] = '\0' ;

/* key options */

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

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

	        if (isdigit(argp[1])) {

	            rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	        } else if (argp[1] == '-') {

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

/* search-name */
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

/* handle the meeting_date name format */
	                    case argopt_mtg:
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            mtgdate = argp ;

	                        break ;

/* help */
	                case argopt_help:
	                    f_help = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;

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
	                        if (avl)
	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

	                    }

	                    break ;

/* default action and user specified help */
	                default:
	                    f_usage = TRUE ;
	                    rs = SR_INVALID ;
	                    bprintf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
			    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl, 
	                                    &pip->debuglevel) ;

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

	                        if (argl)
	                            rs = cfocti(argp,argl,&jobfilemode) ;

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
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &jobtime) ;

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

	                        if (argl)
	                            rs = cfdeci(argp,argl,&jobtime) ;

	                        break ;

/* verbose (level) */
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
	                        f_usage = TRUE ;
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

	        } /* end if (digits as argument or not) */

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

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    usage(pip) ;

/* program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,searchname) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

/* help */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_usage || f_version || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* check some arguments */

	pan = 0 ;
	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    jobdname = cp ;
	    break ;

	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: jobdname=%s\n",jobdname) ;
#endif

/* get an optional job file directory */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv(VARTMPDNAME) ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;

/* what about the directory to create the jobfile locks? */

	if (jobdname == NULL)
	    jobdname = getenv(VARJOBDNAME) ;

	rs1 = SR_OK ;
	if (jobdname == NULL) {

	    for (i = 0 ; jobdirs[i] != NULL ; i += 1) {

	        jobdname = (char *) jobdirs[i] ;
	        if (jobdirs[i][0] != '/') {

	            jobdname = jobdnamebuf ;
	            mkpath2(jobdnamebuf,pip->pr,jobdirs[i]) ;

	        }

	        rs1 = dirok(jobdname) ;
	        if (rs1 >= 0)
	            break ;

	    } /* end for */

	} /* end if */

	if ((rs1 < 0) || (jobdname == NULL))
	    jobdname = pip->tmpdname ;

/* continue with other things */

	if ((jobtime < 0) && (argvalue >= 0))
	    jobtime = argvalue ;

	if (jobtime < 0)
	    jobtime = JOBTIME ;

	if (mtgdate != NULL) {

	    f_meeting = TRUE ;
	    if (*mtgdate == '-')
	        *mtgdate = '\0' ;

	}

	if (jobfilemode > 0777)
	    jobfilemode &= 0777 ;

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: jobdname=%s\n",
	        pip->progname,jobdname) ;

	    bprintf(pip->efp,"%s: jobtime=%u\n",
	        pip->progname,jobtime) ;

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: f_meeting=%u\n",f_meeting) ;
#endif

/* create a job ID file in the specified directory */

	for (i = 0 ; i < TRIES ; i += 1) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: top of for %d\n",i) ;
#endif

	    if (f_meeting) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: mkdatefile()\n") ;
#endif

	        rs = mkdatefile(jobdname,".msg",jobfilemode,jobfname) ;

	    } else {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: mkjobfile() jobdname=%s\n",jobdname) ;
#endif

	        rs = mkjobfile(jobdname,jobfilemode,jobfname) ;

	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: mkfilex() rs=%d jobfname=%s\n",
			rs,jobfname) ;
#endif

	    if ((rs == SR_NOTDIR) || (rs == SR_NOENT) || (rs >= 0))
	        break ;

	    sleep(WAITTIME) ;

	} /* end for (trying for success) */

	if (rs < 0) {
	    ex = EX_TEMPFAIL ;
	    if (! pip->f.quiet)
	    bprintf(pip->efp,"%s: unavailable jobfile (%d)\n",
	        pip->progname,rs) ;
	    goto badjobfile ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: jobfname=%s\n",jobfname) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: jobfname=%s\n",
	        pip->progname,jobfname) ;

/* open the output file */

	if ((ofname != NULL) && (ofname[0] != '\0'))
	    rs = bopen(ofp,ofname,"wct",0666) ;

	else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    if (! pip->f.quiet)
	    bprintf(pip->efp,"%s: unavailable output (%d)\n",
	        pip->progname,rs) ;

	    goto badoutopen ;
	}

	bprintf(ofp,"%s\n",jobfname) ;

	bclose(ofp) ;

/* spawn child to remove the file later */

	bflush(pip->efp) ;

	if (pip->f.remove && (jobtime >= 0)) {

	    if (jobtime > 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main: unlinkd() jf=%s jt=%d\n",
			jobfname,jobtime) ;
#endif

	        rs = unlinkd(jobfname,jobtime) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main: unlinkd() rs=%d\n",rs) ;
#endif

	    } else
	        u_unlink(jobfname) ;

	} /* end if (auto-remove) */

/* we are done! */
done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

badoutopen:
badjobfile:

retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
		pip->progname,ex,rs) ;

ret3:
	if (pip->open.akopts)
	    keyopt_finish(&akopts) ;

ret2:
	if (pip->efp != NULL)
	    bclose(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

ret0:

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


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [<jobdir>] [-r] [-t <timeout>] [-mtg <mtgdate>]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s: \t[-Q] [-D] [-v[=n]] [-HELP] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int dirok(dirname)
const char	dirname[] ;
{
	struct ustat	sb ;

	int	rs ;
	int	preumask ;


	if ((rs = u_stat(dirname,&sb)) < 0) {

	    if (rs == SR_NOEXIST) {

	        preumask = umask(0) ;

	        rs = mkdirs(dirname,JOBDMODE) ;

	        umask(preumask) ;

	        if (rs >= 0)
	            u_chmod(dirname, (JOBDMODE | S_ISVTX)) ;

	    }

	} else {

	    rs = SR_NOTDIR ;
	    if (S_ISDIR(sb.st_mode)) {

	        if ((sb.st_mode & 077) == 077)
	            rs = SR_OK ;

	        if (rs < 0)
	            rs = perm(dirname,-1,-1,NULL,W_OK) ;

	    }

	} /* end if */

	return rs ;
}
/* end subroutine (dirok) */



