/* main (testsocket) */

/* program to test 'socket(3socket)' API */


#define	CF_DEBUGS	0		/* compile-time debug switch */
#define	CF_DEBUG	1		/* run-time debug switch */
#define	CF_PROCSOCK	1		/* process socket stuff */


/* revision history:

	- 2005-05-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ testsocket


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stropts.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<inetaddr.h>
#include	<netorder.h>
#include	<hostinfo.h>
#include	<hostaddr.h>
#include	<sockaddress.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"comsatmsg.h"


/* local defines */

#define	MAXARGINDEX	100
#define	NARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	USERBUFLEN
#define	USERBUFLEN	(NODENAMELEN + (2 * 1024))
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	INET4ADDRLEN
#define	INET4ADDRLEN	sizeof(in_addr_t)
#endif

#ifndef	INET6ADDRLEN
#define	INET6ADDRLEN	16
#endif

#ifndef	INETXADDRLEN
#define	INETXADDRLEN	MAX(INET4ADDRLEN,INET6ADDRLEN)
#endif /* INETXADDRLEN */

#ifndef	INET4_ADDRSTRLEN
#define	INET4_ADDRSTRLEN	16
#endif

#ifndef	INET6_ADDRSTRLEN
#define	INET6_ADDRSTRLEN	46	/* Solaris® says this is 46! */
#endif

#ifndef	INETX_ADDRSTRLEN
#define	INETX_ADDRSTRLEN	MAX(INET4_ADDRSTRLEN,INET6_ADDRSTRLEN)
#endif

#define	COMSATMSGLEN	(MAXPATHLEN+USERNAMELEN+32)

#define	PREPNAME_MAGIC	0x17161524

#ifndef	LOCALHOST
#define	LOCALHOST	"localhost"
#endif

#define	DEFPORTSPEC	"biff"


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getaf(const char *,int) ;
extern int	getprotofamily(int) ;

extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
static int	debugaddrprint(const char *,struct sockaddr *sap) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	hostaddrinfo_makedate[] ;


/* local structures */

struct locinfo_flags {
	uint		dummy:1 ;
} ;

struct locinfo {
	struct locinfo_flags	have, f, changed, final ;
	struct locinfo_flags	init, open ;
	vecstr		stores ;
	PROGINFO	*pip ;
	const char	*muname ;	/* mail-username */
	const char	*portspec ;	/* port-specification */
	char		*msgbuf ;
	int		msglen ;
	int		to ;
	int		af ;		/* address-family */
	int		mo ;		/* mail-offset */
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	locinfo_start(struct locinfo *,PROGINFO *) ;
static int	locinfo_mkmsg(struct locinfo *) ;
static int	locinfo_setentry(struct locinfo *,const char **,
			const char *,int) ;
static int	locinfo_sendmsg(struct locinfo *,struct addrinfo *) ;
static int	locinfo_finish(struct locinfo *) ;

static int	procname(PROGINFO *,bfile *,const char *) ;

#if	CF_PROCSOCK
static int	procsock(PROGINFO *) ;
#endif /* CF_PROCSOCK */


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"of",
	"ef",
	"to",
	"mu",
	"mo",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_tmpdir,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_of,
	argopt_ef,
	argopt_to,
	argopt_mu,
	argopt_mo,
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
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	PROGINFO	pi, *pip = &pi ;
	struct locinfo	li, *lip = &li ;

	USERINFO	u ;

	KEYOPT		akopts ;

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan ;
	int	rs, rs1 ;
	int	v ;
	int	ex = EX_INFO ;
	int	f_optplus, f_optminus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[NARGGROUPS] ;
	char	userbuf[USERBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*efname = NULL ;
	const char	*afspec = NULL ;
	const char	*portspec = NULL ;
	const char	*muname = NULL ;
	const char	*moffset = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* miscellaneous early stuff */

	pip->verboselevel = 1 ;

	pip->lip = lip ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

/* parse arguments */

	for (ai = 0 ; ai < NARGGROUPS ; ai += 1)
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
	    if ((argl > 1) && (f_optplus || f_optminus)) {

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

/* temporary directory */
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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program search name */
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

/* argument list file */
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

/* error file */
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

/* lookup-timeout */
	                case argopt_to:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl) {
	                        rs = cfdecti(argp,argl,&v) ;
	                        lip->to = v ;
	                    }
	                    break ;

/* mail-username */
	                case argopt_mu:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl)
				muname = argp ;
	                    break ;

/* mail-offset */
	                case argopt_mo:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }
	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;
	                    if (argl)
				moffset = argp ;
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

