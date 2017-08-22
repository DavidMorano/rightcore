/* msgs */

/* program to post and manage messages on a UNIX system */


#define	CF_DEBUGS	0	/* compile-time */
#define	CF_DEBUG	0	/* run-time */
#define	CF_BOUNDCHECK	1
#define	CF_FIRSTMSG	0
#define	CF_ANSITERM	1
#define	CF_EXTRABLANK	0
#define V7 		1	/* will look for TERM in the environment */
/* #define CF_OBJECT		/* will object to messages without Subjects */
/* #define CF_REJECT		/* will reject messages without Subjects
			   	(CF_OBJECT must be defined also) */
/* #define UNBUFFERED		/* use unbuffered output */
#define	CF_PCSUSERFILE	1	/* write a PCS style user file? */
#define	CF_PCSCONF	0	/* call 'pcsconf(3pcs)' */


/* revision history:

	= 1995-04-01, David A­D­ Morano

	Extensively revised to allow compatibility with the old AT&T
	BCS Personal Communication System (PCS) utilities.


*/


/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */


#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n" ;
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)msgs.c	5.2 (Berkeley) 86/04/10" ;
static char damid[] = "@(#)msgs	5.2a (AT&T) 95/07/30" ;
#endif /* not lint */


/***************************************************************************

 * msgs - a user bulletin board program
 *

	Synopsis:

 *	msgs [fhlopq] [[-]number]	to read messages
 *	msgs -s				to place messages
 *	msgs -c [-days]			to clean up the bulletin board
 *	msgs -V				print version and exit
 *

 * prompt commands are:

 *	y	print message
 *	n	flush message, go to next message
 *	q	flush message, quit
 *	p	print message, turn on 'pipe thru more' mode
 *	P	print message, turn off 'pipe thru more' mode
 *	-	rdebugprint last message
 *	s[-][<num>] [<filename>]	save message
 *	m[-][<num>]	mail with message in temp mbox
 *	x	exit without flushing this message
 *	<num>	print message number <num>
 *	h	print out the help file contents
 

*************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<dirent.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<setjmp.h>
#include	<errno.h>
#include	<stdio.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<char.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<pcsconf.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"cleanup.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	VARARCHITECTURE
#define	VARARCHITECTURE	"ARCHITECTURE"
#endif

#define	BUFLEN		((MAXPATHLEN * 2) + 20)

#ifndef	PCSBUFLEN
#define	PCSBUFLEN	1024
#endif

#ifndef	FIELDLEN
#define	FIELDLEN	MAXNAMELEN
#endif

#ifndef	BUFSIZE
#define	BUFSIZE		BUFLEN
#endif

#define CMODE		0666	/* bounds file creation mode */
#define U_SUPERUSER	0	/* superuser UID */
#define U_DAEMON	1	/* daemon UID */

#define DAYS		(24*60*60)	/* seconds/day */

#define PROMPT_NEXT	"Next message? [yq]"
#define PROMPT_MORE	"More? [ynq]"
#define PROMPT_NOMORE	"(No more) [q]?"


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getnodedomain(char *,char *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	hmatch() ;
extern int	mkuiname(char *,int,USERINFO *) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,
			const char *, const char *) ;
extern int	isdigitlatin(int) ;

#if	CF_PCSUSERFILE
extern int	pcsuserfile() ;
#endif

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*nxtfld(const char *) ;
extern char	*strshrink(char *), *strbasename(char *) ;
extern char	*timestr_edate(time_t,char *) ;
extern char	*timestr_hdate(time_t,char *) ;
extern char	*mkufname() ;

#ifdef	COMMENT
void	onsusp() ;
#endif
void	onintr() ;
void	gfrsub() ;
void	prmesg() ;
void	ask() ;


/* external variables */


/* global variables */

typedef	int	bool_t ;

struct proginfo	g ;

struct termios	otty ;

FILE	*newmsg ;

jmp_buf	tstpbuf ;

time_t	t ;
time_t	keep ;

bool_t	f_nontext ;
bool_t	f_local ;
bool_t	f_ruptible ;
bool_t	f_totty ;
bool_t	f_header ;
bool_t	f_envfrom ;
bool_t	f_date ;
bool_t	f_from ;
bool_t	f_subject ;
bool_t	f_subj ;
bool_t	f_title ;
bool_t	f_mailer ;
bool_t	f_seenfrom ;
bool_t	f_seensubj ;
bool_t	f_seendate ;
bool_t	f_seenctype ;
bool_t	f_blankline ;
bool_t	printing = NO ;
bool_t	mailing = NO ;
bool_t	quitit = NO ;
bool_t	sending = NO ;
bool_t	intrpflg = NO ;
bool_t	tstpflag = NO ;

bool_t	f_hdrs = NO ;
bool_t	f_queryonly = NO ;
bool_t	f_hush = NO ;
bool_t	f_send = NO ;
bool_t	locomode = NO ;
bool_t	f_pause = YES ;
bool_t	f_clean = NO ;
bool_t	lastcmd = NO ;

int	msg ;
int	prevmsg ;
int	lct ;
int	nlines ;
int	Lpp = 0 ;

char	*sep = "-" ;
char	inbuf[BUFSIZE + 1] ;
char	fname[MAXPATHLEN + 1] ;
char	cmdbuf[MAXPATHLEN + 1] ;
char	hv_subj[FIELDLEN + 1] ;
char	hv_from[FIELDLEN + 1] ;
char	hv_date[FIELDLEN + 1] ;
char	hv_ctype[FIELDLEN + 1] ;
char	*ptr ;
char	*in ;


/* forward referecnces */

int	user_match() ;


/* local variables */

