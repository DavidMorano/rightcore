/* main */

/* generic (pretty much) front end program subroutine */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGNOCONF	0		/* don't allow config file */
#define	CF_RUNMODE	0		/* put RUNMODE in environment */
#define	CF_ACCTAB	1
#define	CF_SETENTRY	1		/* use 'proginfo_setentry()' */


/* revision history:

	= 1998-09-01, David A­D­ Morano

	This program was originally written for PCS.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This subroutine forms the front-end part of a generic PCS type
	of program.  This front-end is used in a variety of PCS programs.

	This subroutine was originally part of the Personal Communications
	Services (PCS) package but can also be used independently from it.
	Historically, this was developed as part of an effort to maintain
	high function (and reliable) email communications in the face
	of increasingly draconian security restrictions imposed on the
	computers in the DEFINITY development organization.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/mkdev.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<keyopt.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<lfm.h>
#include	<varsub.h>
#include	<exitcodes.h>
#include	<mallocstuff.h>
#include	<pwfile.h>
#include	<getax.h>
#include	<srvtab.h>
#include	<acctab.h>
#include	<nistinfo.h>
#include	<localmisc.h>

#include	"builtin.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + REALNAMELEN)
#endif

#ifndef	NULLDEV
#define	NULLDEV		"/dev/null"
#endif

#ifndef	PORTBUFLEN
#define	PORTBUFLEN	20
#endif


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	snddd(char *,int,uint,uint) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	getpwd(char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	listentcp(const char *,const char *,int) ;
extern int	listenfifo(const char *,int,int) ;
extern int	getfname(char *,char *,int,char *) ;
extern int	bopenroot(bfile *,char *,char *,char *,char *,int) ;
extern int	getserial(const char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			const char *,const char *) ;

extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;

extern int	varsub_subbuf(), varsub_merge() ;
extern int	expander(struct proginfo *,char *,int,char *,int) ;
extern int	procfileenv(char *,char *,VECSTR *) ;
extern int	procfilepaths(char *,char *,VECSTR *) ;
extern int	watch(struct proginfo *, BUILTIN *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;
static int	procopts(struct proginfo *,KEYOPT *) ;
static int	progserial(struct proginfo *) ;
static int	checkdir(struct proginfo *,const char *,int) ;
static int	checkfiledir(struct proginfo *,const char *) ;
static int	getprogopts(struct proginfo *,KEYOPT *,vecstr *) ;
static int	procfile(struct proginfo *,int (*)(char *,char *,VECSTR *),
			char *,vecstr *,char *,VECSTR *) ;
static int	caf(struct proginfo *) ;
static int	svarsinit(struct proginfo *,vecstr *) ;
static int	svarsfree(struct proginfo *,vecstr *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"CONFIG",
	"TMPDIR",
	"LOGFILE",
	"HELP",
	"sn",
	"caf",
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
	argopt_sn,
	argopt_caf,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;

static const char *configopts[] = {
	"marktime",
	"defacc",
	"vardir",
	"spooldir",
	"req",
	"shm",
	"orgcode",
	"msfile",
	"mspoll",
	"speedname",
	"nopass",
	NULL
} ;

enum configopts {
	configopt_marktime,
	configopt_defacc,
	configopt_vardir,
	configopt_spooldir,
	configopt_reqfile,
	configopt_shmfile,
	configopt_orgcode,
	configopt_msfile,
	configopt_mspoll,
	configopt_speedname,
	configopt_nopass,
	configopt_overlast
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

/* option terminators */
static const unsigned char 	oterms[32] = {
	0x00, 0x0B, 0x00, 0x00,
	0x09, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	struct servent	*sep ;

	struct ustat	sb ;

	USERINFO	u ;

	KEYOPT		akopts ;

	VECSTR		svars ;

	BUILTIN		bis ;

	SRVTAB_ENT	*srvp ;

	bfile		errfile ;
	bfile		pidfile ;

	vecstr		defines, unsets ;

	varsub		vsdefines, vsexports ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs = SR_OK ;
	int	rs1 ;
	int	len, i ;
	int	loglen = -1 ;
	int	iw, sl, sl2, cl ;
	int	opts ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_caf = FALSE ;
	int	f ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1] ;
	char	buf2[BUFLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*pr = NULL ;
	char	*searchname = NULL ;
	char	*confname = NULL ;
	char	*logfname = NULL ;
	char	*pidfname = NULL ;
	char	*portspec = NULL ;
	char	*sp, *cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

/* we want to open up some files so that the first few FD slots are FULL! */

	if (u_fstat(FD_STDIN,&sb) < 0)
	    u_open("/dev/null",O_RDONLY,0666) ;

	proginfo_start(pip,envv,argv[0],VERSION) ;

	if ((cp = getenv(VARBANNER)) == NULL)
		cp = BANNER ;

	proginfo_setbanner(pip,cp) ;


	pip->f.efile = FALSE ;
	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
	    pip->efp = &errfile ;
	    pip->f.efile = TRUE ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	} else
	    u_open("/dev/null",O_WRONLY,0666) ;

	pip->f.ofile = TRUE ;
	if (u_fstat(FD_STDOUT,&sb) < 0) {
	    pip->f.ofile = FALSE ;
	    u_open("/dev/null",O_WRONLY,0666) ;
	}

/* initialize */

	pip->efp = &errfile ;

	pip->fd_listenpass = -1 ;
	pip->fd_listentcp = -1 ;
	pip->fd_req = -1 ;

	pip->verboselevel = 1 ;
	pip->markint = -1 ;

	lockfname[0] = '\0' ;
	srvfname[0] = '\0' ;
	accfname[0] = '\0' ;
	pidfname[0] = '\0' ;
	vardname[0] = '\0' ;
	spooldname[0] = '\0' ;
	reqfname[0] = '\0' ;
	shmfname[0] = '\0' ;
	passfname[0] = '\0' ;
	passwdfname[0] = '\0' ;
	msfname[0] = '\0' ;
	orgcode[0] = '\0' ;

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
	                                confname = avp ;

	                        } else {

	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                confname = argp ;

	                        }

	                        break ;

