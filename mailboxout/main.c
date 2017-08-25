/* main */

/* fairly generic (PCS) front-end */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* run-time */
#define	CF_GETEXECNAME	1		/* try to use 'getexecname(3c)' ? */
#define	CF_PCSPOLL	0		/* call 'pcspoll(3pcs)' */
#define	CF_MAILDIRS	1		/* use built-in maildirs */
#define	CF_ISSAMEHOST	1		/* use 'issamehostname(3dam)' */


/* revision history:

	= 1998-05-01, David A­D­ Morano
        This code module was completely rewritten to replace any original
        garbage that was here before.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This program just locks a mailbox file and copies it out. After a
        successful copy, the mailbox file is truncated.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<sysexits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<grp.h>
#include	<syslog.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<vecitem.h>
#include	<vecobj.h>
#include	<dater.h>
#include	<sbuf.h>
#include	<sockaddress.h>
#include	<exitcodes.h>
#include	<mallocstuff.h>
#include	<getax.h>
#include	<userinfo.h>
#include	<pcsconf.h>
#include	<mailmsgid.h>
#include	<ctdec.h>
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
extern int	getuid_name(cchar *,int) ;
extern int	getourhe(cchar *,char *,struct hostent *,char *,int) ;
extern int	pcsuserfile() ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	vecstr_loadfile(vecstr *,int,const char *) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	process(struct proginfo *,bfile *,bfile *,vecobj *) ;
extern int	deliver(struct proginfo *,int,struct recip *) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strdcpy3(char *,int,const char *,const char *,const char *) ;
extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* external variables */


/* local structures */

struct errormap {
	int	rs, ex ;
} ;


/* local typedefs */

#if	defined(IRIX) && (! defined(TYPEDEF_INADDRT))
#define	TYPEDEF_INADDRT	1

typedef unsigned int	in_addr_t ;

#endif


/* forward references */

static int	errormap(int) ;
static int	getprogopts(struct proginfo *,KEYOPT *,vecstr *) ;

static int	comsat(struct proginfo *,vecobj *,vecobj *) ;

static int	mkcsmsg(char *,int,const char *,int,uint) ;
static int	parsenodespec(struct proginfo *,int,char *,int,
			char *,const char *) ;

static int	vecobj_addrecip(vecobj *,const char *) ;
static int	vecobj_finishrecips(vecobj *) ;

static int	recip_init(struct recip *,const char *) ;
static int	recip_mo(struct recip *,uint,uint) ;
static int	recip_free(struct recip *) ;


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

static const char	*maildirs[] = {
	"/var/mail",
	"/var/spool/mail",
	"/usr/mail",
	"/usr/spool/mail",
	NULL
} ;

static const char	*trustedusers[] = {
	"root",
	"uucp",
	"nuucp",
	"adm",
	"admin",
	"daemon",
	"listen",
	"pcs",
	"post",
	"smtp",
	"dam",
	"morano",
	NULL
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

static const struct errormap	errormaps[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_ACCESS },
	{ SR_REMOTE, EX_FORWARDED },
	{ SR_NOSPC, EX_NOSPACE },
	{ 0, 0 }
} ;

static const char	*entries[] = {
	":saved",
	"root",
	"adm",
	"uucp",
	"staff",
	"pcs",
	NULL
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct ustat	sb ;
	struct pcsconf	p ;
	struct proginfo	pi, *pip = &pi ;
	struct group	ge ;
	USERINFO	u ;
	KEYOPT		akopts ;

	bfile		errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		tmpfile, *tfp = &tmpfile ;
	vecstr		sets ;
	vecobj		info ;
	vecobj		recips ;
	sigset_t	signalmask ;
	sigset_t	oldsigmask, newsigmask ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	argnum, i ;
	int	rs, rs1 ;
	int	len, fl ;
	int	ex = EX_INFO ;
	int	fd, tfd = -1 ;
	int	fd_debug ;
	int	count, c_processed = 0, c_delivered = 0 ;
	int	sl, cl, bl ;
	int	ul ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_done = FALSE ;
	int	f_version = FALSE ;
	int	f_bad ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*pr = NULL ;
	const char	*ifname = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*fromaddr = NULL ;
	const char	*uu_machine = NULL ;
	const char	*uu_user = NULL ;
	const char	*protospec = NULL ;
	const char	*up, *sp, *cp ;
	char	argpresent[MAXARGGROUPS + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	pcsconfbuf[PCSCONF_LEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	logfname[MAXPATHLEN + 1] ;
	char	buf[BUFSIZE + 1], *bp ;
	char	timebuf[TIMEBUFLEN + 1] ;

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

#if	CF_DEBUGS
	debugprintf("main: BIO stderr opened, rs=%d\n",rs) ;
#endif

	tmpfname[0] = '\0' ;
	logfname[0] = '\0' ;

	initnow(&pip->now,pip->zname,DATER_ZNAMESIZE) ;

	dater_start(&pip->tmpdate,&pip->now,pip->zname,-1) ;


	pip->daytime = pip->now.time ;

	timestr_logz(pip->daytime,pip->stamp) ;

	pip->verboselevel = 1 ;

	pip->loglen = -1 ;

	pip->f.optlogzones = TRUE ;
	pip->f.optlogenv = TRUE ;
	pip->f.optlogmsgid = TRUE ;
	pip->f.optdivert = TRUE ;

/* key options */

	rs = keyopt_start(&akopts) ;

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (! f_done) && (argr > 0)) {

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
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                        }

	                        break ;

/* default action and user specified help */
	                    default:
	                        rs = SR_INVALID ;
	                        f_usage = TRUE ;
	                        f_done = TRUE ;
				bprintf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

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
	                                    rs = cfdeci(avp,avl, 
	                                        &pip->debuglevel) ;

	                            }

	                            break ;

/* quiet */
	                        case 'Q':
	                            pip->f.quiet = TRUE ;
	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
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
	                            f_usage = TRUE ;
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

#if	CF_DEBUGS
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
	    goto usage ;

	if (f_version)
	    goto done ;

/* get some program information */

	if (pr == NULL) {

	    pr = getenv(VARPROGRAMROOT1) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT2) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT3) ;

