/* main (DWD) */

/* Directory Watcher Daemon (DWD) */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG		1		/* switchable debug print-outs */


/* revision history :

	= 1991-09-10, David Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This program will watch a directory for new entries
	and will wait until a new entry stops increasing in size.
	After a new entry stops increasing in size, this program
	will look it up in the service table and will pass the
	file to a another program specified by the service table
	entry that matches the file name.

	Environment variables :

		PROGRAMROOT	root of program files

	Synopsis :

	$ dwd [-C conf] [-polltime] [directory_path] [srvtab] [-V?]



*****************************************************************************/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<ftw.h>
#include	<dirent.h>
#include	<stdlib.h>
#include	<string.h>

#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecelem.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<baops.h>
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
#define	NUMNAMES	12
#define	NAMEBUFLEN	(NUMNAMES * MAXPATHLEN)
#define	TO_PIDFILE	2


/* external subroutines */

extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getpwd(char *,int) ;
extern int	expander() ;

extern char	*strbasename(char *) ;
extern char	*timestrlog() ;
extern char	*delenv() ;


/* externals variables */


/* forwards */


/* local global variabes */


/* local structures */

/* define program invocation option words */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"ROOT",
	"CONFIG",
	"TMPDIR",
	"LOGFILE",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_root,
	argopt_conf,
	argopt_tmpdir,
	argopt_logfile,
	argopt_overlast
} ;

/* exported subroutines */


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	struct ustat		sb ;

	struct proginfo		g, *pip = &g ;

	struct userinfo		u ;

	bfile		errfile, *efp = &errfile ;
	bfile		logfile ;
	bfile		lockfile ;

	CONFIGFILE	cf ;

	SRVTAB		sf ;

	SRVTAB_ENTRY	*sep ;

	JOB		jobs ;

	time_t	daytime ;

	int	argr, argl, aol, akl, avl ;
	int	maxai, pan, npa, kwi, i, j, k, l ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	ex = EX_INFO ;
	int	rs, srs ;
	int	len, sl, iw ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	maxjobs = 0 ;
	int	filetime = JOBIDLETIME ;
	int	fd_debug ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	timebuf[32] ;
	char	*cp ;
	char	*sp ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	pwd[MAXPATHLEN + 1] ;
	char	*configfname = NULL ;
	char	*logfname = NULL ;


	if ((cp = getenv(DEBUGFDVAR1)) == NULL)
		cp = getenv(DEBUGFDVAR2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    esetfd(fd_debug) ;


/* early initialize */

	memset(pip,0,sizeof(struct proginfo)) ;

	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BIO_STDERR,"dwca",0666) >= 0) {
	    pip->f.error = TRUE ;
	pip->efp = &errfile ;
	    bcontrol(efp,BC_LINEBUF,0) ;
	    u_close(2) ;
	} /* end if (we have some STDERR) */

/* initialize */

	pip->f.error = FALSE ;
	pip->f.interrupt = FALSE ;
	pip->f.srvtab = FALSE ;
	pip->f.quiet = FALSE ;
	pip->f.poll = FALSE ;

	pip->debuglevel = 0 ;
	pip->verboselevel = 1 ;
	pip->pollmodetime = POLLMODETIME ;

	pip->pid = getpid() ;		/* this is not the daemon PID */

	pip->lfp = &logfile ;

	pip->polltime = -1 ;

	pip->programroot = NULL ;
	pip->version = VERSION ;
	pip->nodename = NULL ;
	pip->domainname = NULL ;

	pip->command = NULL ;
	pip->username = NULL ;
	pip->groupname = NULL ;
	pip->srvtab = NULL ;
	pip->directory = NULL ;
	pip->interrupt = NULL ;
	pip->lockfname = NULL ;
	pip->pidfname = NULL ;
	pip->tmpdir = TMPDIR ;
	pip->workdir = TMPDIR ;


	configfname = NULL ;
	srs = PRS_OK ;

/* start parsing the arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                if (((argl - 1) > 0) && 
	                    (cfdeci(argp + 1,argl - 1,&pip->polltime) < 0))
	                    goto badarg ;

	            } else {

#if	CF_DEBUGS
	                eprintf("main: got an option\n") ;
#endif

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                    eprintf("main: got an option key w/ a value\n") ;
#endif

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

#if	CF_DEBUGS
	                    eprintf("main: aol=%d avp=\"%s\" avl=%d\n",
	                        aol,avp,avl) ;

	                    eprintf("main: akl=%d akp=\"%s\"\n",
	                        akl,akp) ;
#endif

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

/* do we have a keyword match or should we assume only key letters ? */

