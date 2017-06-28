/* main (pingstat) */

/* generic (pretty much) front end program subroutine */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 2001-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine forms the front-end part of a generic PCS type of
	program.  This front-end is used in a variety of PCS programs.

	This subroutine was originally part of the Personal Communications
	Services (PCS) package but can also be used independently from it.
	Historically, this was developed as part of an effort to maintain high
	function (and reliable) email communications in the face of
	increasingly draconian security restrictions imposed on the computers
	in the DEFINITY development organization.


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
#include	<netdb.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<field.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<vecitem.h>
#include	<vecstr.h>
#include	<char.h>
#include	<ascii.h>
#include	<hostent.h>
#include	<dater.h>
#include	<sockaddress.h>
#include	<lfm.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"pinghost.h"
#include	"pingstatdb.h"
#include	"pingtab.h"
#include	"config.h"
#include	"defs.h"
#include	"configfile.h"


/* local defines */

#define	NDF	"/tmp/pingstat.deb"


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	getpwd(char *,int) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	getportnum(const char *,const char *) ;
extern int	getfname(const char *,const char *,int,char *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	bopenroot(bfile *,char *,char *,char *,char *,int) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			cchar *,cchar *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	gethename(const char *,struct hostent *,char *,int) ;
extern int	listenudp(int,const char *,const char *,int) ;
extern int	openport(int,int,int,SOCKADDRESS *) ;
extern int	initnow(struct timeb *,char *,int) ;
extern int	isasocket(int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const PIVARS *) ;
extern int	progpingtabadd(PROGINFO *,cchar *,int) ;
extern int	progconfigbegin(PROGINFO *) ;
extern int	progconfigend(PROGINFO *) ;
extern int	proginput(PROGINFO *,int) ;
extern int	progoutput(PROGINFO *,ARGINFO *,BITS *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*timestr_log(time_t,char *) ;


/* externals variables */

extern const char	pingstat_makedate[] ;


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;

static int	makedate_get(const char *,const char **) ;
static int	procopts(PROGINFO *,KEYOPT *) ;

static int	procpsdname(PROGINFO *,const char *) ;
static int	procdefportspec(PROGINFO *,char *,int,cchar *,cchar *,int) ;
static int	loadlocalnames(PROGINFO *) ;


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"LOGFILE",
	"CONFIG",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"lf",
	"db",
	"dgram",
	"host",
	"port",
	"logextra",
	"mu",
	NULL
} ;

enum argopts {
	argopt_tmpdir,
	argopt_version,
	argopt_verbose,
	argopt_root,
	argopt_logfile,
	argopt_config,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_lf,
	argopt_db,
	argopt_dgram,
	argopt_host,
	argopt_port,
	argopt_logextra,
	argopt_mu,
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
	{ 0, 0 },
} ;

static cchar	*progopts[] = {
	"defintminping",
	"intminping",
	"intminupdate",
	"marktime",
	"logfile",
	"sumfile",
	NULL
} ;

enum procopts {
	progopt_defintminping,
	progopt_intminping,
	progopt_intminupdate,
	progopt_marktime,
	progopt_logfile,
	progopt_sumfile,
	progopt_overlast
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo, *aip = &ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	USERINFO	u ;
	bfile		errfile ;
	bfile		sumfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		v ;
	int		logfile_type = -1 ;
	int		cl ;
	int		fd = FD_STDIN ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_makedate = FALSE ;
	int		f_usage = FALSE ;
	int		f_input = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*hfname = NULL ;
	const char	*psfname = NULL ;
	const char	*hostspec = NULL ;
	const char	*portspec = NULL ;
	const char	*cp ;
	char		userbuf[USERINFO_LEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->pagesize = getpagesize() ;

	pip->intrun = -1 ;
	pip->intmark = -1 ;
	pip->defintminping = -1 ;
	pip->intminping = -1 ;
	pip->intminupdate = -1 ;
	pip->intmininput = -1 ;
	pip->toping = -1 ;

	pip->f.logprog = OPT_LOGPROG ;

	rs = initnow(&pip->now,pip->zname,DATER_ZNAMESIZE) ;
	if (rs < 0) goto badinitnow ;

	if (rs >= 0)
	    rs = vecstr_start(&pip->pingtabs,0,0) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badtabstart ;
	}

/* start parsing the arguments */

	memset(aip,0,sizeof(ARGINFO)) ;
	aip->argc = argc ;
	aip->argv = (const char **) argv ;

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	aip->ai_max = 0 ;
	aip->ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

#if	CF_DEBUGS
	    debugprintf("main: a=>%t<\n",argp,argl) ;
#endif

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
	        const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp + 1) ;

	        } else if (ach == '-') {

	            aip->ai_pos = ai ;
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

#if	CF_DEBUGS
		    debugprintf("main: ak=>%t<\n",akp,akl) ;
#endif

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

	                case argopt_tmpdir:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->tmpdname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->tmpdname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* version */
	                case argopt_version:
	                    f_makedate = f_version ;
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
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* configuration file */
	                case argopt_config:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->cfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sn = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* argument-list filename */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                afname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                efname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* output filename */
	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ofname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* input file name */
	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ifname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ifname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
			    break ;

	                case argopt_db:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            psfname = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* log file */
	                case argopt_logfile:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            logfile_type = 0 ;
	                            pip->lfname = avp ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                logfile_type = 0 ;
	                                pip->lfname = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_dgram:
	                    pip->final.dgram = TRUE ;
	                    pip->f.dgram = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.dgram = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

	                case argopt_host:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            hostspec = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_port:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            portspec = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_logextra:
	                    pip->f.logextra = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.logextra = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_mu:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdecti(argp,argl,&v) ;
	                            pip->intminupdate = v ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

#if	CF_DEBUGS
			    debugprintf("main: kc=>%c< (%u)\n",kc,kc) ;
#endif

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

/* configuration file */
	                    case 'C':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'R':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* mutex lock PID file */
	                    case 'P':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->pfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* version */
	                    case 'V':
	                        f_makedate = f_version ;
	                        f_version = TRUE ;
	                        break ;

/* binary dump output file */
	                    case 'b':
	                        pip->f.binary = TRUE ;
	                        break ;

	                    case 'd':
	                        pip->f.daemon = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdecti(avp,avl,&v) ;
	                                pip->intrun = v ;
	                            }
	                        }
	                        break ;