/* try to see if a path was given at invocation */

	    if ((pr == NULL) && (pip->progdname != NULL))
	        proginfo_rootprogdname(pip) ;

/* do the special thing */

#if	CF_GETEXECNAME && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	    if ((pr == NULL) && (pip->pr == NULL)) {

	        const char	*pp ;


	        pp = getexecname() ;

	        if (pp != NULL)
	            proginfo_execname(pip,pp) ;

	    }
#endif /* SOLARIS */

	} /* end if (getting a program root) */

	if (pip->pr == NULL) {

	    if (pr == NULL)
	        pr = PROGRAMROOT ;

	    proginfo_setprogroot(pip,pr,-1) ;

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

	if (f_help) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: printhelp() helpfname=%s\n",HELPFNAME) ;
#endif

	    rs = printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: printhelp() rs=%d\n",rs) ;
#endif

	    goto help ;
	}


/* initialization */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv("TMPDIR") ;

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

	if (rs < 0)
	    goto baduser ;

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

	rs = vecstr_start(&sets,0,0) ;

	if (rs < 0)
	    goto badinit1 ;

	rs = pcsconf(pip->pr,NULL,&p,&sets,NULL,
	    pcsconfbuf,PCSCONF_LEN) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pcsconf() rs=%d\n",rs) ;
#endif

#if	CF_PCSPOLL
	if (rs >= 0) {

	    pip->pp = &p ;
	    (void) pcspoll(pip->pr,&p,&sets) ;

	}
#endif /* CF_PCSPOLL */


/* process any program options */

	rs = getprogopts(pip,&akopts,&sets) ;

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

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: logfile_open() \n") ;
	    debugprintf("main: logfname=%s logid=>%s<\n",
	        logfname,u.logid) ;
	}
#endif

	rs = logfile_open(&pip->lh,logfname,0,0666,pip->logid) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: logfile_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    struct utsname	un ;


	    pip->open.logfile = TRUE ;
	    if (pip->loglen > 0)
	        logfile_checksize(&pip->lh,pip->loglen) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: logfile_printf()\n") ;
