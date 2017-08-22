/* main */

/* generic (pretty much) front end program subroutine */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1994-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine forms the front-end part of a generic PCS type of
        program. This front-end is used in a variety of PCS programs.

        This subroutine was originally part of the Personal Communications
        Services (PCS) package but can also be used independently from it.
        Historically, this was developed as part of an effort to maintain high
        function (and reliable) email communications in the face of increasingly
        draconian security restrictions imposed on the computers in the DEFINITY
        development organization.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<logfile.h>
#include	<baops.h>
#include	<varsub.h>
#include	<storebuf.h>
#include	<getax.h>
#include	<userinfo.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"configfile.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	REALNAMELEN
#define	REALNAMELEN	(NUMNAMES * MAXPATHLEN)
#endif

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + REALNAMELEN)
#endif

#define	DEBUG_REGULAR	(pip->debuglevel > 0)
#define	DEBUG_EXTRA	(pip->debuglevel > 1)
#define	DEBUG_DEBUG	(pip->debuglevel > 2)


/* external subroutines */

extern int	snsdd(char *,int,const char *,uint) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	getpwd(char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	getfname(char *,char *,int,char *) ;
extern int	bopenroot(bfile *,char *,char *,char *,char *,int) ;
extern int	varsub_addvec(VARSUB *,VECSTR *) ;
extern int	varsub_subbuf(), varsub_merge() ;
extern int	expander() ;
extern int	procfileenv(char *,char *,VECSTR *) ;
extern int	procfilepaths(char *,char *,VECSTR *) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			const char *,const char *) ;

extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;

extern char	*strbasename(char *) ;
extern char	*strshrink(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* externals variables */


/* forward references */

static int	usage(struct proginfo *) ;
static int	procfile(int (*)(char *,char *,VECSTR *),
			const char *,vecstr *,char *,VECSTR *) ;


/* local structures */


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"CONFIG",
	"TMPDIR",
	"LOGFILE",
	"HELP",
	"af",
	"of",
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
	argopt_af,
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
	{ 0, 0 }
} ;

/* 'conf' for most regular programs */
static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

/* non-'conf' ETC stuff for all regular programs */
static const char	*sched2[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%e/%f",
	"%p/%n.%f",
	NULL
} ;

/* 'conf' and non-'conf' ETC stuff for local searching */
static const char	*sched3[] = {
	"%e/%n/%n.%f",
	"%e/%n/%f",
	"%e/%n.%f",
	"%e/%f",
	"%n.%f",
	"%f",
	NULL
} ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct ustat	sb ;

	struct group	ge ;

	struct proginfo	pi, *pip = &pi ;

	USERINFO	u ;

	CONFIGFILE	cf ;

	VECSTR		defines, unsets, exports ;
	VECSTR		svars ;

	VARSUB		vsh_e, vsh_d ;

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		pidfile ;
	bfile		logfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs ;
	int	len, iw ;
	int	i, j, k ;
	int	loglen = -1 ;
	int	l2, sl ;
	int	logfile_type = -1 ;
	int	c_locks = 0 ;
	int	c_captured = 0 ;
	int	to_lkwait = -1 ;
	int	to_remove = -1 ;
	int	mul_remove = -1 ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_programroot = FALSE ;
	int	f_freeconfigfname = FALSE ;
	int	f_procfileenv = FALSE ;
	int	f_procfilepaths = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	*programroot = NULL ;
	char	buf[BUFLEN + 1], *bp ;
	char	buf2[BUFLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	nodename[NODENAMELEN + 1], domainname[MAXHOSTNAMELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	pidfname[MAXPATHLEN + 1] ;
	char	lockfname[MAXPATHLEN + 1] ;
	char	logfname[MAXPATHLEN + 1] ;
	char	helpfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*configfname = NULL ;
	const char	*defsizespec = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* initialize */

	pip->ppid = pip->pid ;

	pip->lfp = &logfile ;

	pip->verboselevel = 1 ;

	pidfname[0] = '\0' ;
	lockfname[0] = '\0' ;
	logfname[0] = '\0' ;

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

/* do we have a keyword match or should we assume only key letters? */

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
	                            if (avl) {
	                                logfile_type = 0 ;
	                                mkpath1w(logfname,avp,avl) ;
	                            }

	                        } else {

	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)  {

	                                logfile_type = 0 ;
	                                mkpath1w(logfname,argp,argl) ;

	                            }
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

/* argument file */
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

	                    case argopt_help:
	                        f_help = TRUE ;
	                        break ;

/* handle all keyword defaults */
	                    default:
				rs = SR_INVALID ;
	                        bprintf(pip->efp,
					"%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {

	                                    rs = cfdeci(avp,avl,
						&pip->debuglevel) ;

					}
	                            }

	                            break ;