/* program-root */
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

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'a':
	                    case 'f':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            afspec = argp ;
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

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* port specification */
	                    case 'p':
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

/* mail user-name */
	                    case 'u':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            muname = argp ;
	                        break ;
/* verbose */
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

	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
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

/* initialize some other common stuff */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname, pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname, pip->searchname) ;
	}

/* help file */

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check arguments */

	if ((afspec != NULL) && (afspec[0] != '\0')) {
	    rs = getaf(afspec,-1) ;
	    lip->af = rs ;
	}

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto badaddrspace ;
	}

/* user */

	rs = userinfo(&u,userbuf,USERBUFLEN,NULL) ;
	if (rs < 0) {
	    ex = EX_NOUSER ;
	    goto baduser ;
	}

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;
	pip->logid = u.logid ;
	pip->pid = u.pid ;

/* get a TMPDIR */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* sort out the arguments and apply defaults */

	if (portspec == NULL) portspec = DEFPORTSPEC ;

	if (muname == NULL) muname = u.username ;

	lip->portspec = portspec ;
	lip->muname = muname ;

	if ((moffset != NULL) && (moffset[0] != '\0')) {
	    rs = cfdeci(moffset,-1,&v) ;
	    lip->mo = v ;
	}

	if (lip->mo < 0) rs = SR_INVALID ;

	if (rs >= 0) rs = locinfo_mkmsg(lip) ;

	if (rs < 0) goto badarg ;

/* go */

	if ((ofname == NULL) || (ofname[0] == '\0') ||
	    (ofname[0] == '-')) ofname = BFILE_STDOUT ;
	rs = bopen(ofp,ofname,"wct",0666) ;
	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: output unavailable (%d)\n",
	        pip->progname,rs) ;
	    goto badoutopen ;
	}

/* process the arguments */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = procname(pip,ofp,cp) ;
	    if (rs < 0)
	        break ;

	} /* end for (processing positional arguments) */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    bfile	argfile ;

	    if (strcmp(afname,"-") == 0) afname = BFILE_STDIN ;
	    rs = bopen(&argfile,afname,"r",0666) ;
	    if (rs >= 0) {
	        int	len ;
	        char	linebuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(&argfile,linebuf,LINEBUFLEN)) > 0) {
	            len = rs ;

	            if (linebuf[len - 1] == '\n') len -= 1 ;
	            linebuf[len] = '\0' ;

	            cp = linebuf ;
	            if ((cp[0] == '\0') || (cp[0] == '#')) continue ;

	            pan += 1 ;
	    	    rs = procname(pip,ofp,cp) ;

	            if (rs < 0) {
	                bprintf(pip->efp,
	                    "%s: error (%d) in file=%s\n",
	                    pip->progname,rs,cp) ;
	                break ;
	            }

	        } /* end while (reading lines) */

	        bclose(&argfile) ;
	    } else {

	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: inaccessible argument list file (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: \targfile=%s\n",
	                pip->progname,afname) ;
	        }

	    } /* end if */

	} /* end if (processing file argument file list) */

#ifdef	COMMENT
	if ((rs >= 0) && (pan == 0)) {

	    cp = "-" ;
	    pan += 1 ;
	    rs = procname(pip,ofp,cp) ;

	} /* end if (processing requests) */
#endif /* COMMENT */

#if	CF_PROCSOCK
	if (rs >= 0)
	    rs = procsock(pip) ;
#endif /* CF_PROCSOCK */

	bclose(ofp) ;

done:
badoutopen:
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