#endif

	    logfile_printf(&pip->lh,"%s %-14s %s/%s",
	        timestr_logz(pip->now.time,timebuf),
	        pip->progname,
	        VERSION,(pip->f.sysv_ct ? "SYSV" : "BSD")) ;

	    (void) u_uname(&un) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s)",
	        (pip->f.sysv_rt ? "SYSV" : "BSD"),
	        un.sysname,un.release) ;

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

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: adding entry to user list file\n") ;
#endif

	rs = pcsuserfile(pip->pr,USERFNAME,u.nodename,u.username,buf) ;

	if (pip->open.logfile) {

	    if (rs == 1)
	        logfile_printf(&pip->lh,
	            "created the user list file") ;

	    else if (rs < 0)
	        logfile_printf(&pip->lh,
	            "could not access user list file (%d)",
	            rs) ;

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: wrote user file, rs=%d\n",rs) ;
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
	        pip->open.logfileenv = TRUE ;

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

	(void) getbbopts(&g,&sets) ;

#endif /* COMMENT */


	vecstr_finish(&sets) ;


/* we are delivering mail, but are we a trusted user ? */

	i = matstr(trustedusers,u.username,-1) ;

	pip->f.trusted = (i >= 0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: f_trusted=%d\n",pip->f.trusted) ;
#endif


/* find the mail spool directory, as dynamically as possible */

	pip->uid_divert = pip->uid ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: maildname=%s\n",pip->maildname) ;
#endif

#if	CF_MAILDIRS

	if ((pip->maildname == NULL) || (pip->maildname[0] == '\0')) {

	    vecstr	dirs ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: searching for maildir\n") ;
#endif

	    vecstr_start(&dirs,10,0) ;

	    if ((pip->maildname == NULL) &&
	        ((sp = getenv("MAIL")) != NULL)) {

	        cl = sfdirname(sp,-1,&cp) ;

	        if (mailspooldir(pip,cp,cl) >= 0)
	            pip->maildname = mallocstrw(cp,cl) ;

	        else
	            vecstr_add(&dirs,cp,cl) ;

	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: 1 maildname=%s\n",pip->maildname) ;
#endif

	    if (pip->maildname == NULL) {

	        cp = SPOOLDNAME ;
	        cl = -1 ;
	        if (vecstr_findn(&dirs,cp,cl) < 0) {

	            if (mailspooldir(pip,cp,cl) >= 0)
	                pip->maildname = cp ;

	            else
	                vecstr_add(&dirs,cp,cl) ;

	        }

	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: 2 maildname=%s\n",pip->maildname) ;
#endif

	    if (pip->maildname == NULL) {

	        for (i = 0 ; maildirs[i] != NULL ; i += 1) {

	            cp = (char *) maildirs[i] ;
	            cl = -1 ;
	            if (vecstr_findn(&dirs,cp,cl) < 0) {

	                if (mailspooldir(pip,cp,cl) >= 0) {

	                    pip->maildname = cp ;
	                    break ;

	                } else
	                    vecstr_add(&dirs,cp,cl) ;

	            }

	        } /* end for */

	    }

	    vecstr_finish(&dirs) ;

	} /* end if (was not given a mail directory) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: mailspooldir() rs=%d maildname=%s\n",
	        rs,pip->maildname) ;
#endif

#endif /* CF_MAILDIRS */


	rs = SR_NOTFOUND ;
	if (pip->maildname != NULL) {

/* pop something inside the directory because of automounting */

	    for (i = 0 ; entries[i] != NULL ; i += 1) {

	        mkpath2(tmpfname,pip->maildname,entries[i]) ;

	        rs = u_stat(tmpfname,&sb) ;

	        if (rs >= 0)
	            break ;

	    } /* end for */

	    rs = perm(pip->maildname,-1,-1,NULL,W_OK) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: perm() rs=%d \n",rs) ;
#endif

	} /* end if */

	if (rs < 0) {

	    int	f ;


	    logfile_printf(&pip->lh, 
	        "maildir=%s unavailable (%d)\n",
	        pip->maildname,rs) ;

	    if (pip->f.trusted) {

	        openlog(SEARCHNAME,0,LOG_MAIL) ;

	        syslog(LOG_ERR,"maildir=%s unavailable (%d)",
	            pip->maildname,rs) ;

	        closelog() ;

	    } /* end if (logging the problem) */

	    pip->maildname = NULL ;
	    f = pip->f.optdivert && (pip->deadmaildname != NULL) ;

	    if (f)
	        rs = perm(pip->deadmaildname,-1,-1,NULL,W_OK) ;

	    if (f && (rs >= 0)) {
		cchar	*un = DIVERTUSER ;
	        pip->f.diverting = TRUE ;
	        pip->maildname = pip->deadmaildname ;

	        logfile_printf(&pip->lh, 
	            "diverting maildir=%s\n",
	            pip->maildname,rs) ;

		if ((rs = getuid_name(un,-1)) >= 0) {
	            pip->uid_divert = rs ;
		} else if (isNotPresent(rs)) {
	            pip->uid_divert = getuid() ;
		    rs = SR_OK ;
		}
	    }

	} /* end if (mail directory was not available) */


/* get and save some information on the mail spool directory */

	if ((pip->maildname != NULL) && (rs >= 0))
	    rs = u_stat(pip->maildname,&sb) ;

	if (rs < 0)
	    goto badspooldir ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: maildir uid=%d gid=%d\n",
	        sb.st_uid, sb.st_gid) ;
#endif

	pip->uid_maildir = sb.st_uid ;
	pip->gid_maildir = sb.st_gid ;


/* other */


/* process the usernames given at invocation or in the argfile */

	rs = vecobj_start(&recips,sizeof(struct recip),10,0) ;

	if (rs < 0)
	    goto badinit2 ;

#if	CF_DEBUGS
	rs = vecobj_audit(&recips) ;

	debugprintf("main: 1 vecobj_audit() rs=%d\n",rs) ;

	if (rs < 0)
	    goto badinit2 ;

#endif

	count = 0 ;
	f_bad = FALSE ;
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) ||
	            (argv[i][0] == '-') || (argv[i][0] == '+'))
	            continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: arg[%d]=%s\n",i,argv[i]) ;
#endif

	        count += 1 ;

	        rs = vecobj_addrecip(&recips,argv[i]) ;

	        if (rs < 0)
	            f_bad = TRUE ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: recipient addition rs=%d\n",rs) ;
#endif

	    } /* end for (outer) */

	} /* end if (invocation arguments) */

#if	CF_DEBUGS
	rs = vecobj_audit(&recips) ;

	debugprintf("main: 2 vecobj_audit() rs=%d\n",rs) ;

