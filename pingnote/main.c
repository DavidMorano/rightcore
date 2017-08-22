/* main */

/* program to send echo data to a remote host (and back) */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 2001-03-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine forms a program that tests the input-update capability
        (at least a little bit) of the PINGSTAT program.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<netinet/in.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<sockaddress.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<dater.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"pingstatmsg.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	NPARG		2	/* number of positional arguments */

#define	TO_READ		30

#undef	BUFLEN
#define	BUFLEN		((10 * 1024) + MAXHOSTNAMELEN)
#define	MAXLEN		BUFLEN

#define	TAG		23
#define	DURATION	15

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	DENOM		(1000 * 1000)


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncat2(char *,int,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(const char *,int) ;
extern int	initnow(struct timeb *,char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getportnum(const char *,const char *) ;
extern int	dialudp(const char *,const char *,int,int,int) ;
extern int	dialtcp(const char *,const char *,int,int,int) ;
extern int	dialtcpnls(const char *,const char *,int,const char *,int,int) ;
extern int	dialtcpmux(const char *,const char *,int,const char *,
			const char **,int,int) ;
extern int	dialuss(const char *,int,int) ;
extern int	dialusd(const char *,int,int) ;
extern int	dialticotsordnls(const char *,int,const char *,int,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;
extern int	dater_setkey(DATER *,const char *,int,struct timeb *,
			const char *) ;
extern int	isdigitlatin(int) ;

extern int	proginfo_setpiv(PROGINFO *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procmsg(PROGINFO *,char *,int,const char *,int) ;
static int	procout(PROGINFO *,const char *,const char *,int) ;
static int	procdialer(PROGINFO *, const char *,const char *,
			const char *,const char *, const char *,int) ;

static int	procdefportspec(PROGINFO *,char *,int,
			const char *,const char *,int) ;

#ifdef	COMMENT
static int bprintmsg(bfile *,char *,int) ;
#endif


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"HELP",
	"lf",
	"of",
	"cd",
	"port",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_help,
	argopt_lf,
	argopt_of,
	argopt_cd,
	argopt_port,
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

static const char	*akonames[] = {
	"domain",
	NULL
} ;

enum akonames {
	akoname_domain,
	akoname_overlast
} ;

static const char	*dialers[] = {
	"tcp",
	"tcpmux",
	"tcpnls",
	"udp",
	"uss",
	"usd",
	"ticotsordnls",
	NULL
} ;

enum dialers {
	dialer_tcp,
	dialer_tcpmux,
	dialer_tcpnls,
	dialer_udp,
	dialer_uss,
	dialer_usd,
	dialer_ticotsordnls,
	dialer_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	PROGINFO	pi, *pip = &pi ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	bfile		errfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan = 0 ;
	int	rs = SR_OK ;
	int	i ;
	int	ml ;
	int	count = 0 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;
	int	f_usage = FALSE ;
	int	f_outfile = FALSE ;
	int	f_pinghost = FALSE ;
	int	f ;

	const char	*poh = PO_PINGHOST ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS + 1] ;
	char	msgbuf[MSGBUFLEN + 1] ;
	char	srcpath[MAXPATHLEN + 1] ;
	char	localhost[MAXHOSTNAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	hostnamebuf[MAXHOSTNAMELEN + 1] ;
	char	dialspecbuf[MAXHOSTNAMELEN + 1] ;
	const char	*pr = NULL ;
	const char	*logfname = NULL ;
	const char	*ofname = NULL ;
	const char	*pinghost = NULL ;
	const char	*dialspec = NULL ;
	const char	*hostname = NULL ;
	const char	*portspec = NULL ;
	const char	*svcspec = NULL ;
	const char	*datestr = NULL ;
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
	    goto ret0 ;
	}

	proginfo_setbanner(pip,BANNER) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->f.errfile = TRUE ;
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* early initialization */

	pip->verboselevel = 1 ;
	pip->to = -1 ;

/* the arguments */

	if (rs >= 0) {
	    rs = paramopt_start(&aparams) ;
	    pip->open.akopts = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = paramopt_start(&aparams) ;
	    pip->open.aparams = (rs >= 0) ;
	}

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

/* change date */
	                case argopt_cd:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }

	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        datestr = argp ;

	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

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

/* handle all keyword defaults */
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

/* target port */
	                case argopt_port:
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

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    bprintf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
			    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl, 
	                                    &pip->debuglevel) ;

	                        }

	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* count */
	                    case 'c':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            rs = cfdeci(argp,argl,&count) ;

	                        break ;

	                    case 'd':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            dialspec = argp ;

	                        break ;

/* pinghost */
	                    case 'h':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) {
	                            rs = paramopt_loads(&aparams,poh,
	                            	argp,argl) ;
				    f_pinghost = (rs > 0) ;
				}

	                        break ;

	                    case 'l':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            logfname = argp ;

	                        break ;

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

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

	                    case 's':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            svcspec = argp ;

	                        break ;

	                    case 't':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            rs = cfdecti(argp,argl,&pip->to) ;

	                        break ;

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

	        } /* end if (digits or progopts) */

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

	    bflush(pip->efp) ;

	}

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    usage(pip) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pr=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    bprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	}

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some initialization */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (count < 0)
	    count = 0 ;

	initnow(&pip->now,pip->zname,DATER_ZNAMESIZE) ;

	rs = dater_start(&pip->tmpdate,&pip->now,pip->zname,-1) ;
	if (rs < 0)
	    goto badinitdate ;