/* finish up */
baduser:
badaddrspace:
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

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;
	proginfo_finish(pip) ;

ret0:
badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument(s) specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
PROGINFO	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [-u <mu>] [-mo <mo>] [<host(s)> ...]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int locinfo_start(lip,pip)
struct locinfo	*lip ;
PROGINFO	*pip ;
{
	int	rs ;


	if (lip == NULL)
	    return SR_FAULT ;

	memset(lip,0,sizeof(struct locinfo)) ;
	lip->pip = pip ;
	lip->to = -1 ;
	lip->af = AF_UNSPEC ;

	rs = vecstr_start(&lip->stores,0,0) ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(lip)
struct locinfo	*lip ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (lip == NULL)
	    return SR_FAULT ;

	if (lip->msgbuf != NULL) {
	    uc_free(lip->msgbuf) ;
	    lip->msgbuf = NULL ;
	}

	rs1 = vecstr_start(&lip->stores,0,0) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (locinfo_finish) */


static int locinfo_mkmsg(lip)
struct locinfo	*lip ;
{
	PROGINFO	*pip = lip->pip ;

	struct comsatmsg_mailoff	m0 ;

	const int	msglen = COMSATMSGLEN ;

	int	rs = SR_OK ;
	int	len = 0 ;

	char	msgbuf[COMSATMSGLEN+1] ;


	memset(&m0,0,sizeof(struct comsatmsg_mailoff)) ;
	strwcpy(m0.username,lip->muname,USERNAMELEN) ;
	m0.offset = lip->mo ;

	rs = comsatmsg_mailoff(&m0,0,msgbuf,msglen) ;
	len = rs ;
	if (rs >= 0) {
	    char	*p ;
	    int		size = len ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
		memcpy(p,msgbuf,len) ;
		lip->msgbuf = p ;
		lip->msglen = len ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	debugprintf("locinfo_mkmsg: msg=>%t<\n",lip->msgbuf,lip->msglen) ;
	debugprintf("locinfo_mkmsg: ret rs=%d len=%u\n",rs,len) ;
	}
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_mkmsg) */


int locinfo_setentry(lip,epp,vp,vl)
struct locinfo	*lip ;
const char	**epp ;
const char	vp[] ;
int		vl ;
{
	int	rs = SR_OK ;
	int	len = 0 ;


	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! lip->open.stores) {
	    rs = vecstr_start(&lip->stores,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;

	    if (*epp != NULL) oi = vecstr_findaddr(&lip->stores,*epp) ;

	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(&lip->stores,vp,len,epp) ;
	    } else
		*epp = NULL ;

	    if ((rs >= 0) && (oi >= 0))
	        vecstr_del(&lip->stores,oi) ;

	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */


static int locinfo_sendmsg(lip,aip)
struct locinfo	*lip ;
struct addrinfo	*aip ;
{
	PROGINFO	*pip = lip->pip ;
	struct sockaddr	*sap ;
	const int	to = lip->to ;
	int		rs ;
	int		flags = 0 ;
	int		sal ;
	int		fd = -1 ;

	sap = aip->ai_addr ;
	sal = aip->ai_addrlen ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) 
	    debugaddrprint("locinfo_sendmsg: ",sap) ;
#endif

	rs = u_socket(aip->ai_family,aip->ai_socktype,aip->ai_protocol) ;
	if (rs >= 0) {
	    int	fd = rs ;

	    rs = u_sendto(fd,lip->msgbuf,lip->msglen,flags,sap,sal) ;

	    u_close(fd) ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) 
	debugprintf("locinfo_sendmsg: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (locinfo_sendmsg) */


static int procname(pip,ofp,hostname)
PROGINFO	*pip ;
bfile		*ofp ;
const char	hostname[] ;
{
	struct locinfo	*lip = pip->lip ;

	struct addrinfo	hint, *aip ;

	HOSTADDR	ha ;

	const int	proto = IPPROTO_UDP ;

	int	rs ;
	int	rs1 ;

	const char	*hn ;
	const char	*ps ;


	if (hostname == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procname: hostname=%s\n",hostname) ;
#endif

	if ((hostname[0] == '\0') || (hostname[0] == '-'))
	    hostname = LOCALHOST ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: host=%s\n",
	        pip->progname,hostname) ;

/* prepare hints */

	memset(&hint,0,sizeof(struct addrinfo)) ;

	hint.ai_protocol = proto ;
	if (lip->af >= 0) {
	    int pf = getprotofamily(lip->af) ;
	    if (pf >= 0)
	        hint.ai_family = pf ;
	}

#if	CF_DEBUGS
	debugprintf("dialudp: af=%u pf=%u\n",lip->af,hint.ai_family) ;
#endif

/* do the spin */

	hn = hostname ;
	ps = lip->portspec ;

	if ((rs = hostaddr_start(&ha,hn,ps,&hint)) >= 0) {
	    HOSTADDR_CUR	cur ;

	    if ((rs = hostaddr_curbegin(&ha,&cur)) >= 0) {
		int	rs1 ;
		int	c = 0 ;

	        while ((rs1 = hostaddr_enum(&ha,&cur,&aip)) >= 0) {

#if	CF_DEBUGS
		    debugprintf("dialudp: trying proto=%u\n",
			aip->ai_protocol) ;
#endif

		   if ((aip->ai_protocol == 0) ||
	               (aip->ai_protocol == proto)) {

	                c += 1 ;
	                rs = locinfo_sendmsg(lip,aip) ;
	                if (rs >= 0)
	                    break ;

		    } /* end if (protocol match) */

	        } /* end while */

		if ((rs >= 0) && (c == 0)) rs = SR_HOSTUNREACH ;

#if	CF_DEBUGS
		debugprintf("dialudp: done rs=%d c=%u\n",rs,c) ;
#endif

	        hostaddr_curend(&ha,&cur) ;
	    } /* end if (cursor) */

	    hostaddr_finish(&ha) ;
	} /* end if (initialized host addresses) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procname) */


#if	CF_PROCSOCK
static int procsock(PROGINFO *pip)
{
	const int	pf = PF_LINK ;
	const int	stype = SOCK_DGRAM ;
	const int	proto = 0 ;
	int		rs = SR_OK ;
	int		sv[2] ;

	rs = u_socketpair(pf,stype,proto,sv) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsock: u_socketpair() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    int	i ;
	    for (i = 0 ; i < 2 ; i += 1) u_close(sv[i]) ;
	}

	if (rs >= 0) {
		int	fd ;

		rs = u_socket(pf,stype,proto) ;
		fd = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procsock: u_socket() rs=%d\n",rs) ;
#endif

	if (rs >= 0) u_close(fd) ;

	}

	return rs ;
}
#endif /* CF_PROCSOCK */


#if	CF_DEBUGS || CF_DEBUG
static int debugaddrprint(const char *s,struct sockaddr *sap)
{
	SOCKADDRESS	*ap = (SOCKADDRESS *) sap ;
	int	rs = SR_OK ;
	int	af = 0 ;
	int	port = 0 ;
	char	addr[INETXADDRLEN+1] ;
	char	addrstr[INETX_ADDRSTRLEN+1] ;
	addrstr[0] = '\0' ;
	if (rs >= 0) {
	    rs = sockaddress_getaf(ap) ;
	    af = rs ;
	}
	debugprintf("%s af=%u\n",s,af) ;
	if (rs >= 0) {
	    rs = sockaddress_getport(ap) ;
	    port = rs ;
	}
	debugprintf("%s port=%u\n",s,port) ;
	if (rs >= 0)
	rs = sockaddress_getaddr(ap,addr,INETXADDRLEN) ;
	if (rs >= 0) {
	    rs = sninetaddr(addrstr,INETX_ADDRSTRLEN,af,addr) ;
	    debugprintf("%s addr=%s\n",s,addrstr) ;
	}
	return rs ;
}
/* end subroutine (debugaddrprint) */
#endif /* (CF_DEBUGS || CF_DEBUG) */