static const char	*defspooldirs[] = {
	SPOOLDIR1,
	SPOOLDIR2,
	NULL
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	g ;

	struct ustat	sb, sb2 ;

	struct flock	fl ;

	PCSCONF		p ;

	USERINFO	u ;

	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		ufile, *ufp = &ufile ;
	bfile		bndsfile, *bfp = &bndsfile ;

	uid_t	uid ;

	gid_t	gid ;

	bool_t	f_newrc, f_already ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	rcfirst = 0 ;		/* first message to print (from .rc) */
	int	rcback = 0 ;		/* amount to back off of rcfirst */
	int	firstmsg = 0, nextmsg, lastmsg = 0 ;
	int	blast = 0 ;
	int	prs = 0 ;
	int	len, l, fd ;
	int	i ;
	int	err_fd = -1 ;
	int	ex = EX_OK ;
	int	f_lock ;

	const char	*pr = NULL ;
	const char	*spooldir = NULL ;
	const char	*fromnode = NULL ;
	const char	*efname = NULL ;
	const char	*tp ;

	char	userbuf[USERINFO_LEN + 1] ;
	char	pcsbuf[PCSBUFLEN + 1] ;
	char	lbuf[LINEBUFLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	ufname[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	b1fname_buf[MAXPATHLEN + 1], b2fname_buf[MAXPATHLEN + 1] ;
	char	spooldname[MAXPATHLEN + 1] ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*bfname ;
	char	*cp, *cp2 ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	memset(&g,0,sizeof(struct proginfo)) ;

	g.progname = argv[0] ;

	if (efname == NULL) efname = getenv(VAREFNAME) ;

	if ((efname == NULL) || (efname[0] == '\0')) efname = BFILE_STDERR ;

	if ((rs = bopen(efp,efname,"wca",0666)) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

	g.efp = efp ;
	g.ofp = ofp ;
	g.bfp = bfp ;

	g.f.debug = FALSE ;
	g.f.pcsconf = FALSE ;

#if	defined(OSNAME_SunOS)
	g.f.sysv_ct = TRUE ;
#else
	g.f.sysv_ct = FALSE ;
#endif

	g.f.sysv_rt = FALSE ;
	if (u_access("/usr/sbin",R_OK) >= 0)
	    g.f.sysv_rt = TRUE ;

/* get user profile information */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;
	if (rs < 0)
	    goto baduser ;

#if	CF_DEBUGS
	debugprintf("main: user=%s homedir=%s\n",
	    u.username,u.homedname) ;
#endif

/* get our program root */

	if (g.pr == NULL)
	    g.pr = getenv(VARPROGRAMROOT1) ;

	if (g.pr == NULL)
	    g.pr = getenv(VARPROGRAMROOT2) ;

	if (g.pr == NULL)
	    g.pr = getenv(VARPROGRAMROOT2) ;

	if (g.pr == NULL)
	    g.pr = PROGRAMROOT ;

/* get our local node name and domain name */

#ifdef	COMMENT
	domainname[0] = '\0' ;
	getnodedomain(nodename,domainname) ;

	if (domainname[0] != '\0') {

	    snsds(buf,MAXHOSTNAMELEN,nodename,domainname) ;
	    fromnode = mallocstr(buf) ;

	} else
	    fromnode = nodename ;
#else
	if (u.domainname[0] != '\0') {

	    snsds(buf,MAXHOSTNAMELEN,u.nodename,u.domainname) ;
	    fromnode = mallocstr(buf) ;

	} else
	    fromnode = u.nodename ;
#endif /* COMMENT */

/* log file */

	mkpath2(tmpfname,g.pr,LOGFNAME) ;
	g.logfname = mallocstr(tmpfname) ;

/* get the current time-of-day */

	g.daytime = time(NULL) ;

/* make some log entries */

	g.f.log = FALSE ;
	buf[0] = '\0' ;
	if ((g.logfname != NULL) && 
	    (logfile_open(&g.lh,g.logfname,0,0666,u.logid) >= 0)) {

	    g.f.log = TRUE ;
	    logfile_userinfo(&g.lh,&u,g.daytime,g.progname,VERSION) ;

	} /* end if (opening log file) */

/* write user's mail address (roughly as we have it) into the user list file */

#if	CF_PCSUSERFILE

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: writing user file\n") ;
#endif

	rs1 = mkuiname(buf,BUFLEN,&u) ;

	if (rs1 >= 0)
	    rs1 = pcsuserfile(g.pr,USERFNAME,u.nodename,u.username,buf) ;

	if (rs1 == 1) {
	    logfile_printf(&g.lh,
	        "user-list file created\n") ;

	} else if (rs1 < 0)
	    logfile_printf(&g.lh,
	        "user-list file inaccessible (%d)\n",rs1) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: pcsuserfile() rs=%d\n",rs1) ;
#endif

#endif /* CF_PCSUSERFILE */

#ifdef UNBUFFERED
	setbuf(stdout, NULL) ;
#endif

	gtty(fileno(stdout), &otty) ;

	t = time(NULL) ;

	uid = geteuid() ;

	gid = getegid() ;

	seteuid(uid) ;

	f_ruptible = (signal(SIGINT, SIG_IGN) == SIG_DFL) ;

	if (f_ruptible)
	    signal(SIGINT, SIG_DFL) ;

	argc -= 1 ;
	argv += 1 ;
	while ((rs >= 0) && (argc > 0)) {

	    if (isdigit(argv[0][0])) {	/* starting message # */

	        rcfirst = atoi(argv[0]) ;

	    } else if ((argv[0][0] == '-') && isdigit(argv[0][1])) {

/* backward offset */

	        rcback = atoi( &( argv[0][1] ) ) ;

	    } else if ((argv[0][0] == '-') && (argv[0][1] == 'N')) {

	        argc -= 1 ;
	        argv += 1 ;
	        if ((argc > 0) && (argv[0] != NULL))
	            spooldir = argv[0] ;

	    } else {
		const char	*akp = *argv ;

	        while (*akp) {
		    const int	kc = MKCHAR(*akp) ;
	
		    switch (kc) {

	        case '-':
	            break ;

	        case 'D':
	            g.f.debug = TRUE ;
	            break ;

	        case 'V':
	            bprintf(g.efp,
	                "%s: version %s/%s\n",
	                g.progname,
	                VERSION,(g.f.sysv_ct) ? "SYSV" : "BSD") ;
	            goto done ;

	        case 'v':
	            g.verboselevel = 2 ;
	            break ;

	        case 'c':
	            if (! user_match(uid,cleanup_users,gid,cleanup_groups)) {

	                bprintf(g.efp, 
	                    "%s: user is not authorized for cleanup\n",
	                    g.progname) ;

	                prs = 1 ;
	                goto done ;
	            }
	            f_clean = YES ;
	            break ;

	        case 'f':		/* silently */
	            f_hush = YES ;
	            break ;

	        case 'h':		/* headers only */
	            f_hdrs = YES ;
	            break ;

	        case 'l':		/* local msgs only */
	            locomode = YES ;
	            break ;

	        case 'o':		/* option to save last message */
	            lastcmd = YES ;
	            break ;

/* don't pipe thru 'more' during long messages */
	        case 'p':
	            f_pause = NO ;
	            break ;

	        case 'q':		/* query only */
	            f_queryonly = YES ;
	            break ;

	        case 's':		/* sending TO msgs */
	            f_send = YES ;
	            break ;

	        default:
	            bprintf(g.efp,
	                "%s: USAGE> %s [-fhlopq] [[-]<number>]\n",
	                g.progname,g.progname) ;

	            prs = 1 ;
	            goto done ;

	        } /* end switch */

		    akp += 1 ;
		} /* end while */

	    } /* end if */

	    argc -= 1 ;
	    argv += 1 ;

	} /* end while (argument processing) */

/* check if the spool directory is on-line */

	if (spooldir == NULL) spooldir = getenv("MSGSDIR") ;
	if (spooldir == NULL) spooldir = getenv("MSGS_SPOOLDIR") ;

	if (spooldir == NULL) {
	    int	dmode = (R_OK|X_OK) ;
	    for (i = 0 ; defspooldirs[i] != NULL ; i += 1) {

	        tp = defspooldirs[i] ;
	        if (tp[0] != '/') {
		    mkpath2(spooldname,g.pr,tp) ;
		    tp = spooldname ;
	        }

	        if (u_access(tp,dmode) >= 0) {
	            spooldir = (char *) tp ;
		    break ;
	        }

	    } /* end for */

	} /* end if */

	if (spooldir == NULL)
	    goto badspooldir ;

	if (g.f.log)
	    logfile_printf(&g.lh,"spooldir=%s\n",spooldir) ;

/* determine what the bounds file is named */

	bfname = b1fname_buf ;
	mkpath2(b1fname_buf,spooldir,BOUNDS1) ;

	if ((rs = bopen(bfp,b1fname_buf,"rw",0666)) < 0) {

	    bfname = b2fname_buf ;
	    mkpath2(b2fname_buf,spooldir, BOUNDS2) ;

	    if ((rs = bopen(bfp,b2fname_buf,"rw",0666)) < 0) {

	        bfname = b1fname_buf ;
	        if ((rs = bopen(bfp,b1fname_buf,"rwc",0666)) < 0)
	            goto badbounds ;

	        u_chmod(b1fname_buf, CMODE) ;

	        if (g.f.log) {
	            logfile_printf(&g.lh,"created the BOUNDS file\n") ;
	            logfile_printf(&g.lh,"bounds=\n",b1fname_buf) ;
	        }
	    }

	} /* end if (opening a "bounds" file) */

#if	CF_DEBUGS
	debugprintf("main: bounds_file=%s\n",bfname) ;
#endif

/* determine current message bounds, if we can! -- read system bounds file */

/* try to lock the bounds file first */

	f_lock = FALSE ;

	fl.l_type = F_RDLCK ;
	fl.l_whence = SEEK_SET ;
	fl.l_start = 0L ;
	fl.l_len = 0L ;
	if ((rs = bcontrol(bfp,BC_LOCK,LOCKTIMEOUT)) >= 0) {

	    f_lock = TRUE ;

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: system bounds file locked\n") ;
#endif

	} else {

	    if (g.f.log)
	        logfile_printf(&g.lh,"could not lock the BOUNDS file\n",
	            rs) ;

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: bcontrol() rs=%d\n",rs) ;
#endif

	} /* end if (read locking system bounds file) */

/* OK, now we try to actually read it! */

	g.f.bounds = FALSE ;
	if ((len = breadline(bfp,lbuf,LINEBUFLEN)) > 0) {

	    g.f.bounds = TRUE ;

#if	CF_DEBUGS
	    debugprintf("main: read from bounds file\n") ;
#endif

	    if (lbuf[len - 1] == '\n') l -= 1 ;
	    lbuf[len] = '\0' ;

#if	CF_DEBUGS
	    debugprintf("main: >%s",lbuf) ;
#endif

/* get first decimal digit value */

	    cp = lbuf ;
	    while (CHAR_ISWHITE(*cp)) cp += 1 ;

	    cp2 = cp ;
	    while (*cp2 && (! CHAR_ISWHITE(*cp2))) cp2 += 1 ;

	    *cp2 = '\0' ;
	    l = cp2 - cp ;
	    if ((rs = cfdeci(cp,l,&firstmsg)) < 0)
	        firstmsg = 0 ;

/* reset pointers in the line buffer to after the first digit string */

	    cp = cp2 ;
	    if ((len - (l + 1)) > 0) cp = cp2 + 1 ;

/* get second decimal digit value */

	    while (CHAR_ISWHITE(*cp)) cp += 1 ;

	    cp2 = cp ;
	    while (*cp2 && (! CHAR_ISWHITE(*cp2))) cp2 += 1 ;

	    *cp2 = '\0' ;
	    l = cp2 - cp ;
	    if ((rs = cfdeci(cp,l,&lastmsg)) < 0)
	        lastmsg = 0 ;

/* rewind */

	    bseek(bfp,0L,SEEK_SET) ;

	    blast = lastmsg ;	/* save upper bound */

	} /* end if (reading the "bounds" file ) */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to unlock\n") ;
#endif

/* unlock it if we previously locked it */

	if (f_lock) {
	    fl.l_type = F_UNLCK ;
	    bcontrol(bfp,BC_SETLKW,&fl) ;
	}

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: lower=%d upper=%d\n",firstmsg,lastmsg) ;
#endif

/* do we have any reason to suspect that the bounds file is old? */

#if	CF_BOUNDCHECK
	if (g.f.bounds) {

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: checking bounds file integrity\n") ;
#endif

	    if ((rs = bcontrol(bfp,BC_STAT,&sb)) >= 0) {

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: got bounds file status\n") ;
#endif

	        sb2.st_mtime = time(NULL) ;

	        u_stat(spooldir,&sb2) ;

	        if (sb2.st_mtime > sb.st_mtime)
	            g.f.bounds = FALSE ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: check against directory %d\n",
	                g.f.bounds) ;
#endif

	    } /* end if */

#if	CF_FIRSTMSG
	    if (g.f.bounds) {

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: checking bounds against 'firstmsg'\n") ;
#endif

	        mkpath2(buf,spooldir,firstmsg) ;

	        if (u_access(buf,R_OK) < 0)
	            g.f.bounds = FALSE ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: check against 'firstmsg' %d\n",
	                g.f.bounds) ;
#endif

	    } /* end if (checking if the first message is there) */
#endif /* CF_FIRSTMSG */

	} /* end if (bounds file was thought to be OK) */
#endif /* CF_BOUNDCHECK */


	if (f_clean) {

	    keep = t - ((rcback != 0) ? rcback : NDAYS) * DAYS ;

	    logfile_printf(&g.lh,"cleaning %d\n",
	        ((rcback != 0) ? rcback : NDAYS)) ;

	}

/* relocate message bounds? (rebuild) */

	if (f_clean || (! g.f.bounds)) {
	    DIR			*dirp ;
	    struct dirent	*dp ;
	    struct ustat	stbuf ;
	    bool_t		f_seenany = NO ;
	    int		firstmsg_old, lastmsg_old ;
	    int		f_changed ;


	    firstmsg_old = firstmsg ;
	    lastmsg_old = lastmsg ;

#if	CF_DEBUG
	    if (g.f.debug) {
	        debugprintf("main: we are relocating the message bounds\n") ;
	        debugprintf(
	            "main: f_clean=%d f_bounds=%d\n",
	            f_clean,g.f.bounds) ;
	    }
#endif /* CF_DEBUG */

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: relocating the bounds, old=(%d,%d)\n",
	            firstmsg,lastmsg) ;
#endif

	    dirp = opendir(spooldir) ;

	    if (dirp == NULL) {
	        perror(spooldir) ;
	        prs = errno ;
	        goto done ;
	    }

	    firstmsg = 32767 ;
	    lastmsg = 0 ;

	    while ((dp = readdir(dirp)) != NULL) {
	        int	i = 0 ;

	        cp = dp->d_name ;
	        if (dp->d_ino == 0) continue ;

		if (cp[0] == '.') continue ;

	        if (strlen(cp) == 0) continue ;

	        if (f_clean)
	            mkpath2(tmpfname,spooldir,cp) ;

	        while (isdigit(*cp))
	            i = i * 10 + *cp++ - '0' ;

	        if (*cp) continue ;	/* not a message! */

	        if (f_clean) {

	            if (u_stat(tmpfname, &stbuf) != 0)
	                continue ;

	            if ((stbuf.st_mtime < keep) && (stbuf.st_mode & S_IWRITE)) {
	                u_unlink(tmpfname) ;
	                continue ;
	            }

	        } /* end if (cleaning) */

	        if (i > lastmsg)
	            lastmsg = i ;

	        if (i < firstmsg)
	            firstmsg = i ;

	        f_seenany = YES ;

	    } /* end while (reading directory) */

	    closedir(dirp) ;

	    if (! f_seenany) {

/* never lower the upper bound! */

	        if (blast != 0)
	            lastmsg = blast ;

	        firstmsg = lastmsg + 1 ;

	    } else if (blast > lastmsg)
	        lastmsg = blast ;

/* a log message? */

	    f_changed = FALSE ;
	    if (g.f.log && 
	        ((firstmsg != firstmsg_old) || (lastmsg != lastmsg_old))) {

	        f_changed = TRUE ;
	        logfile_printf(&g.lh,"relocating bounds; old=(%u,%u)\n",
	            firstmsg_old,lastmsg_old) ;

	        logfile_printf(&g.lh,"relocated to (%d,%d)\n",
	            firstmsg,lastmsg) ;

	    } /* end if (logging) */

/* if we are not sending a message, update the bounds file */

	    if (f_changed && (! f_send)) {

	        fl.l_type = F_WRLCK ;
	        fl.l_whence = SEEK_SET ;
	        fl.l_start = 0L ;
	        fl.l_len = 0L ;
	        if ((rs = bcontrol(bfp,BC_SETLKW,&fl)) >= 0) {

	            bprintf(g.bfp,"%u %u\n",firstmsg,lastmsg) ;

#ifdef	COMMENT
	            bflush(bfp) ;
#endif

	            fl.l_type = F_UNLCK ;
	            bcontrol(bfp,BC_SETLKW,&fl) ;

	        } /* end if (got file lock) */

	    } /* end if (not sending a message) */

	} /* end if (relocating message bounds) */

/* "Send" mode - place messages in the spool area */

	if (f_send) {
	    const char	*fromname = NULL ;

	    if ((u.name != NULL) && (u.name[0] != '\0')) {
	        fromname = u.name ;

	    } else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0')) {
	        fromname = u.gecosname ;

	    } else if ((u.fullname != NULL) && (u.fullname[0] != '\0')) {
	        fromname = u.fullname ;

	    } else if ((u.mailname != NULL) && (u.mailname[0] != '\0'))
	        fromname = u.mailname ;

#if	CF_PCSCONF
	    if (! g.f.pcsconf) {

	        g.f.pcsconf = TRUE ;
	        rs = pcsconf(g.pr,NULL,&p, NULL,NULL,pcsbuf,PCSBUFLEN) ;

#if	CF_DEBUGS
	        debugprintf("main: got PCS configuration, rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            fromnode = p.fromnode ;
	        } else
	            logfile_printf(&g.lh,
	                "could not get explicit PCS configuration\n") ;

	        if (p.organization == NULL)
	            p.organization = getenv("ORGANIZATION") ;

	        if (p.organization == NULL)
	            p.organization = ORGANIZATION ;

#if	CF_DEBUGS
	        debugprintf("main: fromnode=%s\n",fromnode) ;
#endif

	    } /* end if (getting PCS configuration information) */
#endif /* CF_PCSCONF */

	    nextmsg = lastmsg + 1 ;
	    if (g.f.log)
	        logfile_printf(&g.lh,"sending a message #%u\n",nextmsg) ;

	    bufprintf(fname,MAXPATHLEN,"%s/%u",spooldir,nextmsg) ;

	    newmsg = fopen(fname, "w") ;

	    if (newmsg == NULL) {
	        perror(fname) ;
	        prs = errno ;
	        goto done ;
	    }

	    u_chmod(fname, 0664) ;

/* update the system bounds file */

	    fl.l_type = F_WRLCK ;
	    fl.l_whence = SEEK_SET ;
	    fl.l_start = 0L ;
	    fl.l_len = 0L ;
	    if ((rs = bcontrol(bfp,BC_SETLKW,&fl)) >= 0) {

	        bprintf(g.bfp,"%u %u\n",firstmsg,nextmsg) ;

#ifdef	COMMENT
	        bflush(g.bfp) ;
#endif

	        fl.l_type = F_UNLCK ;
	        bcontrol(bfp,BC_SETLKW,&fl) ;

	    }

	    sending = YES ;
	    if (f_ruptible)
	        signal(SIGINT, onintr) ;

	    f_header = TRUE ;
	    f_envfrom = FALSE ;
	    f_mailer = FALSE ;
	    f_from = FALSE ;
	    f_date = FALSE ;
	    f_subject = FALSE ;
	    f_subj = FALSE ;
	    f_title = FALSE ;
	    f_seensubj = NO ;
	    if (isatty(fileno(stdin))) {
		char	*fmt ;

#if	CF_DEBUG
		if (g.f.debug)
		debugprintf("main: running interactively\n") ;
#endif

	        printf("Message %d:\nFrom %s %sSubject: ",
	            nextmsg, u.username, ctime(&t)) ;

	        fflush(stdout) ;

	        fgets(inbuf, BUFSIZE, stdin) ;

	        putchar('\n') ;

	        fflush(stdout) ;

	        if (strncmp(inbuf,"From ",5) != 0) {

#if	CF_DEBUG
		if (g.f.debug)
		debugprintf("main: adding envelope\n") ;
#endif

	            f_envfrom = TRUE ;
	            fprintf(newmsg, "From %s!%s %s\n",
	                fromnode,u.username,timestr_edate(t,timebuf)) ;
		}

	        fprintf(newmsg,"x-mailer: %s PCS MSGS version %s/%s\n",
	            u.organization,
	            VERSION,(g.f.sysv_ct) ? "SYSV" : "BSD") ;

		if (strpbrk(fromname," \t") != NULL) {
	            fmt = "From:           (%s) %s@%s\n" ;

		} else
	            fmt = "From:           %s <%s@%s>\n" ;

	            fprintf(newmsg, fmt,
	                fromname,u.username,fromnode) ;

	        fprintf(newmsg, "Date:           %s\n",
	        	timestr_hdate(g.daytime,timebuf)) ;

	        fprintf(newmsg, "Subject:        %s",
	            inbuf) ;

/* write out the End-Of-Header line */

	        fprintf(newmsg, "\n") ;

	        f_seensubj = YES ;
		f_subject = TRUE ;
	        f_mailer = TRUE ;
	        f_date = TRUE ;
	        f_from = TRUE ;
	        f_header = FALSE ;

	    } /* end if (running interactively) */

#if	CF_DEBUGS
	    debugprintf("main: f_envfrom=%u\n",f_envfrom) ;
#endif

	    for (;;) {

#if	CF_DEBUGS
	        debugprintf("main: for-loop\n") ;
#endif

	        fgets(inbuf, BUFSIZE, stdin) ;

	        if (feof(stdin) || ferror(stdin)) 
			break ;

#if	CF_DEBUGS
	        debugprintf("main: line>%t<\n",
			inbuf,strlinelen(inbuf,-1,40)) ;
	        debugprintf("main: f_seensubj=%u\n",f_seensubj) ;
#endif

/* check for at least one envelope UNIX "From " header */

#if	CF_DEBUG
		if (g.f.debug) {
	            debugprintf("main: f_envfrom=%u\n",f_envfrom) ;
	            debugprintf("main: inbuf5=>%t<\n",
			inbuf,strlinelen(inbuf,-1,5)) ;
		}
#endif

	        if ((! f_envfrom) && (strncmp(inbuf,"From ",5) == 0)) {
		    f_envfrom = TRUE ;
		}

		if (! f_envfrom) {

#if	CF_DEBUG
		if (g.f.debug)
	            debugprintf("main: adding envelope\n") ;
#endif

	            f_envfrom = TRUE ;
	            fprintf(newmsg,"From %s!%s %s\n",
	                fromnode,u.username,timestr_edate(t,timebuf)) ;

	        } /* end if (adding a UNIX envelope "From " line) */

	        if (f_header) {

	            if (i = hmatch("subject",inbuf)) {
			f_subject = TRUE ;
			f_seensubj = TRUE ;
		    } else if (i = hmatch("subj",inbuf)) {
	                f_subj = TRUE ;
		    } else if (i = hmatch("title",inbuf)) {
	                f_title = TRUE ;
		    } else if (hmatch("x-mailer",inbuf)) {
	                f_mailer = TRUE ;

	            } else if (hmatch("date",inbuf)) {
	                f_date = TRUE ;

	            } else if (hmatch("from",inbuf)) {
	                f_from = TRUE ;

	            } else if (inbuf[0] == '\n') {

	                f_header = FALSE ;

#if	CF_DEBUG
		if (g.f.debug)
	            debugprintf("main: out-of-header f_from=%u\n",
			f_from) ;
#endif

	                if (! f_mailer)
	                    fprintf(newmsg,
	                        "x-mailer: %s PCS MSGS version %s/%s\n",
	                        u.organization,
	                        VERSION,(g.f.sysv_ct) ? "SYSV" : "BSD") ;

	                if (! f_date)
	                    fprintf(newmsg, "Date:           %s\n",
	                        timestr_hdate(g.daytime,timebuf)) ;

	                if (! f_from) {
				const char	*fmt ;

				if (strpbrk(fromname," \t") != NULL) {
	                        fmt = "From:           (%s) %s@%s\n" ;
				} else
	                        fmt = "From:           (%s) <%s@%s>\n" ;

	                    fprintf(newmsg, fmt,
	                        fromname,u.username,fromnode) ;

			} /* end if (adding the from name) */

	            } /* end if (adding missing headers) */

	        } /* end if (header processing) */

/* continue */

	        if (f_header && (! f_seensubj) && 
		    (f_title || f_subj)) {

	            cp = inbuf + i ;
	            while (CHAR_ISWHITE(*cp)) cp += 1 ;

	            if (*cp != '\0') {
			f_seensubj = TRUE ;
	                fprintf(newmsg,"Subject:        %s",cp) ;
		    }

	        } else
	            fputs(inbuf, newmsg) ;

	    } /* end for (reading message to be sent) */

	    fflush(newmsg) ;

#ifdef CF_OBJECT
	    if (! f_seensubj) {

	        printf("NOTICE: Messages should have a Subject field!\n") ;

#ifdef CF_REJECT
	        unlink(fname) ;
#endif

	        prs = 1 ;
	        goto done ;
	    }
#endif /* CF_OBJECT */

	    prs = ferror(stdin) ;

	    goto done ;

	} /* end if (sending) */

	if (f_clean) 
	    goto done ;

