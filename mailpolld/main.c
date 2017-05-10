/* main */

/* front-end (everything) for MAILPOLLD */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1999-02-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	Synopsis:

	$ mailpolld program


*************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<sys/uio.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<lfm.h>
#include	<sockaddress.h>
#include	<connection.h>
#include	<exitcodes.h>
#include	<mallocstuff.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"mcmsg.h"
#include	"msgid.h"



/* local defines */

#define	NPARG		2	/* number of positional arguments */
#define	MAXARGINDEX	100

#define	NARGPRESENT	(MAXARGINDEX/8 + 1)
#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	USERNAMELEN
#define	USERNAMELEN	32
#endif

#ifndef	JOBIDLEN
#define	JOBIDLEN	32
#endif

#define	TO		10
#define	NIOVECS		10
#define	POLLINT		10



/* external subroutines */

extern int	nextfield(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	listenusd(const char *,int,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	makedate[] ;


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;
static int	helpfile(const char *,bfile *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"DEBUG",
	"VERSION",
	"VERBOSE",
	"HELP",
	"LOG",
	"MAKEDATE",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_debug,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_log,
	argopt_makedate,
	argopt_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	struct msghdr	mh ;

	struct pollfd	fds[3] ;

	struct iovec	vecs[NIOVECS] ;

	struct ustat	sb ;

	USERINFO	u ;

	LFM		lk ;

	SOCKADDRESS	from ;

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		infile, *ifp = &infile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan, npa ;
	int	rs = SR_OK ;
	int	rs1 ;
	int	len, i ;
	int	nfds, nfd, to_poll ;
	int	size, blen, sl, re ;
	int	loglen ;
	int	fromlen ;
	int	cl ;
	int	to = -1 ;
	int	fd_stdout, fd_ipc ;
	int	fd_debug = -1 ;
	int	ex = EX_USAGE ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[NARGPRESENT] ;
	char	buf[BUFLEN + 1] ;
	char	tmpbuf[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	char	peername[SOCKADDRESS_NAMELEN + 1] ;
	char	jobidbuf[JOBIDLEN + 1] ;
	char	timebuf[TIMEBUFLEN] ;
	const char	*pr = NULL ;
	const char	*searchname = NULL ;
	const char	*jobid = NULL ;
	const char	*logfname = NULL ;
	const char	*hostname = NULL ;
	const char	*portspec = NULL ;
	const char	*progpath = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;
	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;
#if	CF_DEBUGS
	debugprintf("main: fd_debug=%d\n",fd_debug) ;
#endif
	#endif


	memset(pip,0,sizeof(struct proginfo)) ;

	pip->banner = BANNER ;
	pip->progname = strbasename(argv[0]) ;

	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
	    u_close(2) ;
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

	pip->pr = NULL ;
	pip->helpfname = NULL ;

	pip->debuglevel = 0 ;
	pip->verboselevel = 1 ;

	pip->f.quiet = FALSE ;

/* process program arguments */

	for (ai = 0 ; ai < NARGPRESENT ; ai += 1)
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
	                            pip->pr = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pip->pr = argp ;

	                    }

	                    break ;

/* debug level */
	                case argopt_debug:
	                    pip->debuglevel = 1 ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            rs = cfdeci(avp,avl,
	                                &pip->debuglevel) ;

	                    }

	                    break ;

	                case argopt_version:
	                    f_version = TRUE ;
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

/* help file */
	                case argopt_help:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->helpfname = avp ;

	                    }

	                    f_help  = TRUE ;
	                    break ;

/* log file */
	                case argopt_log:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            logfname = avp ;

	                    }

	                    break ;

/* display the time this program was last "made" */
	                case argopt_makedate:
	                    f_makedate = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
			    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            rs = cfdeci(avp,avl, 
	                                &pip->debuglevel) ;

	                        }

	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

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

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: finished parsing arguments\n",
	        pip->progname) ;