#if	CF_DEBUGS
	                eprintf("main: about to check for a key word match\n") ;
#endif

	                if ((kwi = optmatch(argopts,akp,akl)) >= 0) {

#if	CF_DEBUGS
	                    eprintf("main: got an option keyword, kwi=%d\n",
	                        kwi) ;
#endif

	                    switch (kwi) {

	                    case argopt_tmpdir:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->tmpdir = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pip->tmpdir = argp ;

	                        }

	                        break ;

/* version */
	                    case argopt_version:
	                        f_version = TRUE ;
	                        if (f_optequal) 
					goto badargextra ;

	                        break ;

/* verbose mode */
	                    case argopt_verbose:
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }
	                        }

	                        break ;

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->programroot = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pip->programroot = argp ;

	                        }

	                        break ;

/* configuration file */
	                    case argopt_conf:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					configfname = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
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

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					logfname = argp ;

	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
				f_usage = TRUE ;
				ex = EX_USAGE ;
	                        bprintf(efp,"%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

	                        goto badarg ;

	                    } /* end switch */

	                } else {

#if	CF_DEBUGS
	                    eprintf("main: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        eprintf("main: option key letters\n") ;
#endif

	                        switch (*akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

#if	CF_DEBUGS
	                                eprintf(
	                                    "main: debug flag, avp=\"%W\"\n",
	                                    avp,avl) ;
#endif

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
							&pip->debuglevel) ;

						if (rs < 0)
	                                    goto badargval ;

					}
	                            }

	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* configuration file */
	                        case 'C':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					configfname = argp ;

	                            break ;

/* mutex lock PID file */
	                        case 'P':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pip->pidfname = argp ;

	                            break ;

/* default command string */
	                        case 'c':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
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
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
						&pip->pollmodetime) ;

						if (rs < 0)
	                                    	goto badargval ;

					}
	                            }

	                            break ;

/* quiet mode */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* verbose mode */
	                        case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0) {

	                                    rs = cfdeci(avp,avl,
						&pip->verboselevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
	                            if (srs != PRS_OK)
	                                srs = PRS_USAGE ;

				break ;

	                        default:
	                            srs = PRS_BADOPT ;
					ex = EX_USAGE ;
					f_usage = TRUE ;
	                            bprintf(efp,"%s: unknown option - %c\n",
	                                pip->progname,*akp) ;

	                        } /* end switch */

	                        akp += 1 ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


	if (pip->debuglevel > 0) 
		bprintf(efp,
	    "%s: finished parsing arguments\n",
	    pip->progname) ;

	if (f_version) 
		bprintf(efp,"%s: version %s\n",
	    pip->progname,VERSION) ;

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto earlyret ;

	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: debuglevel=%d\n",
	        pip->progname,pip->debuglevel) ;


/* load the positional arguments */

#if	CF_DEBUG
	if (pip->debuglevel > 1) eprintf(
	    "main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1) 
			eprintf(
	            "main: got a positional argument i=%d pan=%d arg=%s\n",
	            i,pan,argv[i]) ;
#endif

	        switch (pan) {

	        case 0:
	            pip->directory = (char *) argv[i] ;
	            break ;

	        case 1:
	            pip->srvtab = (char *) argv[i] ;
	            break ;

	        default:
	            bprintf(efp,
	                "%s: extra positional arguments ignored\n",
	                pip->progname) ;

	        } /* end switch */

	        pan += 1 ;

	    } /* end if (got a positional argument) */

	} /* end for (loading positional arguments) */


/* miscellaneous */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: programroot\n") ;
#endif

	buf[0] = '\0' ;
	if (pip->programroot == NULL) {

	    pip->programroot = getenv(PROGRAMROOTVAR1) ;

	    if (pip->programroot == NULL) {

	        if ((pip->programroot = getenv(PROGRAMROOTVAR2)) != NULL) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("main: got a PR, pr=%s\n",
	                    pip->programroot) ;
#endif /* CF_DEBUG */

	        }
	    }