/* quiet mode */
	                        case 'Q':
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
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

/* hold time */
			case 'h':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdecti(argp,argl,&pip->holdtime) ;

				    }

	                            break ;

/* lock type */
	                        case 'l':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdeci(argp,argl,&iw) ;

					pip->f.readlock = (iw > 0) ;

				    }

	                            break ;

/* remove timeout */
	                        case 'r':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdecti(argp,argl,&to_remove) ;

				    }

	                            break ;

/* remove file multiplier */
	                        case 'm':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdeci(argp,argl,&mul_remove) ;

				    }

	                            break ;

/* lock to_lkwait */
	                        case 't':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

					rs = cfdecti(argp,argl,&to_lkwait) ;

				    }

	                            break ;

/* verbose mode */
	                        case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {

	                                    rs = cfdeci(avp,avl,
						&pip->verboselevel) ;

	                                }
	                            }


	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
					break ;

	                        default:
				rs = SR_INVALID ;
	                            bprintf(pip->efp,
					"%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

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

	if (DEBUG_EXTRA)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage) 
	    usage(pip) ;

/* get our program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    bprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	}

/* help */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check some initial arguments */

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	    debugprintf("main: initial checking of args to_lkwait=%d\n",
			to_lkwait) ;
#endif

/* get some host/user information */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	if (rs < 0) {
	    ex = EX_NOUSER ;
	    goto retearly ;
	}

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;
	pip->groupname = u.groupname ;
	pip->homedname = u.homedname ;
	pip->logid = u.logid ;

/* PWD */

	proginfo_pwd(pip) ;

/* create the values for the file schedule searching */

	vecstr_start(&svars,6,0) ;

	vecstr_envset(&svars,"p",pip->pr,-1) ;

	vecstr_envset(&svars,"e","etc",-1) ;

	vecstr_envset(&svars,"n",SEARCHNAME,-1) ;

/* prepare to store configuration variable lists */

	if ((rs = vecstr_start(&defines,10,VECSTR_PORDERED)) < 0)
	    goto badlistinit ;

	if ((rs = vecstr_start(&unsets,10,VECSTR_PNOHOLES)) < 0) {

	    vecstr_finish(&defines) ;

	    goto badlistinit ;
	}

	if ((rs = vecstr_start(&exports,10,VECSTR_PNOHOLES)) < 0) {

	    vecstr_finish(&defines) ;

	    vecstr_finish(&unsets) ;

	    goto badlistinit ;
	}


/* load up some initial environment that everyone should have ! */

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	    debugprintf("main: about to do DEFINITFNAME=%s\n",DEFINITFNAME) ;
#endif

	procfileenv(pip->pr,DEFINITFNAME,&exports) ;

/* find a configuration file if we have one */

#if	CF_DEBUG
	if (DEBUG_DEBUG) {

	    debugprintf("main: checking for configuration file\n") ;

	    debugprintf("main: 0 CF=%s\n",configfname) ;

	}