/* continue w/ the trivia argument processing stuff */

	if (f_version) {

	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

#ifdef	COMMENT
	    bprintf(pip->efp,"%s: built %s\n",
	        pip->progname,makedate) ;
#endif

	}

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto earlyret ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

/* get our program root */

	if (pip->pr == NULL)
	    pip->pr = getenv(VARPROGRAMROOT1) ;

	if (pip->pr == NULL)
	    pip->pr = getenv(VARPROGRAMROOT2) ;

	if (pip->pr == NULL)
	    pip->pr = getenv(VARPROGRAMROOT3) ;

	if (pip->pr == NULL)
	    pip->pr = PROGRAMROOT ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: initial programroot=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: programroot=%s\n",
	        pip->progname,pip->pr) ;


	if (pip->searchname == NULL)
	    pip->searchname = getenv(VARSEARCHNAME) ;

	if (pip->searchname == NULL)
	    pip->searchname = SEARCHNAME ;


	if (f_help) {

	    if (pip->helpfname == NULL) {

	        blen = bufprintf(buf,BUFLEN,"%s/%s",
	            pip->pr,HELPFNAME) ;

	        pip->helpfname = (char *) mallocstrw(buf,blen) ;

	    }

	    helpfile(pip->helpfname,pip->efp) ;

	    goto earlyret ;

	} /* end if */

/* who are we ? */

	rs = userinfo(&u,userinfobuf,USERINFO_LEN,NULL) ;
	if (rs < 0) {
	    ex = EX_NOUSER ;
	    goto baduser ;
	}

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;

	pip->pid = u.pid ;

	pip->daytime = time(NULL) ;

/* do the main thing */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking for positional arguments\n") ;
#endif

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	            switch (pan) {

	            case 0:
	                progpath = argv[ai] ;
	                break ;

	            } /* end switch */

	            pan += 1 ;

	    } /* end for (loading positional arguments) */

/* other initialization */

	fd_stdout = FD_STDOUT ;

	if ((hostname == NULL) || (hostname[0] == '\0'))
	    hostname = "rca" ;

	if ((portspec == NULL) || (portspec[0] == '\0'))
	    portspec = "echo" ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: portspec=%s\n", portspec) ;
#endif

/* some more initialization */

	if (to < 0)
		to = 5 ;

/* possibly clean up the user specified JOB ID */

	if (jobid != NULL) {

	    cp = jobid ;
	    i = 0 ;
	    while ((*cp != '\0') && (i < LOGFILE_LOGIDLEN)) {

	        if (isalnum(*cp) || (*cp == '_'))
	            jobidbuf[i++] = *cp ;

	        cp += 1 ;

	    } /* end for */

	    jobid = jobidbuf ;

	} else
	    jobid = u.logid ;


/* do we have an activity log file ? */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 0 logfname=%s\n",logfname) ;
#endif

	rs1 = SR_NOENT ;
	if ((logfname == NULL) || (logfname[0] == '\0'))
	    logfname = LOGFNAME ;

	if ((logfname[0] == '/') || (u_access(logfname,W_OK) >= 0))
	    rs1 = logfile_open(&pip->lh,logfname,0,0666,jobid) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if ((rs1 < 0) && (logfname[0] != '/')) {

	    mkpath2(tmpfname, pip->pr,logfname) ;

	    rs1 = logfile_open(&pip->lh,tmpfname,0,0666,jobid) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: 2 logfname=%s rs=%d\n",tmpfname,rs) ;
#endif

	} /* end if (we tried to open another log file) */

	if (rs1 >= 0) {

	    pip->f.log = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: we opened a logfile\n") ;
#endif

/* we opened it, maintenance this log file if we have to */

	    if (loglen < 0)
	        loglen = LOGSIZE ;

	    logfile_checksize(&pip->lh,loglen) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: we checked its length\n") ;
#endif

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: making log entry\n") ;
#endif

	    logfile_printf(&pip->lh,"%s %s\n",
	        timestr_logz(pip->daytime,timebuf),
	        BANNER) ;

	    logfile_printf(&pip->lh,"%-14s %s/%s\n",
	        pip->progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;


	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s)\n",
	        (u.f.sysv_rt ? "SYSV" : "BSD"),
	        u.sysname,u.release) ;

	    logfile_printf(&pip->lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;

	} /* end if (we have a log file or not) */


