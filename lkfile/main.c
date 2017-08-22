/* main */

/* generic (pretty much) front end program subroutine */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */


/* revision history:

	= 1998-09-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine forms the front-end part of a generic PCS
	type of program.  This front-end is used in a variety of
	PCS programs.

	This subroutine was originally part of the Personal
	Communications Services (PCS) package but can also be used
	independently from it.  Historically, this was developed as
	part of an effort to maintain high function (and reliable)
	email communications in the face of increasingly draconian
	security restrictions imposed on the computers in the DEFINITY
	development organization.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/utsname.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<field.h>
#include	<ids.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<baops.h>
#include	<varsub.h>
#include	<getax.h>
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
#define	REALNAMELEN	100
#endif

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + REALNAMELEN)
#endif

#define	DEBUG_REGULAR	(pip->debuglevel > 0)
#define	DEBUG_EXTRA	(pip->debuglevel > 1)
#define	DEBUG_DEBUG	(pip->debuglevel > 2)


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
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
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			const char *,const char *) ;
extern int	isdigitlatin(int) ;

extern int	expander() ;
extern int	procfileenv(char *,char *,VECSTR *) ;
extern int	procfilepaths(char *,char *,VECSTR *) ;

extern char	*strbasename(char *), *strshrink(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;
static int	procfile(int (*)(char *,char *,VECSTR *),
			char *,VECSTR *,char *,VECSTR *) ;


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"ROOT",
	"CONFIG",
	"TMPDIR",
	"LOGFILE",
	"HELP",
	"af",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_root,
	argopt_conf,
	argopt_tmpdir,
	argopt_logfile,
	argopt_help,
	argopt_af,
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


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	struct ustat	sb ;
	PROGINFO	pi, *pip = &pi ;
	struct group	ge ;
	USERINFO	u ;
	CONFIGFILE	cf ;
	VECSTR		defines, unsets, exports ;
	VECSTR		svars ;
	VARSUB		vsh_e, vsh_d ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	pidfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan, npa ;
	int	argvalue = -1 ;
	int	rs ;
	int	rs1 ;
	int	len, i, j, k ;
	int	loglen = -1 ;
	int	l2, sl ;
	int	vl ;
	int	logfile_type = -1 ;
	int	c_locks = 0 ;
	int	c_captured = 0 ;
	int	to_lkwait = -1 ;
	int	to_remove = -1 ;
	int	mul_remove = -1 ;
	int	v ;
	int	ex = EX_INFO ;
	int	fd_debug = -1 ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_programroot = FALSE ;
	int	f_freeconfigfname = FALSE ;
	int	f_procfileenv = FALSE ;
	int	f_procfilepaths = FALSE ;
	int	f_help = FALSE ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
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
	char	*programroot = NULL ;
	char	*pr = NULL ;
	char	*configfname = NULL ;
	char	*afname = NULL ;
	char	*defsizespec = NULL ;
	char	*vp ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto ret0 ;
	}

	proginfo_setbanner(pip,BANNER) ;

	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {

	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;

	}

	pip->pid = getpid() ;

	pip->ppid = pip->pid ;
	pip->daytime = time(NULL) ;

/* initialize */

	pip->verboselevel = 1 ;

	pip->f.quiet = FALSE ;
	pip->f.log = FALSE ;

	pidfname[0] = '\0' ;
	lockfname[0] = '\0' ;
	logfname[0] = '\0' ;

/* start parsing the arguments */

	rs = SR_OK ;
	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1) 
		argpresent[ai] = 0 ;

	npa = 0 ;			/* number of positional so far */
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
	                                strwcpy(logfname,avp,avl) ;
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
	                                strwcpy(logfname,argp,argl) ;
	                            }

	                        }

	                        break ;

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

/* take input file arguments from STDIN */
	                        case 'f':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

/* we allow a zero length string as a valid a rgument */

	                            afname = argp ;
	                            break ;

