/* main */

/* fairly generic (PCS) front-end */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* run-time */
#define	CF_GETEXECNAME	1		/* try to use 'getexecname(3c)'? */
#define	CF_PCSPOLL	0		/* call 'pcspoll(3pcs)' */
#define	CF_ISSAMEHOST	1		/* use 'issamehostname(3dam)' */


/* revision history:

	= 1999-05-01, David A­D­ Morano

	This code module was completely rewritten to 
	replace any original garbage that was here before.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This program just locks a mailbox file and copies it out.
	After a successful copy, the mailbox file is truncated.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<grp.h>
#include	<syslog.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<vecitem.h>
#include	<vecobj.h>
#include	<dater.h>
#include	<sbuf.h>
#include	<sockaddress.h>
#include	<mallocstuff.h>
#include	<getax.h>
#include	<userinfo.h>
#include	<pcsconf.h>
#include	<mailmsgid.h>
#include	<ctdec.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	(1000000 / 20)
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	BUFLEN		(2 * MAXHOSTNAMELEN)
#define	HOSTBUFLEN	(10 * MAXHOSTNAMELEN)

#define	PROTONAME	"udp"
#define	LOCALHOST	"localhost"

#ifndef	IPPORT_BIFFUDP
#define	IPPORT_BIFFUDP	512
#endif

#define	TO_LOCK		5

#define	O_FLAGS		(O_RDWR)


/* external subroutines */

extern int	snscs(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	headkeymat(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getfname(const char *,const char *,int,char *) ;
extern int	getserial(const char *) ;
extern int	pcsuserfile(const char *,const char *,
			const char *,const char *, char *) ;
extern int	lockfile(int,int,offset_t,offset_t,int) ;
extern int	initnow(struct timeb *,char *,int) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
#endif

extern char	*strdcpy3(char *,int,const char *,const char *,const char *) ;
extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* external variables */


/* local structures */


/* local typedefs */


/* forward references */

static int	usage(struct proginfo *) ;
static int	errormap(int) ;
static int	getprogopts(struct proginfo *,KEYOPT *,vecstr *) ;


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"ROOT",
	"TMPDIR",
	"HELP",
	"if",
	"af",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_root,
	argopt_tmpdir,
	argopt_help,
	argopt_if,
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

static const char *progopts[] = {
	"maildir",
	"deadmaildir",
	"comsat",
	"spam",
	"loglen",
	"logzones",
	"logenv",
	"divert",
	"logmsgid",
	"nospam",
	"norepeat",
	NULL
} ;

enum progopts {
	progopt_maildir,
	progopt_deadmaildir,
	progopt_comsat,
	progopt_spam,
	progopt_loglen,
	progopt_logzones,
	progopt_logenv,
	progopt_divert,
	progopt_logmsgid,
	progopt_nospam,
	progopt_norepeat,
	progopt_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct ustat	sb ;

	struct proginfo	pi, *pip = &pi ;

	struct group	ge ;

	PCSCONF		p ;

	USERINFO	u ;

	KEYOPT		akopts ;

	bfile		errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	vecstr		sets ;

	vecobj		info ;

	sigset_t	signalmask ;
	sigset_t	oldsigmask, newsigmask ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, rs1 ;
	int	i ;
	int	sl, cl, bl ;
	int	len, fl, ul ;
	int	count, c_processed = 0, c_delivered = 0 ;
	int	fd, ifd, tfd = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_version = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	pcsconfbuf[PCSCONF_LEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	logfname[MAXPATHLEN + 1] ;
	char	buf[BUFSIZE + 1], *bp ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*ifname = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*fromaddr = NULL ;
	const char	*uu_machine = NULL ;
	const char	*uu_user = NULL ;
	const char	*protospec = NULL ;
	const char	*up, *sp, *cp ;

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

	tmpfname[0] = '\0' ;
	logfname[0] = '\0' ;

/* get the current time-of-day */

	{
	    initnow(&pip->now,pip->zname,DATER_ZNAMESIZE) ;

	    dater_start(&pip->tmpdate,&pip->now,pip->zname,-1) ;

	} /* end block (getting some current time stuff) */

	pip->daytime = pip->now.time ;

	timestr_logz(pip->daytime,pip->stamp) ;

	pip->verboselevel = 1 ;

	pip->loglen = -1 ;

	pip->f.log = FALSE ;

	pip->f.optlogzones = TRUE ;
	pip->f.optlogenv = TRUE ;
	pip->f.optlogmsgid = TRUE ;
	pip->f.optdivert = TRUE ;

#if	defined(SYSV)
	pip->f.sysv_ct = TRUE ;
#else
	pip->f.sysv_ct = FALSE ;
#endif

	pip->f.sysv_rt = FALSE ;
	if (u_access("/usr/sbin",R_OK) >= 0)
	    pip->f.sysv_rt = TRUE ;

/* process program arguments */

	rs = keyopt_start(&akopts) ;
	if (rs < 0)
	    goto ret3 ;

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
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                            }
	                        }

	                        break ;

/* default action and user specified help */
	                    default:
	                        rs = SR_INVALID ;
	                        bprintf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                    } /* end switch (key words) */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl)
	                                    rs = cfdeci(avp,avl, 
	                                        &pip->debuglevel) ;

	                            }

	                            break ;

