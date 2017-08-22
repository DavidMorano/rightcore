/* main */

/* fitler some text */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1987-09-10, David A­D­ Morano
	This subroutine was originally written but it was probably started from
	any one of the numerous subroutine which perform a similar
	"file-processing" fron end.

*/

/* Copyright © 1987 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program will read the input file, separate one paragraph from
        another and then post each paragraph using the MSGS program.

	Synopsis:

	$0 [input_file [...]] [-DV] [-o offset]


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<bfile.h>
#include	<vecstr.h>
#include	<pcsconf.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<baops.h>
#include	<exitcodes.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define		MAXARGINDEX	100
#define		NARGGROUPS	(MAXARGINDEX/8 + 1)
#define		LINELEN		200
#define		BUFLEN		(MAXPATHLEN + (2 * LINELEN))


/* external subroutines */

extern int	cfdeci(char *,int,int *) ;
extern int	isdigitlatin(int) ;

extern int	getpfopts(struct proginfo *,vecstr *) ;
extern int	procfile(struct proginfo *,bfile *,char *,int) ;

extern char	*strbasename(char *) ;


/* forward references */

static void	helpfile(const char *,bfile *) ;


/* local structures */


/* global data */


/* local variables */

static const char *argopts[] = {
	    "ROOT",
	    "DEBUG",
	    "VERSION",
	    "VERBOSE",
	    "HELP",
	    "TMPDIR",
	    NULL,
} ;

#define	ARGOPT_ROOT		0
#define	ARGOPT_DEBUG		1
#define	ARGOPT_VERSION		2
#define	ARGOPT_VERBOSE		3
#define	ARGOPT_HELP		4
#define	ARGOPT_TMPDIR		5


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;
	USERINFO	u ;
	PCSCONF		cs ;
	logfile		lh ;
	bfile		outfile, *ofp = &outfile ;
	bfile		errfile, *efp = &errfile ;
	vecstr		sets, extras ;

	int	argr, argl, aol, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs = SR_OK ;
	int	maxai, i ;
	int	facts = 0 ;
	int	blen ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *avp ;
	char	argpresent[NARGGROUPS] ;
	char	buf[BUFLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	pcsconfbuf[PCSCONF_LEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	memset(pip,0,sizeof(struct proginfo *)) ;

	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"wca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

	pip->efp = efp ;

	pip->verboselevel = 1 ;

	pip->daytime = time(NULL) ;

/* process program arguments */

	rs = SR_OK ;
	for (ai = 0 ; ai < NARGGROUPS ; ai += 1) argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	maxai = 0 ;
	i = 0 ;
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

	            rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

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
	                    case ARGOPT_ROOT:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->pr = avp ;

	                        } else {

	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pip->pr = argp ;

	                        }

	                        break ;

/* debug level */
	                    case ARGOPT_DEBUG:
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                &pip->debuglevel) < 0) ;

	                        }

	                        break ;

	                    case ARGOPT_VERSION:
	                        f_version = TRUE ;
	                        break ;

	                    case ARGOPT_VERBOSE:
	                        pip->f.verbose = TRUE ;
	                        break ;

/* help file */
	                    case ARGOPT_HELP:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->helpfname = avp ;

	                        }

	                        f_help  = TRUE ;
	                        break ;

/* temporary directory */
	                    case ARGOPT_TMPDIR:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->tmpdname = avp ;

	                        }

	                        break ;

	                    } /* end switch (key words) */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
					if (avl)
	                                rs = cfdeci(avp,avl,&pip->debuglevel) ;

	                            }

	                            break ;

	                        case 'M':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pip->mailername = argp ;

	                            break ;

/* quiet mode */
	                        case 'Q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* no operation (don't post) */
	                        case 'n':
	                            pip->f.no = TRUE ;
	                            break ;

/* output file */
	                        case 'o':
	                        case 'w':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) 
					ofname = argp ;

	                            break ;

/* verbose mode */
	                        case 'v':
	                            pip->f.verbose = TRUE ;
	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
					break ;

	                        default:
				f_usage = TRUE ;
				rs = SR_INVALID ;
				bprintf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

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
	    bprintf(efp,"%s: debuglevel %d\n",
	        pip->progname,pip->debuglevel) ;

/* get our program root (if we have one) */

	if (pip->pr == NULL) {

	    if (pip->pr == NULL)
	        pip->pr = getenv(VARPROGRAMROOT1) ;

	    if (pip->pr == NULL)
	        pip->pr = getenv(VARPROGRAMROOT2) ;

	    if (pip->pr == NULL)
	        pip->pr = getenv(VARPROGRAMROOT3) ;

	    if (pip->pr == NULL)
	        pip->pr = PROGRAMROOT ;

	} /* end if */