#endif /* CF_DEBUG */

	rs = SR_NOEXIST ;
	if ((configfname == NULL) || (configfname[0] == '\0')) {

	    configfname = CONFIGFNAME ;

	    sl = permsched(sched1,&svars,
	        tmpfname,MAXPATHLEN, configfname,R_OK) ;

	    if (sl < 0)
	        sl = permsched(sched3,&svars,
	        tmpfname,MAXPATHLEN, configfname,R_OK) ;

	    if (sl > 0)
	            configfname = tmpfname ;

	    rs = sl ;

	} else {

	    sl = getfname(pip->pr,configfname,1,tmpfname) ;

	    if (sl > 0)
	        configfname = tmpfname ;

		rs = sl ;		/* cause an open failure later */

	} /* end if */

	if (configfname == tmpfname) {

	    f_freeconfigfname = TRUE ;
	    configfname = mallocstr(tmpfname) ;

	}

#if	CF_DEBUG
	if (DEBUG_DEBUG) {

	    debugprintf("main: find rs=%d\n",sl) ;

	    debugprintf("main: 1 CF=%s\n",configfname) ;

	}
#endif /* CF_DEBUG */

/* read in the configuration file if we have one */

	if ((rs >= 0) || (perm(configfname,-1,-1,NULL,R_OK) >= 0)) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	        debugprintf("main: configuration file \"%s\"\n",
	            configfname) ;
#endif

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: configfile=%s\n",
	            pip->progname,configfname) ;

	    if ((rs = configfile_start(&cf,configfname)) < 0)
	        goto badconfig ;

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	        debugprintf("main: we have a good configuration file\n") ;
#endif

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	        debugprintf("main: varsub_start d\n") ;
#endif

/* we must set this mode to 'VARSUB_MBADNOKEY' so that a miss is noticed */

	    varsub_start(&vsh_d,VARSUB_MBADNOKEY) ;

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	        debugprintf("main: varsub_start e\n") ;
#endif

	    varsub_start(&vsh_e,0) ;

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	        debugprintf("main: varsub_start\n") ;
#endif

	    varsub_addva(&vsh_e,(const char **) envv) ;

/* program root from configuration file */

	    if ((cf.root != NULL) && (! f_programroot)) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	            debugprintf("main: processing CF programroot\n") ;
#endif

	        sl = varsub_subbuf(&vsh_d,&vsh_e,cf.root,-1,buf,BUFLEN) ;

		if ((sl > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            proginfo_setprogroot(pip,buf2,l2) ;

	        }

	    } /* end if (configuration file program root) */


/* loop through the DEFINEd variables in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.defines,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	            debugprintf( "main: 0 top, cp=%s\n",cp) ;
#endif

	        sl = varsub_subbuf(&vsh_d,&vsh_e,cp,-1,buf,BUFLEN) ;

		if ((sl > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	                debugprintf("main: 0 about to merge\n") ;
#endif

	            varsub_merge(&vsh_d,&defines,buf2,l2) ;

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	                debugprintf("main: 0 out of merge\n") ;
#endif

	        } /* end if */

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	            debugprintf( "main: 0 bot\n") ;
#endif

	    } /* end for (defines) */

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	        debugprintf("main: done w/ defines\n") ;
#endif


/* all of the rest of the configuration file stuff */

	    if ((cf.workdir != NULL) && (pip->workdname == NULL)) {

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.workdir,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            pip->workdname = mallocstrw(buf2,l2) ;

	        }

	    } /* end if (config file working directory) */


	    if ((cf.pidfname != NULL) && (pidfname[0] == '\0')) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	            debugprintf("main: CF pidfile=%s\n",cf.pidfname) ;
#endif

	        if ((cf.pidfname[0] != '\0') && (cf.pidfname[0] != '-')) {

	            if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.pidfname,
	                -1,buf,BUFLEN)) > 0) &&
	                ((l2 = expander(pip,buf,sl, buf2,BUFLEN)) > 0)) {

	                strwcpy(pidfname,buf2,l2) ;

	            }

	        }

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	            debugprintf("main: pidfname=%s\n",pidfname) ;
#endif

	    } /* end if (configuration file PIDFILE) */


	    if ((cf.lockfname != NULL) && (lockfname[0] == '\0')) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	            debugprintf("main: CF lockfile=%s\n",cf.lockfname) ;