/* ignore the "deliver" flag for old compatibility reasons */
	                        case 'd':
	                            break ;

/* from email address */
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

/* multirecip-mode */
	                        case 'm':
	                            pip->f.multirecip = TRUE ;
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

/* caller-supplied protocol specification */
	                        case 'p':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                protospec = argp ;

	                            break ;

/* quiet */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* mail spool directory */
	                        case 's':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
	                                break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pip->maildname = argp ;

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
	                                rs = cfdeci(argp,argl,&pip->timeout) ;

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

	            } /* end if (digit or not) */

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

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bcontrol(pip->efp,BC_LINEBUF,0) ;

	    bflush(pip->efp) ;

	} /* end if */

	if (f_version)
	    bprintf(pip->efp,"%s: version %s/%s\n",
	        pip->progname,
	        VERSION,(pip->f.sysv_ct ? "SYSV" : "BSD")) ;

	if (f_usage)
	    usage(pip) ;

/* get some program information */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: final pr=%s\n", pip->pr) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

/* get help if requested */

	if (f_help)
	    rs = printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* initialization */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv(VARTMPDNAME) ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;


/* validate the timeout */

	if (pip->timeout <= 1)
	    pip->timeout = DEFTIMEOUT ;

/* some UUCP stuff */

	uu_machine = getenv("UU_MACHINE") ;

	uu_user = getenv("UU_USER") ;

/* get the group information that we need */

	pip->gid_mail = (gid_t) MAILGID ;
	if (getgr_name(&ge,buf,BUFLEN,MAILGROUP) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: username mail gid=%d\n",ge.gr_gid) ;
#endif

	    pip->gid_mail = ge.gr_gid ;

	}

/* get user profile information */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: userinfo()\n",rs) ;
#endif

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;
	if (rs < 0) {
	    ex = EX_NOUSER ;
	    goto retearly ;
	}

	pip->uip = &u ;
	pip->pid = u.pid ;
	pip->username = u.username ;
	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->logid = u.logid ;

	pip->uid = u.uid ;
	pip->euid = u.euid ;
	pip->f.setuid = (pip->uid != pip->euid) ;

	pip->gid = u.gid ;
	pip->egid = u.egid ;
	pip->f.setgid = (pip->gid != pip->egid) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: rs=%d user=%s homedir=%s euid=%d egid=%d\n",
	        rs,u.username,u.homedname,u.euid,u.egid) ;
#endif

/* get the system PCS configuration information */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pcsconf()\n") ;
#endif

	rs = vecstr_start(&sets,20,0) ;
	if (rs < 0)
	    goto badinit1 ;

	rs1 = pcsconf(pip->pr,NULL,&p,&sets,NULL,
	    pcsconfbuf,PCSCONF_LEN) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pcsconf() rs=%d\n",rs) ;
#endif

#if	CF_PCSPOLL
	if (rs1 >= 0) {

	    pip->pp = &p ;
	    pcspoll(pip->pr,&p,&sets) ;

	}
#endif /* CF_PCSPOLL */

/* process any program options */

	rs = getprogopts(pip,&akopts,&sets) ;
	if (rs < 0)
	    goto badinit2 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: getprogopts() rs=%d\n",rs) ;
#endif

/* the "from" (envelope) address */

	if (fromaddr == NULL) {

	    if (pip->f.trusted && (uu_user != NULL))
	        fromaddr = uu_user ;

	    else
	        fromaddr = u.username ;

	}

	pip->fromaddr = mallocstr(fromaddr) ;

/* the protocol specification */

	if (protospec == NULL) {

	    if (uu_machine != NULL) {

	        protospec = buf ;
	        snscs(buf,BUFLEN,"uucp",uu_machine) ;

	    }

	}

	pip->protospec = mallocstr(protospec) ;

