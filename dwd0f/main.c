/* main (DWD) */

/* Directory Watcher Daemon (DWD) */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_FINALROOT	1
#define	CF_NPRINTF	0


/* revision history:

	= 1998-09-10, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program will watch a directory for new entries and will
	wait until a new entry stops increasing in size.  After a new
	entry stops increasing in size, this program will look it up in
	the service table and will pass the file to a another program
	specified by the service table entry that matches the file name.

	Environment variables:

		PROGRAMROOT	root of program files

	Synopsis:

	$ dwd [-C conf] [-polltime] [directory_path] [srvtab] [-V?]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mkdev.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<dirent.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<char.h>
#include	<lfm.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"configfile.h"
#include	"job.h"
#include	"srvtab.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	TO_PIDFILE	2

#define	DEBUGFNAME	"/tmp/dwd.deb"


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	getpwd(char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	isdigitlatin(int) ;

extern int	proginfo_setpiv(PROGINFO *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	expander() ;

extern char	*timestr_logz(time_t,char *) ;
extern char	*delenv(const char *) ;


/* externals variables */


/* local structures */


/* forward references */

static int usage(PROGINFO *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"CONFIG",
	"TMPDNAME",
	"LOGFILE",
	"HELP",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_conf,
	argopt_tmpdir,
	argopt_logfile,
	argopt_help,
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


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct ustat	sb ;
	PROGINFO	pi, *pip = &pi ;
	USERINFO	u ;
	CONFIGFILE	cf ;
	SRVTAB		sf ;
	SRVTAB_ENT	*sep ;
	JOB		jobs ;

	bfile	errfile ;
	bfile	logfile ;
	bfile	lockfile ;

	time_t	daytime = time(NULL) ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan = 0 ;
	int	rs = SR_OK ;
	int	rs1 ;
	int	i, j, k, l ;
	int	len, sl, iw ;
	int	maxjobs = 0 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	filetime = JOBIDLETIME ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_exports = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*configfname = NULL ;
	const char	*logfname = NULL ;
	const char	*sp, *cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	if ((cp = getenv(VARBANNER)) == NULL)
	    cp = BANNER ;

	proginfo_setbanner(pip,cp) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->f.errfile = TRUE ;
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	} /* end if (we have some STDERR) */

/* initialize */

	pip->tmpdname = TMPDNAME ;
	pip->workdname = TMPDNAME ;

	pip->verboselevel = 1 ;
	pip->pollmodetime = POLLMODETIME ;

	pip->pid = getpid() ;		/* this is not the daemon PID */

	pip->lfp = &logfile ;

	pip->polltime = -1 ;

	configfname = NULL ;

/* start parsing the arguments */

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
	                        f_version = TRUE ;
	                        if (f_optequal)
	                            rs = SR_INVALID ;

	                        break ;

/* verbose mode */
	                    case argopt_verbose:
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

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

/* configuration file */
	                    case argopt_conf:
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

/* log file */
	                    case argopt_logfile:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                logfname = avp ;

	                        } else {

	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                logfname = argp ;

	                        }

	                        break ;

				case argopt_help:
					f_help = TRUE ;
					break ;

/* handle all keyword defaults */
	                    default:
	                        rs = SR_INVALID ;
	                        bprintf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    rs = cfdeci(avp,avl,
	                                        &pip->debuglevel) ;

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

/* mutex lock PID file */
	                        case 'P':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pip->pidfname = argp ;

	                            break ;

	                        case 'Q':
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* default command string */
	                        case 'c':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pip->command = argp ;

	                            break ;

/* poll only */
	                        case 'p':
	                            pip->f.poll = TRUE ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    rs = cfdecti(avp,avl,
	                                        &pip->pollmodetime) ;

	                            }

	                            break ;

/* verbose mode */
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

	        } /* end if (digits or options) */

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
	    goto usage ;

	if (f_version)
	    goto retearly ;


/* miscellaneous */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

/* help */

	if (f_help)
	    goto help ;


	ex = EX_OK ;