/* hostspec */
	                    case 'h':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                hostspec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* input mode */
	                    case 'i':
	                        f_input = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdecti(avp,avl,&v) ;
	                                pip->intmininput = v ;
	                            }
	                        }
	                        break ;

/* default ping-interval */
	                    case 'm':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
	                                pip->intminping = v ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* no output */
	                    case 'n':
	                        pip->f.nooutput = TRUE ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = keyopt_loads(&akopts,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* accummulate the 'pingtab' files */
	                    case 'p':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = progpingtabadd(pip,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* quiet mode */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* receive mode */
	                    case 'r':
	                        pip->f.receive = TRUE ;
	                        break ;

/* 'pingstat' file name */
	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                psfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* over-ride ping-timeout */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
	                                if (v > 0) {
	                                    pip->have.toping = TRUE ;
	                                    pip->final.toping = TRUE ;
	                                    pip->toping = v ;
	                                }
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* update mode */
	                    case 'u':
	                        pip->f.update = TRUE ;
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

	        rs = bits_set(&pargs,ai) ;
	        aip->ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    aip->ai_pos = ai ;

	} /* end while (all command line argument processing) */

#if	CF_DEBUGS
	debugprintf("main: ai=%u\n",ai) ;
	debugprintf("main: ai_max=%u\n",aip->ai_max) ;
	debugprintf("main: ai_pos=%u\n",aip->ai_pos) ;
#endif

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUGS
	debugprintf("main: debuglevel=%u¹\n",pip->debuglevel) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u²\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    cchar	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: version %s\n",pn,VERSION) ;
	    if (f_makedate) {
	        cl = makedate_get(pingstat_makedate,&cp) ;
	        bprintf(pip->efp,"%s: makedate %t\n",pn,cp,cl) ;
	    }
	}

/* get our program root */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: con0\n") ;
#endif

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

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

/* help */

	if (f_help) {
	    if ((hfname == NULL) || (hfname[0] == '\0')) {
	        hfname = HELPFNAME ;
	    }
	    printhelp(NULL,pip->pr,pip->searchname,hfname) ;
	} /* end if (help) */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* more option initialization */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: con1\n") ;
#endif

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

/* argument defaults */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: con2\n") ;
#endif

/* get some host/user information */

	uc_sigignore(SIGPIPE) ;

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
	pip->logid = u.logid ;
	pip->uid = u.uid ;
	pip->gid = u.gid ;
	pip->pid = u.pid ;
	pip->euid = u.euid ;
	pip->egid = u.egid ;

	pip->ppid = pip->pid ;