/* quiet mode */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
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
	                                rs = cfdecti(argp,argl,&v) ;
					to_remove = v ;
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
	                                rs = cfdeci(argp,argl,&v) ;
					mul_remove = v ;
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
					rs = cfdecti(argp,argl,&v) ;
					to_lkwait = v ;
				    }

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
				    break ;

	                        } /* end switch */
	                        akp += 1 ;

				if (rs < 0) break ;
	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	    } else {

	        if (ai < MAXARGINDEX) {

	            BASET(argpresent,ai) ;
	            ai_max = ai ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

			rs = SR_INVALID ;
	                f_extra = TRUE ;
	                bprintf(pip->efp,"%s: extra arguments specified\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */

	if (rs < 0)
		goto badarg ;

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,NULL) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help */

	if (f_help) {

	    if ((helpfname == NULL) || (helpfname[0] == '\0'))
	        strcpy(helpfname,HELPFNAME) ;

	    printhelp(NULL,pip->pr,SEARCHNAME,helpfname) ;

	} /* end if (help) */

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
	    ex = EX_OSERR ;
	    goto baduser ;
	}

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	    debugprintf("main: name=\"%s\"\n",u.name) ;
#endif

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;
	pip->homedname = u.homedname ;

/* PWD */

	getpwd(buf,BUFLEN) ;

	pip->pwd = mallocstr(buf) ;

/* create the values for the file schedule searching */

	rs = vecstr_start(&svars,6,0) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badinit ;
	}

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

	    {
		const char	**evv = (const char **) envv ;
	        varsub_addva(&vsh_e,evv) ;
	    }

#if	CF_DEBUG
	if (DEBUG_DEBUG) {
	        debugprintf("main: 0 for\n") ;
	        varsub_dumpfd(&vsh_e,fd_debug) ;
	    }
#endif /* CF_DEBUG */

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

	            strwcpy(logfname,buf2,l2) ;

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


/* what about an 'environ' file ? */

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

#if	CF_DEBUG && 0
	if (DEBUG_DEBUG) {

	                debugprintf("varsub_merge: VSA_D so far \n") ;

	        	varsub_dumpfd(&vsh_d,fd_debug) ;

	                debugprintf("varsub_merge: VSA_E so far \n") ;

	        	varsub_dumpfd(&vsh_e,fd_debug) ;

	            } /* end if (debugging) */
#endif /* CF_DEBUG */

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
	    pip->workdname = WORKDNAME ;

	else if (pip->workdname[0] == '\0')
	    pip->workdname= "." ;

	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0')) {

	    if ((pip->tmpdname = getenv("TMPDIR")) == NULL)
	        pip->tmpdname = TMPDNAME ;

	} /* end if (tmpdname) */

	if (lockfname[0] == '\0')
	    strcpy(lockfname,LOCKFNAME) ;

	if ((sl = getfname(pip->pr,lockfname,1,tmpfname)) > 0)
	    strwcpy(lockfname,tmpfname,sl) ;

	pip->lockfname = lockfname ;

/* can we access the working directory? */

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	    debugprintf("main: access workdname=%s\n",pip->workdname) ;
#endif

	if ((perm(pip->workdname,-1,-1,NULL,X_OK) < 0) || 
	    (perm(pip->workdname,-1,-1,NULL,R_OK) < 0))
	    goto badworking ;

/* do we have an activity log file? */

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	    debugprintf("main: 0 logfname=%s\n",logfname) ;
#endif

	snsd(pip->logid,LOGIDLEN,pip->nodename,(uint) pip->pid) ;

	if (logfname[0] == '\0') {
	    logfile_type = 1 ;
	    strcpy(logfname,LOGFNAME) ;
	}

	sl = getfname(pip->pr,logfname,logfile_type,tmpfname) ;

	if (sl > 0)
	    strwcpy(logfname,tmpfname,sl) ;

	rs1 = logfile_open(&pip->lh,logfname,0,0666,pip->logid) ;

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs1) ;
#endif

	if (rs1 >= 0) {

	    pip->open.log = TRUE ;
	    if (pip->daytime == 0)
	        pip->daytime = time(NULL) ;

	    if (pip->logsize > 0)
	        logfile_checksize(&pip->lh,pip->logsize) ;

	    logfile_userinfo(&pip->lh,&u,
	        pip->daytime,pip->searchname,pip->version) ;

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
	    sprintf(buf,"GID%d",(int) pip->gid) ;

	} else
	    cp = ge.gr_name ;

	pip->groupname = mallocstr(cp) ;

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

	if ((to_lkwait < 0) && (argvalue > 0))
		to_lkwait = argvalue ;

	if (to_lkwait < 0)
		to_lkwait = LKFILE_DEFTIMEOUT ;