/* load the positional arguments */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	        switch (pan) {

	        case 0:
	            pip->directory = (char *) argv[ai] ;
	            break ;

	        case 1:
	            pip->srvtab = (char *) argv[ai] ;
	            break ;

	        default:
	            rs = SR_INVALID ;
	            bprintf(pip->efp,
	                "%s: extra positional arguments specified\n",
	                pip->progname) ;

	        } /* end switch */

	        pan += 1 ;
		if (rs < 0)
			break ;

	} /* end for (loading positional arguments) */

	if (rs < 0)
		goto badarg ;

#ifdef	COMMENT
	(void) delenv(VARPROGRAMROOT2) ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    if (tmpfname[0] != '\0')
		proginfo_getpwd(pip,tmpfname, MAXPATHLEN) ;

	    bprintf(pip->efp,"%s: pwd=%s\n",
	        pip->progname,tmpfname) ;

	} /* end if (some prints) */


	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	if (rs < 0) {
	    ex = EX_NOUSER ;
	    goto baduser ;
	}

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->logid = u.logid ;


	if ((rs = vecstr_start(&pip->exports,100,0)) < 0)
	    goto badinit ;

	if ((rs = vecstr_start(&pip->paths,10,0)) < 0) {

	    vecstr_finish(&pip->exports) ;

	    goto badinit ;
	}

	f_exports = TRUE ;


/* read in a configuration file if we have one */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking for configuration file\n") ;
#endif

	if (configfname == NULL)
	    configfname = getenv(VARCONF) ;

	if (configfname == NULL)
	    configfname = CONFIGFNAME1 ;

	if ((configfname == NULL) || (u_access(configfname,R_OK) < 0))
	    configfname = CONFIGFNAME2 ;

	if (u_access(configfname,R_OK) >= 0) {

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: configuration=%s\n",
	            pip->progname,configfname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we have a configuration file, pwd=%s\n",
	            (getpwd(tmpfname,MAXPATHLEN),tmpfname)) ;
#endif

	    rs = configfile_start(&cf,configfname) ;

	    if (rs < 0)
	        goto badconfig ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: good configuration file=%s\n",
	            configfname) ;
#endif


#ifdef	COMMENT
	    if ((cf.programroot != NULL) && (pip->pr == NULL)) {

	        if ((l = expander(pip,cf.programroot,-1,buf,BUFLEN)) >= 0) {

	            proginfo_setprogroot(pip,buf,l) ;

	        }

	    } /* end if (programroot) */
#endif /* COMMENT */

	    if ((cf.directory != NULL) && (pip->directory == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: expander 1\n") ;
#endif

	        if ((l = expander(pip,cf.directory,-1,buf,BUFLEN)) >= 0)
	            pip->directory = mallocstrw(buf,l) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: expander 1 done\n") ;
#endif

	    }

	    if ((cf.interrupt != NULL) && (pip->interrupt == NULL)) {

	        if ((l = expander(pip,cf.interrupt,-1,buf,BUFLEN)) >= 0)
	            pip->interrupt = mallocstrw(buf,l) ;

	    }

	    if ((cf.workdir != NULL) && (pip->workdname == NULL)) {

	        if ((l = expander(pip,cf.workdir,-1,buf,BUFLEN)) >= 0)
	            pip->workdname = mallocstrw(buf,l) ;

	    }

	    if ((cf.srvtab != NULL) && (pip->srvtab == NULL)) {

	        if ((l = expander(pip,cf.srvtab,-1,buf,BUFLEN)) >= 0)
	            pip->srvtab = mallocstrw(buf,l) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: C srvtab=%s\n", pip->srvtab) ;
#endif

	    }

	    if ((cf.pidfname != NULL) && (pip->pidfname == NULL)) {

	        if ((l = expander(pip,cf.pidfname ,-1,buf,BUFLEN)) >= 0)
	            pip->pidfname = mallocstrw(buf,l) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: pidfname=%s\n",
	                cf.pidfname) ;
#endif

	    }

	    if ((cf.lockfname != NULL) && (pip->lockfname == NULL)) {

	        if ((l = expander(pip,cf.lockfname,-1,buf,BUFLEN)) >= 0)
	            pip->lockfname = mallocstrw(buf,l) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: config lockfile=%s\n",
	                cf.lockfname) ;