/* for a last resort, we use our PWD for the program root */

	    if ((pip->programroot == NULL) &&
	        ((rs = getpwd(buf,BUFLEN)) > 0))
	        pip->programroot = mallocstrn(buf,rs) ;

	} /* end if (getting a program root from environment) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: programroot=%s\n",pip->programroot) ;
#endif


/* time to clean up the environment! */

	(void) delenv(PROGRAMROOTVAR2) ;


	if (pip->debuglevel > 0) {

	    bprintf(efp,"%s: programroot=%s\n",
	        pip->progname,pip->programroot) ;

	    if (buf[0] == '\0')
	        rs = getpwd(buf,BUFLEN) ;

	    bprintf(efp,"%s: pwd=%s\n",
	        pip->progname,buf) ;

	} /* end if (some prints) */


	if ((rs = userinfo(&u,userbuf,USERINFO_LEN,NULL)) < 0)
	    goto baduser ;

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;

	if ((rs = vecstr_start(&pip->exports,10,0)) < 0)
	    goto badinit ;

	if ((rs = vecstr_start(&pip->paths,10,0)) < 0) {

	    vecstr_finish(&pip->exports) ;

	    goto badinit ;
	}


/* read in a configuration file if we have one */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: checking for configuration file\n") ;
#endif

	if (configfname == NULL)
	    configfname = CONFIGFNAME1 ;

	if ((configfname == NULL) || (u_access(configfname,R_OK) < 0))
	    configfname = CONFIGFNAME2 ;

	if (u_access(configfname,R_OK) >= 0) {

	    if (pip->debuglevel > 0)
	        bprintf(efp,"%s: configuration=%s\n",
	            pip->progname,configfname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: we have a configuration file, pwd=%s\n",
	            (getpwd(tmpfname,MAXPATHLEN),tmpfname)) ;
#endif

	    if ((rs = configfile_init(&cf,configfname)) < 0)
	        goto badconfig ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: good configuration file=%s\n",
	            configfname) ;
#endif


#ifdef	COMMENT
	    if ((cf.programroot != NULL) && (pip->programroot == NULL)) {

	        if ((l = expander(pip,cf.programroot,-1,buf,BUFLEN)) >= 0) {

	            pip->programroot = mallocstrn(buf,l) ;

	        }

	    } /* end if (programroot) */
#endif /* COMMENT */

	    if ((cf.directory != NULL) && (pip->directory == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: expander 1\n") ;
#endif

	        if ((l = expander(pip,cf.directory,-1,buf,BUFLEN)) >= 0)
	            pip->directory = mallocstrn(buf,l) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: expander 1 done\n") ;
#endif

	    }

	    if ((cf.interrupt != NULL) && (pip->interrupt == NULL)) {

	        if ((l = expander(pip,cf.interrupt,-1,buf,BUFLEN)) >= 0)
	            pip->interrupt = mallocstrn(buf,l) ;

	    }

	    if ((cf.workdir != NULL) && (pip->workdir == NULL)) {

	        if ((l = expander(pip,cf.workdir,-1,buf,BUFLEN)) >= 0)
	            pip->workdir = mallocstrn(buf,l) ;

	    }

	    if ((cf.srvtab != NULL) && (pip->srvtab == NULL)) {

	        if ((l = expander(pip,cf.srvtab,-1,buf,BUFLEN)) >= 0)
	            pip->srvtab = mallocstrn(buf,l) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: C srvtab=%s\n", pip->srvtab) ;
#endif

	    }

	    if ((cf.pidfname != NULL) && (pip->pidfname == NULL)) {

	        if ((l = expander(pip,cf.pidfname ,-1,buf,BUFLEN)) >= 0)
	            pip->pidfname = mallocstrn(buf,l) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: pidfname=%s\n",
	                cf.pidfname) ;
#endif

	    }

	    if ((cf.lockfname != NULL) && (pip->lockfname == NULL)) {

	        if ((l = expander(pip,cf.lockfname,-1,buf,BUFLEN)) >= 0)
	            pip->lockfname = mallocstrn(buf,l) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: config lockfile=%s\n",
	                cf.lockfname) ;
#endif

	    }

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        eprintf("main: cf_logfname=%s\n",cf.logfname) ;
	        eprintf("main: logfname=%s\n",logfname) ;
	    }