#endif

	        if ((cf.lockfname[0] != '\0') && (cf.lockfname[0] != '-')) {

	            if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.lockfname,
	                -1,buf,BUFLEN)) > 0) &&
	                ((l2 = expander(pip,buf,sl, buf2,BUFLEN)) > 0)) {

	                strwcpy(lockfname,buf2,l2) ;

	            }

	        }

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	            debugprintf("main: lockfile=%s\n",lockfname) ;
#endif

	    } /* end if (configuration file LOCKFNAME) */

#if	CF_DEBUG
	if (DEBUG_DEBUG) {
	        debugprintf("main: so far logfname=%s\n",logfname) ;
	        debugprintf("main: about to get config log filename\n") ;
	    }
#endif

	    if ((cf.logfname != NULL) && (logfile_type < 0)) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	            debugprintf("main: CF logfilename=%s\n",cf.logfname) ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.logfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            mkpath1w(logfname,buf2,l2) ;

	            if (strchr(logfname,'/') != NULL)
	                logfile_type = 1 ;

	        }

#if	CF_DEBUG
	if (DEBUG_DEBUG)
		debugprintf("main: processed CF logfilename=%s\n",
			logfname) ;
#endif

	    } /* end if (configuration file log filename) */


	    if ((cf.timeout != NULL) && (to_lkwait < 0)) {

	        sl = varsub_subbuf(&vsh_d,&vsh_e,cf.timeout,
	            -1,buf,BUFLEN) ;

		if ((sl > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	                        if (cfdecti(buf,sl,&to_lkwait) < 0)
					to_lkwait = -1 ;

	        }

	    } /* end if (to_lkwait) */

	    if ((cf.removemul != NULL) && (mul_remove < 0)) {

	        sl = varsub_subbuf(&vsh_d,&vsh_e,cf.removemul,
	            -1,buf,BUFLEN) ;

		if ((sl > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	                        if (cfdeci(buf,sl,&mul_remove) < 0)
					mul_remove = -1 ;

	        }

	    } /* end if (removal multiplier) */


/* what about an 'environ' file? */

	    if (cf.envfname != NULL) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	            debugprintf("main: 1 envfile=%s\n",cf.envfname) ;
#endif

	        procfileenv(pip->pr,cf.envfname,&exports) ;

	    } else
	        procfile(procfileenv,pip->pr,&svars,
	            ENVFNAME,&exports) ;

	    f_procfileenv = TRUE ;


/* "do" any 'paths' file before we process the environment variables */

	    if (cf.pathfname != NULL)
	        procfilepaths(pip->pr,cf.pathfname,&exports) ;

	    else
	    	procfile(procfilepaths,pip->pr,&svars,
	            PATHSFNAME,&exports) ;

	    f_procfilepaths = TRUE ;


#if	CF_DEBUG
	if (DEBUG_DEBUG)
	        debugprintf("main: processing CF loglen\n") ;
#endif

	    if ((cf.loglen >= 0) && (loglen < 0))
	        loglen = cf.loglen ;

/* loop through the UNSETs in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.unsets,i,&cp) >= 0 ; i += 1) {

	        rs = vecstr_finder(&exports,cp,vstrkeycmp,NULL) ;

	        if (rs >= 0)
	            vecstr_del(&exports,rs) ;

	    } /* end for */


/* loop through the EXPORTed variables in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.exports,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	            debugprintf("main: 1 about to sub> %s\n",cp) ;
#endif

	        sl = varsub_subbuf(&vsh_d,&vsh_e,cp,-1,buf,BUFLEN) ;

		if ((sl > 0) &&
	            ((l2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	                debugprintf("main: 1 about to merge> %W\n",buf,l2) ;
#endif

	            varsub_merge(NULL,&exports,buf2,l2) ;

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	                debugprintf("main: 1 done merging\n") ;
#endif

	        } /* end if (merging) */

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	            debugprintf("main: done subbing & merging\n") ;
#endif

	    } /* end for (exports) */

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	        debugprintf("main: done w/ exports\n") ;
#endif