#endif

	    }

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("main: cf_logfname=%s\n",cf.logfname) ;
	        debugprintf("main: logfname=%s\n",logfname) ;
	    }
#endif

	    if ((cf.logfname != NULL) && (logfname == NULL)) {

	        if ((l = expander(pip,cf.logfname,-1,buf,BUFLEN)) >= 0)
	            logfname = mallocstrw(buf,l) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: C l=%d logfname=%s\n",l,logfname) ;
#endif

	    } /* end if (logfname) */

	    if ((cf.polltime != NULL) && (pip->polltime < 1)) {

	        if ((sl = expander(pip,cf.polltime,-1,buf,BUFLEN)) >= 0) {

	            if (cfdeci(buf,sl,&iw) >= 0)
	                pip->polltime = iw ;

	        }
	    }

	    if ((cf.filetime != NULL) && (filetime < 1)) {

	        if ((sl = expander(pip,cf.filetime,-1,buf,BUFLEN)) >= 0) {

	            if ((cfdeci(buf,sl,&iw) >= 0) && (iw > 1))
	                filetime = iw ;

	        }
	    }

	    if ((cf.maxjobs != NULL) && (maxjobs < 1)) {

	        if ((sl = expander(pip,cf.maxjobs,-1,buf,BUFLEN)) >= 0) {

	            if (cfdeci(buf,sl,&iw) >= 0)
	                maxjobs = iw ;

	        }
	    }



	    for (i = 0 ; vecstr_get(&cf.exports,i,&sp) >= 0 ; i += 1) {

		if (((cp = strchr(sp,'=')) != NULL) && (cp[1] != '\0'))
	        	vecstr_add(&pip->exports,sp,-1) ;

	    } /* end for */

#ifdef	COMMENT
	    for (i = 0 ; (rs = vecstr_get(&cf.paths,i,&sp)) >= 0 ; i += 1)
	        vecstr_add(&pip->paths,sp,-1) ;
#endif

	    configfname = NULL ;
	    configfile_finish(&cf) ;

	} else {

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,
	            "%s: could not find a configuration file\n",
	            pip->progname) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: no configuration file!\n") ;
#endif

	} /* end if */


/* add the passed environment to the exported environment */

	for (i = 0 ; envv[i] != NULL ; i += 1) {

	    if (vecstr_finder(&pip->exports,envv[i],vstrkeycmp,NULL) < 0)
		vecstr_add(&pip->exports,envv[i],-1) ;

	} /* end for */

/* add any variables that SHOULD be there but aren't for some reason */

	cp = "LOGNAME" ;
	if (vecstr_finder(&pip->exports,cp,vstrkeycmp,NULL) < 0)
		vecstr_envadd(&pip->exports,cp,u.username,-1) ;

	cp = "HOME" ;
	if (vecstr_finder(&pip->exports,cp,vstrkeycmp,NULL) < 0)
		vecstr_envadd(&pip->exports,cp,u.homedname,-1) ;


/* before we go too far, are we supposed to observe a lock file? */

	if (pip->lockfname != NULL) {

	    if (pip->lockfname[0] == '\0')
	        pip->lockfname = LOCKFNAME ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we have a 1 lockfile=%s\n",
	            pip->lockfname) ;