/* prepare to display messages */

	if (g.f.log)
	    logfile_printf(&g.lh,"reading messages\n") ;

	f_totty = (isatty(fileno(stdout)) != 0) ;

	f_pause = f_pause && f_totty ;

/* what about the user's MSGSRC file? */

	if ((cp = getenv("MSGSRC")) == NULL)
	    cp = MSGSRC ;

	if ((cp2 = mkufname(u.homedname,cp,ufname)) == NULL) {
	    mkpath2(ufname,u.homedname,cp) ;

	} else if (cp2 != ufname)
	    mkpath1(ufname,cp2) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: user file \"%s\"\n",
	        ufname) ;
#endif

	if (g.f.log)
	    logfile_printf(&g.lh,"ufile=%s\n",ufname) ;

	if (g.verboselevel > 0)
	    printf("user MSGSRC file \"%s\"\n",ufname) ;

	f_newrc = NO ;
	if ((rs = bopen(ufp,ufname, "rw",0666)) >= 0) {

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: reading user's message keep file\n") ;
#endif

	    nextmsg = 0 ;
	    if ((rs = bcontrol(ufp,BC_LOCK,LOCKTIMEOUT)) >= 0) {

	        if ((l = breadline(ufp,lbuf,LINEBUFLEN)) > 0) {

	            if (lbuf[l - 1] == '\n') l -= 1 ;

	            lbuf[l] = '\0' ;
	            if (cfdeci(lbuf,l,&nextmsg) < 0) nextmsg = 0 ;

	        }

	        bcontrol(ufp,BC_UNLOCK,0) ;
	    } /* end if (locked user file) */

	    if (nextmsg > (lastmsg + 1)) {

	        bprintf(g.efp,
	            "%s: warning - bounds have been reset (%d,%d)\n",
	            g.progname,firstmsg, lastmsg) ;

	        if (g.f.log)
	            logfile_printf(&g.lh,"resetting the bounds (%d,%d)\n",
	                firstmsg, lastmsg) ;

	        f_newrc = YES ;

	    } else if (! rcfirst)
	        rcfirst = nextmsg - rcback ;

	} else {

	    f_newrc = YES ;
	    if ((rs = bopen(ufp,ufname,"rwc",0666)) < 0) {

	        bprintf(g.efp,
	            "%s: could not create the user's message file (%d)\n",
	            g.progname,rs) ;

	        prs = BAD ;
	        goto done ;
	    }
	}