/* get groupname */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: con3\n") ;
#endif

	if (rs >= 0) {
	    char	gbuf[GROUPNAMELEN + 1] ;
	    if ((rs = getgroupname(gbuf,GROUPNAMELEN,pip->gid)) >= 0) {
		cchar	**vpp = &pip->groupname ;
	        proginfo_setentry(pip,vpp,gbuf,rs) ;
	    }
	}

/* some other initialization */

	pip->daytime = time(NULL) ;

/* get the local names for this host */

	rs = vecstr_start(&pip->localnames,5,0) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

	rs = loadlocalnames(pip) ;
	if (rs < 0)
	    goto badlocalload ;

/* create the values for the file schedule searching */

	rs = vecstr_start(&pip->svars,6,0) ;
	pip->open.svars = (rs >= 0) ;
	if (rs < 0)
	    goto badvarsinit ;

	vecstr_envset(&pip->svars,"p",pip->pr,-1) ;

	vecstr_envset(&pip->svars,"e","etc",-1) ;

	vecstr_envset(&pip->svars,"n",pip->searchname,-1) ;

/* program configuration */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: con4\n") ;
	    debugprintf("main: cfn=%s\n",pip->cfname) ;
	}
#endif

	rs = progconfigbegin(pip) ;
	pip->f.config = (rs >= 0) ;
	if (rs < 0)
	    goto badprogconfigbegin ;

/* do we have a good default timeout value */

	if (pip->toping < 0)
	    pip->toping = TOPING ;

	if (pip->defintminping < 0)
	    pip->defintminping = INTMINPING ;

	if (pip->intmininput < 0)
	    pip->intmininput = INTMININPUT ;

	if (pip->intminupdate < 0)
	    pip->intminupdate = INTMINUPDATE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: toping=%d\n",pip->toping) ;
	    debugprintf("main: intminping=%d\n",pip->intminping) ;
	    debugprintf("main: intmininput=%d\n",pip->intmininput) ;
	    debugprintf("main: intminupdate=%d\n",pip->intminupdate) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: toping=%d\n",
	        pip->progname,pip->toping) ;
	    bprintf(pip->efp,"%s: intminping=%d\n",
	        pip->progname,pip->intminping) ;
	    bprintf(pip->efp,"%s: intminupdate=%d\n",
	        pip->progname,pip->intminupdate) ;
	}

	if (pip->workdname == NULL) {
	    pip->workdname = WORKDNAME ;

	} else if (pip->workdname[0] == '\0')
	    pip->workdname = "." ;

/* TMP directory */

	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0'))
	    pip->tmpdname = getenv(VARTMPDNAME) ;

	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0'))
	    pip->tmpdname = TMPDNAME ;

/* can we access the working directory? */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: access working directory \"%s\"\n",
	        pip->workdname) ;
#endif

