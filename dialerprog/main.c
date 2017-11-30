/* main */

/* small (rather generic) front-end subroutine */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_SIGNAL	1		/* ? */


/* revision history:

	= 2009-05-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2009 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	Synopsis:

	$ testdialerprog <arguments>


*************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<ids.h>
#include	<userinfo.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"systems.h"
#include	"sysdialer.h"
#include	"cm.h"
#include	"prog.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	HOSTPARTLEN	1024

#ifndef	TO_CONNECT
#define	TO_CONNECT	20
#endif

#ifndef	TO_READ
#define	TO_READ		10
#endif

#ifndef	DEBFNAME
#define	DEBFNAME	"deb"
#endif


/* external subroutines */

extern int	snscs(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,
			const char *,const char *) ;
extern int	sncpy5(char *,int,const char *,const char *,
			const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	nprintf(const char *,const char *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_hdate(time_t,char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	makedate[] ;


/* local structures */

struct errormap {
	int		rs, ex ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	process(PROGINFO *,bfile *,cchar *,cchar *,int, VECSTR *) ;

static void	int_all() ;


/* global variables */

static volatile int	if_int ;


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"DEBUG",
	"VERSION",
	"VERBOSE",
	"HELP",
	"LOG",
	"MAKEDATE",
	"af",
	"if",
	"to",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_debug,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_log,
	argopt_makedate,
	argopt_af,
	argopt_if,
	argopt_to,
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

static cchar	*sysfiles[] = {
	"%p/etc/%n/%n.%f",
	"%p/etc/%n/%f",
	"%p/etc/%n.%f",
	"%p/%n.%f",
	"%n.%f",
	NULL
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	struct ustat	sb ;
	struct sigaction	sigs ;
	KEYOPT		akopts ;
	USERINFO	u ;
	SYSTEMS		sysdb ;
	SYSDIALER	d ;
	CM		con ;
	sigset_t	signalmask ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;
	vecstr		das ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan = 0 ;
	int	npa ;
	int	rs, rs1 ;
	int	len, i, j ;
	int	sl, cl ;
	int	tlen = 0 ;
	int	timeout = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_systems = FALSE ;
	int	f_dialer = FALSE ;
	int	f_cm = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*ifname = NULL ;
	const char	*logfname = NULL ;
	const char	*hostname = NULL ;
	const char	*svcspec = NULL ;
	const char	*fromaddr = NULL ;
	const char	*logpriority = NULL ;
	const char	*logtag = NULL ;
	const char	*tospec = NULL ;
	const char	*ofname = NULL ;
	const char	*sp, *cp ;

	char	argpresent[MAXARGGROUPS] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	fromaddrbuf[MAILADDRLEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;

	if_int = 0 ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	nprintf(DEBFNAME,"main: started\n") ;
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

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

/* perform argument processing */

	rs = keyopt_start(&akopts) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto ret2 ;
	}

	pip->open.akopts = (rs >= 0) ;
	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
	    argpresent[ai] = 0 ;

#if	CF_DEBUGS
	nprintf(DEBFNAME,"main: args\n") ;
#endif

	npa = 0 ;
	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0) && (npa < 1)) {

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

/* debug level */
	                case argopt_debug:
	                    pip->debuglevel = 1 ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            rs = cfdeci(avp,avl,&pip->debuglevel) ;

	                    }
	                    break ;

	                case argopt_version:
	                    f_version = TRUE ;
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

/* help file */
	                case argopt_help:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->helpfname = avp ;

	                    }

	                    f_help  = TRUE ;
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

/* log file */
	                case argopt_log:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            logfname = avp ;

	                    }

	                    break ;

	                case argopt_to:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            tospec = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            tospec = argp ;

	                    }

	                    break ;

/* display the time this program was last "made" */
	                case argopt_makedate:
	                    f_makedate = TRUE ;
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
	                            if (avl)
	                                rs = cfdeci(avp,avl, 
	                                    &pip->debuglevel) ;

	                        }

	                        break ;

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

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

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* from address */
	                    case 'f':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            fromaddr = argp ;

	                        break ;

	                    case 'i':
	                        pip->f.optin = TRUE ;
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

/* log priority */
	                    case 'p':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            logpriority = argp ;

	                        break ;

/* tag */
	                    case 't':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            logtag = argp ;

	                        break ;

	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                            }
	                        }

	                        break ;

/* fall through from above */
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

	        } /* end if (digits as argument or not) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;
		npa += 1 ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUGS
	    debugprintf("main: debuglevel=%u\n", pip->debuglevel) ;
	    debugprintf("main: npa=%u ai_pos=%u\n",npa,ai_pos) ;
#endif /* CF_DEBUG */

/* continue w/ the trivia argument processing stuff */

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_makedate)
	    bprintf(pip->efp,"%s: built %s\n",
	        pip->progname,makedate) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,NULL) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

/* help file */

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_usage || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* check arguments */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    if (BATST(argpresent,ai) && (argv[ai][0] != '\0')) {

	        BACLR(argpresent,ai) ;
	        if (strcmp(argv[ai],"-") != 0)
	            hostname = argv[ai] ;

	        break ;
	    }

	} /* end for */

	if ((hostname == NULL) || (hostname[0] == '\0')) {

	    if ((cp = getenv(LOGHOSTVAR)) != NULL)
	        hostname = cp ;

	}

	if ((hostname == NULL) || (hostname[0] == '\0'))
	    hostname = LOGHOST ;

	if ((svcspec == NULL) || (svcspec[0] == '\0')) {

	    if ((cp = getenv(VARSVC)) != NULL)
	        svcspec = cp ;

	}

	if ((svcspec == NULL) || (svcspec[0] == '\0'))
	    svcspec = SVCSPEC_TESTDIALERPROG ;