#endif

	    if (u_stat(pip->lockfname,&sb) >= 0) {

	        if ((daytime - sb.st_mtime) <= LOCKTIMEOUT) {
	            goto badlock ;
	        } else
	            u_unlink(pip->lockfname) ;

	    } /* end if (good stat) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: past stat in 1 lockfile\n") ;
#endif

	} /* end if (lock file) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: past 1 lockfile\n") ;
#endif


/* before we go too far, are we the only one on this PID mutex? */

	if (pip->pidfname != NULL) {

	    if ((pip->pidfname[0] == '\0') || (pip->pidfname[0] == '-'))
	        pip->pidfname = PIDFNAME ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: pidfname=%s\n",pip->pidfname) ;
#endif

	    rs = lfm_start(&pip->pider,
		pip->pidfname,LFM_TRECORD,TO_PIDFILE,NULL,
	        pip->nodename,pip->username,BANNER) ;

	    if ((rs < 0) && (rs != SR_LOCKLOST) && (rs != SR_AGAIN))
	        goto badpidopen ;

	    if (rs < 0)
	        goto badpidlock ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: we captured the PID lock, rs=%d\n",
	            rs) ;
#endif

	    lfm_finish(&pip->pider) ;		/* releases the lock also */

	} /* end if (we have a mutex PID file) */


/* check program parameters */

	if (pip->command != NULL) {

	    cp = pip->command ;
	    while (CHAR_ISWHITE(*cp))
	        cp += 1 ;

	    if (*cp == '\0')
	        pip->command = NULL ;

	}

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	    debugprintf("main: dir=%s\n",pip->directory) ;
	    debugprintf("main: srvtab=%s\n",pip->srvtab) ;
	    debugprintf("main: command=%s\n",pip->command) ;
	    debugprintf("main: polltime=%d\n",pip->polltime) ;
	}
#endif /* CF_DEBUG */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: checking program parameters\n") ;
#endif

	if ((pip->srvtab == NULL) || (pip->srvtab[0] == '\0'))
	    pip->srvtab = SRVFNAME1 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: 1\n") ;
#endif

	if ((pip->directory == NULL) || (pip->directory[0] == '\0'))
	    pip->directory = DIRECTORY ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: 2\n") ;
#endif

	if (pip->workdname == NULL)
	    pip->workdname = WORKDNAME ;

	else if (pip->workdname[0] == '\0')
	    pip->workdname = "." ;

	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0')) {

	    if ((cp = getenv("TMPDNAME")) != NULL)
	        pip->tmpdname = cp ;

	    else
	        pip->tmpdname = TMPDNAME ;

	}

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: 'interrupt'\n") ;
#endif

	if (pip->interrupt != NULL) {

	    if (pip->interrupt[0] != '\0') {

	        if ((u_access(pip->interrupt,R_OK) < 0) ||
	            (u_stat(pip->interrupt,&sb) < 0) ||
	            ((! S_ISFIFO(sb.st_mode)) && (! S_ISCHR(sb.st_mode)))) {

	            u_unlink(pip->interrupt) ;

	            mktmpfile(tmpfname, (0666 | S_IFIFO), pip->interrupt) ;

	            u_chmod(pip->interrupt,0666) ;

	        }

	    } else
	        pip->interrupt = INTERRUPT ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: intfile=%s\n",pip->interrupt) ;
#endif

	    if ((u_access(pip->interrupt,R_OK) >= 0) &&
	        (u_stat(pip->interrupt,&sb) >= 0) &&
	        (S_ISFIFO(sb.st_mode) || S_ISCHR(sb.st_mode))) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	            debugprintf("main: accessible intfile=%s\n",
			pip->interrupt) ;
#endif

	        pip->f.interrupt = TRUE ;

	    }

	} /* end if (interrupt preparation) */

	if (pip->polltime < 2)
	    pip->polltime = POLLTIME ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	    debugprintf("main: program parameters\n") ;
	    debugprintf("main: dir=%s\n",pip->directory) ;
	    debugprintf("main: srvtab=%s\n",pip->srvtab) ;
	    debugprintf("main: command=%s\n",pip->command) ;
	    debugprintf("main: polltime=%d\n",pip->polltime) ;
	}
#endif /* CF_DEBUG */

/* open the system report log file */