#endif

	if ((afname != NULL) && (afname[0] != '\0')) {
	    bfile	afile ;
	    char	linebuf[LINEBUFLEN + 1] ;

	    if (afname[0] != '-') {
	        rs = bopen(&afile,afname,"r",0666) ;
	    } else
	        rs = bopen(&afile,BFILE_STDIN,"dr",0666) ;

	    if (rs >= 0) {

	        while ((rs = breadline(&afile,linebuf,LINEBUFLEN)) > 0) {
		    len = rs ;

		    if (linebuf[len - 1] == '\n') len -= 1 ;

	            cl = sfshrink(linebuf,len,&cp) ;

	            if ((cl <= 0) && (cp[0] == '#')) continue ;

	            count += 1 ;
	            cp[cl] = '\0' ;

	            rs = vecobj_addrecip(&recips,cp) ;

	            if (rs < 0)
	                f_bad = TRUE ;

	        } /* end while */

	        bclose(&afile) ;
	    } /* end if */

	} /* end if (argument file) */

	if (f_bad) {
	    rs = SR_NOMEM ;
	    goto badinit3 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: recip count=%d\n",count) ;
#endif


/* some file initialization */


/* do we have a COMSAT file ? */

	if ((pip->comsatfname == NULL) || (pip->comsatfname[0] == '+')) {
	    char	csfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: setting default COMSAT filename\n") ;
#endif

	    rs1 = expander(pip,COMSATFNAME,-1,csfname,(MAXPATHLEN - 1)) ;

	    if (csfname[0] != '/') {

	        cl = mkpath2(tmpfname,pip->pr,csfname) ;

	        if (cl > 0)
	            strwcpy(csfname,tmpfname,cl) ;

	    }

	    if (rs1 >= 0)
	        rs1 = perm(csfname,-1,-1,NULL,R_OK) ;

	    if (rs1 >= 0)
	        pip->comsatfname = mallocstrw(csfname,cl) ;

	} else if (pip->comsatfname[0] != '-') {
	    char	csfname[MAXPATHLEN + 1] ;

	    rs1 = expander(pip,pip->comsatfname,-1,csfname,(MAXPATHLEN - 1)) ;

	    if (csfname[0] != '/') {

	        cl = mkpath2(tmpfname,pip->pr,csfname) ;

	        if (cl > 0)
	            strwcpy(csfname,tmpfname,cl) ;

	    }

	    if (rs1 >= 0)
	        rs1 = perm(csfname,-1,-1,NULL,R_OK) ;

	    if (rs1 >= 0) {

	        free(pip->comsatfname) ;

	        pip->comsatfname = mallocstrw(csfname,cl) ;

	    }
	}

	pip->f.comsat = 
	    (pip->comsatfname != NULL) && (pip->comsatfname[0] != '-') ;

/* do we have a spam filter file ? */

	if ((pip->spamfname == NULL) && pip->f.comsat) {
	    char	spamfname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: setting default SPAM filename\n") ;
#endif

	    expander(pip,SPAMFNAME,-1,spamfname,MAXPATHLEN) ;

	    if (spamfname[0] != '/') {

	        cl = mkpath2(tmpfname,pip->pr,spamfname) ;

	        if (cl > 0)
	            strwcpy(spamfname,tmpfname,cl) ;

	    }

	    rs1 = perm(spamfname,-1,-1,NULL,R_OK) ;

	    if (rs1 >= 0)
	        pip->spamfname = mallocstrw(spamfname,cl) ;

	} /* end if (default spam file) */

	pip->f.spam = pip->f.comsat &&
	    (pip->spamfname != NULL) && (pip->spamfname[0] != '-') ;


/* process the input message */

	if ((ifname != NULL) && (ifname[0] != '-')) {
	    rs = bopen(ifp,ifname,"r",0666) ;
	} else
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	if (rs < 0)
	    goto badinopen ;

/* create a temporary file to hold the processed input */

	mkpath2(logfname,pip->tmpdname,"dmailXXXXXXXXX") ;

	rs = mktmpfile(tmpfname,0666,logfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: mktmpfile() rs=%d tmpfname=%s\n",rs,tmpfname) ;
#endif

	if (rs < 0)
	    goto badtmpfile ;

	rs = bopen(tfp,tmpfname,"rwct",0600) ;

	u_unlink(tmpfname) ;

	if (rs < 0)
	    goto badtmpopen ;

/* initialze the container for message information */

	vecobj_start(&info,sizeof(struct msgoff),10,0) ;

/* process the message on the input */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: process()\n") ;
#endif

	rs = process(pip,ifp,tfp,&info) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: process() rs=%d\n",rs) ;
#endif

	if (rs < 0) {
	    bclose(tfp) ;
	    goto ret5 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    bopen(ofp,BFILE_STDOUT,"wct",0666) ;
	    bseek(tfp,0L,SEEK_SET) ;
	    bcopyblock(tfp,ofp,-1) ;
	    bclose(ofp) ;
	}
#endif /* CF_DEBUG */


/* convert the BIO file pointer to a file descriptor */

	rs = bcontrol(tfp,BC_FD,&fd) ;

	if (rs < 0)
	    goto ret5 ;

	rs = u_dup(fd) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: u_dup() fd=%d rs=%d\n",fd,rs) ;
#endif /* CF_DEBUG */

	tfd = rs ;
	bclose(tfp) ;

	if (rs < 0)
	    goto ret5 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    rs = u_fstat(tfd,&sb) ;
	    debugprintf("main: tfd=%d rs=%d size=%lu\n",
	        tfd,rs,sb.st_size) ;
	}
#endif /* CF_DEBUG */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    struct msgoff	*mop ;
	    for (i = 0 ; vecobj_get(&info,i,&mop) >= 0 ; i += 1) {
	        if (mop == NULL) continue ;
	        debugprintf("main: msg=%u off=%u mlen=%u\n",
	            i,mop->offset,mop->mlen) ;
	    } /* end for */
	}
#endif /* CF_DEBUG */