/* the remove-file multiplier */

	if (to_remove < 0) {

#ifdef	COMMENT
		if (mul_remove > 1)
			to_remove = to_lkwait * mul_remove ;

		else
			to_remove = to_lkwait * LKFILE_MULREMOVE ;
#else
		to_remove = LKFILE_DEFREMOVE ;
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
	if (npa > 0) {

	    for (ai = 0 ; ai < argc ; ai += 1) {

	        if (BATST(argpresent,ai) &&
	            (argv[ai][0] != '\0')) {

	        rs = process(pip,ofp,argv[ai],to_lkwait,to_remove) ;
		c_locks += 1 ;
		if (rs >= 0)
	            c_captured += 1 ;

	        pan += 1 ;

		}

	    } /* end for (looping through requested circuits) */

	} /* end if */


/* process any files in the argument filename list file */

	if (afname != NULL) {

		bfile	argfile, *afp = &argfile ;


	    if ((strcmp(afname,"-") == 0) || (afname[0] == '\0'))
	        rs = bopen(afp,BFILE_STDIN,"dr",0666) ;

	    else
	        rs = bopen(afp,afname,"r",0666) ;

	    if (rs >= 0) {

#if	CF_DEBUG
	if (DEBUG_DEBUG)
	            debugprintf("main: processing the argument file list\n") ;
#endif

	        while ((len = breadline(afp,buf,BUFLEN)) > 0) {

	            if (buf[len - 1] == '\n') len -= 1 ;

	            buf[len] = '\0' ;
	            cp = strshrink(buf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	        rs = process(pip,ofp,cp,to_lkwait,to_remove) ;

		c_locks += 1 ;
		if (rs >= 0)
	            c_captured += 1 ;

	            pan += 1 ;

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


	if ((pan == 0) && ((cp = getenv(VARFNAME)) != NULL)) {

#if	CF_DEBUG
	if (DEBUG_DEBUG) {
	        debugprintf("main: trying ENV variable\n") ;
	        debugprintf("main: logtab=%s\n",cp) ;
	    }
#endif

	    rs = SR_NOTFOUND ;
	    if ((u_stat(cp,&sb) >= 0) && S_ISREG(sb.st_mode)) {

	        rs = process(pip,ofp,cp,to_lkwait,to_remove) ;

		c_locks += 1 ;
		if (rs >= 0)
	            c_captured += 1 ;

	        pan += 1 ;
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

	    logfile_printf(&pip->lh,"%s locks=%d captured=%d\n",
		timestr_log(pip->daytime,timebuf),
		c_locks,c_captured) ;

	if (pip->verboselevel > 1)
		bclose(ofp) ;

badoutopen:
done:
	ex = EX_OK ;
	if (c_captured < c_locks)
		ex = EX_TIMEDOUT ;

/* close some more (earlier) daemon stuff */
daemonret2:
daemonret1:

#ifdef	OPTIONAL
	vecstr_finish(&defines) ;

	vecstr_finish(&exports) ;
#endif /* COMMENT */


#if	CF_DEBUG
	if (DEBUG_DEBUG)
	    debugprintf("main: program finishing\n",
	        pip->progname) ;
#endif

	if (f_freeconfigfname && (configfname != NULL))
	    free(configfname) ;

	if (pip->f.log)
	    logfile_close(&pip->lh) ;

	if (pip->lfp != NULL)
	    bclose(pip->lfp) ;

badinit:
baduser:
retearly:
ret2:
	bclose(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

ret0:

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

/* more bad */
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

	    bprintf(pip->efp,"%s: pidfile=%s\n", pip->progname,pip->pidfname) ;

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
	logfile_printf(&pip->lh,
	    "could not open the PID mutex file (rs=%d)\n",
	    rs) ;

	logfile_printf(&pip->lh,"pidfile=%s\n", pip->pidfname) ;

	goto daemonret2 ;

badpidfile2:
	logfile_printf(&pip->lh,
	    "there was a daemon already on the mutex file, PID=%d\n",
	    pip->pid) ;

	goto daemonret2 ;

badret:
	ex = EX_DATAERR ;
	goto done ;
}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [lockfile [...]] [-C conf] [-timeout] [-?v]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s: \t[-af argfile] [-ROOT programroot]\n",
		pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s: \t[-t timeout] [-r removetime]\n",
		pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procfile(func,pr,svp,fname,elp)
int		(*func)(char *,char *,VECSTR *) ;
char		pr[] ;
VECSTR		*svp ;
char		fname[] ;
VECSTR		*elp ;
{
	int	sl ;

	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUGS
	    debugprintf("procfile: 1 fname=%s\n",fname) ;
#endif

	if ((sl = permsched(sched2,svp, tmpfname,MAXPATHLEN, fname,R_OK) ;
	    tmpfname,MAXPATHLEN)) < 0) {
	if (sl < 0) {

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