#if	CF_DEBUG
	if (g.f.debug) {
	    debugprintf("main: stat says %s rs=%d size=%d\n",
	        ((((rs = bcontrol(ufp,BC_STAT,&sb)) >= 0) && 
	        (sb.st_size > 0)) ? "OK" : "BAD"),
	        rs,sb.st_size) ;
	}
#endif

	if (rcfirst) {

	    if (rcfirst > (lastmsg + 1)) {

	        bprintf(g.efp,
	            "%s: warning - the last message is number %d.\n",
	            g.progname,lastmsg) ;

	        rcfirst = nextmsg ;
	    }

	    if (rcfirst > firstmsg)
	        firstmsg = rcfirst;	/* don't set below first msg */

	} /* end if */

#if	CF_DEBUG
	if (g.f.debug) {
	    debugprintf("main: stat says %s rs=%d size=%d\n",
	        ((((rs = bcontrol(ufp,BC_STAT,&sb)) >= 0) && 
	        (sb.st_size > 0)) ? "OK" : "BAD"),
	        rs,sb.st_size) ;
	}
#endif

	if (f_newrc) {

	    nextmsg = firstmsg ;

	    rs = bseek(ufp, 0L, SEEK_SET) ;

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: updating user's file w/ %d rs=%d\n",
	            nextmsg,rs) ;