/* deliver the message to the recipients */

	{
	    struct recip	*rp ;
	    struct msgoff	*mop ;
	    struct md		mo ;
	    MSGID	mids ;
	    MSGID_KEY	midkey ;
	    MSGID_ENT	mid ;
	    int	mn, c ;
	    int	f_mid ;
	    char	*fmt ;

	    if (pip->f.optlogmsgid) {

	        mkpath2(tmpfname,pip->pr,MSGIDDBNAME) ;

	        rs1 = msgid_open(&mids,tmpfname,O_RDWR,0660,
			MAXMSGID) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: 1 msgid_open() rs=%d\n",rs1) ;
#endif

	        if (rs1 < 0)
	            rs1 = msgid_open(&mids,tmpfname,O_RDONLY,0660,
			MAXMSGID) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: 2 msgid_open() rs=%d\n",rs1) ;
#endif

	        f_mid = (rs1 >= 0) ;

	    } else
	        f_mid = FALSE ;

		c = 0 ;
	    for (i = 0 ; vecobj_get(&recips,i,&rp) >= 0 ; i += 1) {

	        if (rp == NULL) continue ;

	        if (pip->debuglevel > 0)
	            bprintf(pip->efp, "%s: recipient=%s\n",
	                pip->progname,rp->recipient) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: recipient=%s\n",rp->recipient) ;
#endif

	        if (pip->open.logfile)
	            logfile_printf(&pip->lh,"recip=%s\n",rp->recipient) ;

	        pip->daytime = time(NULL) ;

		memset(&midkey,0,sizeof(MSGID_KEY)) ;

	        midkey.recip = rp->recipient ;
	        midkey.reciplen = -1 ;
	        for (mn = 0 ; vecobj_get(&info,mn,&mop) >= 0 ; mn += 1) {
	            int	f_repeat = FALSE ;
	            int	f_deliver ;

	            if (mop == NULL) continue ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main: msg=%u off=%u mlen=%u\n",
	                    mn,mop->offset,mop->mlen) ;
#endif

	            mid.count = 0 ;
	            if (f_mid && (mop->messageid != NULL) && 
	                (mop->messageid[0] != '\0')) {

			if ((c & 255) == 255)
				pip->daytime = time(NULL) ;

			midkey.mtime = mop->mtime ;
	                midkey.from = mop->from ;
	                midkey.mid = mop->messageid ;

	                rs1 = msgid_update(&mids,pip->daytime,
				&midkey,&mid) ;

	                if (rs1 == SR_ACCESS)
	                    rs1 = msgid_match(&mids,pip->daytime,
				&midkey,&mid) ;

	                f_repeat = ((rs1 >= 0) && (mid.count > 0)) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4)) {
	                    debugprintf("main: msgid_update() rs=%d\n",rs1) ;
	                    if (rs1 >= 0)
	                        debugprintf("main: MID count=%u\n",mid.count) ;
	                }
#endif

	            } else
	                rs1 = SR_NOTFOUND ;

	            f_deliver = TRUE ;

/* repeat message-id */

	            if (pip->f.optnorepeat && f_repeat)
	                f_deliver = FALSE ;

/* spam */

	            if (pip->f.optnospam && mop->f_spam)
	                f_deliver = FALSE ;

/* deliver or not based on what we know */

	            if (f_deliver)
	                recip_mo(rp,mop->offset,mop->mlen) ;

/* formulate log entry as a result */

	            if (pip->open.logfile) {

	                if (f_repeat) {
	                    fmt = "  %3u %c%c%c c=%u" ;
	                } else
	                    fmt = "  %3u %c%c%c" ;

	                logfile_printf(&pip->lh,fmt,
	                    mn,
	                    ((mop->f_spam) ? 'S' : ' '),
	                    ((f_repeat) ? 'R' : ' '),
	                    ((f_deliver) ? 'D' : ' '),
	                    mid.count) ;

	            } /* end if (logging enabled) */

			c += 1 ;

	        } /* end for (looping through messages) */

	        c_processed += 1 ;
	        rs = 0 ;
	        if (rp->n > 0) {

	            rs = deliver(pip,tfd,rp) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main: deliver() rs=%d\n",rs) ;
#endif

	            if (rs < 0) {

	                if (pip->open.logfile)
	                    logfile_printf(&pip->lh, 
	                        "delivery failure u=%s (%d)\n",
				rp->recipient,rs) ;

	                if (pip->debuglevel > 0)
	                    bprintf(pip->efp, 
				"%s: delivery failure (%d)\n",
	                        pip->progname,rs) ;

	            }

	        } /* end if (delivery) */

	        if ((rs >= 0) && (rp->n > 0))
	            c_delivered += 1 ;

	        rp->rs = rs ;
	        rp->offset = rs ;

	        if (pip->open.logfile) {
	            if (rp->n > 0) {
	                if (rs >= 0) {
	                    logfile_printf(&pip->lh, 
				"  delivery=%u offset=%d",
	                        rp->n,rs) ;
	                } else {
	                    logfile_printf(&pip->lh, 
				"  delivery=%u FAILED (%d)",
	                        rp->n,rs) ;
			}
	            } else {
	                logfile_printf(&pip->lh, 
				"  delivery=0\n") ;
		    }
	        }

	        if ((pip->debuglevel > 0) && (! pip->f.quiet)) {

	            fmt = "%s: recip=%-32s %s (%d)\n" ;
	            if ((rs >= 0) || (rp->n == 0))
	                fmt = "%s: recip=%-32s\n" ;

	            bprintf(pip->efp,fmt,
	                pip->progname,
	                rp->recipient,((rs >= 0) ? "" : "FAILED"),rs) ;

	        }

	    } /* end for (looping through recipients) */