#if	CF_DEBUG
	if (DEBUG_DEBUG)
	        debugprintf("main: processing CF freeing data structures\n") ;
#endif


	    varsub_finish(&vsh_d) ;

	    varsub_finish(&vsh_e) ;

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	        debugprintf("main: processing CF free\n") ;
#endif

	    configfile_finish(&cf) ;

#if	CF_DEBUG
	if (DEBUG_DEBUG) {

	        debugprintf("main: dumping defines\n") ;

	        for (i = 0 ; vecstr_get(&defines,i,&cp) >= 0 ; i += 1)
	            debugprintf("main: define> %s\n",cp) ;

	        debugprintf("main: dumping exports\n") ;

	        for (i = 0 ; vecstr_get(&exports,i,&cp) >= 0 ; i += 1)
	            debugprintf("main: export> %s\n",cp) ;

	    } /* end if (CF_DEBUG) */
#endif /* CF_DEBUG */


	} /* end if (accessed the configuration file) */

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	    debugprintf("main: finished with any configfile stuff\n") ;
#endif


	if (pip->pr == NULL)
	    pip->pr = programroot ;

	if (DEBUG_EXTRA)
	    bprintf(pip->efp,"%s: programroot=%s\n",pip->progname,pip->pr) ;


/* help */

	if (f_help) {

	    if ((helpfname == NULL) || (helpfname[0] == '\0'))
	        strcpy(helpfname,HELPFNAME) ;

	    printhelp(NULL,pip->pr,SEARCHNAME,helpfname) ;

	    vecstr_finish(&svars) ;

	    goto done ;

	} /* end if (help) */

/* put the final value of the program root into the schedule search variables */

	vecstr_envset(&svars,"p",pip->pr,-1) ;

/* load up some environment and execution paths if we have not already */

	if (! f_procfileenv)
	    procfile(procfileenv,pip->pr,&svars,
	        ENVFNAME,&exports) ;


	if (! f_procfilepaths)
	    procfile(procfilepaths,pip->pr,&svars,
	        PATHSFNAME,&exports) ;


/* check program parameters */

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	    debugprintf("main: checking program parameters\n") ;
#endif

	if (pip->workdname == NULL)
	    pip->workdname = WORKDIR ;

	else if (pip->workdname[0] == '\0')
	    pip->workdname = "." ;


	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0')) {

	    if ((pip->tmpdname = getenv("TMPDIR")) == NULL)
	        pip->tmpdname = TMPDIR ;

	} /* end if (tmpdname) */


	if (lockfname[0] == '\0')
	    strcpy(lockfname,LOCKFNAME) ;

	if ((sl = getfname(pip->pr,lockfname,1,tmpfname)) > 0)
	    strwcpy(lockfname,tmpfname,sl) ;

	pip->lockfname = lockfname ;

/* hold time */

	if (pip->holdtime < 0)
		pip->holdtime = 0 ;

/* can we access the working directory? */

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	    debugprintf("main: access working dir=%s\n",
		pip->workdname) ;
#endif

	if ((perm(pip->workdname,-1,-1,NULL,X_OK) < 0) || 
	    (perm(pip->workdname,-1,-1,NULL,R_OK) < 0))
	    goto badworking ;

/* do we have an activity log file? */

	rs = SR_BAD ;
	if (logfname[0] == '\0') {
	    logfile_type = 1 ;
	    mkpath1(logfname,LOGFNAME) ;
	}

	if ((sl = getfname(pip->pr,logfname,logfile_type,tmpfname)) > 0)
	    mkpath1w(logfname,tmpfname,sl) ;

	rs = logfile_open(&pip->lh,logfname,0,0666,pip->logid) ;

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if (rs >= 0) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	        debugprintf("main: we opened a logfile\n") ;
#endif

	    pip->open.log = TRUE ;
	    if (DEBUG_EXTRA)
	        bprintf(pip->efp,"%s: logfile=%s\n",pip->progname,logfname) ;