/* log file */
	                    case argopt_logfile:
				    pip->final.logfname = TRUE ;
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

/* searchname */
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

				case argopt_caf:
				    f_caf = TRUE ;
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
	                                confname = argp ;

	                            break ;

/* mutex lock PID file */
	                        case 'P':
				    pip->final.pidfname = TRUE ;
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pidfname = argp ;

	                            break ;

	                        case 'Q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* daemon mode */
	                        case 'd':
	                            pip->f.daemon = TRUE ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    rs = cfdeci(avp,avl,
	                                        &pip->runint) ;

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

/* TCP port to listen on */
	                        case 'p':
				    pip->final.portspec = TRUE ;
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                portspec = argp ;

	                            break ;

/* reuse the bind addresses */
	                        case 'r':
				    pip->final.reuse = TRUE ;
	                            pip->f.reuse = TRUE ;
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
	if (DEBUGLEVEL(2)) {
		debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
		debugprintf("main: progname=%s\n",pip->progname) ;
	}
#endif /* CF_DEBUG */

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    usage(pip) ;

/* get some program information */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

/* search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,searchname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: PROG pr=%s\n",pip->pr) ;
	    debugprintf("main: PROG searchname=%s\n",pip->searchname) ;
	}
#endif /* CF_DEBUG */

/* help */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n",pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n",pip->searchname) ;
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* load the positional arguments */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	        switch (pan) {

	        case 0:
	            if ((strlen(cp) > 0) && (cp[0] != '-'))
	                confname = cp ;

	            break ;

	        case 1:
	            if ((strlen(cp) > 0) && (cp[0] != '-'))
	                portspec = cp ;

	            break ;

	        default:
	            rs = SR_INVALID ;
	            bprintf(pip->efp,
	                "%s: extra positional arguments specified\n",
	                pip->progname) ;

	        } /* end switch */

	        pan += 1 ;

	} /* end for (loading positional arguments) */

	if (rs < 0)
		goto badarg ;

/* close all files? */

	if (f_caf)
		caf(pip) ;

/* initialize */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	if (rs < 0) {
		ex = EX_NOUSER ;
		goto retbaduser ;
	}

	pip->username = u.username ;
	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->logid = u.logid ;

/* handle UID/GID stuff */

	pip->uid = u.uid ;
	pip->euid = geteuid() ;

	pip->gid = u.gid ;
	pip->egid = getegid() ;

	pip->ppid = pip->pid ;

/* hostname */

	rs1 = snsds(buf,BUFLEN,pip->nodename,pip->domainname) ;

	proginfo_setentry(pip,&pip->hostname,buf,rs1) ;

/* groupname */

	rs1 = getgroupname(buf,BUFLEN,pip->gid) ;

	proginfo_setentry(pip,&pip->groupname,buf,rs1) ;

/* load up the environment options */

	rs = procopts(pip,&akopts) ;

	if (rs < 0) {
		ex = EX_OSERR ;
		goto ret2 ;
	}

	if (pip->tmpdname == NULL)
		pip->tmpdname = getenv(VARTMPDNAME) ;

	if (pip->tmpdname == NULL)
			pip->tmpdname = TMPDNAME ;

	if (pip->defacc == NULL)
		proginfo_setentry(pip,&pip->defacc, "DEFAULT",-1) ;

/* check program parameters */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checking program parameters\n") ;
#endif

	if (pip->workdname == NULL)
	    pip->workdname = WORKDNAME ;

	else if (pip->workdname[0] == '\0')
	    pip->workdname = "." ;

/* VAR directory */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: vardname=%s\n",vardname) ;
#endif

	sl2 = -1 ;
	if (vardname[0] == '\0')
	    sl2 = expander(pip,VARDNAME,-1,vardname,MAXPATHLEN) ;

	if (vardname[0] != '/') {

	    sl = mkpath2(tmpfname,pip->pr,vardname) ;

	    mkpath1(vardname,tmpfname) ;

	}

	pip->vardname = vardname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 2 vardname=%s\n",pip->vardname) ;
#endif

	rs = checkdir(pip,vardname,(0777 | S_ISVTX)) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checkdir() rs=%d vardname=%s\n",
		rs,pip->vardname) ;
#endif

/* spool directory */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: spooldname=%s\n",spooldname) ;
#endif

	sl2 = -1 ;
	if (spooldname[0] == '\0')
	    sl2 = expander(pip,SPOOLDNAME,-1,spooldname,MAXPATHLEN) ;

	if (spooldname[0] != '/') {

	    sl = mkpath2(tmpfname,pip->pr,spooldname) ;

	    strcpy(spooldname,tmpfname) ;

	}

	pip->spooldname = spooldname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 2 spooldname=%s\n",pip->spooldname) ;
#endif

	rs = checkdir(pip,spooldname,(0777 | S_ISVTX)) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checkdir() spooldname=%s\n",pip->spooldname) ;
#endif

/* MTA? */

	if (pip->prog_sendmail == NULL) {

#if	CF_SETENTRY
	    proginfo_setentry(pip,&pip->prog_sendmail,PROG_SENDMAIL,-1) ;
#else
	    pip->prog_sendmail = mallocstr(PROG_SENDMAIL) ;
#endif

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: sendmail=%s\n",
	        pip->prog_sendmail) ;
#endif

	if (pip->markint <= 0)
	    pip->markint = TI_MARKTIME ;

	if (pip->f.mspoll) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: MS polling turned on\n") ;
#endif

	    if (pip->pollint <= 0)
	        pip->pollint = TI_POLLINT ;

	}