/* log ID */

	sp = SERIALFNAME ;
	if (sp[0] != '/') {

	    mkpath2(tmpfname,pip->pr,sp) ;

	    sp = tmpfname ;

	}

	rs = getserial(tmpfname) ;

	if (rs >= 0) {

	    sp = buf ;
	    bp = strwcpy(buf,pip->nodename,4) ;

	    cl = ctdeci(bp,20,rs) ;

	    bp[cl] = '\0' ;
	    buf[15] = '\0' ;

	} else
	    sp = u.logid ;

	pip->logid = mallocstr(sp) ;


/* log file */

	if (logfname[0] == '\0')
	    mkpath2(logfname,pip->pr,LOGFNAME) ;

	if (logfname[0] != '/') {

	    fl = getfname(pip->pr,logfname,1,tmpfname) ;

	    if (fl > 0)
	        strcpy(logfname,tmpfname) ;

	}

	rs1 = logfile_open(&pip->lh,logfname,0,0666,pip->logid) ;
	buf[0] = '\0' ;
	if (rs1 >= 0) {

	    pip->f.log = TRUE ;
	    if (pip->loglen > 0)
	        logfile_checksize(&pip->lh,pip->loglen) ;

	    logfile_printf(&pip->lh,"%s %-14s %s/%s",
	        timestr_logz(pip->now.time,timebuf),
	        pip->progname,
	        VERSION,(pip->f.sysv_ct ? "SYSV" : "BSD")) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s)",
	        (pip->f.sysv_rt ? "SYSV" : "BSD"),
	        u.sysname,u.release) ;

	    buf[0] = '\0' ;
	    if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        strcpy(buf,u.fullname) ;

	    else if ((u.name != NULL) && (u.name[0] != '\0'))
	        strcpy(buf,u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        strcpy(buf,u.gecosname) ;

	    else if (u.mailname != NULL)
	        strcpy(buf,u.mailname) ;

	    if (buf[0] != '\0')
	        logfile_printf(&pip->lh,"%s!%s (%s)",
	            u.nodename,u.username,buf) ;

	    else
	        logfile_printf(&pip->lh,"%s!%s",
	            u.nodename,u.username) ;

	    if (protospec != NULL)
	        logfile_printf(&pip->lh,"proto=%s\n",protospec) ;

	    if (uu_machine != NULL)
	        logfile_printf(&pip->lh,"uu_machine=%s\n",uu_machine) ;

	    if (uu_user != NULL)
	        logfile_printf(&pip->lh,"uu_user=%s\n",uu_user) ;

	} /* end if */