/* continue w/ the trivia argument processing stuff */

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	}

	ex = EX_INFO ;
	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto retearly ;


	if (f_help) {

	    if (pip->helpfname == NULL) {

	        blen = bufprintf(buf,BUFLEN,"%s/%s",
	            pip->pr,HELPFNAME) ;

	        pip->helpfname = mallocstrw(buf,blen) ;

	    }

	    helpfile(pip->helpfname,pip->efp) ;

	    goto retearly ;
	}


/* do we have a good temporary directory ? */

	if ((pip->tmpdname == NULL) || 
		(u_access(pip->tmpdname,W_OK | R_OK) < 0)) {

	    pip->tmpdname = getenv("TMPDIR") ;

	    if ((pip->tmpdname == NULL) || 
		(u_access(pip->tmpdname,W_OK | R_OK) < 0))
	        pip->tmpdname = TMPDNAME ;

	} /* end if */

/* get user information */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

/* get our configuration information from the PCS-wide configuration */

#if	CF_DEBUGS
	debugprintf("main: pcsconf()\n") ;
#endif

	vecstr_start(&sets,10,0) ;

	vecstr_start(&extras,10,0) ;

	rs = pcsconf(pip->pr,NULL,&cs,&sets,&extras,
	    pcsconfbuf,PCSCONF_LEN) ;

/* see if some optional parameters were for us in the PCS-wide configuration */

	(void) getpfopts(pip,&sets) ;

	vecstr_finish(&extras) ;

	vecstr_finish(&sets) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("main: prog_msgs=%s\n",pip->prog_msgs) ;
	    debugprintf("main: spooldname=%s\n",pip->spooldname) ;
	}
#endif

/* log us */

	bufprintf(buf,BUFLEN,"%s/%s",pip->pr,LOGFNAME) ;

	pip->f.log = FALSE ;
	if ((rs = logfile_open(&lh,buf,0,0666,u.logid)) >= 0) {

	    pip->f.log = TRUE ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: opened log file\n") ;
#endif

	    buf[0] = '\0' ;
	    if (u.fullname != NULL)
	        strcpy(buf,u.fullname) ;

	    else if (u.name != NULL)
	        strcpy(buf,u.name) ;

	    else if (u.gecosname != NULL)
	        strcpy(buf,u.gecosname) ;

	    else if (u.mailname != NULL)
	        strcpy(buf,u.mailname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: log name=%s\n",buf) ;
#endif

	    logfile_printf(&lh,"%s %-14s %s/%s\n",
	        timestr_log(pip->daytime,timebuf),
	        pip->progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	    if (buf[0] != '\0')
	        logfile_printf(&lh,"%s!%s (%s)\n",
	            u.nodename,u.username,buf) ;

	        else
	        logfile_printf(&lh,"%s!%s\n",
	            u.nodename,u.username) ;

	    logfile_printf(&lh,
	        "os=%s d=%s\n",
	        (u.f.sysv_rt) ? "SYSV" : "BSD",
	        u.domainname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: wrote log entry\n") ;
#endif

	} /* end if (opened the log file) */


/* write user's mail address (roughly as we have it) into the user list file */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: writing user file\n") ;
#endif

	rs = pcsuserfile(pip->pr,USERFNAME,u.nodename,u.username,buf) ;

	if (rs == 1)
	    logfile_printf(&lh,
	        "created the user list file\n") ;

	else if (rs < 0)
	    logfile_printf(&lh,
	        "could not access user list file (rs %d)\n",
	        rs) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: wrote user file, rs=%d\n",rs) ;
#endif


/* OK, check to see if what we have as working parameters look OK so far */

	if ((pip->prog_rbbpost != NULL) && (pip->prog_rbbpost[0] != '/') &&
	    ((rs = pcsgetprog(pip->pr,buf,pip->prog_rbbpost)) >= 0)) {

		if (rs > 0) {

	    if (pip->prog_rbbpost != NULL)
	        free(pip->prog_rbbpost) ;

	    pip->prog_rbbpost = mallocstr(buf) ;

		}

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: new RBBPOST program=%s\n",
	            pip->prog_msgs) ;
#endif

	} /* end if */

	if ((pip->prog_msgs != NULL) && (pip->prog_msgs[0] != '/') &&
	    ((rs = pcsgetprog(pip->pr,buf,pip->prog_msgs)) >= 0)) {

		if (rs > 0) {

	    if (pip->prog_msgs != NULL)
	        free(pip->prog_msgs) ;

	    pip->prog_msgs = mallocstr(buf) ;

		}

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: new MSGS program=%s\n",
	            pip->prog_msgs) ;
#endif

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: done w/ basic stuff\n") ;
#endif


/* defaults */

	if (pip->mailername == NULL)
	    pip->mailername = MAILERNAME ;

	if (pip->prog_rbbpost == NULL)
	    pip->prog_rbbpost = PROG_RBBPOST ;

	if (pip->prog_msgs == NULL)
	    pip->prog_msgs = PROG_MSGS ;

	if (pip->newsgroup == NULL)
	    pip->newsgroup = DEFNEWSGROUP ;

	if (pip->spooldname == NULL)
	    pip->spooldname = DEFSPOOLDNAME ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("main: prog_rbbpost=%s\n",pip->prog_rbbpost) ;
	    debugprintf("main: prog_msgs=%s\n",pip->prog_msgs) ;
	    debugprintf("main: mailername=%s\n",pip->mailername) ;
	    debugprintf("main: newsgroup=%s\n",pip->newsgroup) ;
	    debugprintf("main: spooldname=%s\n",pip->spooldname) ;
	}
#endif


	ex = EX_DATAERR ;

/* open output file */

	if ((ofname == NULL) || (ofname[0] == '-'))
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	else
	    rs = bopen(ofp,ofname,"wct",0666) ;

	if (rs < 0)
	    goto badoutopen ;


/* processing the input file arguments */

#if	CF_DEBUG
	if (pip->debuglevel > 0)
	    debugprintf("main: processing positional arguments\n") ;
#endif

	pan = 0 ;
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if (BATST(argpresent,i)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 0) debugprintf(
	                "main: got a positional argument i=%d pan=%d arg=%s\n",
	                i,pan,argv[i]) ;
#endif


	            rs = procfile(pip,ofp,argv[i],pan + 1) ;

	            if (rs < 0) {

	                if (! pip->f.quiet)
	                    bprintf(pip->efp,
				"%s: error processing file \"%s\"\n",
	                        pip->progname,argv[i]) ;

	            } else
	                facts += rs ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: files processed so far %d\n",
	                    rs,facts) ;