#endif

	    if ((rs = bcontrol(ufp,BC_LOCK,LOCKTIMEOUT)) >= 0) {

	        rs = bprintf(ufp,"%u\n", nextmsg) ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: bprintf rs=%d\n",rs) ;
#endif

#ifdef	COMMENT
	        bflush(ufp) ;
#endif

	        bcontrol(ufp,BC_UNLOCK,0) ;

	    } /* end if (locked user file) */

	} /* end if (new user file) */

#if	CF_DEBUG
	if (g.f.debug) {
	    debugprintf("main: stat says %s rs=%d size=%d\n",
	        ((((rs = bcontrol(ufp,BC_STAT,&sb)) >= 0) && 
	        (sb.st_size > 0)) ? "OK" : "BAD"),
	        rs,sb.st_size) ;

	}
#endif

#ifdef V7
	if (f_totty) {
	    struct winsize	win ;

	    if (ioctl(fileno(stdout), TIOCGWINSZ, &win) != -1)
	        Lpp = win.ws_row ;

	    if (Lpp <= 0) {

	        if ((tgetent(inbuf, getenv("TERM")) <= 0) || 
	            ((Lpp = tgetnum("li")) <= 0)) {

	            Lpp = NLINES ;
	        }
	    }
	}
#else
	Lpp = 24 ;
#endif

	Lpp -= 6 ;	/* for headers, etc. */

	f_already = NO ;
	prevmsg = firstmsg ;
	printing = YES ;
	if (f_ruptible)
	    signal(SIGINT, onintr) ;