#if	CF_DEBUGS
	    rs = vecobj_audit(&recips) ;

	    debugprintf("main: 4 vecobj_audit() rs=%d\n",rs) ;

#endif

	    if (f_mid)
	        msgid_close(&mids) ;

	} /* end block */


/* do we want to issue notices to COMSAT */

	bflush(pip->efp) ;

	if (pip->open.logfile)
	    logfile_flush(&pip->lh) ;

	if ((pip->comsatfname != NULL) && (pip->comsatfname[0] != '-') &&
	    (c_delivered > 0) &&
	    (uc_fork() == 0)) {

	    int	ex1 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: comsatfname=%s\n",pip->comsatfname) ;
#endif

	    if (tfd >= 0)
	        u_close(tfd) ;

	    rs1 = comsat(pip,&recips,&info) ;

	    if (pip->open.logfile)
	        logfile_printf(&pip->lh,
	            "COMSAT messages sent %d\n",rs1) ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: COMSAT messages sent %d\n",
	            pip->progname,rs1) ;

	    if (pip->open.logfile)
	        logfile_close(&pip->lh) ;

	    bclose(pip->efp) ;

	    ex1 = (rs1 >= 0) ? EX_OK : EX_DATAERR ;
	    uc_exit(ex1) ;
	} /* end if (COMSAT) */

/* free up the message information container */

	vecobj_finish(&info) ;


/* do any optional reporting for multi-recipient mode */

	if (pip->f.multirecip) {

	    rs1 = SR_NOENT ;
	    if ((ofname != NULL) && (ofname[0] != '-')) {
	        rs1 = bopen(ofp,ofname,"wct",0644) ;
	    } else  {
	        rs1 = bopen(ofp,BFILE_STDOUT,"dwct",0644) ;
	    }

	    if (rs1 >= 0) {



	        bclose(ofp) ;

	    } /* end if (opened output) */

	    rs = SR_OK ;
	    ex = EX_OK ;
	} /* end if (multi-recipient mode) */


/* print a regular error message to STDERR if we are NOT in multi-recip mode */

	if ((! pip->f.multirecip) && (rs < 0)) {
	    switch (rs) {
	    case SR_NOENT:
	        ex = EX_NOUSER ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: recipient not found\n",
	                pip->progname) ;
		}
	        break ;
	    case SR_TXTBSY:
	        ex = EX_TEMPFAIL ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: could not capture the mail lock\n",
	                pip->progname) ;
		}
	        break ;
	    case SR_ACCES:
	        ex = EX_ACCESS ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: could not access the mail spool-file\n",
	                pip->progname) ;
		}
	        break ;
	    case SR_REMOTE:
	        ex = EX_FORWARDED ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,"%s: mail is being forwarded\n",
	                pip->progname) ;
		}
	        break ;
	    case SR_NOSPC:
	        ex = EX_NOSPACE ;
	        logfile_printf(&pip->lh,
	            "file-system is out of space\n") ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: local file-system is out of space\n",
	                pip->progname) ;
		}
	        break ;
	    default:
	        ex = EX_UNKNOWN ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,"%s: unknown bad thing (%d)\n",
	                pip->progname,rs) ;
		}
	        break ;
	    } /* end switch */
	} else
	    ex = EX_OK ;

	if (pip->open.logfile) {
	    logfile_printf(&pip->lh,
	        "recipients processed=%u delivered=%u\n",
	        c_processed,c_delivered) ;
	}

ret6:
	if (tfd >= 0)
	    u_close(tfd) ;

ret5:
	vecobj_finish(&info) ;

	bclose(ifp) ;

/* good return from program */
retgood:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%d\n",
	        pip->progname,ex) ;
	}

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	    debugprintf("main: past 'retgood'\n") ;
#endif

ret4:

#if	CF_DEBUGS
	rs = vecobj_audit(&recips) ;
	debugprintf("main: 5 vecobj_audit() rs=%d\n",rs) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: vecobj_finishrecips()\n") ;
#endif

	vecobj_finishrecips(&recips) ;

	vecobj_finish(&recips) ;

ret3:
	if (pip->open.logfileenv) {
	    logfile_close(&pip->envsum) ;
	}

	if (pip->open.logfile) {
	    logfile_close(&pip->lh) ;
	}

done:
retearly:
ret2:
	if (pip->open.akopts) {
	    keyopt_finish(&akopts) ;
	}

	dater_finish(&pip->tmpdate) ;

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	}

ret1:
	proginfo_finish(pip) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* program usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [-d] [-f fromaddr] recip1 [recip2 [...]] [-m]\n",
	    pip->progname,pip->progname) ;

	goto retearly ;

/* help */
help:
	ex = EX_INFO ;
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	goto retearly ;