/* can we access the working directory? */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: access working directory \"%s\"\n",
		pip->workdname) ;
#endif

	if ((perm(pip->workdname,-1,-1,NULL,X_OK) < 0) || 
	    (perm(pip->workdname,-1,-1,NULL,R_OK) < 0))
	    goto badworking ;

/* get a serial number for logging purposes */

	progserial(pip) ;

/* does the log file name need massaging? */

	if (logfname == NULL) {

	    logfile_type = 1 ;
	    rs1 = expander(pip,LOGFNAME,-1,tmpfname,MAXPATHLEN) ;

	}

	sl = getfname(pip->pr,logfname,logfile_type,tmpfname) ;

	if (sl > 0)
	    strwcpy(logfname,tmpfname,sl) ;

	rs = logfile_open(&pip->lh,pip->logfname,0,0666,pip->logid) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if (rs >= 0) {

	    pip->f.log = TRUE ;
	    pip->open.log = TRUE ;
	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: logfile=%s\n",
		pip->progname,pip->logfname) ;

/* we opened it, maintenance this log file if we have to */

	    if (loglen < 0)
	        loglen = LOGSIZE ;

	    logfile_checksize(&pip->lh,loglen) ;

	    pip->daytime = time(NULL) ;

	    logfile_userinfo(&pip->lh,&u,pip->daytime,
		pip->progname,pip->version) ;

	    logfile_printf(&pip->lh,"conf=%s\n",pip->confname) ;

	    logfile_printf(&pip->lh,"pr=%s\n",pip->pr) ;

	} /* end if (we have a log file or not) */

/* load file schedule variables */

	rs = svarsinit(pip,&svars) ;

/* what about the lock file name? */

	if (lockfname[0] != '-') {

	    sl2 = -1 ;
	    if (lockfname[0] == '+')
	        sl2 = expander(pip,LOCKFNAME,-1,lockfname,MAXPATHLEN) ;

	    if (lockfname[0] != '\0') {

	        sl = getfname(pip->pr,lockfname,1,tmpfname) ;

	        if (sl > 0)
	            strwcpy(lockfname,tmpfname,sl) ;

	    }

	} else
	    lockfname[0] = '\0' ;

	pip->lockfname = lockfname ;

/* request filename */

	if (pip->f.daemon) {

	    if (reqfname[0] != '-') {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: cleanup reqfname=%s\n",reqfname) ;
#endif

	        sl2 = -1 ;
	        if ((reqfname[0] == '+') || (reqfname[0] == '\0'))
	            sl2 = expander(pip,REQFNAME,-1,reqfname,MAXPATHLEN) ;

	        if (reqfname[0] != '\0') {

	            sl = getfname(pip->pr,reqfname,1,tmpfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	                debugprintf("main: reqfname getfname() rs=%d\n",sl) ;
#endif

	            if (sl > 0)
	                strwcpy(reqfname,tmpfname,sl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	                debugprintf("main: getfname() sl=%d reqfname=%s\n",
	                    sl,reqfname) ;
#endif

	        }

	        if ((reqfname[0] != '\0') && (reqfname[0] != '/')) {

	            sl = mkpath2(tmpfname,pip->pwd,reqfname) ;

	            strwcpy(reqfname,tmpfname,sl) ;

	        }

	    } /* end if */

	    pip->reqfname = reqfname ;

	    if ((reqfname[0] != '\0') && (reqfname[0] != '-'))
	        logfile_printf(&pip->lh,"req=%s\n",reqfname) ;

	} /* end if (request file for daemon mode) */

/* what about a pass-FD file name? */

	if (! pip->f.nopass) {

	if (passfname[0] != '-') {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("main: cleanup passfname=%s\n",passfname) ;
#endif

	    sl2 = -1 ;
	    if (passfname[0] == '+')
	        sl2 = expander(pip,PASSFNAME,-1,passfname,MAXPATHLEN) ;

	    if (passfname[0] != '\0') {

	        sl = getfname(pip->pr,passfname,1,tmpfname) ;

	        if (sl > 0)
	            strwcpy(passfname,tmpfname,sl) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: getfname() sl=%d passfname=%s\n",
	                sl,passfname) ;
#endif

	    }

	    if ((passfname[0] != '\0') && (passfname[0] != '/')) {

	        sl = mkpath2(tmpfname,pip->pwd,passfname) ;

	        strwcpy(passfname,tmpfname,sl) ;

	    }

	} /* end if */

	pip->passfname = passfname ;

	if ((passfname[0] != '\0') && (passfname[0] != '-'))
	    logfile_printf(&pip->lh,"pass=%s\n",passfname) ;

	} /* end if (FD passing enabled) */

#if	defined(P_PCSUUCPD) && (P_PCSUUCPD == 1)

/* find a PASSWD file if we can */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 0 passwdfname=%s\n",passwdfname) ;
#endif

	rs = SR_NOEXIST ;
	if (passwdfname[0] == '\0') {

	    passwd_type = 1 ;
	    strcpy(passwdfname,PASSWDFNAME) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("main: 0a passwdfname=%s\n",passwdfname) ;
#endif

	    if ((sl = permsched(sched2,&svars,
	        tmpfname,MAXPATHLEN, passwdfname,R_OK)) < 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: 0b not found\n") ;
#endif

	        sl = permsched(sched3,&svars,
	            tmpfname,MAXPATHLEN, passwdfname,R_OK) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2) && (sl < 0))
	            debugprintf("main: 0c not found\n") ;
#endif

	    }

	    if (sl > 0)
	        strcpy(passwdfname,tmpfname) ;

	    rs = sl ;

	} else {

	    if (passwd_type < 0)
	        passwd_type = 0 ;

	    sl = getfname(pip->pr,passwdfname,passwd_type,tmpfname) ;

	    if (sl > 0)
	        strwcpy(passwdfname,tmpfname,sl) ;

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 1 passwdfname=%s\n",passwdfname) ;
#endif

	if ((rs >= 0) || (perm(passwdfname,-1,-1,NULL,R_OK) >= 0)) {

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: passwd=%s\n",pip->progname,passwdfname) ;

	    logfile_printf(&pip->lh,"passwd=%s\n",passwdfname) ;

	    if ((rs = pwfile_open(&pip->passwd,passwdfname)) < 0) {

	        logfile_printf(&pip->lh,
	            "couldn't open passwd file (%d)\n",rs) ;

	        bprintf(pip->efp,"%s: passwd=%s\n",
	            pip->progname,passwdfname) ;

	        bprintf(pip->efp,"%s: couldn't open passwd file (%d)\n",
	            pip->progname,rs) ;

	    } else
	        pip->f.passwd = TRUE ;

	} /* end if (accessing a 'passwd' file) */

	if (rs < 0) {

	    logfile_printf(&pip->lh,
	        "could not open password file (%d) passwd=%s\n",
	        rs,passwdfname) ;

	    goto badpasswd ;
	}

#endif /* P_PCSUUCPD */

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
	    rs = srvtab_check(&pip->stab,pip->daytime,NULL) ;
	    debugprintf("main: after PASSWD srvtab_check() rs=%d\n",rs) ;
	}