/* did the user specify a date? */

	if ((datestr != NULL) && (datestr[0] != '\0')) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: datestr=%s\n",datestr) ;
#endif

	    rs = dater_setkey(&pip->tmpdate,datestr,-1,
	        &pip->now,pip->zname) ;

#if	CF_DEBUG
	    if (pip->debuglevel >= 3)
	        debugprintf("main: dater_setkey() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        pip->f.cdate = TRUE ;
	        rs = dater_startcopy(&pip->cdate,&pip->tmpdate) ;
	    }

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: f_cdate=%d\n",pip->f.cdate) ;
#endif

	} /* end if (user specified date) */

	if (rs < 0)
	    goto badsetdate ;

/* ping-host */

	if (! f_pinghost) {
	    if ((pinghost == NULL) || (pinghost[0] == '\0')) {

	        rs = getnodedomain(localhost,domainname) ;
	        if ((rs >= 0) && pip->f.domain)
	            rs = sncat2(localhost,MAXHOSTNAMELEN,".",domainname) ;

	        pinghost = localhost ;

	    } /* end if (pinghost) */
	    rs = paramopt_loads(&aparams,poh,pinghost,-1) ;
	} /* end if */

	if (rs < 0)
	    goto badpinghost ;

/* get the positional arguments */

	pan = 0 ;			/* number of positional so far */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    switch (pan) {

	    case 0:
	        hostname = cp ;
	        break ;

	    case 1:
	        portspec = cp ;
	        break ;

	    } /* end switch */

	    pan += 1 ;

	} /* end for (looping through requested circuits) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checking hostname=%s portspec=%s\n",
	        hostname,portspec) ;
#endif

/* other initialization */

	srcpath[0] = '\0' ;

	if ((ofname != NULL) && (ofname[0] != '\0'))
		f_outfile = TRUE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: f_outfile=%u\n",f_outfile) ;
#endif

/* parse some stuff */

	if (hostname != NULL) {

	    if ((cp = strchr(hostname,':')) != NULL) {

	        i = cp - hostname ;
	        strwcpy(hostnamebuf,hostname,MAXHOSTNAMELEN) ;

	        hostnamebuf[i] = '\0' ;
	        hostname = hostnamebuf ;
	        if ((portspec == NULL) || (portspec[0] == '\0'))
	            portspec = hostnamebuf + i + 1 ;

	    } /* end if */

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checking dialspec=%s\n",dialspec) ;
#endif

	if ((dialspec != NULL) && (dialspec[0] != '\0')) {

	    if ((cp = strchr(dialspec,':')) != NULL) {

	        i = cp - dialspec ;
	        strwcpy(dialspecbuf,dialspec,MAXHOSTNAMELEN) ;

	        dialspecbuf[i] = '\0' ;
	        dialspec = dialspecbuf ;
	        if ((portspec == NULL) || (portspec[0] == '\0'))
	            portspec = dialspecbuf + i + 1 ;

	    } /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("main: dialspec=%s portspec=%s\n",
	            dialspec,portspec) ;
#endif

	} /* end if */

/* perform */

	{
	    PARAMOPT_CUR	pcur ;

	    int		phl ;

	    char	*php ;


	    paramopt_curbegin(&aparams,&pcur) ;

	    while (rs >= 0) {

	        phl = paramopt_fetch(&aparams,PO_PINGHOST,&pcur,&php) ;
	 	if (phl < 0)
		    break ;

		if (phl == 0) continue ;

		if (pip->debuglevel > 0)
		    bprintf(pip->efp,"%s: pinghost=%s\n",
			pip->progname,php) ;

	        rs = procmsg(pip,msgbuf,MSGBUFLEN,php,count) ;
	        ml = rs ;
	        if (rs >= 0) {

	            if (f_outfile)
	                rs = procout(pip,ofname,msgbuf,ml) ;

	            else
	                rs = procdialer(pip,dialspec,hostname,portspec,svcspec,
	                    msgbuf,ml) ;

	        } /* end if */

	    } /* end while */

	    paramopt_curend(&aparams,&pcur) ;

	} /* end block */

badmsg:
badpinghost:
badsetdate:
	if (pip->f.cdate)
	    dater_finish(&pip->cdate) ;

	dater_finish(&pip->tmpdate) ;

badinitdate:
done:
	if ((rs < 0) && (ex == EX_OK)) {

	    ex = mapex(mapexs,rs) ;

	    switch (rs) {

	    case SR_BUSY:
	        break ;

	    default:
	        if (! pip->f.quiet)
	            bprintf(pip->efp,
	                "%s: could not perform function (%d)\n",
	                pip->progname,rs) ;

	    } /* end switch */

	} /* end if */

retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u rs=%d\n",ex,rs) ;
#endif

ret4:
	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&aparams) ;
	}