/* * main program loop */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: about to go through the loops\n") ;
#endif

	for (msg = firstmsg ; msg <= lastmsg ; msg += 1) {

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: top of loop\n") ;
#endif

#if	CF_DEBUG
	    if (g.f.debug) {
	        debugprintf("main: stat says %s rs=%d size=%d\n",
	            ((((rs = bcontrol(ufp,BC_STAT,&sb)) >= 0) && 
	            (sb.st_size > 0)) ? "OK" : "BAD"),
	            rs,sb.st_size) ;
	    }
#endif

	    bufprintf(fname,MAXPATHLEN,"%s/%u",spooldir,msg) ;

	    if ((newmsg = fopen(fname, "r")) == NULL) continue ;

/* get "From", "Date", and "Subject" fields from this message */

	    gfrsub(newmsg) ;

/* some kind of check */

	    if (locomode && (! f_local)) {
	        fclose(newmsg) ;
	        continue ;
	    }

/* This has to be located here */

	    if (f_queryonly) {
	        printf("There are new messages.\n") ;
	        goto done ;
	    }

/* check if we have a content type that we can handle */

	    f_nontext = 0 ;
	    if (f_seenctype) {
	        cp = strshrink(hv_ctype) ;

	        f_nontext = nontext(cp) ;
	    }

/* continue */

#if	CF_EXTRABLANK
	    if (f_already && (! f_hdrs))
	        putchar('\n') ;
#endif

	    f_already = YES ;

/* print headers of this message (those that we may have) */
again:

#ifdef	COMMENT
	    if (f_totty)
	        signal(SIGTSTP, onsusp) ;
#endif

	    (void) setjmp(tstpbuf) ;

	    nlines = 2 ;
	    printf("Message:    %d\n",msg) ;

	    if ((hv_from != NULL) && (hv_from[0] != '\0')) {

	        nlines += 1 ;
	        printf("From:       %s\n",hv_from) ;

	    }

	    if ((hv_date != NULL) && (hv_date[0] != '\0')) {

	        nlines += 1 ;
	        printf("Date:       %s\n",hv_date) ;

	    }

	    if ((hv_subj != NULL) && (hv_subj[0] != '\0')) {

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: printing subject\n") ;
#endif

	        nlines += 1 ;
	        printf("Subject:    %s\n",hv_subj) ;

	    }

#if	CF_DEBUGS
	    debugprintf("main: NL\n") ;
#endif

	    printf("\n") ;

/* end of displaying the header section */

/* prompt the user for message disposition */

	    lct = linecnt(newmsg) ;

	    if (lct)
	        printf("(%d%slines) ", lct, f_seensubj ? " " : " more ") ;

	    if (f_hdrs) {

	        printf("\n----------\n") ;

	        fclose(newmsg) ;

	        continue ;
	    }

/* ask the user for command */

	    if (f_totty)  {
		ask(spooldir,
	        lct ? PROMPT_MORE : 
	        ((msg == lastmsg) ? PROMPT_NOMORE : PROMPT_NEXT)) ;

	    } else
	        inbuf[0] = 'y' ;

	    if (f_totty)
	        signal(SIGTSTP, SIG_DFL) ;

#if	CF_DEBUG
	    if (g.f.debug) {
	        debugprintf("main: stat says %s rs=%d size=%d\n",
	            ((((rs = stat(ufname,&sb)) >= 0) && 
	            (sb.st_size > 0)) ? "OK" : "BAD"),
	            rs,sb.st_size) ;
	    }
#endif

#if	CF_DEBUGS
	    debugprintf("main: cmnd\n") ;
#endif

/* evaluate the command from the user? */
cmnd:
	    in = inbuf ;
	    switch (*in) {

	    case 'x':
	    case 'X':
	        goto done ;

	    case 'q':
	    case 'Q':
	        quitit = YES ;
	        printf("--Postponed--\n") ;

	        goto done ;

/* intentional fall-thru */

	    case 'n':
	    case 'N':
	        if (msg >= nextmsg) sep = "Flushed" ;

	        prevmsg = msg ;
	        break ;

	    case 'p':
	    case 'P':
	        f_pause = (*in++ == 'p') ;

/* intentional fallthru */

	    case '\n':
	    case 'y':
	    default:
	        if (*in == '-') {
	            msg = prevmsg - 1 ;
	            sep = "replay" ;
	            break ;
	        }

	        if (isdigit(*in)) {
	            msg = next(in) ;
	            sep = in ;
	            break ;
	        }

	        prmesg(lct - nlines) ;

	        prevmsg = msg ;

	    } /* end switch */

	    printf("------------%s------------\n", sep) ;

	    sep = "-" ;
	    if (msg >= nextmsg) {

	        nextmsg = msg + 1 ;
	        rs = bseek(ufp, 0L, SEEK_SET) ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: updating user's file w/ %d rs=%d\n",
	                nextmsg,rs) ;
#endif

	        if ((rs = bcontrol(ufp,BC_LOCK,LOCKTIMEOUT)) >= 0) {

	            rs = bprintf(ufp, "%d\n", nextmsg) ;

	            bcontrol(ufp,BC_UNLOCK,0) ;

	        } /* end if (locked user file) */

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: bprintf rs=%d\n",rs) ;
#endif

	        rs = bflush(ufp) ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: rs=%d\n",rs) ;
#endif

	    } /* end if */

	    if (newmsg != NULL)
	        fclose(newmsg) ;

	    if (quitit)
	        break ;

	} /* end for (looping through the messages) */

/* * make sure the user's MSGSRC file gets updated */

#if	CF_DEBUG
	if (g.f.debug) {
	    debugprintf("main: about to decide to update user's file\n") ;
	    debugprintf("main: stat says %s rs=%d size=%d\n",
	        ((((rs = stat(ufname,&sb)) >= 0) && 
	        (sb.st_size > 0)) ? "OK" : "BAD"),
	        rs,sb.st_size) ;
	}
#endif

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: msg=%d nextmsg=%d\n",msg,nextmsg) ;
#endif

	if (--msg >= nextmsg) {

	    nextmsg = msg + 1 ;
	    rs = bseek(ufp, 0L,SEEK_SET) ;

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: updating user's file w/ %d rs=%d\n",
	            nextmsg,rs) ;
#endif

	    if ((rs = bcontrol(ufp,BC_LOCK,LOCKTIMEOUT)) >= 0) {

	        rs = bprintf(ufp, "%d\n", nextmsg) ;

	        bcontrol(ufp,BC_UNLOCK,0) ;

	    } /* end if (locked user file) */

#if	CF_DEBUG
	    if (g.f.debug)
	        debugprintf("main: rs=%d\n",rs) ;
#endif

	    rs = bflush(ufp) ;

#if	CF_DEBUG
	    if (g.f.debug) {
	        debugprintf("main: rs=%d\n",rs) ;
	        debugprintf("main: stat says %s rs=%d size=%d\n",
	            ((((rs = stat(ufname,&sb)) >= 0) && 
	            (sb.st_size > 0)) ? "OK" : "BAD"),
	            rs,sb.st_size) ;
	    }
#endif

	}

#if	CF_DEBUG
	if (g.f.debug) {
	    debugprintf("main: stat says %s rs=%d size=%d\n",
	        ((((rs = stat(ufname,&sb)) >= 0) && 
	        (sb.st_size > 0)) ? "OK" : "BAD"),
	        rs,sb.st_size) ;

	}
#endif

	if (f_already && (! quitit) && lastcmd && f_totty) {

/* * save or reply to last message? */

	    msg = prevmsg ;
	    ask(spooldir,PROMPT_NOMORE) ;

	    if ((inbuf[0] == '-') || isdigit(inbuf[0]))
	        goto cmnd ;

	}

#if	CF_DEBUG
	if (g.f.debug) {
	    debugprintf("main: stat says %s rs=%d size=%d\n",
	        ((((rs = stat(ufname,&sb)) >= 0) && 
	        (sb.st_size > 0)) ? "OK" : "BAD"),
	        rs,sb.st_size) ;
	}
#endif

	if (! (f_already || f_hush || f_queryonly))
	    printf("No new messages.\n") ;

/* we are done */
done:
	fclose(stdout) ;

	bclose(g.bfp) ;

	bclose(ufp) ;

	logfile_close(&g.lh) ;

	bclose(efp) ;

	ex = (prs >= 0) ? EX_OK : EX_DATAERR ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
badret:
	logfile_close(&g.lh) ;

	bclose(efp) ;

	ex = EX_DATAERR ;
	goto ret0 ;

badspooldir:
	bprintf(efp,"%s: bad spool directory \"%s\"\n",
	    g.progname,spooldir) ;

	goto badret ;

badbounds:
	bprintf(efp,"%s: could not open or create a BOUNDS file\n",
	    g.progname) ;

	goto badret ;