/* we opened it, maintenance this log file if we have to */

	    if (loglen < 0)
	        loglen = LOGSIZE ;

	    logfile_checksize(&pip->lh,loglen) ;

	    pip->daytime = time(NULL) ;

	    logfile_userinfo(&pip->lh,&u,
		pip->daytime,pip->progname,pip->version) ;

	} /* end if (we have a log file or not) */

/* handle UID/GID stuff */

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	    debugprintf("main: continuing with some PID stuff\n") ;
#endif

	pip->uid = u.uid ;
	pip->euid = geteuid() ;

	pip->gid = u.gid ;
	pip->egid = getegid() ;

	rs = getgr_gid(&ge,buf,BUFLEN,pip->gid) ;

	if (rs < 0) {

	    cp = buf ;
	    snsdd(buf,BUFLEN,"G",pip->gid) ;

	} else
	    cp = ge.gr_name ;

	if (pip->groupname == NULL)
	    proginfo_setentry(pip,&pip->groupname,cp,-1) ;

/* before we go too far, are we the only one on this PID mutex ? */

	if ((pip->pidfname != NULL) && (pip->pidfname[0] != '\0')) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	        debugprintf("main: we have a PIDFILE=%s\n",pip->pidfname) ;
#endif

	    if ((rs = bopen(&pidfile,pip->pidfname,"rwc",0664)) < 0)
	        goto badpidfile1 ;

/* capture the lock (if we can) */

	    if ((rs = bcontrol(&pidfile,BC_LOCK,2)) < 0) {

	        bclose(&pidfile) ;

	        goto badpidfile2 ;
	    }

	    bcontrol(&pidfile,BC_TRUNCATE,0L) ;

	    bseek(&pidfile,0L,SEEK_SET) ;

	    bprintf(&pidfile,"%d\n",pip->pid) ;

	    bprintf(&pidfile,"%s %s\n",
	        BANNER,timestr_log(pip->daytime,timebuf)) ;

	    if (userbuf[0] != '\0')
	        bprintf(&pidfile,"host=%s.%s user=%s pid=%d\n",
	            u.nodename,u.domainname,
	            u.username,
	            pip->pid) ;

	    else
	        bprintf(&pidfile,"host=%s.%s pid=%d\n",
	            u.nodename,u.domainname,
	            pip->pid) ;

	    bflush(&pidfile) ;

/* we leave the file open as our mutex lock ! */

	    logfile_printf(&pip->lh,"pidfile=%s\n",pip->pidfname) ;

	    logfile_printf(&pip->lh,"PID mutex captured\n") ;

	    bcontrol(&pidfile,BC_STAT,&sb) ;

	    logfile_printf(&pip->lh,"pidfile device=%ld inode=%ld\n",
	        sb.st_dev,sb.st_ino) ;

	    pip->pidfp = &pidfile ;

	} /* end if (we have a mutex PID file) */

/* clean up some stuff we will no longer need */

	vecstr_finish(&svars) ;

/* are the log table files entries rooted other than a HOME directory ? */

	if (pip->fileroot == NULL)
	    pip->fileroot = pip->homedname ;

/* the to_lkwait value */

	if ((to_lkwait < 0) && (argvalue >= 0))
		to_lkwait = argvalue ;

	if (to_lkwait < 0)
		to_lkwait = LKTEST_DEFTIMEOUT ;


/* the remove-file multiplier */

	if (to_remove < 0) {

#ifdef	COMMENT
		if (mul_remove > 1)
			to_remove = to_lkwait * mul_remove ;

		else
			to_remove = to_lkwait * LKTEST_MULREMOVE ;
#else
		to_remove = LKTEST_DEFREMOVE ;
#endif

	} /* end if (remove time determination) */

#if	CF_DEBUG
	if (DEBUG_DEBUG) {
	        debugprintf("main: final to_lkwait=%d\n",to_lkwait) ;
	        debugprintf("main: final to_remove=%d\n",to_remove) ;
	}