#endif

	    if ((cf.logfname != NULL) && (logfname == NULL)) {

	        if ((l = expander(pip,cf.logfname,-1,buf,BUFLEN)) >= 0)
	            logfname = mallocstrn(buf,l) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: C l=%d logfname=%s\n",l,logfname) ;
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



	    for (i = 0 ; (rs = vecstr_get(&cf.exports,i,&sp)) >= 0 ; i += 1)
	        vecstr_add(&pip->exports,sp,-1) ;

#ifdef	COMMENT
	    for (i = 0 ; (rs = vecstr_get(&cf.paths,i,&sp)) >= 0 ; i += 1)
	        vecstr_add(&pip->paths,sp,-1) ;
#endif

	    (void) configfile_free(&cf) ;

	} else {

	    if (pip->debuglevel > 0)
	        bprintf(efp,
	            "%s: could not find a configuration file\n",
	            pip->progname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 0)
	        eprintf("main: no configuration file !\n") ;
#endif

	} /* end if */


/* before we go too far, are we supposed to observe a lock file ? */

	if (pip->lockfname != NULL) {

	    if (pip->lockfname[0] == '\0') 
		pip->lockfname = LOCKFNAME ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: we have a 1 lockfile=%s\n",
	            pip->lockfname) ;
#endif

	    if (u_stat(pip->lockfname,&sb) >= 0) {

	        u_time(&daytime) ;

	        if ((daytime - sb.st_mtime) <= LOCKTIMEOUT)
	            goto badlock ;

	        else
	            u_unlink(pip->lockfname) ;

	    } /* end if (good stat) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: past stat in 1 lockfile\n") ;
#endif

	} /* end if (lock file) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: past 1 lockfile\n") ;
#endif


/* before we go too far, are we the only one on this PID mutex ? */

	if (pip->pidfname != NULL) {

	    if ((pip->pidfname[0] == '\0') || (pip->pidfname[0] == '-'))
		pip->pidfname = PIDFNAME ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: pidfname=%s\n",pip->pidfname) ;
#endif

		rs = lfm_init(&pip->pider,pip->pidfname,LFM_TRECORD,TO_PIDFILE,
			pip->nodename,pip->username,BANNER) ;

		if ((rs < 0) && (rs != SR_LOCKLOST) && (rs != SR_AGAIN)) 
	        	goto badpidopen ;

		if (rs < 0)
	        	goto badpidlock ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: we captured the PID lock, rs=%d\n",
	            rs) ;
#endif

	    lfm_free(&pip->pider) ;		/* releases the lock also */

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
	if (pip->debuglevel > 1) {

	    eprintf("main: dir=%s\n",pip->directory) ;

	    eprintf("main: srvtab=%s\n",pip->srvtab) ;

	    eprintf("main: command=%s\n",pip->command) ;

	    eprintf("main: polltime=%d\n",pip->polltime) ;

	}
#endif /* CF_DEBUG */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: checking program parameters\n") ;
#endif

	if ((pip->srvtab == NULL) || (pip->srvtab[0] == '\0'))
	    pip->srvtab = SRVFNAME1 ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: 1\n") ;
#endif

	if ((pip->directory == NULL) || (pip->directory[0] == '\0'))
	    pip->directory = DIRECTORY ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: 2\n") ;
#endif

	if (pip->workdir == NULL)
	    pip->workdir = WORKDIR ;

	else if (pip->workdir[0] == '\0')
	    pip->workdir = "." ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: 'tmpdir'\n") ;
#endif

	if ((pip->tmpdir == NULL) || (pip->tmpdir[0] == '\0')) {

	    if ((cp = getenv("TMPDIR")) != NULL)
	        pip->tmpdir = cp ;

	    else
	        pip->tmpdir = TMPDIR ;

	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: 'interrupt'\n") ;
#endif

	if (pip->interrupt != NULL) {

	    if (pip->interrupt[0] != '\0') {

	        if ((u_access(pip->interrupt,R_OK) < 0) ||
	            (u_stat(pip->interrupt,&sb) < 0) ||
	            ((! S_ISFIFO(sb.st_mode)) && (! S_ISCHR(sb.st_mode)))) {

	            u_unlink(pip->interrupt) ;

	            mktmpfile(tmpfname, (0666|S_IFIFO), pip->interrupt) ;

	            u_chmod(pip->interrupt,0666) ;

	        }

	    } else
	        pip->interrupt = INTERRUPT ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: intfile=%s\n",pip->interrupt) ;
#endif

	    if ((u_access(pip->interrupt,R_OK) >= 0) &&
	        (u_stat(pip->interrupt,&sb) >= 0) &&
	        (S_ISFIFO(sb.st_mode) || S_ISCHR(sb.st_mode))) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: accessible intfile=%s\n",pip->interrupt) ;
#endif

	        pip->f.interrupt = TRUE ;

	    }

	} /* end if (interrupt preparation) */

	if (pip->polltime < 2)
		pip->polltime = POLLTIME ;