/* other initialization */

	if (progpath == NULL)
	    progpath = PROG_MAILPOLL ;


/* do the deed */

	while (rs >= 0) {

	        struct mcmsg_report	m1 ;

	        MSGID		db ;

	        MSGID_KEY	midkey ;

	        MSGID_ENT	mid ;

	        CONNECTION	conn ;

	    int	ifd = FD_STDIN ;
	    int	opts, sl ;
	    int	pid ;
	        int	f_repeat = FALSE ;

	    const char	*sp ;


	opts = 0 ;
	    fromlen = sizeof(SOCKADDRESS) ;
	    rs = uc_recvfrome(ifd,buf,BUFLEN,0,
	        (struct sockaddr *) &from,&fromlen,to,opts) ;

	    len = rs ;
	    if (rs < 0)
		break ;

	    if (rs > 0) {

	        mkpath2(tmpfname,pip->pr,"var/mailpolld") ;

	        msgid_open(&db,tmpfname,(O_RDWR | O_CREAT),0666,100) ;

	        mcmsg_report(buf,MSGBUFLEN,1,&m1) ;

	        memset(&midkey,0,sizeof(MSGID_KEY)) ;

	        midkey.recip = m1.mailuser ;
	        midkey.reciplen = -1 ;

	        midkey.mtime = pip->daytime ;
	        midkey.from = m1.from ;
	        midkey.mid = m1.msgid ;

	        rs1 = msgid_update(&db,pip->daytime,
	            &midkey,&mid) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: msgid_update() rs=%d\n",rs1) ;
#endif

	        if (rs1 == SR_ACCESS) {

	            rs1 = msgid_match(&db,pip->daytime,
	                &midkey,&mid) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main: msgid_match() rs=%d\n",rs1) ;
#endif

	        }

	        f_repeat = ((rs1 >= 0) && (mid.count > 0)) ;

	        if (fromlen > 0) {

	            struct mcmsg_ack	m2 ;

	            in_addr_t	addr1, addr2 ;

	            int		af ;

	            char	replybuf[MSGBUFLEN + 1] ;


/* can we get the peername of the other end of this socket, if a socket? */

	            connection_start(&conn,pip->domainname) ;

/* send a reply */

	            memset(&m2,0,sizeof(struct mcmsg_ack)) ;

	            m2.tag = m1.tag ;
	            m2.seq = m1.seq + 1 ;
	            m2.rc = mcmsgrc_ok ;

	            blen = mcmsg_ack(replybuf,MSGBUFLEN,0,&m2) ;

	            rs = u_sendto(ifd,replybuf,blen,0,&from,fromlen) ;

	            logfile_printf(&pip->lh,"reply sent rs=%d\n",rs) ;

	            if (f_repeat)
	                logfile_printf(&pip->lh,"repeat\n") ;

	            logfile_printf(&pip->lh,"tag=\\x%08x seq=%u\n",
	                m1.tag,m1.seq) ;

/* try to get the name of the sending host machine */

	            rs = sockaddress_getaf(&from) ;
		    af = rs ;

	            if ((rs >= 0) && (af == AF_INET)) {

	                rs = sockaddress_getaddr(&from,
	                    (char *) &addr1,sizeof(in_addr_t)) ;

	                addr2 = htonl(0x40c05865) ;

	                if ((rs >= 0) && 
	                    (memcmp(&addr1,&addr2,sizeof(in_addr_t)) == 0)) {

	                    rs = strwcpy(peername,"levo.levosim.org",-1) - 
	                        peername ;

	                } else
	                    rs = -1 ;

	            } /* end if (address family INET4) */

	            if (rs < 0)
	                rs = connection_peername(&conn,&from,fromlen,peername) ;

	            if (rs > 0)
	                logfile_printf(&pip->lh,"from host=%s\n",peername) ;

	            if (rs > 0) {

	                if (m1.cluster[0] != '\0')
	                    logfile_printf(&pip->lh,"cluster=%s\n",m1.cluster) ;

	                logfile_printf(&pip->lh,"mailuser=%s\n",m1.mailuser) ;

	                if (m1.msgid[0] != '\0')
	                    logfile_printf(&pip->lh,"msgid=%s\n",m1.msgid) ;

	                if (m1.from[0] != '\0')
	                    logfile_printf(&pip->lh,"from=%s\n",m1.from) ;

	                logfile_printf(&pip->lh,"msglen=%u\n",m1.mlen) ;

	            }

	            connection_finish(&conn) ;
	        } /* end if (had a FROM address) */

#ifdef	COMMENT
	        if (len > 0) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: msg len=%d >%t<\n",len,buf,len) ;
#endif

	            sl = nextfield(buf,len,&sp) ;

	            sp[sl] = '\0' ;
	            logfile_printf(&pip->lh,"username=%t\n",
	                sp,MIN(sl,LOGIDLEN)) ;

	        }
#endif /* COMMENT */

	        logfile_flush(&pip->lh) ;

	        bflush(pip->efp) ;

/* release the parent */

	        rs = uc_fork() ;

	        if (rs == 0) {

/* spawn the real server */

	            rs = uc_fork() ;
		    pid = rs ;

	            if (rs == 0) {
	                char	*av[4] ;

	                bclose(pip->efp) ;

	                for (i = 0 ; i < 3 ; i += 1)
	                    u_close(i) ;

	                u_open("/dev/null",O_RDONLY,0666) ;

	                u_open("/dev/null",O_WRONLY,0666) ;

	                u_open("/dev/null",O_WRONLY,0666) ;

	                av[0] = progpath ;
	                av[1] = sp ;
	                av[2] = NULL ;
	                u_execv(progpath,av) ;

	                uc_exit(EX_NOEXEC) ;

	            } /* end if (child) */

	            if (rs > 0) {

	                int	childstat ;


	                logfile_printf(&pip->lh,"request pid=%d\n",pid) ;

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("main: waiting for child pid=%d\n",pid) ;
#endif

	                u_waitpid(pid,&childstat,0) ;

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("main: child exited pid=%d\n",pid) ;
#endif

	            } else
	                logfile_printf(&pip->lh,
	                    "failed to start server (%d)\n",rs) ;

	        } /* end if (child) */

	        msgid_close(&db) ;
	    } /* end if (received some data) */

	} /* end while */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: done\n") ;
#endif

done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

	u_close(fd_stdout) ;

ret2:
	if (pip->f.log)
	    logfile_close(&pip->lh) ;

/* close off and get out! */
baduser:
earlyret:
ret1:
	bclose(pip->efp) ;

ret0:
	return ex ;

/* what are we about? */
usage:
	usage(pip) ;

	goto earlyret ;

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto badarg ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [<progpath>] [-V] \n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int helpfile(f,ofp)
const char	f[] ;
bfile		*ofp ;
{
	bfile	file, *ifp = &file ;

	int	rs ;


	if ((f == NULL) || (f[0] == '\0'))
	    return SR_NOENT ;

	if ((rs = bopen(ifp,f,"r",0666)) >= 0) {

	    rs = bcopyblock(ifp,ofp,-1) ;

	    bclose(ifp) ;

	}

	return rs ;
}
/* end subroutine (helpfile) */