baduser:
	bprintf(efp,"%s: could not determine username (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

}
/* end subroutine (main) */


void prmesg(length)
int	length ;
{
	FILE	*outf, *inf ;

	int	c ;


#ifndef NOMETAMAIL
	if ((getenv("NOMETAMAIL") == NULL) && f_nontext) {

	    struct termios		ttystatein, ttystateout ;

	    int		code ;

	    char	Cmd[(2 * MAXPATHLEN) + 1] ;


	    sprintf(Cmd, "%s %s -m MSGS %s", 
	        PROG_METAMAIL,f_pause ? "-p" : "", fname) ;

	    tcgetattr(fileno(stdin),&ttystatein) ;

	    tcgetattr(fileno(stdout),&ttystateout) ;

	    code = system(Cmd) ;

	    tcsetattr(fileno(stdin),TCSADRAIN,&ttystatein) ;

	    tcsetattr(fileno(stdout),TCSADRAIN,&ttystateout) ;

	    return ;
	}
#endif

	if (f_pause && (length > Lpp)) {

	    signal(SIGPIPE, SIG_IGN) ;

	    signal(SIGQUIT, SIG_IGN) ;

	    sprintf(cmdbuf, "%s -%d",PROG_PAGER, Lpp) ;

	    outf = popen(cmdbuf, "w") ;

	    if (outf == NULL) {
	        outf = stdout ;
	    } else
	        setbuf(outf, NULL) ;

	} else
	    outf = stdout ;

	if (f_seensubj)
	    putc('\n', outf) ;

	while (fgets(inbuf, BUFSIZE, newmsg)) {

	    fputs(inbuf, outf) ;

	    if (ferror(outf)) {

	        clearerr(outf) ;

	        break ;
	    }

	} /* end while */

	if (outf != stdout) {

	    pclose(outf) ;

	    signal(SIGPIPE, SIG_DFL) ;

	    signal(SIGQUIT, SIG_DFL) ;

	} else {

	    fflush(stdout) ;

	}

/* trick to force wait on output */

	stty(fileno(stdout), &otty) ;

}
/* end subroutine (prmesg) */


void onintr(v)
int	v ;
{
	int	prs = 0 ;


	signal(SIGINT, onintr) ;

	if (mailing)
	    unlink(fname) ;

	if (sending) {

	    unlink(fname) ;

	    puts("--Killed--") ;

	    prs = 1 ;
	    goto early ;
	}

	if (printing) {

	    putchar('\n') ;

	    if (f_hdrs) {

	        prs = 0 ;
	        goto early ;
	    }

	    sep = "Interrupt" ;
	    if (newmsg)
	        fseek(newmsg, 0L,SEEK_END) ;

	    intrpflg = YES ;
	}

	return ;

early:
	exit(prs) ;
}
/* end subroutine (onintr) */


#ifdef	COMMENT

/* * We have just gotten a susp.  Suspend and prepare to resume.  */
void onsusp(v)
int	v ;
{

	signal(SIGTSTP, SIG_DFL) ;

	sigsetmask(0) ;

	kill(0, SIGTSTP) ;

	signal(SIGTSTP, onsusp) ;

	if (! mailing)
	    longjmp(tstpbuf,1) ;

}
/* end subroutine (onsusp) */

#endif /* COMMENT */


int linecnt(f)
FILE	*f ;
{
	offset_t	oldpos ;

	int	l = 0 ;

	char	lbuf[BUFSIZE + 1] ;


	oldpos = ftell(f) ;

	while (fgetline(f,lbuf, BUFSIZE) > 0)
	    l += 1 ;

	clearerr(f) ;

	fseek(f, oldpos,SEEK_SET) ;

	return l ;
}
/* end subroutine (linecnt) */


int next(buf)
char *buf ;
{
	int	i ;


	sscanf(buf, "%d", &i) ;

	sprintf(buf, "Goto %d", i) ;

	return (--i) ;
}
/* end subroutine (next) */


void ask(spooldir,prompt)
char	*spooldir ;
char	*prompt ;
{
	FILE	*cpfrom, *cpto ;

	bfile	helpfile, *hfp = &helpfile ;

	offset_t	oldpos ;

	int	len ;
	int	c_inch ;
	int	n, cmsg ;

	char	helpfname[MAXPATHLEN + 1] ;
	char	*cp ;


	printf("%s ", prompt) ;

	fflush(stdout) ;

	intrpflg = NO ;
	fgets(inbuf,BUFSIZE,stdin) ;

#if	CF_ANSITERM
	printf("\033[A\033[K\033[A") ;

	fflush(stdout) ;
#endif

	if (intrpflg)
	    inbuf[0] = 'x' ;

/* * handle 'mail' and 'save' here */

	cp = inbuf ;
	while (CHAR_ISWHITE(*cp)) cp += 1 ;

	c_inch = (*cp & 0xFF) ;
	if ((c_inch == 's') || (c_inch == 'm')) {

	    if (cp[1] == '-')
	        cmsg = prevmsg ;

	    else if (isdigit(cp[1])) {
	        cmsg = atoi(&cp[1]) ;
	    } else
	        cmsg = msg ;

	    bufprintf(fname,MAXPATHLEN,"%s/%u",spooldir,cmsg) ;

	    oldpos = ftell(newmsg) ;

	    cpfrom = fopen(fname, "r") ;

	    if (! cpfrom) {

	        printf("Message %d not found\n", cmsg) ;

	        ask(spooldir,prompt) ;

	        return ;
	    }

	    if (c_inch == 's') {

	        in = nxtfld(cp) ;

	        if (*in) {

	            for (n = 0 ; in[n] > ' ' ; n += 1)
	                fname[n] = in[n] ;

	            fname[n] = NULL ;

	        } else
	            strcpy(fname, "Messages") ;

	    } else {

	        strcpy(fname, TEMP) ;

	        mktemp(fname) ;

	        sprintf(cmdbuf,"%s -f %s", PROG_MAILER, fname) ;

	        mailing = YES ;
	    }

	    cpto = fopen(fname, "a") ;

	    if (! cpto) {

	        perror(fname) ;

	        mailing = NO ;
	        fseek(newmsg, oldpos, 0) ;

	        ask(spooldir,prompt) ;

	        return ;
	    }

	    while ((n = fread(inbuf, 1, BUFSIZE, cpfrom)) > 0)
	        fwrite(inbuf, 1, n, cpto) ;

	    fclose(cpfrom) ;

	    fclose(cpto) ;

	    fseek(newmsg, oldpos,SEEK_SET) ;	/* reposition current message */

	    if (c_inch == 's') {
	        printf("Message %d saved in \"%s\"\n", cmsg, fname) ;

	    } else {

	        system(cmdbuf) ;

	        unlink(fname) ;

	        mailing = NO ;
	    }

	    ask(spooldir,prompt) ;

	} else if ((c_inch == 'h') || (c_inch == '?')) {

	    mkpath2(helpfname,g.pr,CMDHELPFNAME) ;

	    if (bopen(hfp,helpfname,"r",0666) >= 0) {

	        while ((len = breadline(hfp,helpfname,MAXPATHLEN)) > 0) {

	            fwrite(helpfname,1,len,stdout) ;

	        } /* end while */

	        bclose(hfp) ;

	    } else {

	        printf("sorry, no help was found\n") ;

	    } /* end if (opened the help file) */

	    ask(spooldir,prompt) ;

	} /* end if */

}
/* end subroutine (ask) */


