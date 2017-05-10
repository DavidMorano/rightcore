/* main */

/* generic front-end */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1


/* revision history:

	= 1992-03-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**********************************************************************

	This subroutine forms a program that sends data to a remote
	INET host to its 'echo' service.


***********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/utsname.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<time.h>
#include	<signal.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<getxusername.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	NPARG		2	/* number of positional arguments */

#undef	BUFLEN
#define	BUFLEN		((10 * 1024) + MAXHOSTNAMELEN)


/* external variables */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	matostr(const char **,int,const char *,int) ;

extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"LOGFILE",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_logfile,
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
	struct proginfo	pi, *pip = &pi ;

	USERINFO	u ;

	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	pan, ai ;
	int	rs = SR_OK ;
	int	rs1 ;
	int	argnum, i, len, sl, sl2, cl ;
	int	s, fd ;
	int	timeout = -1 ;
	int	logfile_type = -1, loglen = -1 ;
	int	fd_debug = -1 ;
	int	v ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_done = FALSE ;
	int	f_quiet = FALSE ;
	int	f_ignore = FALSE ;
	int	f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS + 1] ;
	char	buf[BUFLEN + 1] ;
	char	nodename[NODENAMELEN + 1], domainname[MAXHOSTNAMELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	logfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	srcpath[MAXPATHLEN + 1] ;
	char	hostnamebuf[MAXHOSTNAMELEN + 1] ;
	char	dialspecbuf[MAXHOSTNAMELEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*hostname = NULL ;
	const char	*portspec = NULL ;
	const char	*svcspec = NULL ;
	const char	*searchname = NULL ;
	const char	*sp, *cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	memset(pip,0,sizeof(struct proginfo)) ;

	pip->progname = strbasename(argv[0]) ;

	if ((rs = bopen(efp,BFILE_STDERR,"dwca",0666)) >= 0) {
		pip->f.errfile = TRUE ;
	    pip->efp = &errfile ;
	    bcontrol(efp,BC_LINEBUF,0) ;
	}

/* early initialization */

	pip->debuglevel = 0 ;
	pip->verboselevel = 1 ;

	pip->f.daemon = FALSE ;

	logfname[0] = '\0' ;

/* the arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((! f_done) && (argr > 0)) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                if (cfdeci(argp + 1,argl - 1,&argnum))
	                    goto badargval ;

	            } else {

#if	CF_DEBUGS
	                debugprintf("main: got an option\n") ;
#endif

	                aop = argp + 1 ;
	                akp = aop ;
	                aol = argl - 1 ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key w/ a value\n") ;
#endif

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + argl - 1 - avp ;
	                    aol = akl ;
	                    f_optequal = TRUE ;

	                } else {

	                    avl = 0 ;
	                    akl = aol ;

	                }

/* do we have a keyword match or should we assume only key letters ? */

	                if ((kwi = matostr(argopts,2,aop,aol)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option keyword, kwi=%d\n",
	                        kwi) ;
#endif

	                    switch (kwi) {

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->pr = avp ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pip->pr = argp ;

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

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)  {

	                                logfile_type = 0 ;
	                                strwcpy(logfname,argp,argl) ;

	                            }
	                        }

	                        break ;

	                    } /* end switch (key words) */

	                } else {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        debugprintf("main: option key letters\n") ;
#endif

	                        switch (*aop) {

	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {

	                                    rs = cfdeci(avp,avl, 
	                                        &pip->debuglevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            bprintf(efp,"%s: version %s\n",
	                                pip->progname,VERSION) ;

	                            break ;

	                        case 'd':
					pip->f.daemon = TRUE ;
	                            break ;

	                        case 'l':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

					cl = MIN(argl,(MAXPATHLEN - 1)) ;
	                                strwcpy(logfname,argp,cl) ;

	                            }

	                            break ;

	                        case 'p':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                portspec = argp ;

	                            break ;

	                        case 'q':
	                            f_quiet = TRUE ;
	                            break ;

	                        case 't':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdeci(argp,argl,&timeout) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }

	                            break ;

	                        case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {

	                                    rs = cfdeci(avp,avl, 
	                                        &pip->verboselevel) ;

	                                    if (rs < 0)
	                                        goto badargval ;

	                                }
	                            }

	                            break ;

	                        case '?':
					ex = EX_USAGE ;
	                            f_usage = TRUE ;
					break ;

	                        default:
					ex = EX_USAGE ;
	                            f_usage = TRUE ;
	                            bprintf(efp,"%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

	                        } /* end switch */

	                        akp += 1 ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digit or not) */

	        } else {

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGGROUPS) {

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

	if (rs < 0) goto badarg ;

#if	CF_DEBUGS
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
	debugprintf("main: past getting arguments f_usage=%d f_vesion=%d\n",
	    f_usage,f_version) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(efp,"%s: debuglevel=%u\n",pip->progname,pip->debuglevel) ;
	    bflush(efp) ;
	}

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    goto usage ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: checking arguments\n") ;
#endif

	pan = 0 ;			/* number of positional so far */
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) && (argv[i][0] != '\0')) 
			continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: arg=\"%s\"\n",argv[i]) ;
#endif

	        switch (pan) {

	        case 0:
	            hostname = argv[i] ;
	            break ;

	        case 1:
	            portspec = argv[i] ;
	            break ;

	        } /* end switch */

	        pan += 1 ;

	    } /* end for */

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking hostname=%s portspec=%s\n",
	        hostname,portspec) ;