/* write user's mail address (roughly as we have it) into the user list file */

	rs1 = pcsuserfile(pip->pr,USERFNAME,u.nodename,u.username,buf) ;

	if (pip->f.log) {

	    if (rs1 == 1)
	        logfile_printf(&pip->lh,
	            "created the user list file") ;

	    else if (rs1 < 0)
	        logfile_printf(&pip->lh,
	            "could not access user list file (%d)", rs1) ;

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: wrote user file, rs=%d\n",rs1) ;
#endif

/* try to open an environment summary log also */

	if (pip->f.optlogenv) {

	    logfname[0] = '\0' ;

	    if (logfname[0] == '\0')
	        mkpath2(logfname,pip->pr,LOGENVFNAME) ;

	    if (logfname[0] != '/') {

	        fl = getfname(pip->pr,logfname,1,tmpfname) ;

	        if (fl > 0)
	            strcpy(logfname,tmpfname) ;

	    }

	    rs = logfile_open(&pip->envsum,logfname,0,0666,pip->logid) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: logfile_open() rs=%d\n",rs) ;
#endif

	    if (rs >= 0)
	        pip->f.logenv = TRUE ;

	} /* end if (option to log envelope information) */

/* make the maillock address */

#ifdef	COMMENT
	bp = strwcpy(buf,pip->nodename,(BUFLEN - 2)) ;

	*bp++ = '!' ;
	bp = strwcpy(bp,pip->username,(BUFLEN - (bp - buf))) ;

	bl = bp - buf ;

#else /* COMMENT */

	bl = sncpy3(buf,BUFLEN,pip->nodename,"!",pip->username) ;

#endif /* COMMENT */

	pip->lockaddr = mallocstrw(buf,bl) ;

/* get some startup flags if there are any */

#ifdef	COMMENT

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking for options\n") ;
#endif

	getbbopts(&g,&sets) ;

#endif /* COMMENT */

	vecstr_finish(&sets) ;

/* other */


/* some file initialization */


/* process the input message */

	ifd = FD_STDIN ;
	if ((ifname != NULL) && (ifname[0] != '-')) {

	    rs = uc_open(ifname,O_FLAGS,0666) ;
	    ifd = rs ;
	    if (rs < 0) {
		ex = EX_NOINPUT ;
	        goto badinopen ;
	    }

	}

	rs = lockfile(ifd,F_LOCK,0L,0L,TO_LOCK) ;
	if (rs < 0) {
	    ex = EX_TEMPFAIL ;
	    goto badinlock ;
	}

/* output */

	tfd = FD_STDOUT ;
	if ((ofname != NULL) && (ofname[0] != '-')) {

	    rs = uc_open(ofname,O_WRONLY,0666) ;
	    tfd = rs ;
	    if (rs < 0) {
		ex = EX_CANTCREAT ;
	        goto badtmpopen ;
	    }

	}

/* do the copy */

	rs = uc_copy(ifd,tfd,-1) ;

	if (rs >= 0) {
	    rs = uc_fsync(tfd) ;
	    if (rs >= 0)
	        rs = uc_ftruncate(ifd,0L) ;

	}

	if (tfd >= 0)
	    u_close(tfd) ;

badtmpopen:
badinlock:
	u_close(ifd) ;

/* print a regular error message to STDERR */
badinopen:
done:
	if (rs < 0) {

	    switch (rs) {

	    case SR_NOENT:
	        ex = EX_NOUSER ;
	        if (! pip->f.quiet)
	            bprintf(pip->efp,
	                "%s: recipient not found\n",
	                pip->progname) ;

	        break ;

	    case SR_TXTBSY:
	        ex = EX_TEMPFAIL ;
	        if (! pip->f.quiet)
	            bprintf(pip->efp,
	                "%s: could not capture the mail lock\n",
	                pip->progname) ;

	        break ;

	    case SR_ACCES:
	        ex = EX_ACCESS ;
	        if (! pip->f.quiet)
	            bprintf(pip->efp,
	                "%s: could not access the mail spool-file\n",
	                pip->progname) ;

	        break ;

	    case SR_REMOTE:
	        ex = EX_FORWARDED ;
	        if (! pip->f.quiet)
	            bprintf(pip->efp,"%s: mail is being forwarded\n",
	                pip->progname) ;

	        break ;

	    case SR_NOSPC:
	        ex = EX_NOSPACE ;
	        logfile_printf(&pip->lh,
	            "file-system is out of space\n") ;

	        if (! pip->f.quiet)
	            bprintf(pip->efp,
	                "%s: local file-system is out of space\n",
	                pip->progname) ;

	        break ;

	    default:
	        ex = EX_UNKNOWN ;
	        if (! pip->f.quiet)
	            bprintf(pip->efp,"%s: unknown bad thing (%d)\n",
	                pip->progname,rs) ;

	        break ;

	    } /* end switch */

	} else
	    ex = EX_OK ;

	if (pip->f.log)
	    logfile_printf(&pip->lh,
	        "recipients processed=%u delivered=%u\n",
	        c_processed,c_delivered) ;

/* good return from program */
badinit2:
badinit1:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%d (%d)\n",
	        pip->progname,ex,rs) ;

	if (pip->f.logenv)
	    logfile_close(&pip->envsum) ;

	if (pip->f.log)
	    logfile_close(&pip->lh) ;

retearly:
ret4:
	if (pip->open.akopts)
	    keyopt_finish(&akopts) ;