#endif

/* OK, we do it */

	ex = EX_DATAERR ;
	if (pip->verboselevel > 1)
		bopen(ofp,BFILE_STDOUT,"dwct",0666) ;


	varsub_start(&vsh_d,VARSUB_MBADNOKEY) ;

	varsub_start(&vsh_e,0) ;


	varsub_addvec(&vsh_d,&defines) ;

	varsub_addvec(&vsh_e,&exports) ;


/* process the positional arguments */

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	    debugprintf("main: checking for positional arguments\n") ;
#endif

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	        pan += 1 ;
	        rs = process(pip,ofp,cp,to_lkwait,to_remove) ;

		c_locks += 1 ;
		if (rs >= 0)
	            c_captured += 1 ;

	    } /* end for (looping through requested circuits) */

/* process any files in the argument filename list file */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {

		bfile		argfile, *afp = &argfile ;


#if	CF_DEBUG
	if (DEBUG_DEBUG)
	        debugprintf("main: we have an argument file list\n") ;
#endif

	    if (strcmp(afname,"-") == 0)
	        rs = bopen(afp,BFILE_STDIN,"dr",0666) ;

	    else
	        rs = bopen(afp,afname,"r",0666) ;

	    if (rs >= 0) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	            debugprintf("main: processing the argument file list\n") ;
#endif

	        while ((rs = breadline(afp,buf,BUFLEN)) > 0) {

		    len = rs ;
	            if (buf[len - 1] == '\n') 
			len -= 1 ;

	            buf[len] = '\0' ;
	            cp = strshrink(buf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            pan += 1 ;
	        rs = process(pip,ofp,cp,to_lkwait,to_remove) ;

		c_locks += 1 ;
		if (rs >= 0)
	            c_captured += 1 ;

	        } /* end while (reading lines) */

	        bclose(afp) ;

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	            debugprintf("main: done processing the argument file list\n") ;
#endif

	    } else {

	        if (! pip->f.quiet) {

	            bprintf(pip->efp,
	                "%s: could not open the argument list file\n",
	                pip->progname) ;

	            bprintf(pip->efp,"\trs=%d argfile=%s\n",rs,afname) ;

	        }

	    }

	} /* end if (processing file argument file list */

	if ((rs >= 0) && (pan == 0) && 
		((cp = getenv(LKTEST_NAMEVAR)) != NULL)) {

#if	CF_DEBUG
	if (DEBUG_DEBUG) {
	        debugprintf("main: trying ENV variable\n") ;
	        debugprintf("main: logtab=%s\n",cp) ;
	    }
#endif

	    rs = SR_NOTFOUND ;
	    if ((u_stat(cp,&sb) >= 0) && S_ISREG(sb.st_mode)) {

	        pan += 1 ;
	        rs = process(pip,ofp,cp,to_lkwait,to_remove) ;

		c_locks += 1 ;
		if (rs >= 0)
	            c_captured += 1 ;

	    }

	} /* end if */


	varsub_finish(&vsh_d) ;

	varsub_finish(&vsh_e) ;


	ex = EX_DATAERR ;
	if (pan <= 0) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	        debugprintf("main: no lock name were specified, rs=%d\n",rs) ;
#endif

	    ex = EX_INVALID ;
	    bprintf(pip->efp,"%s: no lock names were specified\n",
	        pip->progname) ;

	} /* end if (not log table file was specified or found) */

	if (DEBUG_EXTRA) {

	    bprintf(pip->efp,"%s: locks=%d captured=%d\n",
	        pip->progname,
		c_locks,c_captured) ;

	}

	pip->daytime = time(NULL) ;

	    logfile_printf(&pip->lh,"%s locks=%d captured=%d\n",
		timestr_log(pip->daytime,timebuf),
		c_locks,c_captured) ;

	if (pip->verboselevel > 1)
		bclose(ofp) ;


	ex = EX_OK ;
	if (c_captured < c_locks)
		ex = EX_TIMEDOUT ;