#endif /* CF_DEBUG */

/* MS file */

	if (pip->f.mspoll) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: 1 msfname=%s\n",msfname) ;
#endif

	    rs = SR_NOEXIST ;
	    if (msfname[0] == '\0')
	        sl = expander(pip,MSFNAME,-1,msfname,MAXPATHLEN) ;

	    if (msfname[0] != '/') {

	        sl = mkpath2(tmpfname,pip->pr,msfname) ;

	        if (sl > 0)
	            strwcpy(msfname,tmpfname,sl) ;

	    } /* end if */

	    pip->msfname = msfname ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: 2 msfname=%s\n",pip->msfname) ;
#endif

	} /* end if (MS) */

/* organization code */

	if (orgcode[0] == '\0')
	    strwcpy(orgcode,ORGCODE,NISTINFO_ORGSIZE) ;

	pip->orgcode = orgcode ;

/* load up some environment and execution paths if we have not already */

	if (! f_procfileenv) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: 2 procfileenv() \n") ;
#endif

	    procfile(pip,procfileenv,pip->pr,&svars,
	        XENVFNAME,&pip->exports) ;

	}

	if (! f_procfilepaths) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: 2 procfilepaths() \n") ;
#endif

	    procfile(pip,procfilepaths,pip->pr,&svars,
	        XPATHFNAME,&pip->exports) ;

	}

/* if we are a daemon program, try to bind our INET port */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: about to check if we are a daemon\n") ;
#endif

	if (pip->f.daemon) {

	    in_addr_t	addr ;


#if	CF_DEBUG && 0
	if (DEBUGLEVEL(2)) {
	        debugprintf("main: daemon mode\n") ;
	        d_whoopen("2") ;
	    }
#endif /* CF_DEBUG */

/* look up some miscellaneous stuff in various databases */

	    if (portspec == NULL) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: default port \n") ;
#endif

	        sep = getservbyname(PORTNAME, "tcp") ;

	        if (sep != NULL)
	            portspec = PORTNAME ;

	        else
	            portspec = PORTNUM ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: done looking at port stuff so far\n") ;
#endif

	    } /* end if (no port specified) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("main: about to listen, portspec=%s\n",portspec) ;
#endif

/* start to listen on our specified receive points */

	    pip->fd_listenpass = -1 ;
	    if ((passfname[0] != '\0') && (passfname[0] != '-') &&
		(! pip->f.nopass)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: listenpass() passfname=%s\n",passfname) ;
#endif

	        cl = sfdirname(passfname,-1,&cp) ;

	        if (cl > 0) {

	            strwcpy(tmpfname,cp,cl) ;

	            if (u_stat(tmpfname,&sb) < 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: mkdirs() dname=%s\n",tmpfname) ;
#endif

	                mkdirs(tmpfname,0755) ;

		    }
	        }

		if (pip->debuglevel > 0)
			bprintf(pip->efp,"%s: passfile=%s\n",
				pip->progname,passfname) ;

	        rs = listenfifo(passfname,0622,0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: listnefifo() rs=%d\n",rs) ;
#endif

	        if (rs < 0) {

	            logfile_printf(&pip->lh,
			"could not listen on PASS (%d)\n",
	                rs) ;

	            goto badlisten1 ;
	        }

	        pip->fd_listenpass = rs ;
		uc_closeonexec(pip->fd_listenpass,TRUE) ;

	        rs1 = u_stat(passfname,&sb) ;

	        if ((rs1 >= 0) && ((sb.st_mode & S_IWOTH) == 0))
	            u_chmod(passfname,0622) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: fd_listenpass=%d\n",pip->fd_listenpass) ;
#endif

	        logfile_printf(&pip->lh,"listening on PASS '%s'\n",
	            strbasename(passfname)) ;

	    } /* end if (listen on pass PASS FIFO) */

	    pip->fd_listentcp = -1 ;
	    if (portspec[0] != '-') {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: listen TCP portspec=%s\n",portspec) ;
#endif

		if (pip->debuglevel > 0)
			bprintf(pip->efp,"%s: portspec=%s\n",
				pip->progname,portspec) ;

	        rs = listentcp(NULL,portspec,pip->f.reuse) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: listnetcp() rs=%d\n",rs) ;
#endif

	        if (rs < 0) {

	            logfile_printf(&pip->lh,
			"could not listen on TCP (%d)\n",
	                rs) ;

	            goto badlisten2 ;
	        }

	        pip->fd_listentcp = rs ;
		uc_closeonexec(pip->fd_listentcp,TRUE) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	            debugprintf("main: fd_listentcp=%d\n",pip->fd_listentcp) ;
#endif

	        logfile_printf(&pip->lh,"listening on TCP port '%s'\n",
	            portspec) ;

	    } /* end if (listen on TCP) */

/* background ourselves */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf( "main: become a daemon?\n") ;
#endif

	    if (pip->f.log)
	        logfile_flush(&pip->lh) ;

	    bflush(pip->efp) ;

	    if (pip->debuglevel == 0) {

#ifdef	COMMENT
	        for (i = 0 ; i < 3 ; i += 1) {
	            u_close(i) ;
	            (void) u_open("/dev/null",O_RDONLY,0666) ;
	        } /* end for */
#endif /* COMMENT */

	        rs = uc_fork() ;

	        if (rs < 0) {
	            logfile_printf(&pip->lh,
	                "cannot fork daemon (%d)\n",rs) ;
	            uc_exit(EX_OSERR) ;
	        }

	        if (rs > 0)
	            uc_exit(EX_OK) ;

	        u_setsid() ;

	    } /* end if (backgrounding) */

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(2)) {
	        debugprintf("main: after daemon backgrounding\n") ;
	        d_whoopen("3") ;
	    }