#endif

	            pan += 1 ;

	        } /* end if (got a positional argument) */

	    } /* end for (loading positional arguments) */

	} else {

	    rs = procfile(pip,ofp,"-",pan + 1) ;

	    if (rs < 0) {

	        if (! pip->f.quiet)
	            bprintf(pip->efp,"%s: error processing file \"%s\"\n",
	                pip->progname,argv[i]) ;

	    } else
	        facts += rs ;

	    pan += 1 ;

	} /* end if */


/* close the output file */

	bclose(ofp) ;

	if ((pip->debuglevel > 0) || (! pip->f.quiet)) {

	    bprintf(efp,"%s: files processed %d, facts posted %d\n",
	        pip->progname,pan,facts) ;

	}

	ex = EX_OK ;

/* let's get out of here !! */
done:


/* close off and get out ! */
exit:
retearly:
	bclose(efp) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* what are we about? */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-o outfile] [infile [...]]\n",
	    pip->progname,pip->progname) ;

	bprintf(efp,"%s: \t[-DV]\n",pip->progname) ;

	bprintf(efp,
	    "%s: \tinfile      input file\n",
		pip->progname) ;

	bprintf(efp,
	    "%s: \t-w outfile  output file\n",
		pip->progname) ;

	bprintf(efp,
	    "%s: \t-D          debugging flag\n",
		pip->progname) ;

	bprintf(efp,
	    "%s: \t-V          program version\n",
		pip->progname) ;

	goto retearly ;

/* bad stuff comes here */
badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",pip->progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto retearly ;

/* handle those little programs that happen later on in the program */
badoutopen:
	bprintf(efp,"%s: could not open output file \"%s\" (rs %d)\n",
	    pip->progname,ofname,rs) ;


badret:
	ex = EX_DATAERR ;
	goto retearly ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static void helpfile(f,ofp)
const char	f[] ;
bfile		*ofp ;
{
	bfile	file, *ifp = &file ;


	if ((f == NULL) || (f[0] == '\0')) 
	    return ;

	if (bopen(ifp,f,"r",0666) >= 0) {

	    bcopyblock(ifp,ofp,-1) ;

	    bclose(ifp) ;

	}

}
/* end subroutine (helpfile) */