ret3:
	dater_finish(&pip->tmpdate) ;

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

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs = SR_OK ;
	int	wlen  = 0 ;

	rs = bprintf(pip->efp,
	    "%s: USAGE> %s -if <mailfile> [-v]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutines (usage) */


/* process program options */
static int getprogopts(pip,kop,setsp)
struct proginfo	*pip ;
KEYOPT		*kop ;
vecstr		*setsp ;
{
	KEYOPT_CUR	kcur ;

	int	rs ;
	int	i, oi, val, cl ;
	int	nlen, klen, vlen ;

	char	*kp, *vp ;
	char	*cp ;


	nlen = strlen(pip->searchname) ;

/* load up the environment options */

	if ((cp = getenv(VAROPTS)) != NULL)
	    keyopt_loads(kop,cp,-1) ;

/* system-wide options? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("getprogopts: scanning system options\n") ;
#endif

	for (i = 0 ; vecstr_get(setsp,i,&cp) >= 0 ; i += 1) {

	    char	*cp2 ;


	    if (cp == NULL)
	        continue ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("getprogopts: conf >%s<\n",cp) ;
#endif

	    if (! headkeymat(pip->searchname,cp,-1))
	        continue ;

/* we have one of ours, separate the keyname from the value */

	    cp += (nlen + 1) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("getprogopts: key=%s\n",cp) ;
#endif

#ifdef	COMMENT
	    keyopt_loads(kop,cp,-1) ;
#else
	    kp = cp ;
	    vp = NULL ;
	    vlen = 0 ;
	    if ((cp = strchr(cp,'=')) != NULL) {

	        vp = cp + 1 ;
	        vlen = -1 ;
	    }

	    keyopt_loadvalue(kop,kp,vp,vlen) ;

#endif /* COMMENT */

	} /* end for (system options) */


/* process all of the options that we have so far */

	keyopt_curbegin(kop,&kcur) ;

	while ((rs = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	    KEYOPT_CUR	vcur ;

	    int	f_value = FALSE ;


	    klen = rs ;

/* get the first (non-zero length) value for this key */

	    vlen = -1 ;
	    keyopt_curbegin(kop,&vcur) ;

/* use only the first of any option with the same key */

	    while ((rs = keyopt_enumvalues(kop,kp,&vcur,&vp)) >= 0) {

	        f_value = TRUE ;
	        vlen = rs ;
	        if (vlen > 0)
	            break ;

	    } /* end while */

	    keyopt_curend(kop,&vcur) ;

/* do we support this option? */

	    if ((oi = matostr(progopts,2,kp,klen)) >= 0) {

	        switch (oi) {

	        case progopt_maildir:
	            if ((vlen > 0) && (pip->maildname == NULL))
	                pip->maildname = mallocstrw(vp,vlen) ;

	            break ;

	        case progopt_deadmaildir:
	            if ((vlen > 0) && (pip->deadmaildname == NULL))
	                pip->deadmaildname = mallocstrw(vp,vlen) ;

	            break ;

	        case progopt_comsat:
	            cp = "+" ;
	            cl = 1 ;
	            if (vlen > 0) {
			const int	ch = MKCHAR(vp[0]) ;
	                if (isdigitlatin(ch)) {
	                    if (cfdeci(vp,vlen,&val) >= 0)
	                        cp = (val > 0) ? "+" : "-" ;
	                } else {
	                    cp = vp ;
	                    cl = vlen ;
	                }
	            }

	            if (pip->comsatfname == NULL)
	                pip->comsatfname = mallocstrw(cp,cl) ;

	            break ;

	        case progopt_spam:
	            if ((vlen > 0) && (pip->comsatfname == NULL))
	                pip->spamfname = mallocstrw(vp,vlen) ;

	            break ;

	        case progopt_loglen:
	            if ((vlen > 0) && (cfdeci(vp,vlen,&val) >= 0))
	                pip->loglen = val ;

	            break ;

	        case progopt_logzones:
	            if ((vlen > 0) && (cfdeci(vp,vlen,&val) >= 0))
	                pip->f.optlogzones = (val > 0) ;

	            break ;

	        case progopt_logenv:
	            if ((vlen > 0) && (cfdeci(vp,vlen,&val) >= 0))
	                pip->f.optlogenv = (val > 0) ;

	            break ;

	        case progopt_divert:
	            if ((vlen > 0) && (cfdeci(vp,vlen,&val) >= 0))
	                pip->f.optdivert = (val > 0) ;

	            break ;

	        case progopt_logmsgid:
	            if ((vlen > 0) && (cfdeci(vp,vlen,&val) >= 0))
	                pip->f.optlogmsgid = (val > 0) ;

	            break ;

	        case progopt_nospam:
	            if ((vlen > 0) && (cfdeci(vp,vlen,&val) >= 0))
	                pip->f.optnospam = (val > 0) ;

	            break ;

	        case progopt_norepeat:
	            if ((vlen > 0) && (cfdeci(vp,vlen,&val) >= 0))
	                pip->f.optnorepeat = (val > 0) ;

	            break ;

	        } /* end switch */

	    } /* end if (got a match) */

	} /* end while */

	keyopt_curend(kop,&kcur) ;

	return OK ;
}
/* end subroutine (getprogopts) */