/* do we have an activity log file? */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 0 logfname=%s\n",pip->lfname) ;
#endif

	if ((pip->lfname == NULL) || (pip->lfname[0] == '\0')) {
	    logfile_type = 1 ;
	    proginfo_setentry(pip,&pip->lfname,LOGFNAME,-1) ;
	}

	if (pip->lfname != NULL) {
	    cl = getfname(pip->pr,pip->lfname,logfile_type,tmpfname) ;
	    if (cl > 0) {
		cchar	**vpp = &pip->lfname ;
	        proginfo_setentry(pip,vpp,tmpfname,cl) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 1 logfname=%s\n",pip->lfname) ;
#endif

	rs1 = SR_NOENT ;
	if (pip->lfname != NULL) {
	    const char	*lfname = pip->lfname ;
	    const char	*logid = pip->logid ;
	    if ((rs1 = logfile_open(&pip->lh,lfname,0,0666,logid)) >= 0) {
	        pip->open.logprog = TRUE ;

	        if (pip->daytime == 0)
	            pip->daytime = time(NULL) ;

	        if (pip->logsize > 0)
	            logfile_checksize(&pip->lh,pip->logsize) ;

	        logfile_userinfo(&pip->lh,&u,pip->daytime,
	            pip->progname,pip->version) ;

	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: log=%s\n",
	                pip->progname,pip->lfname) ;

	    } /* end if (logfile-open) */
	} /* end if (we have a log file or not) */

/* find and open the PINGSTATDB file if we have one */

	if ((psfname == NULL) || (psfname[0] == '\0')) {
	    rs = mkpath3(tmpfname,pip->pr,VARDNAME,PSFNAME) ;
	    psfname = tmpfname ;
	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 2 psfname=%s\n",psfname) ;
#endif

	if ((rs >= 0) && (pip->f.update || f_input)) {
	    rs = procpsdname(pip,psfname) ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: ps=%s\n",
	        pip->progname,psfname) ;
	}

	if (pip->open.logprog)
	    logfile_printf(&pip->lh,"ps=%s\n",psfname) ;

	if (rs >= 0) {
	    int	oflags = (O_RDWR | O_CREAT) ;
	    rs = pingstatdb_open(&pip->ps,psfname,oflags,0666) ;
	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: pingstatdb_open() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badpingstat ;

/* before we go too far, are we the only one on this PID mutex? */

	if ((pip->pfname != NULL) && (pip->pfname[0] != '\0')) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we have a PIDFNAME=%s\n",pip->pfname) ;
#endif

/* we leave the file open as our mutex lock ! */

	    logfile_printf(&pip->lh,"pidfile=%s\n",pip->pfname) ;

	    logfile_printf(&pip->lh,"PID mutex captured\n") ;

	} /* end if (we have a mutex PID file) */

/* clean up some stuff we will no longer need */

	if (pip->open.svars) {
	    pip->open.svars = FALSE ;
	    vecstr_finish(&pip->svars) ;
	}

/* grab any PINGTABs passed in the environment variable */

	if ((! f_input) && ((cp = getenv(VARPINGTAB)) != NULL)) {

	    rs = progpingtabadd(pip,cp,-1) ;
	    if (rs <= 0)
	        goto badpingtab ;

	} /* end if (environment variable) */

/* open a summary file (if we have one) */

	if ((pip->sumfname != NULL) && (pip->sumfname[0] != '\0')) {

	    rs1 = bopen(&sumfile,pip->sumfname,"wca",0666) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: sumfname=%s rs=%d\n",
	            pip->sumfname,rs) ;
#endif

	    if (rs1 >= 0)
	        pip->sumfp = &sumfile ;

	    if ((rs1 < 0) && (! pip->f.quiet)) {

	        bprintf(pip->efp,
	            "%s: summary-file unavailable (%d)\n",
	            pip->progname,rs1) ;

	        logfile_printf(&pip->lh,
	            "summary-file unavailable (%d)\n",
	            rs1) ;

	    } /* end if (announcing failure) */

	} /* end if (opening the summary file) */