/* connection timeout */

	if ((tospec != NULL) && (tospec[0] != '\0')) {

	    rs = cfdeci(tospec,-1,&timeout) ;
	    if (rs < 0) {
	        ex = EX_USAGE ;
	        goto retearly ;
	    }

	} /* end if */

	if (timeout < 0)
	    timeout = TO_CONNECT ;

	if ((logpriority == NULL) || (logpriority[0] == '\0'))
	    logpriority = LOGPRIORITY ;

	if ((logtag == NULL) || (logtag[0] == '\0'))
	    logtag = LOGTAG ;

/* who are we? */

	ids_load(&pip->id) ;

	rs = userinfo(&u,userinfobuf,USERINFO_LEN,NULL) ;
	if (rs < 0)
	    goto baduser ;

	pip->pid = u.pid ;
	pip->domainname = u.domainname ;
	pip->nodename = u.nodename ;
	pip->username = u.username ;

/* other initialization */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: host=%s\n",hostname) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: host=%s\n",
	        pip->progname,hostname) ;

/* prepapre for other initialization */

	pip->daytime = time(NULL) ;

	rs = vecstr_start(&das,20,0) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badaddrinit ;
	}

	rs = vecstr_add(&das,"prog",-1) ;

	pan = 0 ;

	if (rs >= 0) {
	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = vecstr_add(&das,cp,-1) ;

	    if (rs < 0) break ;
	    if (if_int) break ;
	} /* end for (handling positional arguments) */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: hostname=%s\n",hostname) ;
		for (i = 0 ; vecstr_get(&das,i,&cp) >= 0 ; i += 1) {
			if (cp == NULL) continue ;
	    		debugprintf("main: a%u=>%s<\n",i,cp) ;
		}
	}
#endif /* CF_DEBUG */

/* do the main thing */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: performing main processing\n") ;
#endif

	(void) sigemptyset(&signalmask) ;

#if	CF_SIGNAL
	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGHUP,&sigs,NULL) ;

	sigs.sa_handler = int_all ;
	sigs.sa_mask = signalmask ;
	sigs.sa_flags = 0 ;
	u_sigaction(SIGPIPE,&sigs,NULL) ;
#endif /* CF_SIGNAL */

	if (rs < 0)
	    goto badaddradd ;

/* open output */

	if ((ofname != NULL) && (ofname[0] != '\0')) {
	    rs = bopen(ofp,ofname,"wct",0644) ;
	} else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0644) ;

	if (rs < 0) {
		ex = EX_CANTCREAT ;
		bprintf(pip->efp,"%s: could not open output (%d)\n",
	    	pip->progname,rs) ;

		goto badoutopen ;
	}

/* perform the processing */

	if (rs >= 0)
	    rs = process(pip,ofp,hostname,svcspec,timeout,&das) ;

/* close output */

	bclose(ofp) ;

badoutopen:
	len = vecstr_count(&das) ;

badaddradd:
	vecstr_finish(&das) ;

badaddrinit:
baduser:
	ids_release(&pip->id) ;

	if (if_int)
	    rs = SR_INTR ;

done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

/* close off and get out! */
retearly:
ret3:
	if (pip->open.akopts)
	    keyopt_finish(&akopts) ;

ret2:
	if (pip->efp != NULL)
	    bclose(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

ret0:
badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_shcat: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad argument stuff */
badarg:
	ex = EX_USAGE ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


void int_all(signum)
int	signum ;
{


	if_int = TRUE ;

}


static int usage(pip)
PROGINFO	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [recipient(s) ...]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s: \t[-oi] [-i] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int process(pip,ofp,hostname,svcspec,timeout,alp)
PROGINFO	*pip ;
bfile		*ofp ;
const char	hostname[] ;
const char	svcspec[] ;
int		timeout ;
VECSTR		*alp ;
{
	SYSDIALER_ARGS	da ;
	PROG		po ;

	int	rs = SR_OK ;
	int	len ;
	int	to = -1 ;
	int	opts = 0 ;

	const char	**av = NULL ;

	char	buf[BUFLEN + 1] ;


	memset(&da,0,sizeof(SYSDIALER_ARGS)) ;

	da.pr = pip->pr ;
	da.timeout = timeout ;

	vecstr_getvec(alp,&av) ;

	da.argv = (const char **) av ;

/* make the call to open */

	rs = prog_open(&po,&da,hostname,svcspec,NULL) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: prog_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    while ((rs = prog_reade(&po,buf,BUFLEN,to,opts)) > 0) {
		len = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: line=>%t<\n",
		buf,strlinelen(buf,len,40)) ;
#endif

		rs = bwrite(ofp,buf,len) ;

		if (rs < 0) break ;
	    } /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: end-while rs=%d\n",rs) ;
#endif

	    prog_close(&po) ;
	} /* end if */

	return rs ;
}
/* end subroutine (process) */