#endif

/* other initialization */

	srcpath[0] = '\0' ;

	if ((logfname == NULL) || (logfname[0] == '\0')) {

	if ((cp = getenv(LOGFILEVAR)) != NULL)
		strwcpy(logfname,cp,(MAXPATHLEN - 1)) ;

	} 

/* parse some stuff */

	if (hostname != NULL) {

	    if ((cp = strchr(hostname,':')) != NULL) {

	        i = cp - hostname ;
	        strwcpy(hostnamebuf,hostname,(MAXHOSTNAMELEN - 1)) ;

	        hostnamebuf[i] = '\0' ;
	        hostname = hostnamebuf ;
	        if ((portspec == NULL) || (portspec[0] == '\0'))
	            portspec = hostnamebuf + i + 1 ;

	    } /* end if */

	} /* end if */

	if (portspec == NULL)
		portspec = PORTSPEC ;

/* get some host/user information */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;
	pip->logid = u.logid ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: UI name=%s\n",u.name) ;
#endif

	if (rs < 0) {

	    getnodedomain(nodename,domainname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: got node/domain\n") ;
#endif

	    pip->nodename = nodename ;
	    pip->domainname = domainname ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: about to get username\n") ;
#endif

	    getusername(buf,USERNAMELEN,-1) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: got username\n") ;
#endif

	    pip->username = mallocstr(buf) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: done w/ name stuff\n") ;
#endif

	} /* end if (got some user information or not) */

/* check other stuff */

	if (timeout < 0)
		timeout = TO_RUN ;

/* does the log file name need massaging ? */

	rs = SR_BAD ;
	sl2 = -1 ;
	if (logfname[0] == '\0') {

	    logfile_type = 1 ;
	    sl2 = expander(pip,LOGFNAME,-1,logfname,MAXPATHLEN) ;

	}

	sl = getfname(pip->pr,logfname,logfile_type,tmpfname) ;

	if (sl > 0)
	    strwcpy(logfname,tmpfname,sl) ;

	rs = logfile_open(&pip->lh,logfname,0,0666,pip->logid) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if (rs >= 0) {

	    struct utsname	un ;


	    pip->f.log = TRUE ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we opened a logfile\n") ;
#endif

	    pip->f.log = TRUE ;
	    if (pip->debuglevel > 0)
	        bprintf(efp,"%s: logfile=%s\n",pip->progname,logfname) ;

/* we opened it, maintenance this log file if we have to */

	    if (loglen < 0)
	        loglen = LOGSIZE ;

	    logfile_checksize(&pip->lh,loglen) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we checked its length\n") ;
#endif

	    pip->daytime = time(NULL) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: making log entry\n") ;
#endif

	    if (pip->f.daemon) {

	        logfile_printf(&pip->lh,"%s %s %s\n",
	            timestr_logz(pip->daytime,timebuf),
	            BANNER,
	            "started") ;

	        logfile_printf(&pip->lh,"%-14s %s/%s\n",
	            pip->progname,
	            VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	    } else
	        logfile_printf(&pip->lh,"%s %-14s %s/%s\n",
	            timestr_logz(pip->daytime,timebuf),
	            pip->progname,
	            VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	    (void) u_uname(&un) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s) domain=%s\n",
	        (u.f.sysv_rt ? "SYSV" : "BSD"),
	        un.sysname,un.release,
	        pip->domainname) ;

	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        bufprintf(buf,BUFLEN,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        bufprintf(buf,BUFLEN,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        bufprintf(buf,BUFLEN,"(%s)",u.fullname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: LF name=%s\n",buf) ;
#endif

	    logfile_printf(&pip->lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;

#ifdef	COMMENT
	    logfile_printf(&pip->lh,"configfile=%s\n",configfname) ;
#endif

	    logfile_printf(&pip->lh,"pr=%s\n",pip->pr) ;

	} /* end if (we have a log file or not) */




/* do it */

	rs = process(pip,hostname,portspec,timeout) ;



done:
	ex = EX_OK ;

	if (rs < 0) {

	    ex = EX_DATAERR ;

		if (! pip->f.quiet)
	    bprintf(efp,"%s: exiting bad rs=%d\n",
	        pip->progname,rs) ;

	}

	if (pip->f.log)
		logfile_close(&pip->lh) ;

retearly:
ret4:

ret3:

ret2:
	if (srcpath[0] != '\0')
	    u_unlink(srcpath) ;

#ifdef	COMMENT
	bclose(ofp) ;
#endif

badret:
ret1:
	bclose(efp) ;

ret0:
badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* handle some other stuff */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-V] [-s] [-q] [-i] [-v] timehost(s)",
	    pip->progname,pip->progname) ;

	bprintf(efp,
	    " [-l logfile] [-D]\n") ;

	goto retearly ;

/* handle bad arguments */
badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badextra:
	bprintf(efp,"%s: extra command line arguments ignored\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value specified\n",
	    pip->progname) ;

	goto badarg ;

badhost:
	bprintf(efp,"%s: no host was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto retearly ;

/* handle other things */
badconnect:
	ex = EX_UNAVAILABLE ;
	bprintf(efp,"%s: could not connect to host (%d)\n",
	    pip->progname,rs) ;

	goto ret2 ;

}
/* end subroutine (main) */


/* local subroutines */