/* bad stuff */
badargextra:
	bprintf(pip->efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(pip->efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badargnum:
	bprintf(pip->efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto retearly ;


baduser:
	ex = EX_NOUSER ;
	bprintf(pip->efp,"%s: could not get user information (%d)\n",
	    pip->progname,rs) ;

	goto ret3 ;


badinit3:
	ex = EX_OSERR ;
	goto ret4 ;

badinit2:
badinit1:
	ex = EX_OSERR ;
	goto ret3 ;


badspooldir:
	ex = EX_OSERR ;
	bprintf(pip->efp,"%s: could not access spool directory (%d)\n",
	    pip->progname,rs) ;

	goto ret3 ;

/* bad stuff that need some extra attention */
badinopen:
	ex = EX_NOINPUT ;
	bprintf(pip->efp,"%s: could not open standard input (%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badtmpfile:
badtmpopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could create-open tempory file (%d)\n",
	    pip->progname,rs) ;

	bclose(ifp) ;

/* come here for a bad return from the program */
badret:

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	    debugprintf("main: badret\n") ;
#endif

	goto ret4 ;

}
/* end subroutine (main) */


/* local subroutines */


static int errormap(int val)
{
	int	i, ex ;

	for (i = 0 ; errormaps[i].rs != 0 ; i += 1) {
	    if (errormaps[i].rs == val) break ;
	} /* end for */

	ex = (errormaps[i].rs != 0) ? errormaps[i].ex : EX_UNKNOWN ;

	return ex ;
}
/* end subroutine (errormap) */


/* process program options */
static int getprogopts(pip,kop,setsp)
struct proginfo	*pip ;
KEYOPT		*kop ;
vecstr		*setsp ;
{
	KEYOPT_CUR	kcur ;

	int	rs, i, oi, val, cl ;
	int	nlen, klen, vlen ;

	char	*kp, *vp ;
	char	*cp ;


	nlen = strlen(pip->searchname) ;

/* load up the environment options */

	if ((cp = getenv(VAROPTS)) != NULL) {
	    keyopt_loads(kop,cp,-1) ;
	}

/* system-wide options? */

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
	        if (vlen > 0) break ;
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

#if	CF_DEBUG
	                    if (pip->debuglevel > 1)
	                        debugprintf("getprogopts: opt comsat=%s\n",cp) ;
#endif

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


/* add a recipient to the recipient list */
static int vecobj_addrecip(op,name)
vecobj		*op ;
const char	name[] ;
{
	struct recip	*rp, entry ;
	int		rs, i ;

#if	CF_DEBUGS
	debugprintf("vecobj_addrecip: name=%s\n",name) ;
#endif

	if (name[0] == '\0')
	    return 0 ;

	for (i = 0 ; (rs = vecobj_get(op,i,&rp)) >= 0 ; i += 1) {
	    if (rp == NULL) continue ;
	    if (strcmp(rp->recipient,name) == 0) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("vecobj_addrecip: end search rs=%d\n",rs) ;
#endif

	if (rs == SR_NOTFOUND) {
	    rs = recip_init(&entry,name) ;
	    if (rs >= 0) {
	        rs = vecobj_add(op,&entry) ;
	        if (rs < 0)
	            recip_free(&entry) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("vecobj_addrecip: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (vecobj_addrecip) */


/* free up recipients */
static int vecobj_finishrecips(op)
vecobj		*op ;
{
	struct recip	*rp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(op,i,&rp) >= 0 ; i += 1) {
	    if (rp != NULL) {
	        rs1 = vecitem_finish(&rp->mds) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("vecobj_finishrecips: ret\n") ;
#endif

	return rs ;
}
/* end subroutine (vecobj_finishrecips) */


static int recip_init(rp,name)
struct recip	*rp ;
const char	name[] ;
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("recip_init: name=%s\n",name) ;
#endif

	memset(rp,0,sizeof(struct recip)) ;

	rs = vecitem_start(&rp->mds,10,0) ;

	strwcpy(rp->recipient,name,LOGNAMELEN) ;

#if	CF_DEBUGS
	debugprintf("recip_init: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (recip_init) */


static int recip_mo(rp,offset,mlen)
struct recip	*rp ;
uint	offset, mlen ;
{
	struct md	mo ;
	int		rs ;

	mo.offset = offset ;
	mo.mlen = mlen ;
	rs = vecitem_add(&rp->mds, &mo,sizeof(struct md)) ;

	if (rs >= 0)
	    rp->n += 1 ;

	return rs ;
}


static int recip_free(rp)
struct recip	*rp ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecitem_finish(&rp->mds) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (recip_free) */


/* issue notices to COMSAT */
static int comsat(pip,rsp,mip)
struct proginfo	*pip ;
vecobj		*rsp ;
vecobj		*mip ;
{
	struct hostent	he, *hep ;
	struct servent	se ;
	struct recip	*rp ;
	struct md	*mdp ;
	VECSTR		h ;
	SOCKADDRESS	sa ;
	in_addr_t	addr ;

	int	rs, i, j, k, ul, blen, n ;
	int	sendflags = 0, salen ;
	int	fd ;
	int	defport, port ;

	char	nodename[NODENAMELEN + 1] ;
	char	hostbuf[HOSTBUFLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	*nsp, *np, *up ;


/* get some information about COMSAT */

	defport = IPPORT_BIFFUDP ;

	rs = uc_getservbyname("comsat", PROTONAME, &se,buf,BUFLEN) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("comsat: uc_getservbyname() rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    defport = (int) ntohs(se.s_port) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("comsat: defport=%d\n",defport) ;
#endif


	rs = u_socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP) ;
	fd = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("comsat: u_socket() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

/* get the COMSAT hosts */

	rs = vecstr_start(&h,NNODES,0) ;

	if (rs < 0)
	    goto ret1 ;

	if (pip->comsatfname[0] != '+') {
	    vecstr_loadfile(&h,0,pip->comsatfname) ;
	} else {
	    vecstr_add(&h,LOCALHOST,-1) ;
	}

/* do the notices */

	n = 0 ;
	for (i = 0 ; vecstr_get(&h,i,&nsp) >= 0 ; i += 1) {
	    if (nsp == NULL) continue ;

/* separate the node-spec into node and port */

	    np = nodename ;
	    port = parsenodespec(pip,defport,buf,BUFLEN,nodename,nsp) ;

	    if (port >= 0) {

/* continue */

#if	CF_ISSAMEHOST
	        if (issamehostname(np,pip->nodename,pip->domainname))
	            np = LOCALHOST ;
#else
	        if (strcmp(np,pip->nodename) == 0)
	            np = LOCALHOST ;
#endif

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("comsat: i=%d np=%s\n",i,np) ;
#endif

	        hep = &he ;
	        rs = getourhe(np,NULL,hep,hostbuf,HOSTBUFLEN) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("comsat: getourhe() rs=%d\n",rs) ;
#endif

	        if ((rs >= 0) && (hep->h_addrtype == AF_INET)) {
	            struct msgoff	*mop ;
	            uint	off ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                in_addr_t	ia ;
	                memcpy(&ia,hep->h_addr,sizeof(int)) ;
	                debugprintf("comsat: got INET address=\\x%08x\n",
	                    ntohl(ia)) ;
	            }
#endif /* CF_DEBUG */

	            salen = sockaddress_start(&sa,AF_INET,hep->h_addr,port,0) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("comsat: sockaddress_start() rs=%d\n",
			salen) ;
#endif

	            for (j = 0 ; vecobj_get(rsp,j,&rp) >= 0 ; j += 1) {
	                if (rp == NULL) continue ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(4))
	                    debugprintf("comsat: recip=%s rs=%d offset=%u\n",
	                        rp->recipient,rp->rs,rp->offset) ;
#endif

	                if (rp->rs < 0) continue ;

	                up = rp->recipient ;
	                ul = strlen(up) ;

	                off = rp->offset ;
	                for (k = 0 ; vecitem_get(&rp->mds,k,&mdp) >= 0 ; 
				k += 1) {

	                    if (mdp == NULL) continue ;

#if	CF_DEBUG
	                    if (DEBUGLEVEL(4))
	                        debugprintf("comsat: msg=%u off=%u mlen=%u\n", 
	                            k,off,mdp->mlen) ;
#endif

	                    blen = mkcsmsg(buf,BUFLEN,up,ul,off) ;

	                    if (blen > 1) {

	                        rs = u_sendto(fd,buf,blen,sendflags,
					&sa,salen) ;

#if	CF_DEBUG
	                        if (DEBUGLEVEL(4))
	                            debugprintf("comsat: u_sendto() rs=%d\n",
					rs) ;
#endif

	                        if (rs >= 0)
	                            n += 1 ;

	                    }

	                    off += mdp->mlen ;

	                } /* end for (messages) */

	            } /* end for (looping through recipients) */

	            sockaddress_finish(&sa) ;
	        } /* end if (got an host address) */

	    } /* end if (had a possible port to contact) */

	} /* end for (hosts) */

	vecstr_finish(&h) ;

ret1:
	u_close(fd) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("comsat: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (comsat) */


/* parse a COMSAT specification into node and port */
static int parsenodespec(pip,defport,buf,buflen,nodename,nsp)
struct proginfo	*pip ;
int		defport ;
char		buf[] ;
int		buflen ;
char		nodename[] ;
const char	*nsp ;
{
	struct servent	se ;
	int	rs = SR_OK ;
	int	port = defport ;
	int	nl, pl ;
	int	cl ;
	char	*np, *pp ;
	char	*cp ;

	pp = NULL ;
	if ((cp = strchr(nsp,':')) != NULL) {

	    nl = sfshrink(nsp,(cp - nsp),&np) ;

	    pl = sfshrink((cp + 1),-1,&pp) ;

	    if (pl == 0)
	        pp = NULL ;

	} else
	    nl = sfshrink(nsp,-1,&np) ;

	strwcpy(nodename,np,MIN(nl,NODENAMELEN)) ;

	if (pp != NULL) {
	    const int	ch = MKCHAR(pp[0]) ;

	    if (isdigitlatin(ch)) {

	        rs = cfdeci(pp,pl,&port) ;

	    } else {

	        rs = uc_getservbyname(pp, PROTONAME, &se,buf,buflen) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("parsenodespec: uc_getservbyname() rs=%d\n",
			rs) ;
#endif

	        if (rs >= 0)
	            port = (int) ntohs(se.s_port) ;

	    } /* end if (numeric or alpha) */

	} /* end if (had a port spec) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("parsenodespec: ret rs=%d port=%d\n",rs,port) ;
#endif

	return (rs >= 0) ? port : rs ;
}
/* end subroutine (parsenodespec) */


/* make (marshall) the COMSAT message itself */
static int mkcsmsg(buf,buflen,up,ul,val)
char		buf[] ;
int		buflen ;
const char	*up ;
int		ul ;
uint		val ;
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;

	if ((rs = sbuf_start(&b,buf,buflen)) >= 0) {

	sbuf_strw(&b,up,ul) ;

	sbuf_char(&b,'@') ;

	sbuf_deci(&b,val) ;

	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return rs ;
}
/* end subroutine (mkcsmsg) */