#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    eprintf("main: program parameters\n") ;

	    eprintf("main: dir=%s\n",pip->directory) ;

	    eprintf("main: srvtab=%s\n",pip->srvtab) ;

	    eprintf("main: command=%s\n",pip->command) ;

	    eprintf("main: polltime=%d\n",pip->polltime) ;

	}
#endif /* CF_DEBUG */


/* open the system report log file */

#ifdef	COMMENT
	pip->f.log = FALSE ;
	if ((rs = bopen(pip->lfp,logfname,"wca",0664)) >= 0) {

	    pip->f.log = TRUE ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: system log rs=%d\n",rs) ;
#endif

	    u_time(&daytime) ;

	    bprintf(lfp,"%s: %s Directory Watcher Daemon (DWD) started\n",
	        pip->progname,timestrlog(daytime,timebuf)) ;

	    bflush(pip->lfp) ;

	} /* end if (opening log file) */
#endif /* COMMENT */


/* can we access the working directory ? */

	if ((u_access(pip->workdir,X_OK) < 0) ||
	    (u_access(pip->workdir,R_OK) < 0)) 
		goto badworking ;


/* what about the queue directory */

	if (pip->directory[0] != '/') {

	    rs = getpwd(pwd,MAXPATHLEN) ;

	    if (rs < 0) goto badqueue ;

	    bufprintf(tmpfname,MAXPATHLEN,"%s/%s",pwd,pip->directory) ;

	    pip->directory = mallocstr(tmpfname) ;

	} /* end if */

	if (maxjobs < 1)
		maxjobs = MAXJOBS ;


/* initialize the job table */

	if (vecelem_init(&jobs,10,0) < 0) 
		goto badinit ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) eprintf(
	    "main: intialized the job table \n") ;
#endif


/* do we have an activity log file ? */

	rs = BAD ;
	if ((logfname != NULL) && (logfname[0] != '\0'))
	    rs = logfile_open(&pip->lh,logfname,0,0666,LOGID) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: logfile_open() 1 rs=%d lf=%s\n",rs,logfname) ;
#endif

	if ((rs < 0) && (u_access(LOGFNAME,W_OK) >= 0))
	    rs = logfile_open(&pip->lh,LOGFNAME,0,0666,LOGID) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: logfile_open() 2 rs=%d lf=%s\n",rs,LOGFNAME) ;
#endif

	if (rs >= 0) {

	    (void) time(&daytime) ;

	    logfile_printf(&pip->lh,"\n") ;

	    logfile_printf(&pip->lh,"\n") ;

	    logfile_printf(&pip->lh,"%s %s started\n",
	        timestrlog(daytime,timebuf),
	        BANNER) ;

	    logfile_printf(&pip->lh,"%-14s %s/%s\n",
	        pip->progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	} /* end if (we have a log file) */


/* process the server table file */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		eprintf("main: checking for service table file, srvtab=%s\n",
	    pip->srvtab) ;
#endif

	pip->f.srvtab = FALSE ;
	if (u_access(pip->srvtab,R_OK) >= 0)
	    pip->f.srvtab = TRUE ;

	else
	    pip->srvtab = SRVFNAME2 ;

	if (pip->f.srvtab || (u_access(pip->srvtab,R_OK) >= 0)) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: we have a service table file=%s\n",
	            pip->srvtab) ;
#endif

	    if (pip->debuglevel > 0)
	        bprintf(efp,"%s: srvtab=%s\n",pip->progname,pip->srvtab) ;

	    pip->f.srvtab = TRUE ;
	    if ((rs = srvtab_init(&sf,pip->srvtab,NULL)) < 0)
	        goto badsrv ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) 
		eprintf(
	        "main: we have a good service table file\n") ;