#endif /* CF_DEBUG */

	    pip->pid = getpid() ;

	} /* end if (daemon mode) */

/* we start! */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: starting\n") ;
#endif

/* if we are a daemon, check out PID mutex again proper */

	if (pip->f.daemon) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("main: more daemon stuff\n") ;
#endif

	    if (rs == 0)
	        logfile_printf(&pip->lh,"backgrounded pid=%d\n",pip->pid) ;

	    if ((pip->pidfname != NULL) && (pip->pidfname[0] != '\0')) {

		int	devmajor, devminor ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
		debugprintf("main: we have a PIDFNAME=%s\n",
		pip->pidfname) ;
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

	        bprintf(&pidfile,"%s!%s\n",pip->nodename,pip->username) ;

	        bprintf(&pidfile,"%s %s\n",
	            timestr_logz(pip->daytime,timebuf),pip->banner) ;

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

		devmajor = major(sb.st_dev) ;

		devminor = minor(sb.st_dev) ;

	        logfile_printf(&pip->lh,
		    "pidfile device=%u,%u inode=%lu\n",
	            devmajor,devminor,sb.st_ino) ;

	        pip->pidfp = &pidfile ;

	    } /* end if (we have a mutex PID file) */

/* OK, pop the lock file if we have one */

	    if ((pip->lockfname != NULL) && (pip->lockfname[0] != '\0')) {

	        rs = lfm_start(&pip->lfile,
			pip->lockfname,LFM_TCREATE,TO_LOCK, NULL,
			pip->nodename,pip->username,pip->banner) ;

	        pip->f.lockfile = (rs >= 0) ;

	    } /* end if (lock file) */

	} /* end if (daemon mode) */