ret3:
	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

ret2:
	bclose(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* handle bad arguments */
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
PROGINFO	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s <rhost> [-d <dialer>] [-h <name>]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s:  [-lf <logfile>] [-D] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(pip,kop)
PROGINFO	*pip ;
KEYOPT		*kop ;
{
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

	if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {

	while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

/* get the first value for this key */

	    vl = keyopt_fetch(kop,kp,NULL,&vp) ;

/* do we support this option? */

	    if ((oi = matostr(akonames,2,kp,kl)) >= 0) {
	        uint	uv ;

	        switch (oi) {

	        case akoname_domain:
	            if (! pip->final.domain) {
	                pip->have.domain = TRUE ;
	                pip->final.domain = TRUE ;
	                pip->f.domain = TRUE ;
	                if (vl > 0) {
			    rs = optbool(vp,vl) ;
	                    pip->f.domain = (rs > 0) ;
			}
	            }
	            break ;

	        } /* end switch */

	        c += 1 ;

	    } /* end if (valid option) */

	    if (rs < 0) break ;
	} /* end while (looping through key options) */

	keyopt_curend(kop,&kcur) ;
	} /* end if (cursor) */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procmsg(pip,mbuf,mlen,pinghost,count)
PROGINFO	*pip ;
char		mbuf[] ;
int		mlen ;
const char	pinghost[] ;
int		count ;
{
	int	rs = SR_OK ;
	int	ml = 0 ;

	const char	*mt = NULL ;

	char	*cp ;


	if (pinghost == NULL)
	    return SR_FAULT ;

	if (pinghost[0] == '\0')
	    return SR_INVALID ;

/* fill in the message fields */

	if (pip->f.cdate || (count > 0)) {

	    struct pingstatmsg_uptime	m1 ;

	    time_t	timechange ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procmsg: msg->uptime\n") ;
#endif

	    mt = "uptime" ;
	    memset(&m1,0,sizeof(struct pingstatmsg_uptime)) ;

/* the change date and count */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procmsg: count=%d\n",count) ;
#endif

	    timechange = 0 ;
	    if (pip->f.cdate)
	        dater_gettime(&pip->cdate,&timechange) ;

	    m1.timechange = timechange ;
	    m1.count = count ;

/* the regular stuff */

	    m1.timestamp = (uint) pip->daytime ;
	    cp = strwcpy(m1.hostname,pinghost,MAXHOSTNAMELEN) ;
	    m1.hostnamelen = (cp - m1.hostname) ;

	    ml = pingstatmsg_uptime(&m1,0,mbuf,mlen) ;

	} else {

	    struct pingstatmsg_update	m0 ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procmsg: msg->update\n") ;
#endif

	    mt = "update" ;
	    memset(&m0,0,sizeof(struct pingstatmsg_update)) ;

	    m0.timestamp = (uint) pip->daytime ;
	    cp = strwcpy(m0.hostname,pinghost,MAXHOSTNAMELEN) ;
	    m0.hostnamelen = (cp - m0.hostname) ;

	    ml = pingstatmsg_update(&m0,0,mbuf,mlen) ;

	} /* end if */

	if ((rs >= 0) && (pip->debuglevel > 0) && (mt != NULL))
	    bprintf(pip->efp,"%s: mt=%s (%u)\n",
		pip->progname,mt,ml) ;

ret0:

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procmsg: ret rs=%d ml=%u\n",rs,ml) ;
#endif

	return (rs >= 0) ? ml : rs ;
}
/* end subroutine (procmsg) */


static int procout(pip,ofname,mbuf,mlen)
PROGINFO	*pip ;
const char	ofname[] ;
const char	mbuf[] ;
int		mlen ;
{
	bfile	outfile, *ofp = &outfile ;

	int	rs ;
	int	wlen = 0 ;

	const char	*ostr ;


	if (ofname == NULL)
	    return SR_FAULT ;

	if (ofname == '\0')
	    return SR_INVALID ;

	if (mbuf == NULL)
	    return SR_FAULT ;

	if (mlen < 0)
	    return SR_INVALID ;

	ostr = "wct" ;
	if (ofname[0] == '-') {
	    ofname = BFILE_STDOUT ;
	    ostr = "dwct" ;
	}

	rs = bopen(ofp,ofname,ostr,0666) ;
	if (rs < 0)
	    goto ret0 ;

	rs = bwrite(ofp,mbuf,mlen) ;

	bclose(ofp) ;

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int procdialer(pip,dialspec,hostname,portspec,svcspec,mbuf,mlen)
PROGINFO	*pip ;
const char	dialspec[] ;
const char	hostname[] ;
const char	portspec[] ;
const char	svcspec[] ;
const char	mbuf[] ;
int		mlen ;
{
	int	rs = SR_OK ;
	int	s ;
	int	af = AF_INET ;
	int	timeout = pip->to ;
	int	dialer = -1 ;
	int	dpn = PORT_PINGSTAT ;
	int	wlen = 0 ;

	const char	*dps = PORTSPEC_PINGSTAT ;
	const char	*protoname ;

	char	portbuf[SVCNAMELEN + 1] ;
	char	srcpath[MAXPATHLEN + 1] ;


	if (mbuf == NULL)
	    return SR_FAULT ;

	if (mlen < 0)
	    return SR_INVALID ;

	srcpath[0] = '\0' ;
	if (timeout <= 0)
	    timeout = TO_DIAL ;

/* find the dialer */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: find dialspec=%s\n",dialspec) ;
#endif

	dialer = dialer_udp ;
	if ((dialspec != NULL) && (dialspec[0] != '\0'))
	    dialer = matostr(dialers,2,dialspec,-1) ;

	if (dialer < 0) {
	    rs = SR_UNATCH ;
	    goto badnodial ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: dialer=%d\n",dialer) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: portspec=%s\n",portspec) ;
	    debugprintf("main: svcspec=%s\n",svcspec) ;
	}
#endif /* CF_DEBUG */

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: dialer=%s(%u)\n",
		pip->progname,dialers[dialer],dialer) ;

/* look up some miscellaneous stuff in various databases */

	switch (dialer) {

	case dialer_tcp:
	case dialer_udp:
	    if ((hostname == NULL) || (hostname[0] == '\0'))
	        rs = SR_INVALID ;

	    if ((portspec == NULL) || (portspec[0] == '\0')) {

	        if ((svcspec != NULL) && (svcspec[0] != '\0'))
	            portspec = svcspec ;

	        if (portspec == NULL) {
		    protoname = (dialer == dialer_tcp) ? "tcp" : "udp" ;
	            portspec = portbuf ;
		    rs = procdefportspec(pip,portbuf,SVCNAMELEN,
			protoname,dps,dpn) ;
		}

	    }

	    break ;

	case dialer_tcpmux:
	case dialer_tcpnls:
	    if ((hostname == NULL) || (hostname[0] == '\0'))
	        rs = SR_INVALID ;

	    break ;

	} /* end switch */

	if (rs < 0)
	    goto baddial ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: portspec=%s\n",
		pip->progname,portspec) ;

	switch (dialer) {

	case dialer_tcpmux:
	case dialer_tcpnls:
	case dialer_ticotsordnls:
	    if ((svcspec == NULL) || (svcspec[0] == '\0'))
	        svcspec = SVCSPEC_PINGSTAT ;

	    break ;

	} /* end switch */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: 2 dialer=%s(%d)\n",dialers[dialer],dialer) ;