#endif

#if	CF_DEBUG
	        if (pip->debuglevel > 1) {

	    for (i = 0 ; (rs = srvtab_get(&sf,i,&sep)) >= 0 ; i += 1) {

	            eprintf("main: got a service entry\n") ;

	            eprintf("main: srvtab service=%s\n",sep->service) ;

	            eprintf("main: srvtab program=%s\n",sep->program) ;

	            eprintf("main: srvtab args=%s\n",sep->args) ;

	            eprintf("main: srvtab username=%s\n",sep->username) ;

	            eprintf("main: srvtab groupname=%s\n",sep->groupname) ;

#ifdef	COMMENT
	        if (sep->program != NULL) {

	            if (u_access(sep->program,X_OK) < 0)
	                goto badsrv ;

	        }
#endif /* COMMENT */

	    } /* end for */

	} /* end if */
#endif /* CF_DEBUG */

	    logfile_printf(&pip->lh,"srvtab=%s\n",
	        pip->srvtab) ;

	} /* end if (have a service table file) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: done w/ srvtab file ?\n") ;
#endif

	if ((pip->command == NULL) && (! pip->f.srvtab))
	    goto badnosrv ;


/* miscellaneous logs */

	    logfile_printf(&pip->lh,"poll=%d\n",
	        pip->polltime) ;


/* become a daemon program */

#if	CF_DEBUG
	if (pip->debuglevel > 1) 
		eprintf("main: become a daemon ?\n") ;
#endif

	bclose(efp) ;

#if	CF_DEBUG
	if (pip->debuglevel == 0)
	    eclose() ;
#endif

	rs = BAD ;
	if (pip->debuglevel == 0) {

#ifdef	COMMENT
	    for (i = 0 ; i < 3 ; i += 1) 
		u_close(i) ;
#endif /* COMMENT */

	    rs = uc_fork() ;

	    if (rs < 0) {

	        logfile_printf(&pip->lh,"could not fork daemon (rs %d)\n",
	            rs) ;

	        u_exit(PRS_BADFORK) ;
	    }

/* parent exits */

	    if (rs > 0) 
		u_exit(EX_OK) ;

/* child becomes a session leader */

	    setsid() ;

	} /* end if (becoming a daemon) */

/* we start ! */

#if	CF_DEBUG
	if (pip->debuglevel > 1) 
		eprintf("main: starting to watch for new jobs\n") ;
#endif

	pip->pid = getpid() ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) 
		eprintf("main: final PID=%d\n",pip->pid) ;
#endif

	if (userbuf[0] != '\0')
	    u.pid = pip->pid ;


/* create a log ID */

	bufprintf(buf,BUFLEN,"%d.%s",pip->pid,LOGID) ;

	pip->logid = mallocstr(buf) ;



	logfile_setid(&pip->lh,pip->logid) ;

	if (rs == 0) {

	    logfile_printf(&pip->lh,"backgrounded\n") ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: backgrounded\n") ;
#endif

	}


/* before we go too far, are we supposed to use a lock file ? */

	pip->lockfp = NULL ;
	if ((pip->lockfname != NULL) && (pip->lockfname[0] != '\0')) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: we have a 2 lockfile=%s\n",pip->lockfname) ;
#endif

	    if ((srs = bopen(&lockfile,pip->lockfname,"rwce",0444)) < 0)
	        goto badlock2 ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {

	        bcontrol(&lockfile,BC_FD,&k) ;

	        eprintf("main: lockfile FD=%d\n",k) ;
	    }
#endif /* CF_DEBUG */

/* capture the lock (if we can) */

	    if ((srs = bcontrol(&lockfile,BC_LOCK,2)) < 0)
	        goto badlock2 ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: we captured the PID lock 2, rs=%d\n",
	            srs) ;