/* OK, now do whatever needed for the particular program mode we're in */

	if (f_input) {

	    if ((! pip->f.daemon) && (! pip->f.dgram) && isasocket(fd)) {
	        int	ans ;
	        int	anslen = sizeof(int) ;

	        rs1 = u_getsockopt(fd,SOL_SOCKET,SO_TYPE,&ans,&anslen) ;
	        if ((rs1 >= 0) && (anslen == sizeof(int))) {
	            if (ans == SOCK_DGRAM)
	                pip->f.dgram = TRUE ;
	        }

	    } /* end if (data-gram determination) */

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: mode=input\n", pip->progname) ;

	    if (pip->open.logprog) {
	        cp = (pip->f.dgram) ? "mode=input (dgram)" : "mode=input" ;
	        logfile_printf(&pip->lh,cp) ;
	    }

	    if (pip->f.daemon) {
	        const char	*dps = PORTSPEC_PINGSTAT ;
	        const char	*protoname = "udp" ;
	        const int	af = AF_INET ;
	        const int	dpn = PORT_PINGSTAT ;

	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: server\n", pip->progname) ;

	        if (pip->open.logprog)
	            logfile_printf(&pip->lh,"server") ;

	        pip->f.dgram = TRUE ;
	        if (pip->debuglevel == 0) {

	            bflush(pip->efp) ;

	            logfile_flush(&pip->lh) ;

	            rs = uc_fork() ;
	            if (rs == 0) {
	                int	i ;
	                for (i = 0 ; i < 3 ; i += 1) u_close(i) ;
	                u_setsid() ;
	            } else if (rs > 0)
	                uc_exit(EX_OK) ;

	        } /* end if */

	        if (rs >= 0) {
	            char	psbuf[SVCNAMELEN + 1] ;

	            if ((portspec == NULL) || (portspec[0] == '\0'))
	                portspec = dps ;

	            rs = procdefportspec(pip,psbuf,SVCNAMELEN,protoname,
	                portspec,dpn) ;

	            if (rs >= 0) {
	                rs = listenudp(af,hostspec,psbuf,0) ;
	                fd = rs ;
	            }
	        } /* end if */

	    } /* end if (daemon) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: proginput() \n") ;
#endif

	    if (rs >= 0) {
	        rs = proginput(pip,fd) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: proginput() rs=%d\n",rs) ;
#endif

	    if (pip->open.logprog && (rs < 0))
	        logfile_printf(&pip->lh,"exiting (%d)",rs) ;

	} else {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: progoutput() \n") ;
#endif

	    pip->afname = afname ;
	    pip->ofname = ofname ;
	    rs = progoutput(pip,aip,&pargs) ;
	    pan = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: progoutput() rs=%d\n",rs) ;
#endif

	} /* end if (program mode) */

/* close the summary file (if opened) */

	if (pip->sumfp != NULL) {
	    bclose(pip->sumfp) ;
	    pip->sumfp = NULL ;
	}

	if ((rs >= 0) && (! f_input) && (pan <= 0)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: no hosts or PINGTAB files specified\n") ;
#endif

	    rs = SR_INVALID ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,"%s: no hosts or PINGTAB files specified\n",
	        pip->progname) ;

	} /* end if (not log table file was specified or found) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: total hosts=%u processed=%u up=%u\n",
	        pip->c_hosts,pip->c_processed,pip->c_up) ;
#endif

	if ((rs >= 0) && (! f_input)) {

	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: total hosts=%u processed=%u up=%u\n",
	            pip->progname,
	            pip->c_hosts,pip->c_processed,pip->c_up) ;
	        bprintf(pip->efp,"%s: pingtabs=%u\n",
	            pip->progname,
	            pip->c_pingtabs) ;
	    } /* end if */

	    logfile_printf(&pip->lh,"hosts=%u processed=%u up=%u\n",
	        pip->c_hosts,pip->c_processed,pip->c_up) ;

	    ex = ((pip->c_hosts - pip->c_up) == 0) ? EX_OK : EX_NOHOST ;

	} /* end if (not in input-mode) */

badpingtab:
	pingstatdb_close(&pip->ps) ;

/* close some more earlier stuff */
badpingstat:
	if (pip->open.logprog) {
	    pip->open.logprog = FALSE ;
	    logfile_close(&pip->lh) ;
	}

	if (pip->f.config) {
	    pip->f.config = FALSE ;
	    progconfigend(pip) ;
	}

badprogconfigbegin:
	if (pip->open.svars) {
	    pip->open.svars = FALSE ;
	    vecstr_finish(&pip->svars) ;
	}

badvarsinit:
badlocalload:
	vecstr_finish(&pip->localnames) ;

badlocstart:
baduserinfo:

	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,"%s: invalid query (%d)\n",
	                pip->progname,rs) ;
	        }
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
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u rs=%d\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	vecstr_finish(&pip->pingtabs) ;

