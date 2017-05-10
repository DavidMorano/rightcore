/* main (testopenport) */

/* program to test binding a socket */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	1		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 1989-03-01, David A­D­ Morano

	This subroutine was originally written. 


	= 1998-06-01, David A­D­ Morano

	I enhanced the program a little.


	= 2013-03-01, David A­D­ Morano

	I added logging of requests to a file.
	This would seem to be an appropriate security precaution.


*/

/* Copyright © 1989,1998,2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ testopenport 


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<limits.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<vecpstr.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<msgbuf.h>
#include	<sockaddress.h>
#include	<getax.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"openport.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	MBUFLEN		MSGBUFLEN

#define	DEBUGFNAME	"/tmp/openport.deb"


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	vecpstr_adduniq(VECPSTR *,const char *,int) ;
extern int	getportnum(const char *,const char *) ;
extern int	hasalldig(const char *,int) ;

extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	mkuibang(char *,int,USERINFO *) ;
extern int	mkuiname(char *,int,USERINFO *) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			const char *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
#endif

extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct query {
	const char	*uidp ;
	const char	*protop ;
	const char	*portp ;
	int		uidlen ;
	int		protolen ;
	int		portlen ;
} ;

struct prototupple {
	int		pf ;
	int		ptype ;
	int		proto ;
	const char	*name ;
} ;


/* forward references */

static int	usage(struct proginfo *) ;

static int	process(struct proginfo *,const char *,const char *,
			VECPSTR *,int) ;


/* local variables */

static volatile int	if_exit ;
static volatile int	if_int ;

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"lf",
	"db",
	"query",
	"runint",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_lf,
	argopt_db,
	argopt_query,
	argopt_runint,
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
	{ SR_PERM, EX_NOPERM },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const struct prototupple	socknames[] = {
	{ PF_INET, SOCK_STREAM, IPPROTO_TCP, "tcp" },
	{ PF_INET, SOCK_STREAM, 0, "tcp" },
	{ PF_INET, SOCK_DGRAM, IPPROTO_UDP, "udp" },
	{ PF_INET, SOCK_DGRAM, 0, "udp" },
#ifdef	PF_INET6
	{ PF_INET6, SOCK_STREAM, IPPROTO_TCP, "tcp6" },
	{ PF_INET6, SOCK_STREAM, 0, "tcp6" },
	{ PF_INET6, SOCK_DGRAM, IPPROTO_UDP, "udp6" },
	{ PF_INET6, SOCK_DGRAM, 0, "udp6" },
#endif /* PF_INET6 */
	{ 0, 0, NULL }
} ;

static const char	*defprotos[] = {
	"tcp",
	"udp",
	"ddp",
	NULL
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	VECPSTR		al ;

	USERINFO	u ;

	bfile		errfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan = 0 ;
	int	rs, rs1 ;
	int	v ;
	int	cl ;
	int	cfd = FD_STDIN ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_inopen = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*logdname = LOGDNAME ;
	char	argpresent[MAXARGGROUPS] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	tmpfname[MAXPATHLEN+ 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*logfname = NULL ;
	const char	*dbfname = NULL ;
	const char	*cp ;


	if_int = 0 ;
	if_exit = 0 ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;

/* start parsing the arguments */

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
	    argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {

	        if (isdigit(argp[1])) {

	            argval = (argp + 1) ;

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

/* keyword match or only key letters? */

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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sn = argp ;
	                    }
	                    break ;

/* argument-list file */
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

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            efname = argp ;
	                    }
	                    break ;

/* output file name */
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

/* input file name */
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

/* log filename */
	                case argopt_lf:
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

/* data-base filename */
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            dbfname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            dbfname = argp ;
	                    }
	                    break ;

/* query mode */
	                case argopt_query:
	                    pip->f.query = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.query = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_runint:
			    cp = NULL ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
				    cp = avp ;
				    cl = avl ;
				}
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
				    cp = argp ;
				    cl = argl ;
				}
	                    }
			    if (cp != NULL) {
				rs = cfdecti(cp,cl,&v) ;
				pip->intrun = v ;
			    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

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

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* all mode */
	                    case 'a':
	                        pip->f.all = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.all = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* binder mode mode */
	                    case 'b':
	                        pip->f.binder = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.binder = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
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

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	}

	if ((rs >= 0) && (pip->debuglevel == 0)) {
	    if ((cp = getenv(VARDEBUGLEVEL)) != NULL) {
	        rs = optvalue(cp,-1) ;
	        pip->debuglevel = rs ;
	    }
	}

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

/* get some program information */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some initialization */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (pip->f.binder) {
	    dbfname = NULL ;
	} else {
	    if (dbfname == NULL) dbfname = getenv(VARDBFNAME) ;
	}
	if (dbfname == NULL) dbfname = USERPORTSFNAME ;

	if ((ifname != NULL) && (ifname[0] != '\0')) {
	    if (ifname[0] != '-') {
	        f_inopen = TRUE ;
	        rs = uc_open(ifname,O_RDWR,0777) ;
	        cfd = rs ;
	    }
	    if (rs < 0) goto badinopen ;
	}

	if (pip->intrun <= 0) pip->intrun = INTRUN ;

/* other initialization */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;
	if (rs < 0) {
	    ex = EX_NOUSER ;
	    bprintf(pip->efp,
	        "%s: userinfo failure (%d)\n",
	        pip->progname,rs) ;
	    goto baduserinfo ;
	}

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;
	pip->homedname = u.homedname ;
	pip->name = u.name ;
	pip->org = u.organization ;
	pip->logid = u.logid ;
	pip->uid = u.uid ;

	pip->daytime = time(NULL) ;

	if (pip->logsize == 0) pip->logsize = LOGSIZE ;