#endif

	    bcontrol(&lockfile,BC_TRUNCATE,0L) ;

	    bseek(&lockfile,0L,SEEK_SET) ;

	    bprintf(&lockfile,"%d\n",pip->pid) ;

	    bprintf(&lockfile,"%s!%s\n",pip->nodename,pip->username) ;

	    bprintf(&lockfile,"%s %s\n",
	        timestrlog(daytime,timebuf),
	        BANNER) ;

	    bprintf(&lockfile,"%-14s %s/%s\n",
	        pip->progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	    if (userbuf[0] != '\0')
	        bprintf(&lockfile,"d=%s %s!%s pid=%d\n",
	            u.domainname,
	            u.nodename,
	            u.username,
	            pip->pid) ;

	    bflush(&lockfile) ;

/* we leave the file open as our mutex lock ! */

	    logfile_printf(&pip->lh,"lockfile=%s\n",pip->lockfname) ;

	    logfile_printf(&pip->lh,"lock file captured\n") ;

	    bcontrol(&lockfile,BC_STAT,&sb) ;

	    logfile_printf(&pip->lh,"lockfile device=\\x%08lx inode=%ld\n",
	        sb.st_dev,sb.st_ino) ;

	    pip->lockfp = &lockfile ;

	} /* end if (capturing lock file) */

/* before we go too far, are we the only one on this PID mutex ? */

	if ((pip->pidfname != NULL) && (pip->pidfname[0] != '\0')) {

		LFM_INFO	pli ;


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: LFM pidname=%s\n",pip->pidfname) ;
#endif

	rs = lfm_init(&pip->pider,pip->pidfname,LFM_TRECORD,TO_PIDFILE,
		pip->nodename,pip->username,BANNER) ;

	if (rs < 0)
	        goto badpidfile2 ;

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

/* we leave the file open as our mutex lock ! */

	    logfile_printf(&pip->lh,"pidfile=%s\n",pip->pidfname) ;

	    logfile_printf(&pip->lh,"PID mutex captured\n") ;

	    lfm_getinfo(&pip->pider,&pli) ;

	    logfile_printf(&pip->lh,"pidfile device=\\x%08lx inode=%ld\n",
	        pli.dev,pli.ino) ;

		if (pip->debuglevel > 0) {

		bprintf(pip->efp,"%s: pidfile=%s\n",pip->pidfname);

		bprintf(pip->efp,"%s: PID file dev=%08x ino=%d\n",
			pli.dev,pli.ino) ;

		}

	} /* end if (we have a mutex PID file) */


/* make some last log entries before we get into bad boogying ! */

	if (userbuf[0] != '\0') {

		struct utsname	un ;


	    u_time(&daytime) ;

	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

		(void) u_uname(&un) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s) pid=%d\n",
	        u.f.sysv_rt ? "SYSV" : "BSD",
		un.sysname,un.release,
		pip->pid) ;

	    logfile_printf(&pip->lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;

	    logfile_printf(&pip->lh,"dir=%s\n",pip->directory) ;

	    if (pip->f.interrupt)
	        logfile_printf(&pip->lh,"intfile=%s\n",
	            pip->interrupt) ;

	    logfile_printf(&pip->lh,"%s finished initializing\n",
	        timestrlog(daytime,timebuf)) ;

	} /* end if (making log entries) */


/* do the thing ! */

	srs = watch(pip,&sf,&jobs,maxjobs,filetime) ;


/* and we are done ? and out of here */

	ex = EX_DATAERR ;

	if ((pip->lockfname != NULL) && (pip->lockfname[0] != '\0'))
	    u_unlink(pip->lockfname) ;

	if ((pip->pidfname != NULL) && (pip->pidfname[0] != '\0'))
	    lfm_free(&pip->pider) ;

	if (srs < 0) 
		goto daemonret ;

	ex = EX_OK ;


/* we are done */
done:

#if	CF_DEBUG
	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: program finishing\n",
	        pip->progname) ;
#endif

ret3:
	if ((configfname != NULL) && (configfname[0] != '\0'))
	    configfile_free(&cf) ;

ret2:
	if (pip->lfp != NULL) 
		bclose(pip->lfp) ;

ret1:
	bclose(efp) ;

ret0:
	return ex ;

earlyret:
	bclose(efp) ;

	return PRS_OK ;

daemonret:
	if (pip->lfp != NULL) 
		bclose(pip->lfp) ;

	return srs ;

/* error types of returns */
badret:
	bclose(efp) ;

	return srs ;

badretlog:
	bclose(pip->lfp) ;

	srs = PRS_BADLOG ;
	goto badret ;