#ifdef	COMMENT
	if ((rs = bopen(pip->lfp,logfname,"wca",0664)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: system log rs=%d\n",rs) ;
#endif

	        daytime = time(NULL) ;

	    bprintf(lfp,"%s: %s Directory Watcher Daemon (DWD) started\n",
	        pip->progname,timestr_logz(daytime,timebuf)) ;

	    bflush(pip->lfp) ;

	} /* end if (opening log file) */
#endif /* COMMENT */


/* can we access the working directory? */

	if ((u_access(pip->workdname,X_OK) < 0) ||
	    (u_access(pip->workdname,R_OK) < 0))
	    goto badworking ;


/* what about the queue directory */

	if (pip->directory[0] != '/') {

	    rs = proginfo_getpwd(pip,tmpfname,MAXPATHLEN) ;

	    if (rs < 0)
	        goto badqueue ;

	    mkpath2(tmpfname, pip->pwd,pip->directory) ;

	    pip->directory = mallocstr(tmpfname) ;

	} /* end if */

	if (maxjobs < 1)
	    maxjobs = MAXJOBS ;


	if ((pip->pwd == NULL) || (pip->pwd[0] == '\0'))
		proginfo_getpwd(pip,NULL,0) ;


/* do we have an activity log file? */

	rs = SR_NOENT ;
	if ((logfname != NULL) && (logfname[0] != '\0'))
	    rs = logfile_open(&pip->lh,logfname,0,0666,pip->logid) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: logfile_open() 1 rs=%d lf=%s\n",rs,logfname) ;
#endif

	if ((rs < 0) && (u_access(LOGFNAME,W_OK) >= 0))
	    rs = logfile_open(&pip->lh,LOGFNAME,0,0666,pip->logid) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: logfile_open() 2 rs=%d lf=%s\n",rs,LOGFNAME) ;
#endif

	if (rs >= 0) {

	    pip->f.log = TRUE ;
	        daytime = time(NULL) ;

	    logfile_printf(&pip->lh,"") ;

	    logfile_printf(&pip->lh,"") ;

	    logfile_printf(&pip->lh,"%s %s started",
	        timestr_logz(daytime,timebuf),
	        BANNER) ;

	    logfile_printf(&pip->lh,"%-14s %s/%s",
	        pip->progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	    logfile_printf(&pip->lh,"pr=%s",pip->pr) ;

	} /* end if (we have a log file) */


/* initialize the job table */

	rs = job_init(&jobs) ;

	if (rs < 0)
	    goto badinit ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
		debugprintf("main: intialized the job table \n") ;
#endif


/* process the server table file */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: checking for service table file, srvtab=%s\n",
	        pip->srvtab) ;
#endif

	pip->f.srvtab = FALSE ;
	if (u_access(pip->srvtab,R_OK) >= 0)
	    pip->f.srvtab = TRUE ;

	else
	    pip->srvtab = SRVFNAME2 ;

	if (pip->f.srvtab || (u_access(pip->srvtab,R_OK) >= 0)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: we have a service table file=%s\n",
	            pip->srvtab) ;
#endif

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: srvtab=%s\n",
	            pip->progname,pip->srvtab) ;

	    rs = srvtab_open(&sf,pip->srvtab,NULL) ;

	    pip->f.srvtab = (rs >= 0) ;
	    if (rs < 0)
	        goto badsrv ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: we have a good service table file\n") ;
#endif

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {

	        for (i = 0 ; (rs = srvtab_get(&sf,i,&sep)) >= 0 ; i += 1) {

	            debugprintf("main: got a service entry\n") ;

	            debugprintf("main: srvtab service=%s\n",sep->service) ;

	            debugprintf("main: srvtab program=%s\n",sep->program) ;

	            debugprintf("main: srvtab args=%s\n",sep->args) ;

	            debugprintf("main: srvtab username=%s\n",sep->username) ;

	            debugprintf("main: srvtab groupname=%s\n",sep->groupname) ;

#ifdef	COMMENT
	            if (sep->program != NULL) {

	                if (u_access(sep->program,X_OK) < 0)
	                    goto badsrv ;

	            }
#endif /* COMMENT */

	        } /* end for */

	    } /* end if */