#endif

	switch (dialer) {

	case dialer_tcp:
	    rs = dialtcp(hostname,portspec,af,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_tcpmux:
	    rs = dialtcpmux(hostname,portspec,af,svcspec,NULL,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_tcpnls:
	    rs = dialtcpnls(hostname,portspec,af,svcspec,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_udp:
	    rs = dialudp(hostname,portspec,af,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_ticotsordnls:
	    if ((portspec == NULL) || (portspec[0] == '\0'))
	        portspec = hostname ;

	    rs = dialticotsordnls(portspec,-1,svcspec,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_uss:
	    if ((portspec == NULL) || (portspec[0] == '\0'))
	        portspec = hostname ;

	    rs = dialuss(portspec,timeout,0) ;
	    s = rs ;
	    break ;

	case dialer_usd:
	    if ((portspec == NULL) || (portspec[0] == '\0'))
	        portspec = hostname ;

	        rs = dialusd(portspec,timeout,1) ;
	        s = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	        debugprintf("main: dialusd() rs=%d\n",rs) ;
#endif

	    break ;

	} /* end switch (dialer selection) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: 3 dialer rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badconnect ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: connected host=%s\n",
	        pip->progname,hostname) ;

	if (rs >= 0) {
	    rs = u_write(s,mbuf,mlen) ;
	    wlen = rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: u_write() rs=%d \n",rs) ;
#endif

	if (s >= 0) {
	    u_close(s) ;
	    s = -1 ;
	}

badconnect:
	if (srcpath[0] != '\0') {
	    u_unlink(srcpath) ;
	    srcpath[0] = '\0' ;
	}

baddial:
badnodial:
ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procdialer) */


static int procdefportspec(pip,psbuf,pslen,protoname,dps,dpn)
PROGINFO	*pip ;
char		psbuf[] ;
int		pslen ;
const char	*protoname ;
const char	*dps ;
int		dpn ;
{
	int	rs ;

	if ((rs = getportnum(protoname,dps)) >= 0) {
	    rs = sncpy1(psbuf,pslen,dps) ;
	} else if (rs == SR_NOENT) {
	    rs = ctdeci(psbuf,pslen,dpn) ;
	}

	return rs ;
}
/* end subroutine (procdefportspec) */


#ifdef	COMMENT

static int bprintmsg(ofp,buf,buflen)
bfile	*ofp ;
char	buf[] ;
int	buflen ;
{
	struct sysmisc_request	m0 ;

	struct sysmisc_loadave	m1 ;

	int	rs ;
	int	type ;

	char	timebuf[TIMEBUFLEN + 1] ;


	type = (int) ((uint) buf[0]) ;

	switch (type) {

	case sysmisctype_request:
	    rs = sysmisc_request(buf,buflen,1,&m0) ;

	    bprintf(ofp,"msglen=%d\n",rs) ;

	    type = m0.type ;
	    bprintf(ofp,"type=%d\n",type) ;

	    bprintf(ofp,"tag=%d\n",m0.tag) ;

	    bprintf(ofp,"duration=%d\n",m0.duration) ;

	    bprintf(ofp,"interval=%d\n",m0.interval) ;

	    bprintf(ofp,"addrfamily=%04x\n",m0.addrfamily) ;

	    bprintf(ofp,"addrport=%04x\n",m0.addrport) ;

	    bprintf(ofp,"addrhost0=%04x\n",m0.addrhost[0]) ;

	    bprintf(ofp,"addrhost1=%04x\n",m0.addrhost[1]) ;

	    bprintf(ofp,"addrhost2=%04x\n",m0.addrhost[2]) ;

	    bprintf(ofp,"addrhost3=%04x\n",m0.addrhost[3]) ;

	    bprintf(ofp,"opts=%04x\n",m0.opts) ;

	    break ;

	case sysmisctype_loadave:
	    rs = sysmisc_loadave(buf,buflen,1,&m1) ;

	    bprintf(ofp,"msglen=%d\n",rs) ;

	    type = m1.type ;
	    bprintf(ofp,"type=%d\n",type) ;

	    bprintf(ofp,"tag=%d\n",m1.tag) ;

	    bprintf(ofp,"rc=%d\n",m1.rc) ;

	    bprintf(ofp,"timestamp=%s\n",
	        timestr_log(m1.timestamp,timebuf)) ;

	    bprintf(ofp,"provider=%d\n",m1.provider) ;

	    bprintf(ofp,"hostid=%d\n",m1.hostid) ;

	    bprintf(ofp,"la_1min=%d\n",m1.la_1min) ;

	    bprintf(ofp,"la_5min=%d\n",m1.la_5min) ;

	    bprintf(ofp,"la_15min=%d\n",m1.la_15min) ;

	} /* end subroutine */

	return rs ;
}
/* end subroutine (bprintmsg) */

#endif /* COMMENT */