/* USAGE> dwd [-C conf] [-polltime] [directory_path] [srvtab] [-V?] */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-C conf] [-polltime] [directory] [srvtab] [-?v] ",
	    pip->progname,pip->progname) ;

	bprintf(efp,"[-D[=n]]\n") ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",pip->progname) ;

	srs = PRS_BADNUM ;
	goto badret ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	srs = PRS_BADEXTRA ;
	goto badret ;

badargval:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	srs = PRS_BADVALUE ;
	goto badret ;

badworking:
	bprintf(efp,"%s: could not access the working directory \"%s\"\n",
	    pip->progname,pip->workdir) ;

	srs = PRS_BADWORKING ;
	goto badret ;

badqueue:
	bprintf(efp,"%s: could not process the queue directory\n",
	    pip->progname) ;

	srs = PRS_BADQUEUE ;
	goto badret ;

baduser:
	bprintf(efp,"%s: could not get user information (rs %d)\n",
	    pip->progname,rs) ;

	srs = PRS_BADUSER ;
	goto badret ;

badinit:
	bprintf(efp,"%s: could not initialize list structures (rs %d)\n",
	    pip->progname,rs) ;

	srs = PRS_BADINT ;
	goto badret ;

badsrv:
	bprintf(efp,"%s: bad service table file (rs %d)\n",
	    pip->progname,rs) ;

	srs = PRS_BADSRV ;
	goto badret ;

badnosrv:
	bprintf(efp,"%s: no service table file and no command specified\n",
	    pip->progname) ;

	srs = PRS_NOSRV ;
	goto badret ;

baddir:
	bprintf(efp,"%s: bad working directory specified, rs=%d\n",
	    pip->progname,rs) ;

	srs = PRS_BADDIR ;
	goto badret ;

badconfig:
	bprintf(efp,
	    "%s: error (rs %d) in configuration file starting at line %d\n",
	    pip->progname,rs,cf.badline) ;

	srs = PRS_BADCONFIG ;
	goto badret ;

badlock:
	if (! pip->f.quiet) {

	    bprintf(efp,
	        "%s: there was a lock file \"%s\" already\n",
	        pip->progname,pip->lockfname) ;

	    if (bopen(&lockfile,pip->lockfname,"r",0666) >= 0) {

	        while ((len = bgetline(&lockfile,buf,BUFLEN)) > 0) {

	            bprintf(efp,"%s: pidfile> %W",
	                pip->progname,
	                buf,len) ;

	        }

	        bclose(&lockfile) ;

	    }
	}

	srs = PRS_BADLOCK ;
	goto badret ;

badpidopen:
	bprintf(efp,
	    "%s: could not open the PID lock file (rs=%d)\n",
	    pip->progname,rs) ;

	bprintf(efp,
	    "%s: PID file=%s\n",
	    pip->progname,pip->pidfname) ;

	srs = PRS_BADPIDOPEN ;
	goto badret ;

badpidlock:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: PID lock failed, rs=%d\n",rs) ;
#endif

	if ((pip->debuglevel > 0) || (! pip->f.quiet)) {

	bprintf(efp,
	    "%s: could not open the PID lock file (rs=%d)\n",
	    pip->progname,rs) ;

	bprintf(efp,
	    "%s: PID file=%s\n",
	    pip->progname,pip->pidfname) ;

	}

	srs = PRS_BADPIDLOCK ;
	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument(s) given\n",
	    pip->progname) ;

	srs = PRS_BADARG ;
	goto badret ;

badlock2:
	logfile_printf(&pip->lh,
	    "there was a daemon already on the lock file \"%s\"\n",
	    pip->lockfname) ;

	bclose(&lockfile) ;

	if (bopen(&lockfile,pip->lockfname,"r",0666) >= 0) {

	    while ((len = bgetline(&lockfile,buf,BUFLEN)) > 0) {

	        logfile_printf(&pip->lh,"lockfile> %W",buf,len) ;

	    } /* end while */

	    bclose(&lockfile) ;

	}

	srs = PRS_BADLOCK2 ;
	goto daemonret ;

badpidfile2:
	if ((pip->debuglevel > 0) || (! pip->f.quiet))
	logfile_printf(&pip->lh,
	    "could not capture the PID lock file (%d)\n",rs) ;

	srs = PRS_BADPID2 ;
	goto daemonret ;

}
/* end subroutine (main) */