#endif /* CF_DEBUG */

	    logfile_printf(&pip->lh,"srvtab=%s",
	        pip->srvtab) ;

	} /* end if (have a service table file) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: done w/ srvtab file?\n") ;
#endif

	if ((pip->command == NULL) && (! pip->f.srvtab))
	    goto badnosrv ;


/* miscellaneous logs */

	logfile_printf(&pip->lh,"poll=%d",
	    pip->polltime) ;


/* become a daemon program */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: become a daemon?\n") ;
#endif

	bclose(pip->efp) ;

#if	CF_DEBUG
	if (pip->debuglevel == 0)
	    debugclose() ;
#endif

	rs = BAD ;
	if (pip->debuglevel == 0) {

#ifdef	COMMENT
	    for (i = 0 ; i < 3 ; i += 1)
	        u_close(i) ;
#endif /* COMMENT */

	    if (pip->f.log)
	        logfile_flush(&pip->lh) ;

	    rs = uc_fork() ;

	    if (rs < 0) {

	        logfile_printf(&pip->lh,"could not fork daemon (%d)",
	            rs) ;

	        uc_exit(PRS_BADFORK) ;
	    }

/* parent exits */

	    if (rs > 0)
	        uc_exit(EX_OK) ;

/* child becomes a session leader */

	    u_setsid() ;

	} /* end if (becoming a daemon) */

/* we start! */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: starting to watch for new jobs\n") ;
#endif

	pip->pid = getpid() ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: final PID=%d\n",(int) pip->pid) ;
#endif

	if (userbuf[0] != '\0')
	    u.pid = pip->pid ;


/* create a log ID */

	logfile_setid(&pip->lh,pip->logid) ;

	if (rs == 0) {

	    logfile_printf(&pip->lh,"backgrounded") ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: backgrounded\n") ;
#endif

	}