badtabstart:
badinitnow:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

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

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<host(s)>] [-p <pingtab(s)>] [-q]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-u] [-R <pr>] [-C <conf>] [-P <pidmutex>] [-db <db>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* get the date out of the ID string */
static int makedate_get(cchar *md,cchar **rpp)
{
	int		ch ;
	const char	*sp ;
	const char	*cp ;

	if (rpp != NULL) *rpp = NULL ;

	if ((cp = strchr(md,CH_RPAREN)) == NULL)
	    return SR_NOENT ;

	while (CHAR_ISWHITE(*cp))
	    cp += 1 ;

	ch = MKCHAR(*cp) ;
	if (! isdigitlatin(ch)) {
	    while (*cp && (! CHAR_ISWHITE(*cp))) cp += 1 ;
	    while (CHAR_ISWHITE(*cp)) cp += 1 ;
	} /* end if (skip over the name) */

	sp = cp ;
	if (rpp != NULL)
	    *rpp = cp ;

	while (*cp && (! CHAR_ISWHITE(*cp)))
	    cp += 1 ;

	return (cp - sp) ;
}
/* end subroutine (makedate_get) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getenv(VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	v ;
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(progopts,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {

	                case progopt_marktime:
	                    if (! pip->final.marktime) {
	                        pip->have.marktime = TRUE ;
	                        pip->final.marktime = TRUE ;
	                        pip->f.marktime = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.marktime = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case progopt_defintminping:
	                case progopt_intminping:
	                case progopt_intminupdate:
	                    rs1 = (vl > 0) ? cfdecti(vp,vl,&v) : SR_NOENT ;
	                    if (rs1 >= 0) {
	                        switch (oi) {
	                        case progopt_defintminping:
	                            if (! pip->final.defintminping) {
	                                pip->have.defintminping = TRUE ;
	                                pip->final.defintminping = TRUE ;
	                                pip->f.defintminping = TRUE ;
	                                pip->defintminping = v ;
	                            }
	                            break ;
	                        case progopt_intminping:
	                            if (! pip->final.intminping) {
	                                pip->have.intminping = TRUE ;
	                                pip->final.intminping = TRUE ;
	                                pip->f.intminping = TRUE ;
	                                pip->intminping = v ;
	                            }
	                            break ;
	                        case progopt_intminupdate:
	                            if (! pip->final.intminupdate) {
	                                pip->have.intminupdate = TRUE ;
	                                pip->final.intminupdate = TRUE ;
	                                pip->f.intminupdate = TRUE ;
	                                pip->intminupdate = v ;
	                            }
	                            break ;
	                        } /* end switch */
	                    } /* end if */
	                    break ;

	                case progopt_sumfile:
	                    break ;

	                case progopt_logfile:
	                    break ;

	                } /* end switch */
	                c += 1 ;
	            } /* end if (valid option) */

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procpsdname(PROGINFO *pip,cchar *psfname)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		cl  ;
	int		f_created = FALSE ;
	const char	*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (psfname == NULL) return SR_FAULT ;

	if (psfname[0] == '\0') return SR_INVALID ;

	if ((cl = sfdirname(psfname,-1,&cp)) > 0) {
	    if ((rs = mkpath1w(tmpfname,cp,cl)) >= 0) {
	        if (u_stat(tmpfname,&sb) == SR_NOENT) {
	            f_created = TRUE ;
	            rs = mkdirs(tmpfname,0777) ;
	        }
	    }
	}

	return (rs >= 0) ? f_created : rs ;
}
/* end subroutine (procpsdname) */


static int procdefportspec(pip,psbuf,pslen,protoname,dps,dpn)
PROGINFO	*pip ;
char		psbuf[] ;
int		pslen ;
const char	*protoname ;
const char	*dps ;
int		dpn ;
{
	int		rs ;
	if ((rs = getportnum(protoname,dps)) >= 0) {
	    rs = sncpy1(psbuf,pslen,dps) ;
	} else if (rs == SR_NOENT) {
	    rs = ctdeci(psbuf,pslen,dpn) ;
	}
	return rs ;
}
/* end subroutine (procdefportspec) */


static int loadlocalnames(PROGINFO *pip)
{
	HOSTENT		he ;
	const int	helen = getbufsize(getbufsize_he) ;
	const int	hlen = MAXPATHLEN ;
	int		rs ;
	int		rs1 ;
	int		i ;
	int		n = 0 ;
	const char	*np ;
	const char	*hnp ;
	char		hbuf[MAXHOSTNAMELEN + 1] ;
	char		*hebuf ;

	snsds(hbuf,hlen,pip->nodename,pip->domainname) ;

	if ((rs = uc_malloc((helen+1),&hebuf)) >= 0) {
	    for (i = 0 ; i < 2 ; i += 1) {

	        hnp = (i == 0) ? pip->nodename : hbuf ;
	        if ((rs1 = gethename(hnp,&he,hebuf,helen)) >= 0) {
	            HOSTENT_CUR	cur ;

	            if ((rs = hostent_curbegin(&he,&cur)) >= 0) {
	                while (hostent_enumname(&he,&cur,&np) >= 0) {
	                    if (vecstr_find(&pip->localnames,np) < 0) {
	                        rs = vecstr_add(&pip->localnames,np,-1) ;
	                    }
	                    if (rs < 0) break ;
	                } /* end while */
	                hostent_curend(&he,&cur) ;
	            } /* end if */

	        } /* end if (gethename) */

	        if (rs < 0) break ;
	    } /* end for */
	    rs1 = uc_free(hebuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (memory-allocation) */

	if (rs >= 0) {
	    n = vecstr_count(&pip->localnames) ;
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (loadlocalnames) */