/* log us */

	if (logfname == NULL) logfname = getenv(VARLOGFNAME) ;

	if (logfname == NULL) {
	    logfname = tmpfname ;
	    mkpath3(tmpfname,pip->pr,logdname,pip->searchname) ;
	}

	rs1 = logfile_open(&pip->lh,logfname,0,0666,pip->logid) ;

	if (rs1 >= 0) {
	    pip->open.logfile = TRUE ;

	    if (pip->logsize > 0)
	        logfile_checksize(&pip->lh,pip->logsize) ;

	    logfile_userinfo(&pip->lh,&u,
	        pip->daytime,pip->searchname,pip->version) ;

	} /* end if (logfile_open) */

/* loop through arguments */

	rs = vecpstr_start(&al,4,0,0) ;
	if (rs < 0) goto badalstart ;

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = vecpstr_adduniq(&al,cp,-1) ;

	    if (if_int || if_exit) break ;
	    if (rs < 0) break ;
	} /* end for (handling positional arguments) */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afname,"-") == 0)
	        afname = BFILE_STDIN ;

	    if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            int	len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            pan += 1 ;
	            rs = vecpstr_adduniq(&al,lbuf,len) ;

	            if (if_int || if_exit) break ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        bclose(afp) ;
	    } else {
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: argument file inaccessible (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",
	                pip->progname,afname) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

/* OK, we're good to go */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: f_bind=%u\n",pip->f.binder) ;
	    debugprintf("main: f_all=%u\n",pip->f.all) ;
	    debugprintf("main: f_query=%u\n",pip->f.query) ;
	}
#endif

	if (rs >= 0)
	    rs = process(pip,dbfname,ofname,&al,cfd) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: process() rs=%d\n",rs) ;
#endif

	vecpstr_finish(&al) ;

/* finish up and out */
badalstart:
	if (pip->open.logfile) {
	    pip->open.logfile = FALSE ;
	    logfile_close(&pip->lh) ;
	}

baduserinfo:
	if (f_inopen && (cfd >= 0)) {
	    f_inopen = FALSE ;
	    u_close(cfd) ;
	}

/* we are done */
badinopen:
done:
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet)
	            bprintf(pip->efp,"%s: invalid query (%d)\n",
	                pip->progname,rs) ;
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_AGAIN:
	        ex = EX_TEMPFAIL ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} /* end if */

retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	proginfo_finish(pip) ;

badprogstart:

#if	CF_DEBUGS || CF_DEBUG
	debugclose() ;
#endif

	return ex ;

/* the bad things */
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
	int	rs ;
	int	wlen = 0 ;

	const char	*pn = pip->progname ;
	const char	*fmt ;


	fmt = "%s: USAGE> %s \n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int process(pip,dbfname,ofname,alp,cfd)
struct proginfo	*pip ;
const char	*dbfname ;
const char	*ofname ;
VECPSTR		*alp ;
int		cfd ;
{
	    SOCKADDRESS	listen, from ;

	    time_t	starttime ;
	    time_t	daytime = time(NULL) ;

	char	abuf[SOCKADDRESS_LEN +1] ;

	const int	alen = SOCKADDRESS_LEN ;

	int	rs = SR_OK ;
	    int	pf, ptype, proto ;
	    int	port ;
	    int	lfd ;
	    int	fromlen ;
	int	to_run = pip->intrun ;

	const char	*cp ;


	    pf = PF_INET6 ;
	    ptype = SOCK_STREAM ;
	    proto = IPPROTO_TCP ;

	    port = 1023 ;
	memset(abuf,0,alen) ;

	if ((rs = sockaddress_start(&listen,pf,abuf,port,0)) >= 0) {

	    rs = openport(pf,ptype,proto,&listen) ;
	    lfd = rs ;

#if	CF_DEBUG
		if (DEBUGLEVEL(4))
	    debugprintf("main/process: openport() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
		rs = u_listen(lfd,9) ;

#if	CF_DEBUG
		if (DEBUGLEVEL(4))
	    debugprintf("main/process: u_listen() rs=%d\n",rs) ;
#endif

	    }

	    starttime = daytime ;
	    while ((rs >= 0) && (to_run > (daytime - starttime))) {

	        fromlen = sizeof(SOCKADDRESS) ;
	        if ((rs = u_accept(lfd,&from,&fromlen)) >= 0) {
	        	int	fd = rs ;

#if	CF_DEBUG
		if (DEBUGLEVEL(4))
	    debugprintf("main/process: accepted fd=%u\n",fd) ;
#endif

	        cp = "hello world\n" ;
	        rs = uc_writen(fd,cp,strlen(cp)) ;

#if	CF_DEBUG
		if (DEBUGLEVEL(4))
	    debugprintf("main/process: uc_writen() rs=%u\n",rs) ;
#endif

	        u_close(fd) ;
		} /* end if */

#if	CF_DEBUG
		if (DEBUGLEVEL(4))
	    debugprintf("main/process: serviced rs=%d\n",rs) ;
#endif

	        daytime = time(NULL) ;

	    } /* end while */

#if	CF_DEBUG
		if (DEBUGLEVEL(4))
	    debugprintf("main/process: while done\n") ;
#endif

	    u_close(lfd) ;

	    sockaddress_finish(&listen) ;
	} /* end if (sockaddress) */

#if	CF_DEBUG
		if (DEBUGLEVEL(4))
	    debugprintf("main/process: ret rs=%d\n",rs) ;
#endif

	return rs ;
} 
/* end subroutine (process) */