/* before we go too far, are we supposed to use a lock file? */

	pip->lockfp = NULL ;
	if ((pip->lockfname != NULL) && (pip->lockfname[0] != '\0')) {

		int	devmajor, devminor ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: we have a 2 lockfile=%s\n",pip->lockfname) ;
#endif

	    if ((rs = bopen(&lockfile,pip->lockfname,"rwce",0444)) < 0)
	        goto badlock2 ;

/* capture the lock (if we can) */

	    if ((rs = bcontrol(&lockfile,BC_LOCK,2)) < 0)
	        goto badlock2 ;

	    bcontrol(&lockfile,BC_TRUNCATE,0L) ;

	    bseek(&lockfile,0L,SEEK_SET) ;

	    bprintf(&lockfile,"%d\n",(int) pip->pid) ;

	    bprintf(&lockfile,"%s!%s\n",pip->nodename,pip->username) ;

	    bprintf(&lockfile,"%s %s\n",
	        timestr_logz(daytime,timebuf),
	        BANNER) ;

	    bprintf(&lockfile,"%-14s %s/%s\n",
	        pip->progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	    if (userbuf[0] != '\0')
	        bprintf(&lockfile,"d=%s %s!%s pid=%d\n",
	            u.domainname,
	            u.nodename,
	            u.username,
	            (int) pip->pid) ;

	    bflush(&lockfile) ;

/* we leave the file open as our mutex lock! */

	    logfile_printf(&pip->lh,"lockfile=%s",pip->lockfname) ;

	    logfile_printf(&pip->lh,"lock file captured") ;

	    bcontrol(&lockfile,BC_STAT,&sb) ;

		devmajor = major(sb.st_dev) ;

		devminor = minor(sb.st_dev) ;

	    logfile_printf(&pip->lh,"lockfile device=%u,%u inode=%lu",
	        devmajor,devminor,sb.st_ino) ;

	    pip->lockfp = &lockfile ;

	} /* end if (capturing lock file) */

/* before we go too far, are we the only one on this PID mutex? */

	if ((pip->pidfname != NULL) && (pip->pidfname[0] != '\0')) {

	    LFM_INFO	pli ;

		int	devmajor, devminor ;


	    rs = lfm_start(&pip->pider,
		pip->pidfname,LFM_TRECORD,TO_PIDFILE,NULL,
	        pip->nodename,pip->username,BANNER) ;

	    if (rs < 0)
	        goto badpidfile2 ;

	    pip->f.pidfile = TRUE ;
	    lfm_printf(&pip->pider,"%-14s %s/%s\n",
	        pip->progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	    if (userbuf[0] != '\0')
	        lfm_printf(&pip->pider,"d=%s %s!%s pid=%d\n",
	            u.domainname,
	            u.nodename,
	            u.username,
	            pip->pid) ;

	    lfm_flush(&pip->pider) ;

/* we leave the file open as our mutex lock! */

	    logfile_printf(&pip->lh,"pidfile=%s",pip->pidfname) ;

	    logfile_printf(&pip->lh,"PID mutex captured") ;

	    lfm_getinfo(&pip->pider,&pli) ;

		devmajor = major(pli.dev) ;

		devminor = minor(pli.dev) ;

	        logfile_printf(&pip->lh,
		    "pidfile device=%u,%u inode=%u",
	            devmajor,devminor,pli.ino) ;

	    if (pip->debuglevel > 0) {

	        bprintf(pip->efp,"%s: pidfile=%s\n",pip->pidfname) ;

	        bprintf(pip->efp,"%s: PID file device=%u,%u inode=%u\n",
	            devmajor,devminor,pli.ino) ;

	    }

	} /* end if (we have a mutex PID file) */


/* make some last log entries before we get into bad boogying! */

	if (pip->f.log && (userbuf[0] != '\0')) {

	        daytime = time(NULL) ;

	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        bufprintf(buf,BUFLEN,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        bufprintf(buf,BUFLEN,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        bufprintf(buf,BUFLEN,"(%s)",u.fullname) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s) pid=%d",
	        u.f.sysv_rt ? "SYSV" : "BSD",
	        u.sysname,u.release,
	        pip->pid) ;

	    logfile_printf(&pip->lh,"%s!%s %s",
	        u.nodename,u.username,buf) ;

	    logfile_printf(&pip->lh,"dir=%s",pip->directory) ;

	    if (pip->f.interrupt)
	        logfile_printf(&pip->lh,"intfile=%s",
	            pip->interrupt) ;

	    logfile_printf(&pip->lh,"%s finished initializing",
	        timestr_logz(daytime,timebuf)) ;

	    logfile_flush(&pip->lh) ;

	} /* end if (making log entries) */


/* do the thing! */

	rs = watch(pip,&sf,&jobs,maxjobs,filetime) ;


/* and we are done? and out of here */

	if ((pip->lockfname != NULL) && (pip->lockfname[0] != '\0'))
	    u_unlink(pip->lockfname) ;

	if ((pip->pidfname != NULL) && (pip->pidfname[0] != '\0'))
	    lfm_finish(&pip->pider) ;

	if (pip->f.srvtab)
		srvtab_close(&sf) ;

	job_free(&jobs) ;


	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;


/* we are done */
done:
ret4:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

	if (pip->f.log)
	    logfile_close(&pip->lh) ;

ret3:
	if ((configfname != NULL) && (configfname[0] != '\0'))
	    configfile_finish(&cf) ;

ret2:
	if (f_exports) {

	vecstr_finish(&pip->paths) ;

	vecstr_finish(&pip->exports) ;

	}

	if (pip->lfp != NULL)
	    bclose(pip->lfp) ;

baduser:
ret1:
retearly:
	bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

	return ex ;

/* USAGE> dwd [-C conf] [-polltime] [directory_path] [srvtab] [-V?] */
usage:
	usage(pip) ;

	goto retearly ;

/* help */
help:
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	    goto retearly ;

/* bad argument usage */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp, "%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

/* errors other than argument related */
badworking:
	bprintf(pip->efp,
	    "%s: could not access the working directory \"%s\"\n",
	    pip->progname,pip->workdname) ;

	ex = EX_CONFIG ;
	goto badret ;

badqueue:
	bprintf(pip->efp,"%s: could not process the queue directory (%d)\n",
	    pip->progname,rs) ;

	ex = EX_CONFIG ;
	goto badret ;

badinit:
	bprintf(pip->efp,"%s: could not initialize list structures (%d)\n",
	    pip->progname,rs) ;

	ex = EX_UNAVAILABLE ;
	goto badret ;

badsrv:
	bprintf(pip->efp,"%s: bad service table file (%d)\n",
	    pip->progname,rs) ;

	ex = EX_CONFIG ;
	goto badret ;

badnosrv:
	bprintf(pip->efp,
	    "%s: no service table file and no command specified\n",
	    pip->progname) ;

	ex = EX_CONFIG ;
	goto badret ;

baddir:
	bprintf(pip->efp,"%s: bad working directory specified (%d)\n",
	    pip->progname,rs) ;

	ex = EX_CONFIG ;
	goto badret ;

badconfig:
	bprintf(pip->efp,
	    "%s: error (%d) in configuration file starting at line %d\n",
	    pip->progname,rs,cf.badline) ;

	ex = EX_CONFIG ;
	goto badret ;

badlock:
	if (! pip->f.quiet) {

	    bprintf(pip->efp,
	        "%s: there was a lock file \"%s\" already\n",
	        pip->progname,pip->lockfname) ;

	    if (bopen(&lockfile,pip->lockfname,"r",0666) >= 0) {

	        while ((len = breadline(&lockfile,buf,BUFLEN)) > 0) {

	            bprintf(pip->efp,"%s: pidfile> %t",
	                pip->progname,
	                buf,len) ;

	        }

	        bclose(&lockfile) ;

	    }
	}

	ex = EX_MUTEX ;
	goto badret ;

badpidopen:
	bprintf(pip->efp,
	    "%s: could not open the PID lock file (%d)\n",
	    pip->progname,rs) ;

	bprintf(pip->efp,
	    "%s: PID file=%s\n",
	    pip->progname,pip->pidfname) ;

	ex = EX_MUTEX ;
	goto badret ;

badpidlock:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: PID lock failed, rs=%d\n",rs) ;
#endif

	if ((pip->debuglevel > 0) || (! pip->f.quiet)) {

	    bprintf(pip->efp,
	        "%s: could not open the PID lock file (%d)\n",
	        pip->progname,rs) ;

	    bprintf(pip->efp,
	        "%s: PID file=%s\n",
	        pip->progname,pip->pidfname) ;

	}

	ex = EX_MUTEX ;
	goto badret ;

badlock2:
	logfile_printf(&pip->lh,
	    "there was a daemon already on the lock file \"%s\"\n",
	    pip->lockfname) ;

	bclose(&lockfile) ;

	if (bopen(&lockfile,pip->lockfname,"r",0666) >= 0) {

	    while ((len = breadline(&lockfile,buf,BUFLEN)) > 0) {

	        logfile_printf(&pip->lh,"lockfile> %t",buf,len) ;

	    } /* end while */

	    bclose(&lockfile) ;

	}

	ex = EX_MUTEX ;
	goto daemonret ;

badpidfile2:
	if ((pip->debuglevel > 0) || (! pip->f.quiet))
	    logfile_printf(&pip->lh,
	        "could not capture the PID lock file (%d)\n",rs) ;

	ex = EX_MUTEX ;
	goto daemonret ;

/* other situations come here */
daemonret:
	goto done ;


badret:
	ex = EX_DATAERR ;
	goto done ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
PROGINFO	*pip ;
{
	int	rs ;


	bprintf(pip->efp,
	    "%s: USAGE> %s [-C conf] [-polltime] [directory] [srvtab] [-?v] ",
	    pip->progname,pip->progname) ;

	rs = bprintf(pip->efp,"[-D[=n]]\n") ;

	return rs ;
}
/* end subroutine (usage) */