void gfrsub(infile)
FILE	*infile ;
{
	int	len, i ;

	char	env_from[FIELDLEN + 1] ;
	char	env_date[FIELDLEN + 1] ;
	char	*cp ;


	f_seensubj = f_seenfrom = f_seendate = NO ;
	f_seenctype = NO ;
	f_local = YES ;
	hv_subj[0] = hv_from[0] = hv_date[0] = '\0' ;
	hv_ctype[0] = '\0' ;
	env_from[0] = env_date[0] = '\0' ;

/* is this a normal message? */

#if	CF_DEBUGS
	debugprintf("gfrsub: about to loop\n") ;
#endif

	while (((len = fgetline(infile,inbuf,BUFSIZE)) > 0) && 
	    (! (f_blankline = (inbuf[0] == '\n')))) {

	    if (inbuf[len - 1] == '\n') len -= 1 ;
	    inbuf[len] = '\0' ;

#if	CF_DEBUGS
	    debugprintf("gfrsub: >%s<\n",inbuf) ;
#endif

/* check for an envelope "from" header */

	    if (strncmp(inbuf, "From ", 5) == 0) {
	        int	slen ;

#if	CF_DEBUGS
	        debugprintf("gfrsub: got an evelope header\n") ;
#endif

	        ptr = env_from ;
	        in = nxtfld(inbuf) ;

	        while (*in && (*in > ' ')) {

	            if (*in == ':' || *in == '@' || *in == '!')
	                f_local = NO ;

	            *ptr++ = *in++ ;

	        } /* end while */

	        if ((ptr > env_from) && (ptr[-1] == '\n'))
	            ptr[-1] = '\0' ;

	        *ptr = '\0' ;
	        if (*(in = nxtfld(in)))
	            strwcpy(env_date,in,FIELDLEN) ;

	        if (((slen = strlen(env_date)) > 0) &&
	            (env_date[slen - 1] == '\n'))
	            env_date[slen - 1] = '\0' ;

	    } /* end if (envelope "from" and possible envelope "date") */

/* extract "From" line */

	    if ((! f_seenfrom) && (i = hmatch("from",inbuf))) {

#if	CF_DEBUGS
	        debugprintf("gfrsub: got from\n") ;
#endif

	        cp = inbuf + i ;
	        while (CHAR_ISWHITE(*cp)) cp += 1 ;

	        strwcpy(hv_from,cp,FIELDLEN) ;

	        if (hv_from[0] != '\0')
	            f_seenfrom = YES ;

	    } /* end if ("from") */

/* extract "Date" header */

	    if ((! f_seendate) && 
	        (i = hmatch("date",inbuf))) {

#if	CF_DEBUGS
	        debugprintf("gfrsub: got a date\n") ;
#endif

	        cp = inbuf + i ;
	        while (CHAR_ISWHITE(*cp)) cp += 1 ;

	        strwcpy(hv_date,cp,FIELDLEN) ;

	        if (hv_date[0] != '\0')
	            f_seendate = YES ;

	    } /* end if ("date") */

/* extract "Subject" line */

	    if ((! f_seensubj) && 
	        ((i = hmatch("title",inbuf)) ||
	        (i = hmatch("subj",inbuf)) ||
	        (i = hmatch("subject",inbuf)))) {

#if	CF_DEBUGS
	        debugprintf("gfrsub: got subject (or title)\n") ;
#endif

	        cp = inbuf + i ;
	        while (CHAR_ISWHITE(*cp)) cp += 1 ;

#if	CF_DEBUGS
	        debugprintf("gfrsub: checking for a value subject\n") ;
	        debugprintf("gfrsub: checking >%s<\n",cp) ;
#endif

	        strwcpy(hv_subj,cp,FIELDLEN) ;

	        if (hv_subj[0] != '\0')
	            f_seensubj = YES ;

	    } /* end if ("subject") */

/* extract "Content-Type" header */

	    if ((! f_seenctype) && 
	        (i = hmatch("content-type",inbuf))) {

#if	CF_DEBUGS
	        debugprintf("gfrsub: got a content type\n") ;
#endif

	        cp = inbuf + i ;
	        while (CHAR_ISWHITE(*cp)) cp += 1 ;

#if	CF_DEBUGS
	        debugprintf("gfrsub: checking for a CTYPE value \n") ;
	        debugprintf("gfrsub: checking >%s<\n",cp) ;
#endif

	        strwcpy(hv_ctype,cp,FIELDLEN) ;

	        if (hv_ctype[0] != '\0')
	            f_seenctype = YES ;

	    } /* end if ("content-type") */

	} /* end while (extracting headers) */

/* add certain headers (if we can) if they are not already present */

	if (! f_seensubj)
	    strwcpy(hv_subj, "*no_subject*",FIELDLEN) ;

	if (! f_seenfrom) {

	    if (env_from[0] != '\0') {
	        strwcpy(hv_from,env_from,FIELDLEN) ;

	    } else
	        strwcpy(hv_from,"unknown <postmaster>",FIELDLEN) ;

	}

	if ((! f_seendate) && (env_date[0] != '\0'))
	    strwcpy(hv_date,env_date,FIELDLEN) ;

}
/* end subroutine (gfrsub) */


char *nxtfld(s)
const char	*s ;
{


/* skip over this field */

	while (*s && (! CHAR_ISWHITE(*s))) 
		s += 1 ;

/* find start of next field */

	while (CHAR_ISWHITE(*s)) 
		s += 1 ;

	return ((char *) s) ;
}
/* end subroutine (nxtfld) */


int user_match(uid,ulist,gid,glist)
uid_t	uid ;
char	*ulist[] ;
gid_t	gid ;
char	*glist[] ;
{
	struct passwd	*pp ;

	struct group	*gp ;

	gid_t	groups[NGROUPS_MAX + 1] ;

	int	rs ;
	int	i, j ;

	const char	*un ;

/* check if this user is in the MSGS cleanup group */

	if ((rs = u_getgroups(NGROUPS_MAX,groups)) >= 0) {
	for (i = 0 ; glist[i] != NULL ; i += 1) {
	    if ((gp = getgrnam(glist[j])) == NULL) continue ;

	    if (gid == gp->gr_gid) return TRUE ;

	    for (j = 0 ; j < rs ; j += 1)
	        if (groups[j] == gp->gr_gid) return TRUE ;

	} /* end for */
	} /* end if (u_getgroups) */

/* check if this user is on our list of OK users */

	i = 0 ;
	un = ulist[i] ;
	while ((un != NULL) && (*un != '\0')) {

	    if (((pp = getpwnam(un)) != NULL) &&
	        (pp->pw_uid == uid)) return TRUE ;

	    un = ulist[i++] ;
	}

	return FALSE ;
}
/* end subroutine (user_match) */


int nontext(s)
char	*s ;
{
	char	*t ;


	if (s == NULL) return 1 ;

	while (*s && isspace(*s)) s += 1 ;

	for (t = s ; *t != '\0' ; t += 1)
	    if (isupper(*t)) *t = tolower(*t) ;

	while ((t > s) && isspace(*--t)) ;

	if (((t - s) == 3) && (strncmp(s, "text", 4) == 0)) return 0 ;

	if (strncmp(s, "text/plain", 10) != 0) return 1 ;

	while ((t = (char *) strchr(s, ';')) != NULL) {

	    t += 1 ;
	    while (*t && isspace(*t)) t += 1 ;

	    if (strncmp(t, "charset", 7) == 0) {

	        if ((s = (char *) strchr(t, '=')) != NULL) {

	            s += 1 ;
	            while (*s && isspace(*s)) s += 1 ;

	            if (strncmp(s, "us-ascii", 8) == 0) return 0 ;

	        }

	        return 1 ;
	    }

	    t = (char *) strchr(t, ';') ;

	} /* end while */

	return 0 ; /* no charset, was text/plain */
}
/* end subroutine (nontext) */