/* close some more (earlier) daemon stuff */
done:
daemonret2:
daemonret1:
ret5:

#ifdef	OPTIONAL
	vecstr_finish(&defines) ;

	vecstr_finish(&exports) ;
#endif /* COMMENT */


#if	CF_DEBUG
	if (DEBUG_DEBUG)
	    debugprintf("main: program finishing\n",
	        pip->progname) ;
#endif

ret4:
	if (f_freeconfigfname && (configfname != NULL))
	    free(configfname) ;

ret3:
	if (pip->open.log)
	    logfile_close(&pip->lh) ;

ret2:
	if (pip->lfp != NULL)
	    bclose(pip->lfp) ;

retearly:
ret1:
	bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
badarg:
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

badretlog:
	bclose(pip->lfp) ;

	goto badret ;

badworking:
	bprintf(pip->efp,"%s: could not access workdir=%s\n",
	    pip->progname,pip->workdname) ;

	goto badret ;

badlistinit:
	bprintf(pip->efp,"%s: could not initialize list structures (%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badconfig:
	bprintf(pip->efp,
	    "%s: configfile=%s\n",
	    pip->progname,configfname) ;

	bprintf(pip->efp,
	    "%s: error (%d) in configuration file starting at line %d\n",
	    pip->progname,rs,cf.badline) ;

	goto badret ;

badpidopen:
	bprintf(pip->efp,
	    "%s: could not open the PID file (rs=%d)\n",
	    pip->progname,rs) ;

	bprintf(pip->efp,"%s: pidfile=%s\n", pip->progname,pip->pidfname) ;

	goto badret ;

badpidlock:
	if (! pip->f.quiet) {

	    bprintf(pip->efp,
	        "%s: could not lock the PID file (rs=%d)\n",
	        pip->progname,rs) ;

	    bprintf(pip->efp,
		"%s: pidfile=%s\n", pip->progname,pip->pidfname) ;

	    while ((len = breadline(&pidfile,buf,BUFLEN)) > 0) {

	        bprintf(pip->efp,"%s: pidfile> %W",
	            pip->progname,
	            buf,len) ;

	    }

	    bclose(&pidfile) ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: PID lock failed, rs=%d\n",rs) ;
#endif

	goto badret ;

badpidfile1:
	if (pip->open.log)
	logfile_printf(&pip->lh,
	    "could not open the PID mutex file (%d)\n",
	    rs) ;

	logfile_printf(&pip->lh,"pidfile=%s\n", pip->pidfname) ;

	goto daemonret2 ;

badpidfile2:
	if (pip->open.log)
	logfile_printf(&pip->lh,
	    "there was a daemon already on the mutex file, PID=%d\n",
	    pip->pid) ;

	goto daemonret2 ;

badret:
	ex = EX_DATAERR ;
	goto done ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen ;


	wlen = 0 ;
	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [<lockfile> [...]] [-C <conf>] [-<timeout>] [-?v]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s: \t[-af <argfile] [-ROOT <pr>]\n",
		pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s: \t[-t <timeout>] [-r <removetime>]\n",
		pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procfile(func,pr,svp,fname,elp)
int		(*func)(char *,char *,VECSTR *) ;
const char	pr[] ;
VECSTR		*svp ;
char		fname[] ;
VECSTR		*elp ;
{
	int	sl ;

	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUGS
	    debugprintf("procfile: 1 fname=%s\n",fname) ;
#endif

	if ((sl = permsched(sched2,svp,
	    tmpfname,MAXPATHLEN, fname,R_OK)) < 0) {

	    if ((sl = permsched(sched3,svp,
	        tmpfname,MAXPATHLEN, fname,R_OK)) > 0)
	        fname = tmpfname ;

	} else if (sl > 0)
	    fname = tmpfname ;

#if	CF_DEBUGS
	    debugprintf("procfile: 2 fname=%s\n",fname) ;
#endif

	if (sl >= 0)
	    return (*func)(pr,fname,elp) ;

	return SR_NOEXIST ;
}
/* end subroutine (procfile) */