/* find and open the server file */

	rs = SR_NOEXIST ;
	if (srvfname[0] == '\0') {

	    srvtab_type = 1 ;
	    mkpath1(srvfname,SRVFNAME) ;

	    if ((sl = permsched(sched2,&svars,
	        tmpfname,MAXPATHLEN, srvfname,R_OK)) < 0) {

	        if ((sl = permsched(sched3,&svars,
	            tmpfname,MAXPATHLEN, srvfname,R_OK)) > 0)
	            mkpath1(srvfname,tmpfname) ;

	    } else if (sl > 0)
	        mkpath1(srvfname,tmpfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("main: 0 srvfname=%s\n",srvfname) ;
#endif

	    rs = sl ;

	} else {

	    if (srvtab_type < 0)
	        srvtab_type = 0 ;

	    sl = getfname(pip->pr,srvfname,srvtab_type,tmpfname) ;

	    if (sl > 0)
	        snwcpy(srvfname,MAXPATHLEN,tmpfname,sl) ;

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 1 rs=%d srvfname=%s\n",rs,srvfname) ;
#endif

	if ((rs >= 0) || (perm(srvfname,-1,-1,NULL,R_OK) >= 0)) {

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: srvtab=%s\n",pip->progname,srvfname) ;

	    logfile_printf(&pip->lh,"srvtab=%s\n",srvfname) ;

	    if ((rs = srvtab_open(&pip->stab,srvfname,NULL)) < 0) {

	        logfile_printf(&pip->lh,"bad (%d) server file\n",rs) ;

	        bprintf(pip->efp,"%s: srvtab=%s\n",
	            pip->progname,srvfname) ;

	        bprintf(pip->efp,"%s: bad (%d) server file\n",
	            pip->progname,rs) ;

	    } else
	        pip->f.srvtab = TRUE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	        debugprintf("main: service entries>\n") ;
	        for (i = 0 ; srvtab_get(&pip->stab,i,&srvp) >= 0 ; 
	            i += 1) {
	            if (srvp == NULL) continue ;
	            debugprintf("main: got an entry i=%d\n",i) ;
	            if (srvp->service != NULL) {
	                debugprintf("main: non-NULL service name\n") ;
	                debugprintf("main: service=%s\n",srvp->service) ;
	            }
	        } /* end for */
	    }
#endif /* CF_DEBUG */

	} /* end if (accessing a 'srvtab' file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: SRVTAB rs=%d\n",rs) ;
#endif

/* find and open the Access Table file if we have one */

#if	CF_ACCTAB
	rs = SR_NOEXIST ;
	if (accfname[0] == '\0') {

	    acctab_type = 1 ;
	    mkpath1(accfname,ACCFNAME) ;

	    if ((sl = permsched(sched2,&svars,
	        tmpfname,MAXPATHLEN, accfname,R_OK)) < 0) {

	        if ((sl = permsched(sched3,&svars,
	            tmpfname,MAXPATHLEN, accfname,R_OK)) > 0)
	            mkpath1(accfname,tmpfname) ;

	    } else if (sl > 0)
	        mkpath1(accfname,tmpfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("main: 0 accfname=%s\n",accfname) ;
#endif

	    rs = sl ;

	} else {

	    if (acctab_type < 0)
	        acctab_type = 0 ;

	    sl = getfname(pip->pr,accfname,acctab_type,tmpfname) ;

	    if (sl > 0)
	        strwcpy(accfname,tmpfname,sl) ;

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 1 rs=%d accfname=%s\n",rs,accfname) ;
#endif

	pip->f.acctab = FALSE ;
	if ((rs >= 0) || (perm(accfname,-1,-1,NULL,R_OK) >= 0)) {

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: acctab=%s\n",pip->progname,accfname) ;

	    logfile_printf(&pip->lh,"acctab=%s\n",accfname) ;

	    if ((rs = acctab_open(&pip->atab,accfname,NULL)) < 0) {

	        logfile_printf(&pip->lh,"bad (%d) access file\n",rs) ;

	        bprintf(pip->efp,"%s: acctab=%s\n",
	            pip->progname,srvfname) ;

	        bprintf(pip->efp,"%s: bad (%d) access file\n",
	            pip->progname,rs) ;

	    } else
	        pip->f.acctab = TRUE ;

	} /* end if (accessing a 'acctab' file) */

#endif /* CF_ACCTAB */

/* open the built-in servers */

	builtin_init(&bis,pip) ;

/* set an environment variable for the program run mode */

#if	CF_RUNMODE
	rs = vecstr_finder(&pip->exports,"RUNMODE",vstrkeycmp,&cp) ;

	if (rs >= 0)
	    vecstr_del(&pip->exports,rs) ;

	sl = bufprintf(tmpfname,MAXPATHLEN,"RUNMODE=%s",
		pip->searchname) ;

	vecstr_add(&exports,tmpfname,sl) ;
#endif /* CF_RUNMODE */

	if (vecstr_finder(&pip->exports,"HZ",vstrkeycmp,NULL) < 0) {

	    sl = bufprintf(tmpfname,MAXPATHLEN,"HZ=%ld",CLK_TCK) ;

	    vecstr_add(&pip->exports,tmpfname,sl) ;

	}

	if (vecstr_finder(&pip->exports,"PATH",vstrkeycmp,NULL) < 0) {

	    sl = bufprintf(buf,BUFLEN,"PATH=%s",DEFPATH) ;

	    if (sl > 0)
	        vecstr_add(&pip->exports,buf,sl) ;

	}

/* clean up some stuff we will no longer need */

	svarsfree(pip,&svars) ;

/* create the table substitutions for use later */

	varsub_start(&pip->subs,0) ;

/* load up the configuration define variables */

	varsub_addvec(&pip->subs,&defines) ;

/* load up the environment variables */

	varsub_addva(&pip->subs,(const char **) pip->envv) ;

/* we are done initializing */

	if (pip->f.daemon && pip->open.log) {

	    pip->daytime = time(NULL) ;

	    logfile_printf(&pip->lh,"%s finished initializing\n",
	        timestr_logz(pip->daytime,timebuf)) ;

	    logfile_flush(&pip->lh) ;

	} /* end if (making log entries) */

/* fill in some server information that we have so far */

#ifndef	COMMENT
	rs = watch(pip,&bis) ;
#else
	rs = SR_OK ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: watch() rs=%d\n",rs) ;
#endif

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

/* release the table substitutions */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: freeing subs \n") ;
#endif

	varsub_finish(&pip->subs) ;

/* close the daemon stuff */
daemonret2:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: freeing builtin \n") ;
#endif

	builtin_free(&bis) ;

	if (pip->f.acctab)
	    acctab_close(&pip->atab) ;

	if (pip->f.srvtab)
	    srvtab_close(&pip->stab) ;

	if (pip->f.lockfile)
	    lfm_finish(&pip->lfile) ;

/* close some more (earlier) daemon stuff */
daemonret1:
	if (pip->fd_listentcp >= 0)
	    u_close(pip->fd_listentcp) ;

	if (pip->fd_listenpass >= 0)
	    u_close(pip->fd_listenpass) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: freeing defines exports\n") ;
#endif

ret5:
	if (pip->f.passwd)
	    pwfile_close(&pip->passwd) ;

ret4:
	if (pip->f.log)
	    logfile_close(&pip->lh) ;

ret3:

retearly:
retbaduser:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

ret2:
	if (pip->open.akopts)
	    keyopt_finish(&akopts) ;

	if (pip->efp != NULL)
	    bclose(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad argument usage */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

badspool:

badlistinit:
	bprintf(pip->efp,"%s: could not initialize list structures (%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badsrv:
	bprintf(pip->efp,"%s: bad service table file (%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badconfig:
	bprintf(pip->efp,
	    "%s: conf=%s\n",
	    pip->progname,confname) ;

	bprintf(pip->efp,
	    "%s: error (%d) in configuration file starting at line %d\n",
	    pip->progname,rs,cf.badline) ;

	goto badret ;

badpidopen:
	ex = EX_MUTEX ;
	bprintf(pip->efp,
	    "%s: could not open the PID file (%d)\n",
	    pip->progname,rs) ;

	bprintf(pip->efp, "%s: pidfile=%s\n", pip->progname,pidfname) ;

	goto retearly ;

badpidlock:
	ex = EX_MUTEX ;
	if (! pip->f.quiet) {

	    bprintf(pip->efp,
	        "%s: could not lock the PID file (%d)\n",
	        pip->progname,rs) ;

	    bprintf(pip->efp, "%s: pidfile=%s\n", pip->progname,pip->pidfname) ;

	    while ((len = breadline(&pidfile,buf,BUFLEN)) > 0) {

	        bprintf(pip->efp,"%s: pidfile> %t",
	            pip->progname,
	            buf,len) ;

	    }

	} /* end if */

	bclose(&pidfile) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: PID lock failed, rs=%d\n",rs) ;
#endif

	goto retearly ;

badworking:
	ex = EX_OSFILE ;
	bprintf(pip->efp,"%s: could not access the working directory \"%s\"\n",
	    pip->progname,pip->workdname) ;

	goto ret3 ;

badpasswd:
	ex = EX_OSFILE ;
	bprintf(pip->efp,"%s: could not find a passwd file\n",
	    pip->progname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: no passwd file rs=%d\n",rs) ;
#endif

	goto ret4 ;

badport:
	bprintf(pip->efp,
	    "%s: bad port specified\n",
	    pip->progname) ;

	goto daemonret1 ;

badlisten1:
	ex = EX_DATAERR ;
	bprintf(pip->efp,
	    "%s: could not listen to our server PASS (%d)\n",
	    pip->progname,rs) ;

	goto daemonret1 ;

badlisten2:
	ex = EX_DATAERR ;
	bprintf(pip->efp,
	    "%s: could not listen to our server socket (%d)\n",
	    pip->progname,rs) ;

	goto daemonret1 ;

badpidfile1:
	ex = EX_MUTEX ;
	logfile_printf(&pip->lh,
	    "could not open the PID mutex file (%d)\n",
	    rs) ;

	logfile_printf(&pip->lh, "pidfile=%s\n", pip->pidfname) ;

	goto daemonret1 ;

badpidfile2:
	ex = EX_MUTEX ;
	logfile_printf(&pip->lh,
	    "there was a daemon already on the mutex file, PID=%d\n",
	    pip->pid) ;

	goto daemonret1 ;

/* error types of returns */
badret:
	ex = EX_DATAERR ;
	goto retearly ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int usage(pip)
struct proginfo	*pip ;
{
	int	rs = SR_OK ;
	int	wlen = 0 ;


	if (rs >= 0) {
	    rs = bprintf(pip->efp,
	        "%s: USAGE> %s [{<conf>|-} [<port>]] [-C <conf>] [-p <port>]\n",
	        pip->progname,pip->progname) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    rs = bprintf(pip->efp,"%s:  [-Q] [-D[=n]] [-V]\n",
	        pip->progname) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(pip,kop)
struct proginfo	*pip ;
KEYOPT		*kop ;
{
	struct locinfo	*lip = pip->cip ;

	KEYOPT_CUR	kcur ;

	int	rs = SR_OK ;
	int	oi ;
	int	kl, vl ;
	int	c = 0 ;

	char	*kp, *vp ;
	char	*cp ;


	if ((cp = getenv(VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs < 0)
	    goto ret0 ;

/* process program options */

	keyopt_curbegin(kop,&kcur) ;

	while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

/* get the first value for this key */

	    vl = keyopt_fetch(kop,kp,NULL,&vp) ;

/* do we support this option? */

	    if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	        uint	uv ;

		int	iw ;


	        switch (oi) {

	            case configopt_marktime:
	                if ((pip->markint <= 0) && (vp != NULL) &&
	                    (cfdecti(vp,vlen,&iw) >= 0)) {

	                    pip->markint = iw ;

	                } /* end if */

	                break ;

	            case configopt_mspoll:
	                pip->f.mspoll = TRUE ;
	                if ((pip->pollint <= 0) && (vp != NULL) &&
	                    (cfdecti(vp,vlen,&iw) >= 0)) {

	                    pip->pollint = iw ;

	                } /* end if */

	                break ;

	            case configopt_nopass:
			pip->f.nopass = TRUE ;
	                if ((vp != NULL) &&
	                    (cfdeci(vp,vlen,&iw) >= 0)) {

				pip->f.nopass = (iw > 0) ;

			}

			break ;

	            case configopt_defacc:
	                if ((cp != NULL) && (pip->defacc == NULL))
	           	rs = proginfo_setentry(pip,&pip->defacc,vp,vl) ;

	                break ;

	            case configopt_vardir:
	                if ((cp != NULL) && (pip->vardname == NULL))
	           	rs = proginfo_setentry(pip,&pip->vardname,vp,vl) ;

	                break ;

	            case configopt_spooldir:
	                if ((cp != NULL) && (pip->spooldname == NULL))
	           	rs = proginfo_setentry(pip,&pip->spooldname,vp,vl) ;

	                break ;

	            case configopt_reqfile:
	                if ((cp != NULL) && (pip->regfname == NULL))
	           	rs = proginfo_setentry(pip,&pip->regfname,vp,vl) ;

	                break ;

	            case configopt_shmfile:
	                if ((cp != NULL) && (pip->shmfname == NULL))
	           	rs = proginfo_setentry(pip,&pip->shmfname,vp,vl) ;

			break ;

	            case configopt_msfile:
	                if ((cp != NULL) && (pip->msfname == NULL))
	           	rs = proginfo_setentry(pip,&pip->msfname,vp,vl) ;

	                break ;

	            case configopt_orgcode:
	                if ((cp != NULL) && (pip->orgcode == NULL))
	           	rs = proginfo_setentry(pip,&pip->orgcode,vp,vl) ;

	                break ;

	            case configopt_speedname:
	                if ((vp != NULL) && (pip->speedname != NULL))
	                        proginfo_setentry(pip,&pip->speedname,vp,vl) ;

	                break ;

	        } /* end switch */

	        c += 1 ;

	    } /* end if (valid option) */

	} /* end while (looping through key options) */

	keyopt_curend(kop,&kcur) ;

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int progserial(pip)
struct proginfo	*pip ;
{
	int	rs1 ;
	int	fl ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	buf[MAXPATHLEN + 1] ;


	rs1 = expander(pip,SERIALFNAME1,-1,tmpfname,MAXPATHLEN) ;

	if (rs1 >= 0) {

	    fl = getfname(pip->pr,tmpfname,1,buf) ;

	    if (fl > 0)
	        strwcpy(tmpfname,buf,fl) ;

	    rs1 = getserial(tmpdname) ;

	}

	if (rs1 < 0) {

	    rs1 = expander(pip,SERIALFNAME2,-1,tmpfname,MAXPATHLEN) ;

	    if (rs1 >= 0) {

	        fl = getfname(pip->pr,tmpfname,1,buf) ;

	        if (fl > 0)
	            strwcpy(tmpfname,buf,fl) ;

	        rs1 = getserial(tmpfname) ;

	    }

	}

	pip->serial = (rs1 >= 0) ? rs1 : pip->pid ;
	return rs1 ;
}
/* end subroutine (progserial) */


static int procfile(pip,func,pr,svp,fname,elp)
struct proginfo	*pip ;
int		(*func)(char *,char *,VECSTR *) ;
char		pr[] ;
vecstr		*svp ;
char		fname[] ;
VECSTR		*elp ;
{
	int	rs ;
	int	sl ;

	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procfile: 1 fname=%s\n",fname) ;
#endif

	sl = permsched(sched2,svp,
	    tmpfname,MAXPATHLEN, fname,R_OK) ;

	if (sl < 0)
	    sl = permsched(sched3,svp,
	        tmpfname,MAXPATHLEN, fname,R_OK) ;

	if (sl > 0)
	    fname = tmpfname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procfile: 2 fname=%s\n",fname) ;
#endif

	rs = SR_NOEXIST ;
	if (sl >= 0) {

	    rs = (*func)(pr,fname,elp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procfile: procfunc() rs=%d\n",rs) ;
#endif

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procfile) */


/* check if a directory exists */
static int checkdir(pip,dname,mode)
struct proginfo	*pip ;
const char	dname[] ;
int		mode ;
{
	struct ustat	sb ;

	int	rs ;


#if	CF_DEBUGS
	debugprintf("checkdir: entered\n") ;
#endif

	if ((dname == NULL) || (dname[0] == '\0'))
	    return SR_INVALID ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("checkdir: spooldir=%s\n",
	        pip->spooldname) ;
#endif

	rs = u_stat(dname,&sb) ;

	if (rs < 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("checkdir: stat() rs=%d\n", rs) ;
	        debugprintf("checkdir: mkdirs() dname=%s\n", dname) ;
	}
#endif

	    rs = mkdirs(dname,mode) ;

	    if (rs < 0) {

	        logfile_printf(&pip->lh,
	            "could not create spool directory (%d)\n",rs) ;

	        bprintf(pip->efp,
	            "%s: could not create spool directory (%d)\n",
	            pip->progname,rs) ;

	    }

	} else if (! S_ISDIR(sb.st_mode))
	    rs = SR_NOTDIR ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("checkdir: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (checkdir) */


/* check if the directory of a file is present */
static int checkfiledir(pip,fname)
struct proginfo	*pip ;
const char	fname[] ;
{
	struct ustat	sb ;

	int	rs ;
	int	sl ;

	char	dname[MAXPATHLEN + 2] ;
	char	*dn ;


	if ((fname == NULL) || (fname[0] == '\0'))
	    return SR_INVALID ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("checkfiledir: fname=%s\n",
	        fname) ;
#endif

	sl = sfdirname(fname,-1,&dn) ;

	if (sl <= 0)
	    return SR_OK ;

	strwcpy(dname,dn,sl) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("checkfiledir: dname=%s\n",
	        dname) ;
#endif

	rs = u_stat(dname,&sb) ;

	if (rs < 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("checkfiledir: stat() rs=%d\n", rs) ;
	        debugprintf("checkfiledir: mkdirs() dname=%s\n", dname) ;
	}
#endif

	    rs = mkdirs(dname,0775) ;

	    if (rs < 0) {

	        logfile_printf(&pip->lh,
	            "could not create directory (%d)\n",rs) ;

	        bprintf(pip->efp,
	            "%s: could not create directory (%d)\n",
	            pip->progname,rs) ;

	    }

	} else if (! S_ISDIR(sb.st_mode))
	    rs = SR_NOTDIR ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("checkfiledir: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (checkfiledir) */


static int caf(pip)
struct proginfo	*pip ;
{
	int	rs1 ;
	int	i, oflags ;
	int	c = 0 ;


	for (i = 0 ; i < 3 ; i += 1) {

	    rs1 = u_close(i) ;

	    if (rs1 >= 0)
	        c += 1 ;

	}

	for (i = 0 ; i < 3 ; i += 1) {

	    oflags = (i == 0) ? O_RDONLY : O_WRONLY ;
	    u_open(NULLDEV,oflags,0666) ;

	}

	return c ;
}
/* end subroutine (caf) */


static int svarsinit(pip,svp)
struct proginfo	*pip ;
vecstr	*svp ;
{
	int	rs ;


	rs = vecstr_start(svp,6,0) ;
	pip->open.svars = (rs >= 0) ;
	if (rs < 0)
	    goto ret0 ;

	rs = vecstr_envset(svp,"p",pip->pr,-1) ;

	if (rs >= 0)
	rs = vecstr_envset(svp,"e","etc",-1) ;

	if (rs >= 0)
	rs = vecstr_envset(svp,"n",pip->searchname,-1) ;

	if (rs < 0) {
		pip->open.svars = FALSE ;
		vecstr_finish(svp) ;
	}

ret0:
	return rs ;
}
/* end subroutine (svarsinit) */


static int svarsfree(pip,svp)
struct proginfo	*pip ;
vecstr	*svp ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->open.svars && (svp != NULL)) {
		pip->open.svars = FALSE ;
		rs1 = vecstr_finish(svp) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (svarsfree) */


