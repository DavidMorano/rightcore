/* proghandle_telserv */

/* handle this service request */


#define	P_TELSERV	1		/* which basic function */

#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_TTCOMPAT	1		/* push 'ttcompat' STREAMS module? */
#define	CF_TTCOMPATPOP	0		/* pop 'ttcompat' if it is there */
#define	CF_MKUTMPID	1		/* use 'mkutmpid()' */
#define	CF_MAKEUTX	0		/* use 'makeutx()' */
#define	CF_LOCALSVC	0		/* local service determination */
#define	CF_LOGALT	1		/* user alternative logging */
#define	CF_IOFATAL	0		/* IO errors use 'fatal()' */
#define	CF_CLEARGROUPS	0		/* clear supplemental groups */
#define	CF_CHMOD	0		/* change mode of term-line */
#define	CF_EXTRACLOSE	1		/* close extra FDs */


#pragma ident	"@(#)in.telnetd.c	1.37	99/10/21 SMI"	/* SVr4.0 1.9 */

/*
 * Copyright (c) 1986-1998 by Sun Microsystems, Inc.
 * All rights reserved.
 */

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 *
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 *
 *
 *
 * 		Copyright Notice
 *
 * Notice of copyright on this source code product does not indicate
 * publication.
 *
 * 	(c) 1986 to 1998  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *	All rights reserved.
 *
 */

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*
 * Telnet server.
 */


#include	<envstandards.h>	/* MUST be first to configure */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include	<sys/utsname.h>

#include <sys/wait.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/filio.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/tihdr.h>
#include <sys/logindmux.h>
#include <sys/telioctl.h>

#ifdef SYSV
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/tihdr.h>
#endif /* SYSV */

#include <netinet/in.h>
#include <arpa/telnet.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <fcntl.h>
#include	<termios.h>
#include <syslog.h>
#include <netdb.h>
#include <signal.h>
#include <stddef.h>
#include	<stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef SYSV
#include <utmp.h>
#endif

#ifdef SYSV
#include <sac.h>	/* for SC_WILDC */
#include <utmpx.h>
#include <sys/ttold.h>
#include <malloc.h>
#endif /* SYSV */

#include <security/pam_appl.h>
#include	<pwd.h>
#include <errno.h>
#include <stdio.h>

#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"standing.h"
#include	"builtin.h"


/* local defines */

#if	P_TELNET
#define	PROGNAME	"telnetd"
#endif

#if	P_TELSERV
#define	PROGNAME	"telserv"
#endif

#ifndef	PROGNAME
#define	PROGNAME	"telunk"
#endif

#ifndef	VARTERM
#define	VARTERM		"TERM"
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	SVCLEN
#define	SVCLEN		MAXNAMELEN
#endif

#define	TERMLEN		100

#ifndef	FD_ALLZERO
#define	FD_ALLZERO(sp)		fd_allzero(sp)
#endif

#define	DEBFNAME		"/tmp/telnetd.deb"

#define	USERNAME_GUEST		"guest"
#define	USERNAME_WRITE		"write"

#define	TELSERV_SERVICES	"/etc/telserv.services"

#define	W_OPTIONS	(WNOHANG)

#define	NCHILDGONE	3

#ifdef SYSV
#define	bzero(s, n)	memset((s), 0, (n))
#define	bcopy(a, b, c)	memcpy((b), (a), (c))
#endif /* SYSV */

#define	OPT_NO			0		/* won't do this option */
#define	OPT_YES			1		/* will do this option */
#define	OPT_YES_BUT_ALWAYS_LOOK	2
#define	OPT_NO_BUT_ALWAYS_LOOK	3

#ifndef	B_FALSE
#define	B_FALSE		0
#endif

#ifndef	B_TRUE
#define	B_TRUE		1
#endif

#ifndef	bzero
#define	bzero(a,b)	memset((a),0,(b) ;
#endif


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	mkutmpid(char *,int,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	isproc(pid_t) ;
extern int	audit_settid(int);	/* set terminal ID */

extern int	progserve(struct proginfo *,STANDING *,BUILTIN *,
			struct clientinfo *,vecstr *,
			const char *,const char **) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdomain(char *) ;


/* external variables */

extern char	**environ ;


/* local structures */

/*
 * The env_list linked list is used to store the environment variables
 * until the final exec of login.  A malevolent client might try to
 * send an environment variable intended to affect the telnet daemon's
 * execution.  Right now the BANNER expansion is the only instance.
 * Note that it is okay to pass the environment variables to login
 * because login protects itself against environment variables mischief.
 */

struct envlist {
	struct envlist	*next;
	char		*name;
	char		*value;
	int		delete;
} ;


/* forward references */

static int	procsvcspec(struct proginfo *,vecstr *,char *,int) ;

static int	doit(struct proginfo *,STANDING *,BUILTIN *,
			struct clientinfo *,int,struct sockaddr_storage *) ;
static int	telnet(struct proginfo *,struct clientinfo *,pid_t,int,int) ;
static int	mkpam(struct proginfo *,const char *,const char *) ;

static int	tcsetdefault(int) ;
static int	tcspeednonzero(int) ;
static int	tcnoecho(int) ;
static int	readstream();
static int	telrcv() ;
static int	send_oob(int fd, char *ptr, int count);
static int	setenv(const char *name, const char *value, int rdebugwrite);
static int	removemod(int f, char *modname);
static int	fatal(int,const char *) ;

static int	termsecure(const char *,char *) ;
static int	telserv_service(const char *,const char *) ;

#if	CF_DEBUG || CF_DEBUGS
static int	debugfstat(const char *,int) ;
#endif

static void	drainstream();
static void	unsetenv(const char *name);
static void	suboption();
static void	showbanner() ;
static void	cleanup();

static int	fd_allzero(fd_set *) ;


/* local (?) variables */

pid_t pid;

char	remopts[256];
char	myopts[256];

uchar_t	doopt[] = { (uchar_t)IAC, (uchar_t)DO, '%', 'c', 0 };
uchar_t	dont[] = { (uchar_t)IAC, (uchar_t)DONT, '%', 'c', 0 };
uchar_t	will[] = { (uchar_t)IAC, (uchar_t)WILL, '%', 'c', 0 };
uchar_t	wont[] = { (uchar_t)IAC, (uchar_t)WONT, '%', 'c', 0 };

struct sockaddr_in6 sin6 = { AF_INET6 };

/*
 * I/O data buffers, pointers, and counters.
 */

char	ptyibuf[BUFSIZ], *ptyip = ptyibuf;

char	ptyobuf[BUFSIZ], *pfrontp = ptyobuf, *pbackp = ptyobuf;

char	*netibuf, *netip;

static int	netibufsize;

#define	NIACCUM(c)	{   *netip++ = c; \
			    ncc++; \
			}

/* the remote system seems to NOT be an old 4.2 */
int	not42 = 1;

char	netobuf[BUFSIZ], *nfrontp = netobuf, *nbackp = netobuf;
char	*neturg = 0;		/* one past last bye of urgent data */

char	binshellvar[] = "SHELL=/bin/sh";
char	defaultfile[] = "/etc/default/telnetd";
char	bannervar[] = "BANNER=";

#ifdef SYSV
char BANNER1[] = "\r\n\r\n",
    BANNER2[] = "\r\n\r\0\r\n\r\0";
#else
char BANNER1[] = "\r\n\r\n4.3 BSD UNIX(r) (",
    BANNER2[] = ")\r\n\r\0\r\n\r\0";
#endif /* SYSV */

/* buffer for sub-options */

char	subbuffer[100], *subpointer = subbuffer, *subend = subbuffer;

#define	SB_CLEAR()	subpointer = subbuffer;
#define	SB_TERM()	{ subend = subpointer; SB_CLEAR(); }
#define	SB_ACCUM(c)	if (subpointer < (subbuffer+sizeof (subbuffer))) { \
				*subpointer++ = (c); \
			}
#define	SB_GET()	((*subpointer++)&0xff)
#define	SB_EOF()	(subpointer >= subend)
#define	SB_LEN()	(subend - subpointer)

unsigned char *subsave;

#define	SB_SAVE()	subsave = subpointer;
#define	SB_RESTORE()	subpointer = subsave;

/*
 * State for recv fsm
 */
#define	TS_DATA		0	/* base state */
#define	TS_IAC		1	/* look for double IAC's */
#define	TS_CR		2	/* CR-LF ->'s CR */
#define	TS_SB		3	/* throw away begin's... */
#define	TS_SE		4	/* ...end's (suboption negotiation) */
#define	TS_WILL		5	/* will option negotiation */
#define	TS_WONT		6	/* wont " */
#define	TS_DO		7	/* do " */
#define	TS_DONT		8	/* dont " */

int	ncc;
int	master;		/* master side of pty */
int	pty;		/* side of pty that gets ioctls */
int	net;
int	inter;
int	SYNCHing = 0;		/* we are in TELNET SYNCH mode */
int	state = TS_DATA;

int env_ovar = -1;	/* XXX.sparker */
int env_ovalue = -1;	/* XXX.sparker */

boolean_t	telmod_init_done = B_FALSE;

char	*line;


static struct envlist *envlist_head = NULL;

/*
 * The following are some clocks used to decide how to interpret
 * the relationship between various variables.
 */

struct {
    time_t
	system,			/* what the current time is */
	echotoggle,		/* last time user entered echo character */
	modenegotiated,		/* last time operating mode negotiated */
	didnetreceive,		/* last time we read data from network */
	ttypeopt,		/* ttype will/won't received */
	ttypesubopt,		/* ttype subopt is received */
	getterminal,		/* time started to get terminal information */
	xdisplocopt,		/* xdisploc will/wont received */
	xdisplocsubopt,		/* xdisploc suboption received */
	nawsopt,		/* window size will/wont received */
	nawssubopt,		/* window size received */
	environopt,		/* environment option will/wont received */
	oenvironopt,		/* "old" environ option will/wont received */
	environsubopt,		/* environment option suboption received */
	oenvironsubopt,		/* "old environ option suboption received */
	gotDM;			/* when did we last see a data mark */
} clocks ;

int init_neg_done;

#define	settimer(x)	(clocks.x = ++clocks.system)
#define	sequenceIs(x, y)	(clocks.x < clocks.y)

char	*terminaltype = NULL ;
char	*envinit[2];


/* local variables */

#if	CF_LOCALSVC

static const char *termtypes[] = {
	"screen",
	"vt100",
	"vt101",
	"vt102",
	"vt220",
	"vt240",
	"vt320",
	"vt340",
	"vt420",
	"vt440",
	"vt520",
	"vt540",
	"ansi",
	"xterm",
	NULL
} ;

static const char *services[] = {
	"talker",			/* historical */
	"talk",
	"weather",
	NULL
} ;

#endif /* CF_LOCALSVC */


/*
 * new_env
 *
 * Used to add an environment variable and value to the
 * linked list structure.
 */

static int
new_env(const char *name, const char *value)
{
	struct envlist *env, *index;

	env = malloc(sizeof (struct envlist));
	if (env == NULL)
		return (1);
	if ((env->name = strdup(name)) == NULL) {
		free(env);
		return (1);
	}
	if ((env->value = strdup(value)) == NULL) {
		free(env->name);
		free(env);
		return (1);
	}
	env->delete = 0;
	env->next = envlist_head;
	envlist_head = env;
	return (0);
}

/*
 * del_env
 *
 * Used to delete an environment variable from the linked list
 * structure.  We just set a flag because we will delete the list
 * anyway before we exec login.
 */

static int
del_env(const char *name)
{
	struct envlist *env;

	for (env = envlist_head; env; env = env->next) {
		if (strcmp(env->name, name) == 0) {
			env->delete = 1;
			break;
		}
	}
	return (0);
}

static int
issock(int fd)
{
	struct ustat stats;

	if (fstat(fd, &stats) == -1)
		return (0);
	return (S_ISSOCK(stats.st_mode));
}


int proghandle(pip,sop,bop,cip) 
struct proginfo		*pip ;
STANDING		*sop ;
BUILTIN			*bop ;
struct clientinfo	*cip ;
{
	struct sockaddr_storage from;

	socklen_t fromlen;

	int	on = 1 ;

	int 	issocket;

	char	*cp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("proghandle: entered\n") ;
#endif /* CF_DEBUG */

	if (!(netibuf = (char *)malloc(BUFSIZ)))
		syslog(LOG_ERR, "netibuf malloc failed\n");
	netibufsize = BUFSIZ;

	bzero(netibuf, BUFSIZ);

	netip = netibuf;

	openlog("telserv", LOG_PID | LOG_ODELAY, LOG_DAEMON);

	issocket = issock(0);
	if (!issocket)
		fatal(0, "stdin is not a socket file descriptor");

	fromlen = (socklen_t)sizeof (from);
	bzero((char *)&from, sizeof (from));

	if (getpeername(0, (struct sockaddr *)&from, &fromlen) < 0) {

#ifdef	COMMENT
		fprintf(stderr, "%s: ", argv[0]);
#endif

		perror("getpeername");
		_exit(1);
	}

	if (audit_settid(0)) {	/* set terminal ID */

#ifdef	COMMENT
		fprintf(stderr, "%s: ", argv[0]);
#endif

		perror("audit");
		exit(1);
	}

	if (setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, (const char *)&on,
						sizeof (on)) < 0) {
		syslog(LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");
	}

#if	defined(SO_OOBINLINE)
	if (setsockopt(net, SOL_SOCKET, SO_OOBINLINE, (char *)&on,
	    sizeof (on)) < 0) {
		syslog(LOG_WARNING, "setsockopt (SO_OOBINLINE): %m");
	}
#endif	/* defined(SO_OOBINLINE) */

	doit(pip,sop,bop,cip,0, &from);

	close(0) ;

	close(1) ;

	return 0 ;
}
/* end subroutine (proghandle) */


/* local subroutines */


static int procsvcspec(pip,nelp,svcspec,svcspeclen)
struct proginfo		*pip ;
vecstr			*nelp ;
char			svcspec[] ;
int			svcspeclen ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	vi ;
	int	sl ;
	int	len = 0 ;

	const char	*tp, *cp ;
	const char	*sp ;


	if (nelp == NULL)
	    return SR_FAULT ;

	if (svcspec == NULL)
	    return SR_FAULT ;

	svcspec[0] = '\0' ;
	rs1 = vecstr_search(nelp,VARTERM,vstrkeycmp,&sp) ;
	vi = rs1 ;
	if (rs1 < 0)
	    goto ret0 ;

	if ((tp = strchr(sp,'=')) == NULL)
	    goto ret0 ;

	sl = (tp - sp) ;
	cp = (tp + 1) ;
	if ((tp = strchr(cp,'_')) != NULL) {

	    len = strwcpy(svcspec,(tp + 1),svcspeclen) - svcspec ;

	    if (len > 0) {
	        rs = vecstr_add(nelp,sp,sl) ;
	        vecstr_del(nelp,vi) ;
	    }

	} /* end if */

ret0:
	return (rs >= 0) ? len : rs ;
}
/* end subroutines (procsvcspec) */


/*
 * ttloop
 *
 *	A small subroutine to flush the network output buffer, get some data
 * from the network, and pass it through the telnet state machine.  We
 * also flush the pty input buffer (by dropping its data) if it becomes
 * too full.
 */
void
ttloop()
{


	if (nfrontp-nbackp) {
		netflush();
	}
	ncc = read(net, netibuf, netibufsize);
	if (ncc < 0) {
		syslog(LOG_INFO, "ttloop:  read: %m\n");
		exit(1);
	} else if (ncc == 0) {
		syslog(LOG_INFO, "ttloop:  peer died: %m\n");
		exit(1);
	}
	netip = netibuf;
	telrcv();		/* state machine */
	if (ncc > 0) {
		pfrontp = pbackp = ptyobuf;
		telrcv();
	}
}

void
send_do(int option)
{
	*nfrontp++ = (uchar_t)IAC;
	*nfrontp++ = (uchar_t)DO;
	*nfrontp++ = option;
}
/*
 * getterminaltype
 *
 *	Ask the other end to send along its terminal type.
 * Output is the variable terminaltype filled in.
 */

void
getterminaltype()
{


	settimer(getterminal);

	send_do(TELOPT_TTYPE);
	remopts[TELOPT_TTYPE] = OPT_YES_BUT_ALWAYS_LOOK;

	send_do(TELOPT_NAWS);
	remopts[TELOPT_NAWS] = OPT_YES_BUT_ALWAYS_LOOK;

	send_do(TELOPT_XDISPLOC);
	remopts[TELOPT_XDISPLOC] = OPT_YES_BUT_ALWAYS_LOOK;

	send_do(TELOPT_NEW_ENVIRON);
	remopts[TELOPT_NEW_ENVIRON] = OPT_YES_BUT_ALWAYS_LOOK;

	send_do(TELOPT_OLD_ENVIRON);
	remopts[TELOPT_OLD_ENVIRON] = OPT_YES_BUT_ALWAYS_LOOK;

	while (sequenceIs(ttypeopt, getterminal) ||
	    sequenceIs(nawsopt, getterminal) ||
	    sequenceIs(xdisplocopt, getterminal) ||
	    sequenceIs(environopt, getterminal) ||
	    sequenceIs(oenvironopt, getterminal)) {
		ttloop();
	}
	if (remopts[TELOPT_TTYPE] == OPT_YES) {
		static uchar_t sbbuf[] = { (uchar_t)IAC, (uchar_t)SB,
		    (uchar_t)TELOPT_TTYPE, (uchar_t)TELQUAL_SEND,
		    (uchar_t)IAC, (uchar_t)SE };

		bcopy(sbbuf, nfrontp, sizeof (sbbuf));
		nfrontp += sizeof (sbbuf);
	}
	if (remopts[TELOPT_XDISPLOC] == OPT_YES) {
		static uchar_t sbbuf[] = { (uchar_t)IAC, (uchar_t)SB,
		    (uchar_t)TELOPT_XDISPLOC, (uchar_t)TELQUAL_SEND,
		    (uchar_t)IAC, (uchar_t)SE };

		bcopy(sbbuf, nfrontp, sizeof (sbbuf));
		nfrontp += sizeof (sbbuf);
	}
	if (remopts[TELOPT_NEW_ENVIRON] == OPT_YES) {
		static uchar_t sbbuf[] = { (uchar_t)IAC, (uchar_t)SB,
		    (uchar_t)TELOPT_NEW_ENVIRON, (uchar_t)TELQUAL_SEND,
		    (uchar_t)IAC, (uchar_t)SE };

		bcopy(sbbuf, nfrontp, sizeof (sbbuf));
		nfrontp += sizeof (sbbuf);
	}
	if (remopts[TELOPT_OLD_ENVIRON] == OPT_YES) {
		static uchar_t sbbuf[] = { (uchar_t)IAC, (uchar_t)SB,
		    (uchar_t)TELOPT_OLD_ENVIRON, (uchar_t)TELQUAL_SEND,
		    (uchar_t)IAC, (uchar_t)SE };

		bcopy(sbbuf, nfrontp, sizeof (sbbuf));
		nfrontp += sizeof (sbbuf);
	}
	if (remopts[TELOPT_TTYPE] == OPT_YES) {
		while (sequenceIs(ttypesubopt, getterminal)) {
			ttloop();
		}
	}
	if (remopts[TELOPT_XDISPLOC] == OPT_YES) {
		while (sequenceIs(xdisplocsubopt, getterminal)) {
			ttloop();
		}
	}
	if (remopts[TELOPT_NEW_ENVIRON] == OPT_YES) {
		while (sequenceIs(environsubopt, getterminal)) {
			ttloop();
		}
	}
	if (remopts[TELOPT_OLD_ENVIRON] == OPT_YES) {
		while (sequenceIs(oenvironsubopt, getterminal)) {
			ttloop();
		}
	}
	init_neg_done = 1;
}
/* end subroutine (getterminaltype) */


/*
 * Get a pty, scan input lines.
 */
static int doit(pip,sop,bop,cip,f, who)
struct proginfo		*pip ;
STANDING		*sop ;
BUILTIN			*bop ;
struct clientinfo	*cip ;
int f;
struct sockaddr_storage *who;
{
	struct sgttyb b;
	struct hostent *hp;
	struct	stat	buf;
	struct	protocol_arg	telnetp;
	struct	strioctl	telnetmod;
	struct	envlist	*env, *next;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;

	    vecstr	envs ;

	int	rs = SR_OK ;
	int	cl ;
	int i, p, t, tt;
	int c;
	int	ptmfd;	/* fd of logindmux connected to pty */
	int	netfd;	/* fd of logindmux connected to netf */
	    int	opts = VECSTR_OCOMPACT ;
	int	nsize = 0;
	int	f_write = FALSE ;

	char	svcspec[SVCSPECLEN + 1] ;
	char	svcargs[SVCARGSLEN + 1] ;
	char abuf[INET6_ADDRSTRLEN];
	char host_name[MAXHOSTNAMELEN + 1];
	char *host;
	char	*cp ;

	socklen_t wholen;
#ifdef SYSV
	char *slavename;
	extern char *ptsname();
#endif /* SYSV */


#if	CF_EXTRACLOSE
	if (cip->fd_input >= 3) {
	    u_close(cip->fd_input) ;
	    cip->fd_input = -1 ;
	}

	if (cip->fd_output >= 3) {
	    u_close(cip->fd_output) ;
	    cip->fd_output = -1 ;
	}
#endif /* CF_EXTRACLOSE */

#ifdef	SYSV
	if ((p = open("/dev/ptmx", O_RDWR | O_NOCTTY)) == -1) {
		fatalperror(f, "open /dev/ptmx", errno);
	}
	if (grantpt(p) == -1)
		fatal(f, "could not grant slave pty");
	if (unlockpt(p) == -1)
		fatal(f, "could not unlock slave pty");
	if ((slavename = ptsname(p)) == NULL)
		fatal(f, "could not enable slave pty");
	dup2(f, 0);
	if ((t = open(slavename, O_RDWR | O_NOCTTY)) == -1)
		fatal(f, "could not open slave pty");
	if (ioctl(t, I_PUSH, "ptem") == -1)
		fatalperror(f, "ioctl I_PUSH ptem", errno);
	if (ioctl(t, I_PUSH, "ldterm") == -1)
		fatalperror(f, "ioctl I_PUSH ldterm", errno);

#if	CF_TTCOMPAT
	if (ioctl(t, I_PUSH, "ttcompat") == -1)
		fatalperror(f, "ioctl I_PUSH ttcompat", errno);
#endif /* CF_TTCOMPAT */

	line = slavename;

#else /* SYSV */

	for (c = 'p'; c <= 's'; c++) {
		struct ustat stb;

		line = "/dev/ptyXX";
		line[strlen("/dev/pty")] = c;
		line[strlen("/dev/ptyp")] = '0';
		if (stat(line, &stb) < 0)
			break;
		for (i = 0; i < 16; i++) {
			line[strlen("/dev/ptyp")] = "0123456789abcdef"[i];
			p = open(line,O_RDWR) ;
			if (p > 0)
				goto gotpty;
		}
	}
	fatal(f, "All network ports in use");
	/*NOTREACHED*/
gotpty:
	dup2(f, 0);
	line[strlen("/dev/")] = 't';
	t = open("/dev/tty", O_RDWR);
	if (t >= 0) {
		ioctl(t, TIOCNOTTY, 0);
		close(t);
	}
	t = open(line, O_RDWR);
	if (t < 0)
		fatalperror(f, line, errno);

#endif /* SYSV */

#ifdef SYSV
	pty = t;
#else
	pty = p;
#endif /* SYSV */

#if	CF_TTCOMPAT
/* verify a non-zero speed */
	if (ioctl(t, TIOCGETP, &b) == -1)
		syslog(LOG_INFO, "ioctl TIOCGETP pty t: %m\n");
	b.sg_flags = CRMOD|XTABS|ANYP;
	/* XXX - ispeed and ospeed must be non-zero */
	b.sg_ispeed = B38400;
	b.sg_ospeed = B38400;
	if (ioctl(t, TIOCSETN, &b) == -1)
		syslog(LOG_INFO, "ioctl TIOCSETN pty t: %m\n");
	if (ioctl(pty, TIOCGETP, &b) == -1)
		syslog(LOG_INFO, "ioctl TIOCGETP pty pty: %m\n");
	b.sg_flags &= ~ECHO;
	if (ioctl(pty, TIOCSETN, &b) == -1)
		syslog(LOG_INFO, "ioctl TIOCSETN pty pty: %m\n");

#else /* CF_TTCOMPAT */

#ifdef	COMMENT
	tcspeednonzero(t) ;
#else
	tcsetdefault(t) ;
#endif

	tcnoecho(pty) ;

#endif /* CF_TTCOMPAT */

/* we made the pseudo terminal, turn off interrupts on it!!! */

#if	CF_TTCOMPAT
	{
		struct termios	ts ;

		int	rs ;


		rs = uc_tcgetattr(t,&ts) ;
		if (rs >= 0) {
			ts.c_lflag &= (~ (ISIG | IEXTEN)) ;
			uc_tcsetattr(t,TCSANOW,&ts) ;
		}

	} /* end block */
#endif /* CF_TTCOMPAT */

/* OK, continue with other stuff */

	if (who->ss_family == AF_INET) {
		sin = (struct sockaddr_in *)who;
		wholen = sizeof (struct sockaddr_in);
	} else if (who->ss_family == AF_INET6) {
		sin6 = (struct sockaddr_in6 *)who;
		wholen = sizeof (struct sockaddr_in6);
	} else {
		syslog(LOG_ERR, "unknown address family %d\n",
		    who->ss_family);
		fatal(f, "getpeername: unknown address family\n");
	}

	if (getnameinfo((const struct sockaddr *) who, wholen, host_name,
	    sizeof (host_name), NULL, 0, 0) == 0) {

		host = host_name;

	} else {

		if (who->ss_family == AF_INET6) {
			if (IN6_IS_ADDR_V4MAPPED(&sin6->sin6_addr)) {
				struct in_addr ipv4_addr;

				IN6_V4MAPPED_TO_INADDR(&sin6->sin6_addr,
				    &ipv4_addr);
				host = (char *)inet_ntop(AF_INET,
				    &ipv4_addr, abuf, sizeof (abuf));
			} else {
				host = (char *)inet_ntop(AF_INET6,
				    &sin6->sin6_addr, abuf,
				    sizeof (abuf));
			}
		} else if (who->ss_family == AF_INET) {
				host = (char *)inet_ntop(AF_INET,
				    &sin->sin_addr, abuf, sizeof (abuf));
			}

	}

	/*
	 * get terminal type.
	 */
	getterminaltype();

	if (terminaltype != NULL)
		terminaltype[TERMLEN] = '\0' ;

	syslog(LOG_INFO, "host=%s",host) ;

	if (terminaltype != NULL)
		syslog(LOG_INFO, "term=%s",(terminaltype + 5)) ;

	else
		syslog(LOG_INFO, "term=") ;

	/*
	 * Note that sockmod has to be removed since readstream assumes
	 * a "raw" TPI endpoint (e.g. it uses getmsg).
	 */
	if (removemod(f, "sockmod") < 0)
		fatalperror(f, "couldn't remove sockmod", errno);

	if (ioctl(f, I_PUSH, "telmod") < 0)
		fatalperror(f, "ioctl I_PUSH telmod", errno);

	if (!ncc)
		netip = netibuf;

	/*
	 * readstream will do a getmsg till it receives M_PROTO type
	 * T_DATA_REQ from telnetmodopen().  This signals that all data
	 * in-flight before telmod was pushed has been received at the
	 * stream head.
	 */
	while ((nsize = readstream(f, netibuf, ncc + netip - netibuf)) > 0) {
		ncc += nsize;
	}
	if (nsize < 0) {
		fatalperror(f, "readstream failed\n", errno);
	}

	/*
	 * open logindmux drivers and link them with network and ptm
	 * file descriptors.
	 */
	if ((ptmfd = open("/dev/logindmux", O_RDWR)) == -1) {
		fatalperror(f, "open /dev/logindmux", errno);
	}
	if ((netfd = open("/dev/logindmux", O_RDWR)) == -1) {
		fatalperror(f, "open /dev/logindmux", errno);
	}

	if (ioctl(ptmfd, I_LINK, p) < 0)
		fatal(f, "ioctl I_LINK of /dev/ptmx failed\n");
	if (ioctl(netfd, I_LINK, f) < 0)
		fatal(f, "ioctl I_LINK of tcp connection failed\n");

	/*
	 * Figure out the device number of ptm's mux fd, and pass that
	 * to the net's mux.
	 */
	if (fstat(ptmfd, &buf) < 0) {
		fatalperror(f, "fstat ptmfd failed", errno);
	}
	telnetp.dev = buf.st_rdev;
	telnetp.flag = 0;

	telnetmod.ic_cmd = LOGDMX_IOC_QEXCHANGE;
	telnetmod.ic_timout = -1;
	telnetmod.ic_len = sizeof (struct protocol_arg);
	telnetmod.ic_dp = (char *)&telnetp;

	if (ioctl(netfd, I_STR, &telnetmod) < 0)
		fatal(netfd, "ioctl LOGDMX_IOC_QEXCHANGE of netfd failed\n");

	/*
	 * Figure out the device number of the net's mux fd, and pass that
	 * to the ptm's mux.
	 */
	if (fstat(netfd, &buf) < 0) {
		fatalperror(f, "fstat netfd failed", errno);
	}
	telnetp.dev = buf.st_rdev;
	telnetp.flag = 1;

	telnetmod.ic_cmd = LOGDMX_IOC_QEXCHANGE;
	telnetmod.ic_timout = -1;
	telnetmod.ic_len = sizeof (struct protocol_arg);
	telnetmod.ic_dp = (char *)&telnetp;

	if (ioctl(ptmfd, I_STR, &telnetmod) < 0)
		fatal(netfd, "ioctl LOGDMX_IOC_QEXCHANGE of ptmfd failed\n");

	net = netfd;
	master = ptmfd;

	/* * Show banner that getty never gave.  */

	if (pip->f.showsysbanner)
		showbanner() ;

	if ((pid = fork()) < 0)
		fatalperror(netfd, "fork", errno);

	if (pid)
		telnet(pip,cip,pid,net,master) ;

	/*
	 * The child process needs to be the session leader
	 * and have the pty as its controlling tty.  Thus we need
	 * to re-open the slave side of the pty now without
	 * the O_NOCTTY flag that we have been careful to
	 * use up to this point.
	 */

#ifdef SYSV
	(void) setsid();
#else
	(void) setpgrp(0, 0);	/* BSD compatible setsid() */
#endif /* SYSV */

	tt = open(line, O_RDWR) ;
	if (tt < 0)
		fatalperror(netfd, line, errno);

	(void) close(netfd);
	(void) close(ptmfd);
	(void) close(f);
	(void) close(p);
	(void) close(t);

	if (tt != 0)
		dup2(tt, 0);
	if (tt != 1)
		dup2(tt, 1);

#ifdef	COMMENT
	if (tt != 2)
		dup2(tt, 2);
#endif

	if (tt > 2)
		close(tt);

#if	CF_CHMOD
	u_chmod(line,0620) ;
#endif

	if (terminaltype != NULL)
		setenv("TERM",(terminaltype+5), 1);

#if	CF_DEBUGS
	syslog(LOG_DEBUG, "term=%s\n", (terminaltype + 5)) ;
#endif

	/*
	 * 	-h : pass on name of host.
	 *		WARNING:  -h is accepted by login if and only if
	 *			getuid() == 0.
	 * 	-p : don't clobber the environment (so terminal type stays set).
	 */

/* System V login expects a UTMP entry to already be there */

	{
		struct utmpx ut;

		int	uel ;


		memset(&ut, 0,sizeof (ut)) ;

		strncpy(ut.ut_user,pip->username, sizeof(ut.ut_user));

		strncpy(ut.ut_line, (line + 5), sizeof(ut.ut_line));

		uel = sizeof(ut.ut_host) ;
		strncpy(ut.ut_host,host,uel) ;

		ut.ut_syslen = strnlen(host,uel) ;

		ut.ut_pid = getpid();

#if	CF_MKUTMPID

#if	CF_DEBUGS
		nprintf(DEBFNAME,"doit: mkutmpid()\n") ;
		nprintf(DEBFNAME,"doit: termdev=%s\n",line) ;
#endif

		rs = mkutmpid(ut.ut_id,4,line,-1) ;

#if	CF_DEBUGS
		nprintf(DEBFNAME,"doit: mkutmpid() rs=%d id=>%t<\n",
			rs,ut.ut_id,strnlen(ut.ut_id,4)) ;
#endif

#else /* CF_MKUTMPID */

		ut.ut_id[0] = 't';
		ut.ut_id[1] = (char)SC_WILDC;
		ut.ut_id[2] = (char)SC_WILDC;
		ut.ut_id[3] = (char)SC_WILDC;

#endif /* CF_MKUTMPID */

		ut.ut_type = USER_PROCESS;
		ut.ut_exit.e_termination = 0;
		ut.ut_exit.e_exit = 0;
		(void) time(&ut.ut_tv.tv_sec);

#if	CF_MAKEUTX

		if (makeutx(&ut) == NULL)
			syslog(LOG_INFO, "%s: makeutx() failed",
			PROGNAME);

#else /* CF_MAKEUTX */

		if (pututxline(&ut) == NULL)
			syslog(LOG_INFO, "%s: pututxline() failed",
			PROGNAME);

#endif /* CF_MAKEUTX */

	} /* end block */

/* clear the supplemental group list */

#if	CF_CLEARGROUPS
	{
		gid_t	gids[2] ;


		gids[0] = -1 ;
		u_setgroups(0,gids) ;

	} /* end block */
#endif /* CF_CLEARGROUPS */

#if	CF_TTCOMPATPOP
	removemod(FD_STDIN,"ttcompat") ;
#endif

	/*
	 * Load in the cached environment variables and either
	 * set/unset them in the environment.
	 */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("proghandle/doit: termtype=%s\n",
		(terminaltype + 5)) ;
#endif

	opts = VECSTR_OCOMPACT ;
	if ((rs = vecstr_start(&envs,10,opts)) >= 0) {

		const char	*sav[1] ;


		rs = vecstr_envadd(&envs,VARTERM,(terminaltype + 5),-1) ;

		for (next = envlist_head; (rs >= 0) && next ; ) {

		    env = next;
		    rs = vecstr_envadd(&envs,env->name,env->value,-1);
		    if (rs < 0)
			break ;

#ifdef	COMMENT
		if (env->delete)
			(void) unsetenv(env->name);
		else
			(void) setenv(env->name, env->value, 1);
#endif /* COMMENT */

		free(env->name);
		free(env->value);
		next = env->next;
		free(env);

		} /* end for */

/* find server and perform final (roughly) server processing */

		svcspec[0] = '\0' ;
		svcargs[0] = '\0' ;
		if (rs >= 0) {

/* pop off the service name */

		rs = procsvcspec(pip,&envs,svcspec,SVCSPECLEN) ;
		cl = rs ;

		if (rs >= 0) {
		    if ((cl > 0) && (svcspec[cl- 1] == '\n'))
	    	        cl -= 1 ;
		    svcspec[cl] = '\0' ;
		}

		} /* end if */

		if (rs >= 0) {
		    sav[0] = NULL ;
		    rs = progserve(pip,sop,bop,cip,&envs,svcspec,sav) ;
	 	}

		vecstr_finish(&envs) ;

	} /* end if (initialized) */

	if (pid == 0)
	    uc_exit(EX_OK) ;

	/*NOTREACHED*/
	return 0 ;
}
/* end subroutine (doit) */


static int fatal(f, msg)
int 		f ;
const char	*msg;
{
	char buf[BUFSIZ];

	(void) sprintf(buf, "telnetd: %s.\r\n", msg);
	(void) write(f, buf, strlen(buf));
	exit(1);
	/*NOTREACHED*/
}

ffprintf(f, msg)
	int f;
	char *msg;
{
	char buf[BUFSIZ];

	(void) sprintf(buf, "telnetd: %s", msg);
	(void) write(f, buf, strlen(buf));
}

fatalperror(f, msg, lerrno)
	int f;
	char *msg;
	int lerrno;
{
	char buf[BUFSIZ];


	(void) sprintf(buf, "%s: %s\r\n", msg, strerror(lerrno));
	fatal(f, buf);
	/*NOTREACHED*/
}


/*
 * Check a descriptor to see if out of band data exists on it.
 */


stilloob(s)
int	s;		/* socket number */
{
	static struct timeval timeout = { 0, 0 };
	fd_set	excepts;
	int value;

	do {
		FD_ZERO(&excepts);
		FD_SET(s, &excepts);
		value = select(s+1, (fd_set *)0, (fd_set *)0, &excepts,
		    &timeout);
	} while ((value == -1) && (errno == EINTR));

	if (value < 0) {
		fatalperror(pty, "select", errno);
	}
	if (FD_ISSET(s, &excepts)) {
		return (1);
	} else {
		return (0);
	}
}

/*
 * Main loop.  Select from pty and network, and
 * hand data to telnet receiver finite state machine
 * when it receives telnet protocol. Regular data
 * flow between pty and network takes place through
 * in-kernel telnet streams module (telmod).
 */
static int telnet(pip,cip,pid,net, master)
struct proginfo		*pip ;
struct clientinfo	*cip ;
pid_t			pid ;
int			net ;
int			master ;
{
	struct	strioctl	telnetmod;

	struct timeval		to ;

	int	rs1 ;
	int	cstat ;
	int	nmax ;
	int	nsize = 0;
	int	on = 1;
	int	cnoprog = 0 ;
	int	f_isproc = TRUE ;

	char	mode;
	char	binary_in = 0, binary_out = 0;


	to.tv_sec = 4 ;
	to.tv_usec = 0 ;

	if (cip->fd_input >= 0) {
	    u_close(cip->fd_input) ;
	    cip->fd_input = -1 ;
	}

	if (cip->fd_output >= 0) {
	    u_close(cip->fd_output) ;
	    cip->fd_output = -1 ;
	}

	if (ioctl(net, FIONBIO, &on) == -1)
		syslog(LOG_INFO, "ioctl FIONBIO net: %m\n");
	if (ioctl(master, FIONBIO, &on) == -1)
		syslog(LOG_INFO, "ioctl FIONBIO pty p: %m\n");
	signal(SIGTSTP, SIG_IGN);
	signal(SIGCHLD, (void (*)())cleanup);
	setpgrp();

	/*
	 * Request to do remote echo and to suppress go ahead.
	 */
	if (!myopts[TELOPT_ECHO]) {
	    dooption(TELOPT_ECHO);
	}
	if (!myopts[TELOPT_SGA]) {
	    dooption(TELOPT_SGA);
	}
	/*
	 * Is the client side a 4.2 (NOT 4.3) system?  We need to know this
	 * because 4.2 clients are unable to deal with TCP urgent data.
	 *
	 * To find out, we send out a "DO ECHO".  If the remote system
	 * answers "WILL ECHO" it is probably a 4.2 client, and we note
	 * that fact ("WILL ECHO" ==> that the client will echo what
	 * WE, the server, sends it; it does NOT mean that the client will
	 * echo the terminal input).
	 */
	sprintf(nfrontp, (char *)doopt, TELOPT_ECHO);
	nfrontp += sizeof (doopt)-2;
	remopts[TELOPT_ECHO] = OPT_YES_BUT_ALWAYS_LOOK;

	/*
	 * Call telrcv() once to pick up anything received during
	 * terminal type negotiation.
	 */
	telrcv();

	netflush();
	ptyflush();

	for (;;) {

		fd_set ibits, obits, xbits;
		register int c;
		    int	f = TRUE ;


		if (ncc < 0)
			break;

		FD_ZERO(&ibits);
		FD_ZERO(&obits);
		FD_ZERO(&xbits);

		/*
		 * If we couldn't flush all our output to the network,
		 * keep checking for when we can.
		 */
		if (nfrontp - nbackp)
			FD_SET(net, &obits);
		/*
		 * Never look for input if there's still
		 * stuff in the corresponding output buffer
		 */
		if (pfrontp - pbackp) {
			FD_SET(master, &obits);
		} else {
			FD_SET(net, &ibits);
		}
		if (!SYNCHing) {
			FD_SET(net, &xbits);
		}

#define	max(x, y)	(((x) < (y)) ? (y) : (x))


		/*
		 * make an ioctl to telnet module (net side) to send
		 * binary mode of telnet daemon. binary_in and
		 * binary_out are 0 if not in binary mode.
		 */

		if (binary_in != myopts[TELOPT_BINARY] ||
			binary_out != remopts[TELOPT_BINARY]) {

			mode = 0;
			if (myopts[TELOPT_BINARY] != OPT_NO)
				mode |= TEL_BINARY_IN;

			if (remopts[TELOPT_BINARY] != OPT_NO)
				mode |= TEL_BINARY_OUT;

			telnetmod.ic_cmd = TEL_IOC_MODE;
			telnetmod.ic_timout = -1;
			telnetmod.ic_len = 1;
			telnetmod.ic_dp = &mode;

			syslog(LOG_DEBUG, "TEL_IOC_MODE binary has changed\n");

			if (ioctl(net, I_STR, &telnetmod) < 0)
				fatal(net, "ioctl TEL_IOC_MODE failed\n");
			binary_in = myopts[TELOPT_BINARY];
			binary_out = remopts[TELOPT_BINARY];
		}

		if (state == TS_DATA) {
			if ((nfrontp == nbackp) &&
				(pfrontp == pbackp)) {
				if (ioctl(net, I_NREAD, &nsize) < 0)
					fatalperror(net,
					    "ioctl I_NREAD failed\n", errno);
				if (nsize)
					drainstream(nsize);

				/*
				 * make an ioctl to reinsert remaining data at
				 * streamhead. After this, ioctl reenables the
				 * telnet lower put queue. This queue was
				 * noenabled by telnet module after sending
				 * protocol/urgent data to telnetd.
				 */
				telnetmod.ic_cmd = TEL_IOC_ENABLE;
				telnetmod.ic_timout = -1;
				if (ncc || nsize) {
					telnetmod.ic_len = ncc + nsize;
					telnetmod.ic_dp = netip;
				} else {
					telnetmod.ic_len = 0;
					telnetmod.ic_dp = NULL;
				}
				if (ioctl(net, I_STR, &telnetmod) < 0) {

#if	CF_IOFATAL
				fatal(net, "ioctl TEL_IOC_ENABLE failed\n");
#else
				break ;
#endif

				}
				telmod_init_done = B_TRUE;

				netip = netibuf;
				bzero(netibuf, netibufsize);

				ncc = 0;
			}
		} else {
			/*
			 * state not changed to TS_DATA and hence, more to read
			 * send ioctl to get one more message block.
			 */
			telnetmod.ic_cmd = TEL_IOC_GETBLK;
			telnetmod.ic_timout = -1;
			telnetmod.ic_len = 0;
			telnetmod.ic_dp = NULL;

			if (ioctl(net, I_STR, &telnetmod) < 0) {

#if	CF_IOFATAL
				fatal(net, "ioctl TEL_IOC_GETBLK failed\n");
#else
				break ;
#endif

			}

		} /* end if */

		    f = f && FD_ALLZERO(&ibits) ;
		    f = f && FD_ALLZERO(&obits) ;
		    f = f && FD_ALLZERO(&xbits) ;
		    if (f)
			break ;

/* enter the poll loop */

		c = 0 ;
		cnoprog = 0 ;
		f_isproc = TRUE ;
		nmax = (max(net,master) + 1) ;
		while (c == 0) {

		    fd_set sibits, sobits, sxbits;


		    memcpy(&sibits,&ibits,sizeof(fd_set)) ;
		    memcpy(&sobits,&obits,sizeof(fd_set)) ;
		    memcpy(&sxbits,&xbits,sizeof(fd_set)) ;

		    c = select(nmax, &sibits, &sobits, &sxbits,&to) ;

		    if (c == 0) {

			rs1 = u_waitpid(pid,&cstat,W_OPTIONS) ;
			f_isproc = ((rs1 != SR_CHILD) && (rs1 <= 0)) ;

#if	CF_DEBUG
		if (DEBUGLEVEL(5))
		debugprintf("proghandle/telnet: u_waitpid() rs=%d f=%u\n",
			rs1,f_isproc) ;
#endif

			if (rs1 == 0)
			    f_isproc = isproc(pid) ; /* this is double check! */

#if	CF_DEBUG
		if (DEBUGLEVEL(5))
		debugprintf("proghandle/telnet: isproc() f=%u\n",
		f_isproc) ;
#endif

			if (! f_isproc)
			    cnoprog += 1 ;

			if (cnoprog >= NCHILDGONE)
			    break ;

		    } /* end if (no network activity) */

		} /* end while (waiting for network activity) */

		if (cnoprog >= 10)
			break ;

		if (c < 1) {

			if (c == -1) {
				if (errno == EINTR) {
					continue;
				}
			}
			sleep(5);
			continue;
		}

		/*
		 * Any urgent data?
		 */
		if (FD_ISSET(net, &xbits)) {
			SYNCHing = 1;
		}

		/*
		 * Something to read from the network...
		 */
		if (FD_ISSET(net, &ibits)) {
#if	!defined(SO_OOBINLINE)
			/*
			 * In 4.2 (and 4.3 beta) systems, the
			 * OOB indication and data handling in the kernel
			 * is such that if two separate TCP Urgent requests
			 * come in, one byte of TCP data will be overlaid.
			 * This is fatal for Telnet, but we try to live
			 * with it.
			 *
			 * In addition, in 4.2 (and...), a special protocol
			 * is needed to pick up the TCP Urgent data in
			 * the correct sequence.
			 *
			 * What we do is:  if we think we are in urgent
			 * mode, we look to see if we are "at the mark".
			 * If we are, we do an OOB receive.  If we run
			 * this twice, we will do the OOB receive twice,
			 * but the second will fail, since the second
			 * time we were "at the mark", but there wasn't
			 * any data there (the kernel doesn't reset
			 * "at the mark" until we do a normal read).
			 * Once we've read the OOB data, we go ahead
			 * and do normal reads.
			 *
			 * There is also another problem, which is that
			 * since the OOB byte we read doesn't put us
			 * out of OOB state, and since that byte is most
			 * likely the TELNET DM (data mark), we would
			 * stay in the TELNET SYNCH (SYNCHing) state.
			 * So, clocks to the rescue.  If we've "just"
			 * received a DM, then we test for the
			 * presence of OOB data when the receive OOB
			 * fails (and AFTER we did the normal mode read
			 * to clear "at the mark").
			 */
		    if (SYNCHing) {
			int atmark = 0;

			if (ioctl(net, SIOCATMARK, (char *)&atmark) == -1)
				syslog(LOG_INFO, "ioctl SIOCATMARK: %m\n");
			if (atmark) {
			    ncc = recv(net, netibuf, netibufsize, MSG_OOB);
			    if ((ncc == -1) && (errno == EINVAL)) {
				ncc = read(net, netibuf, netibufsize);
				if (sequenceIs(didnetreceive, gotDM)) {
				    SYNCHing = stilloob(net);
				}
			    }
			} else {
			    ncc = read(net, netibuf, netibufsize);
			}
		    } else {
			ncc = read(net, netibuf, netibufsize);
		    }
		    settimer(didnetreceive);
#else	/* !defined(SO_OOBINLINE)) */
		    ncc = read(net, netibuf, netibufsize);
#endif	/* !defined(SO_OOBINLINE)) */
		    if (ncc < 0 && errno == EWOULDBLOCK)
			ncc = 0;
		    else {
			if (ncc <= 0) {
			    break;
			}
			netip = netibuf;
		    }
		}

		if (FD_ISSET(net, &obits) && (nfrontp - nbackp) > 0)
			netflush();

		if ((c > 0) && (ncc > 0))
			telrcv();

		if (FD_ISSET(master, &obits) && (pfrontp - pbackp) > 0)
			ptyflush();

	} /* end for */

	cleanup();

	return 0 ;
}
/* end subroutine (telnet) */


static int telrcv()
{
	register int c;


	while (ncc > 0) {
		if ((&ptyobuf[BUFSIZ] - pfrontp) < 2)
			return;
		c = *netip & 0377;
		/*
		 * Once we hit data, we want to transition back to
		 * in-kernel processing.  However, this code is shared
		 * by getterminaltype()/ttloop() which run before the
		 * in-kernel plumbing is available.  So if we are still
		 * processing the initial option negotiation, even TS_DATA
		 * must be processed here.
		 */
		if (c != IAC && state == TS_DATA && init_neg_done)
			break;
		netip++;
		ncc--;
		switch (state) {

		case TS_CR:
			state = TS_DATA;
			/* Strip off \n or \0 after a \r */
			if ((c == 0) || (c == '\n')) {
				break;
			}
			/* FALLTHRU */

		case TS_DATA:
			if (c == IAC) {
				state = TS_IAC;
				break;
			}
			if (inter > 0)
				break;
			/*
			 * We map \r\n ==> \r, since
			 * We now map \r\n ==> \r for pragmatic reasons.
			 * Many client implementations send \r\n when
			 * the user hits the CarriageReturn key.
			 *
			 * We USED to map \r\n ==> \n, since \r\n says
			 * that we want to be in column 1 of the next
			 * line.
			 */
			if (c == '\r' && (myopts[TELOPT_BINARY] == OPT_NO)) {
				state = TS_CR;
			}
			*pfrontp++ = c;
			break;

		case TS_IAC:
			switch (c) {

			/*
			 * Send the process on the pty side an
			 * interrupt.  Do this with a NULL or
			 * interrupt char; depending on the tty mode.
			 */
			case IP:
				interrupt();
				break;

			case BREAK:
				sendbrk();
				break;

			/*
			 * Are You There?
			 */
			case AYT:
				strcpy(nfrontp, "\r\n[Yes]\r\n");
				nfrontp += 9;
				break;

			/*
			 * Abort Output
			 */
			case AO: 
				{
					struct ltchars tmpltc;

					ptyflush();	/* half-hearted */
					if (ioctl(pty, TIOCGLTC, &tmpltc) == -1)
						syslog(LOG_INFO,
						    "ioctl TIOCGLTC: %m\n");
					if (tmpltc.t_flushc != '\377') {
						*pfrontp++ = tmpltc.t_flushc;
					}
					netclear();	/* clear buffer back */
					*nfrontp++ = (uchar_t)IAC;
					*nfrontp++ = (uchar_t)DM;
					neturg = nfrontp-1; /* off by one XXX */
					netflush();
					netflush(); /* XXX.sparker */
					break;
				}

			/*
			 * Erase Character and
			 * Erase Line
			 */
			case EC:
			case EL: {
					struct sgttyb b;
					char ch;

					ptyflush();	/* half-hearted */
					if (ioctl(pty, TIOCGETP, &b) == -1)
						syslog(LOG_INFO,
						    "ioctl TIOCGETP: %m\n");
					ch = (c == EC) ?
						b.sg_erase : b.sg_kill;
					if (ch != '\377') {
						*pfrontp++ = ch;
					}
					break;
				}

			/*
			 * Check for urgent data...
			 */
			case DM:
#if	!defined(SO_OOBINLINE) /* XXX.sparker */
				SYNCHing = stilloob(net);
				settimer(gotDM);
#endif
				break;


			/*
			 * Begin option subnegotiation...
			 */
			case SB:
				state = TS_SB;
				SB_CLEAR();
				continue;

			case WILL:
				state = TS_WILL;
				continue;

			case WONT:
				state = TS_WONT;
				continue;

			case DO:
				state = TS_DO;
				continue;

			case DONT:
				state = TS_DONT;
				continue;

			case IAC:
				*pfrontp++ = c;
				break;
			}
			state = TS_DATA;
			break;

		case TS_SB:
			if (c == IAC) {
				state = TS_SE;
			} else {
				SB_ACCUM(c);
			}
			break;

		case TS_SE:
			if (c != SE) {
				if (c != IAC) {
					SB_ACCUM((uchar_t)IAC);
				}
				SB_ACCUM(c);
				state = TS_SB;
			} else {
				SB_TERM();
				suboption();	/* handle sub-option */
				state = TS_DATA;
			}
			break;

		case TS_WILL:
			if (remopts[c] != OPT_YES)
				willoption(c);
			state = TS_DATA;
			continue;

		case TS_WONT:
			if (remopts[c] != OPT_NO)
				wontoption(c);
			state = TS_DATA;
			continue;

		case TS_DO:
			if (myopts[c] != OPT_YES)
				dooption(c);
			state = TS_DATA;
			continue;

		case TS_DONT:
			if (myopts[c] != OPT_NO) {
				dontoption(c);
			}
			state = TS_DATA;
			continue;

		default:
			syslog(LOG_ERR, "telnetd: panic state=%d\n", state);
			printf("telnetd: panic state=%d\n", state);
			exit(1);
		}

	} /* end while */

	return 0 ;
}
/* end subroutine (telrcv) */


willoption(option)
	int option;
{
	uchar_t *fmt;

	switch (option) {

	case TELOPT_BINARY:
		mode(RAW, 0);
		fmt = doopt;
		break;

	case TELOPT_ECHO:
		not42 = 0;		/* looks like a 4.2 system */
		/*
		 * Now, in a 4.2 system, to break them out of ECHOing
		 * (to the terminal) mode, we need to send a "WILL ECHO".
		 * Kludge upon kludge!
		 */
		if (myopts[TELOPT_ECHO] == OPT_YES) {
		    dooption(TELOPT_ECHO);
		}
		fmt = dont;
		break;

	case TELOPT_TTYPE:
		settimer(ttypeopt);
		goto common;

	case TELOPT_NAWS:
		settimer(nawsopt);
		goto common;

	case TELOPT_XDISPLOC:
		settimer(xdisplocopt);
		goto common;

	case TELOPT_NEW_ENVIRON:
		settimer(environopt);
		goto common;

	case TELOPT_OLD_ENVIRON:
		settimer(oenvironopt);
		goto common;
common:
		if (remopts[option] == OPT_YES_BUT_ALWAYS_LOOK) {
			remopts[option] = OPT_YES;
			return;
		}
		/*FALLTHRU*/
	case TELOPT_SGA:
		fmt = doopt;
		break;

	case TELOPT_TM:
		fmt = dont;
		break;

	default:
		fmt = dont;
		break;
	}
	if (fmt == doopt) {
		remopts[option] = OPT_YES;
	} else {
		remopts[option] = OPT_NO;
	}
	sprintf(nfrontp, (char *)fmt, option);
	nfrontp += sizeof (dont) - 2;
}

wontoption(option)
	int option;
{
	uchar_t *fmt;

	switch (option) {
	case TELOPT_ECHO:
		not42 = 1;		/* doesn't seem to be a 4.2 system */
		break;

	case TELOPT_BINARY:
		mode(0, RAW);
		break;

	case TELOPT_TTYPE:
		settimer(ttypeopt);
		break;

	case TELOPT_NAWS:
		settimer(nawsopt);
		break;

	case TELOPT_XDISPLOC:
		settimer(xdisplocopt);
		break;

	case TELOPT_NEW_ENVIRON:
		settimer(environopt);
		break;

	case TELOPT_OLD_ENVIRON:
		settimer(oenvironopt);
		break;

	}

	fmt = dont;
	remopts[option] = OPT_NO;
	sprintf(nfrontp, (char *)fmt, option);
	nfrontp += sizeof (doopt) - 2;
}

dooption(option)
	int option;
{
	uchar_t *fmt;

	switch (option) {

	case TELOPT_TM:
		fmt = wont;
		break;

	case TELOPT_ECHO:
		mode(ECHO|CRMOD, 0);
		fmt = will;
		break;

	case TELOPT_BINARY:
		mode(RAW, 0);
		fmt = will;
		break;

	case TELOPT_SGA:
		fmt = will;
		break;

	case TELOPT_LOGOUT:
		/*
		 * Options don't get much easier.  Acknowledge the option,
		 * and then clean up and exit.
		 */
		nfrontp += sprintf(nfrontp, (char *)will, option);
		netflush();
		cleanup();
		/*NOTREACHED*/

	default:
		fmt = wont;
		break;
	}
	if (fmt == will) {
	    myopts[option] = OPT_YES;
	} else {
	    myopts[option] = OPT_NO;
	}
	sprintf(nfrontp, (char *)fmt, option);
	nfrontp += sizeof (doopt) - 2;
}


dontoption(option)
int option;
{
	uchar_t *fmt;

	switch (option) {
	case TELOPT_ECHO:
		/*
		 * we should stop echoing, since the client side will be doing
		 * it, but keep mapping CR since CR-LF will be mapped to it.
		 */
		mode(0, ECHO);
		fmt = wont;
		break;

	default:
		fmt = wont;
		break;
	}

	if (fmt = wont) {
		myopts[option] = OPT_NO;
	} else {
		myopts[option] = OPT_YES;
	}
	sprintf(nfrontp, (char *)fmt, option);
	nfrontp += sizeof (wont) - 2;
}

/*
 * suboption()
 *
 *	Look at the sub-option buffer, and try to be helpful to the other
 * side.
 *
 *	Currently we recognize:
 *
 *	Terminal type is
 */

static void
suboption()
{
	int subchar;


	switch (subchar = SB_GET()) {

	case TELOPT_TTYPE: 
		{		/* Yaaaay! */
		static char terminalname[TERMLEN + 1] = "TERM=";

		settimer(ttypesubopt);

		if (SB_GET() != TELQUAL_IS) {
			return;	/* ??? XXX but, this is the most robust */
		}

		terminaltype = terminalname+strlen(terminalname);

		while (terminaltype < (terminalname + sizeof (terminalname) -
		    1) && !SB_EOF()) {
			register int c;

			c = SB_GET();
			if (isupper(c)) {
				c = tolower(c);
			}
			*terminaltype++ = c;    /* accumulate name */
		}
		*terminaltype = 0;
		terminaltype = terminalname;
		break;
	}

	case TELOPT_NAWS: {
		struct winsize ws;

		if (SB_EOF()) {
			return;
		}
		ws.ws_col = SB_GET() << 8;
		if (SB_EOF()) {
			return;
		}
		ws.ws_col |= SB_GET();
		if (SB_EOF()) {
			return;
		}
		ws.ws_row = SB_GET() << 8;
		if (SB_EOF()) {
			return;
		}
		ws.ws_row |= SB_GET();
		ws.ws_xpixel = 0; ws.ws_ypixel = 0;
		(void) ioctl(pty, TIOCSWINSZ, &ws);
		break;
	}

	case TELOPT_XDISPLOC: {
		if (SB_EOF() || SB_GET() != TELQUAL_IS) {
			return;
		}
		settimer(xdisplocsubopt);
		subpointer[SB_LEN()] = '\0';
		if ((new_env("DISPLAY", subpointer)) == 1)
			perror("malloc");
		break;
	}

	case TELOPT_NEW_ENVIRON:
	case TELOPT_OLD_ENVIRON: {
		register int c;
		register char *cp, *varp, *valp;

		if (SB_EOF())
			return;
		c = SB_GET();
		if (c == TELQUAL_IS) {
			if (subchar == TELOPT_OLD_ENVIRON)
				settimer(oenvironsubopt);
			else
				settimer(environsubopt);
		} else if (c != TELQUAL_INFO) {
			return;
		}

		if (subchar == TELOPT_NEW_ENVIRON) {
		    while (!SB_EOF()) {
			c = SB_GET();
			if ((c == NEW_ENV_VAR) || (c == ENV_USERVAR))
				break;
		    }
		} else
		{
#ifdef	ENV_HACK
			/*
			 * We only want to do this if we haven't already decided
			 * whether or not the other side has its VALUE and VAR
			 * reversed.
			 */
			if (env_ovar < 0) {
				register int last = -1;	/* invalid value */
				int empty = 0;
				int got_var = 0, got_value = 0, got_uservar = 0;

			/*
			 * The other side might have its VALUE and VAR values
			 * reversed.  To be interoperable, we need to determine
			 * which way it is.  If the first recognized character
			 * is a VAR or VALUE, then that will tell us what
			 * type of client it is.  If the fist recognized
			 * character is a USERVAR, then we continue scanning
			 * the suboption looking for two consecutive
			 * VAR or VALUE fields.  We should not get two
			 * consecutive VALUE fields, so finding two
			 * consecutive VALUE or VAR fields will tell us
			 * what the client is.
			 */
				SB_SAVE();
				while (!SB_EOF()) {
					c = SB_GET();
					switch (c) {
					case OLD_ENV_VAR:
						if (last < 0 ||
						    last == OLD_ENV_VAR ||
						    (empty &&
						    (last == OLD_ENV_VALUE)))
							goto env_ovar_ok;
						got_var++;
						last = OLD_ENV_VAR;
						break;
					case OLD_ENV_VALUE:
						if (last < 0 ||
						    last == OLD_ENV_VALUE ||
						    (empty &&
						    (last == OLD_ENV_VAR)))
							goto env_ovar_wrong;
						got_value++;
						last = OLD_ENV_VALUE;
						break;
					case ENV_USERVAR:
						/*
						 * count strings of USERVAR
						 * as one
						 */
						if (last != ENV_USERVAR)
							got_uservar++;
						if (empty) {
							if (last ==
							    OLD_ENV_VALUE)
								goto
								    env_ovar_ok;
							if (last == OLD_ENV_VAR)
								goto \
								env_ovar_wrong;
						}
						last = ENV_USERVAR;
						break;
					case ENV_ESC:
						if (!SB_EOF())
							c = SB_GET();
						/* FALL THROUGH */
					default:
						empty = 0;
						continue;
					}
					empty = 1;
				}
				if (empty) {
					if (last == OLD_ENV_VALUE)
						goto env_ovar_ok;
					if (last == OLD_ENV_VAR)
						goto env_ovar_wrong;
				}
			/*
			 * Ok, the first thing was a USERVAR, and there
			 * are not two consecutive VAR or VALUE commands,
			 * and none of the VAR or VALUE commands are empty.
			 * If the client has sent us a well-formed option,
			 * then the number of VALUEs received should always
			 * be less than or equal to the number of VARs and
			 * USERVARs received.
			 *
			 * If we got exactly as many VALUEs as VARs and
			 * USERVARs, the client has the same definitions.
			 *
			 * If we got exactly as many VARs as VALUEs and
			 * USERVARS, the client has reversed definitions.
			 */
				if (got_uservar + got_var == got_value) {
env_ovar_ok:
					env_ovar = OLD_ENV_VAR;
					env_ovalue = OLD_ENV_VALUE;
				} else if (got_uservar + got_value == got_var) {
env_ovar_wrong:
					env_ovar = OLD_ENV_VALUE;
					env_ovalue = OLD_ENV_VAR;
					DIAG(TD_OPTIONS, {sprintf(nfrontp,
					    "ENVIRON VALUE and VAR are "
					    "reversed!\r\n");
					    nfrontp += strlen(nfrontp); });

				}
			}
			SB_RESTORE();
#endif

			while (!SB_EOF()) {
				c = SB_GET();
				if ((c == env_ovar) || (c == ENV_USERVAR))
					break;
			}
		}

		if (SB_EOF())
			return;

		cp = varp = (char *)subpointer;
		valp = 0;

		while (!SB_EOF()) {
			c = SB_GET();
			if (subchar == TELOPT_OLD_ENVIRON) {
				if (c == env_ovar)
					c = NEW_ENV_VAR;
				else if (c == env_ovalue)
					c = NEW_ENV_VALUE;
			}
			switch (c) {

			case NEW_ENV_VALUE:
				*cp = '\0';
				cp = valp = (char *)subpointer;
				break;

			case NEW_ENV_VAR:
			case ENV_USERVAR:
				*cp = '\0';
				if (valp) {
					if ((new_env(varp, valp)) == 1) {
						perror("malloc");
					}
				} else {
					(void) del_env(varp);
				}
				cp = varp = (char *)subpointer;
				valp = 0;
				break;

			case ENV_ESC:
				if (SB_EOF())
					break;
				c = SB_GET();
				/* FALL THROUGH */
			default:
				*cp++ = c;
				break;
			}
		}
		*cp = '\0';
		if (valp) {
			if ((new_env(varp, valp)) == 1) {
				perror("malloc");
			}
		} else {
			(void) del_env(varp);
		}
		break;
	}  /* end of case TELOPT_NEW_ENVIRON */

	default:
		;
	}
}

mode(on, off)
	int on, off;
{
	struct sgttyb b;

	ptyflush();
	if (ioctl(pty, TIOCGETP, &b) == -1)
		syslog(LOG_INFO, "ioctl TIOCGETP: %m\n");
	b.sg_flags |= on;
	b.sg_flags &= ~off;
	if (ioctl(pty, TIOCSETN, &b) == -1)
		syslog(LOG_INFO, "ioctl TIOCSETN: %m\n");
}

/*
 * Send interrupt to process on other side of pty.
 * If it is in raw mode, just write NULL;
 * otherwise, write intr char.
 */
interrupt()
{
	struct sgttyb b;
	struct tchars tchars;

	ptyflush();	/* half-hearted */
	if (ioctl(pty, TIOCGETP, &b) == -1)
		syslog(LOG_INFO, "ioctl TIOCGETP: %m\n");
	if (b.sg_flags & RAW) {
		*pfrontp++ = '\0';
		return;
	}
	*pfrontp++ = 
	    ioctl(pty, TIOCGETC, &tchars) < 0 ? '\177' : tchars.t_intrc ;
}

/*
 * Send quit to process on other side of pty.
 * If it is in raw mode, just write NULL;
 * otherwise, write quit char.
 */
sendbrk()
{
	struct sgttyb b;
	struct tchars tchars;

	ptyflush();	/* half-hearted */
	ioctl(pty, TIOCGETP, &b);
	if (b.sg_flags & RAW) {
		*pfrontp++ = '\0';
		return;
	}
	*pfrontp++ = 
	    ioctl(pty, TIOCGETC, &tchars) < 0 ? '\034' : tchars.t_quitc ;
}

ptyflush()
{
	int n;

	if ((n = pfrontp - pbackp) > 0)
		n = write(master, pbackp, n);
	if (n < 0)
		return;
	pbackp += n;
	if (pbackp == pfrontp)
		pbackp = pfrontp = ptyobuf;
}

/*
 * nextitem()
 *
 *	Return the address of the next "item" in the TELNET data
 * stream.  This will be the address of the next character if
 * the current address is a user data character, or it will
 * be the address of the character following the TELNET command
 * if the current address is a TELNET IAC ("I Am a Command")
 * character.
 */

char *
nextitem(current)
char	*current;
{
	if ((*current&0xff) != IAC) {
		return (current+1);
	}
	switch (*(current+1)&0xff) {
	case DO:
	case DONT:
	case WILL:
	case WONT:
		return (current+3);
	case SB:		/* loop forever looking for the SE */
	{
		register char *look = current+2;

		for (;;) {
			if ((*look++&0xff) == IAC) {
				if ((*look++&0xff) == SE) {
					return (look);
				}
			}
		}
	}
	default:
		return (current+2);
	}
}


/*
 * netclear()
 *
 *	We are about to do a TELNET SYNCH operation.  Clear
 * the path to the network.
 *
 *	Things are a bit tricky since we may have sent the first
 * byte or so of a previous TELNET command into the network.
 * So, we have to scan the network buffer from the beginning
 * until we are up to where we want to be.
 *
 *	A side effect of what we do, just to keep things
 * simple, is to clear the urgent data pointer.  The principal
 * caller should be setting the urgent data pointer AFTER calling
 * us in any case.
 */

netclear()
{
	register char *thisitem, *next;
	char *good;
#define	wewant(p)	((nfrontp > p) && ((*p&0xff) == IAC) && \
				((*(p+1)&0xff) != EC) && ((*(p+1)&0xff) != EL))

	thisitem = netobuf;

	while ((next = nextitem(thisitem)) <= nbackp) {
		thisitem = next;
	}

	/* Now, thisitem is first before/at boundary. */

	good = netobuf;	/* where the good bytes go */

	while (nfrontp > thisitem) {
		if (wewant(thisitem)) {
			int length;

			next = thisitem;
			do {
				next = nextitem(next);
			} while (wewant(next) && (nfrontp > next));
			length = next-thisitem;
			bcopy(thisitem, good, length);
			good += length;
			thisitem = next;
		} else {
			thisitem = nextitem(thisitem);
		}
	}

	nbackp = netobuf;
	nfrontp = good;		/* next byte to be sent */
	neturg = 0;
}


/*
 *  netflush
 *		Send as much data as possible to the network,
 *	handling requests for urgent data.
 */

netflush()
{
	int n;


	if ((n = nfrontp - nbackp) > 0) {
		/*
		 * if no urgent data, or if the other side appears to be an
		 * old 4.2 client (and thus unable to survive TCP urgent data),
		 * write the entire buffer in non-OOB mode.
		 */
		if ((neturg == 0) || (not42 == 0)) {
			n = write(net, nbackp, n);	/* normal write */
		} else {
			n = neturg - nbackp;
			/*
			 * In 4.2 (and 4.3) systems, there is some question
			 * about what byte in a sendOOB operation is the "OOB"
			 * data.  To make ourselves compatible, we only send ONE
			 * byte out of band, the one WE THINK should be OOB
			 * (though we really have more the TCP philosophy of
			 * urgent data rather than the Unix philosophy of OOB
			 * data).
			 */
			if (n > 1) {
				/* send URGENT all by itself */
				n = write(net, nbackp, n-1);

			} else {
				/* URGENT data */
				n = send_oob(net, nbackp, n);

			}
		}
	}
	if (n < 0) {
		if (errno == EWOULDBLOCK)
			return;
		/* should blow this guy away... */
		return;
	}
	nbackp += n;
	if (nbackp >= neturg) {
		neturg = 0;
	}
	if (nbackp == nfrontp) {
		nbackp = nfrontp = netobuf;
	}
}

static void cleanup()
{

	/*
	 * If the TEL_IOC_ENABLE ioctl hasn't completed, then we need to
	 * handle closing differently.  We close "net" first and then
	 * "master" in that order.  We do close(net) first because
	 * we have no other way to disconnect forwarding between the network
	 * and master.  So by issuing the close()'s we ensure that no further
	 * data rises from TCP.  A more complex fix would be adding proper
	 * support for throwing a "stop" switch for forwarding data between
	 * logindmux peers.  It's possible to block in the close of the tty
	 * while the network still receives data and the telmod module is
	 * TEL_STOPPED.  A denial-of-service attack generates this case,
	 * see 4102102.
	 */

	if (!telmod_init_done) {
		(void) close(net);
		(void) close(master);
	}
	rmut();
#ifndef SYSV
	vhangup();
#endif
	exit(1);
}
/* end subroutine (cleanup) */


#ifdef SYSV

rmut()
{
	pam_handle_t    *pamh;

	struct utmpx *up;
	struct utmpx ut;
	char user[33], ttyn[33], rhost[258];
	char	*cp ;


	/* while cleaning up don't allow disruption */
	signal(SIGCHLD, SIG_IGN);

	setutxent();
	while (up = getutxent()) {
		if (up->ut_pid == pid) {
			if (up->ut_type == DEAD_PROCESS) {
				/*
				 * Cleaned up elsewhere.
				 */
				break;
			}

			strncpy(user, up->ut_user, sizeof (up->ut_user));
			user[sizeof (up->ut_user)] = '\0';
			strncpy(ttyn, up->ut_line, sizeof (up->ut_line));
			ttyn[sizeof (up->ut_line)] = '\0';
			strncpy(rhost, up->ut_host, sizeof (up->ut_host));
			rhost[sizeof (up->ut_host)] = '\0';

#if	P_TELSERV
	cp = "telserv" ;
#else
	cp = "telnet" ;
#endif

			if ((pam_start(cp, user, NULL, &pamh))
							== PAM_SUCCESS) {
				(void) pam_set_item(pamh, PAM_TTY, ttyn);
				(void) pam_set_item(pamh, PAM_RHOST, rhost);
				(void) pam_close_session(pamh, 0);
				pam_end(pamh, PAM_SUCCESS);
			}

			up->ut_type = DEAD_PROCESS;
			up->ut_exit.e_termination = WTERMSIG(0);
			up->ut_exit.e_exit = WEXITSTATUS(0);
			(void) time(&up->ut_tv.tv_sec);

			if (modutx(up) == NULL) {
				/*
				 * Since modutx failed we'll
				 * write out the new entry
				 * ourselves.
				 */
				(void) pututxline(up);
				updwtmpx("wtmpx", up);
			}
			break;
		}
	}

	endutxent();

	signal(SIGCHLD, (void (*)())cleanup);
}
/* end subroutine (rmut) */

#else /* !SYSV */


#define	SCPYN(a, b)	strncpy(a, b, sizeof (a))
#define	SCMPN(a, b)	strncmp(a, b, sizeof (a))

rmut()
{
	register f;
	int found = 0;
	struct utmp *u, *utmp;
	int nutmp;
	struct ustat statbf;
	struct	utmp wtmp;
	char	wtmpf[]	= WTMP_FILE;
	char	utmpf[] = UTMP_FILE;


	f = open(utmpf, O_RDWR);
	if (f >= 0) {
		fstat(f, &statbf);
		utmp = (struct utmp *)malloc(statbf.st_size);
		if (!utmp)
			syslog(LOG_ERR, "utmp malloc failed");
		if (statbf.st_size && utmp) {
			nutmp = read(f, utmp, statbf.st_size);
			nutmp /= sizeof (struct utmp);

			for (u = utmp; u < &utmp[nutmp]; u++) {
				if (SCMPN(u->ut_line, line+5) ||
				    u->ut_name[0] == 0)
					continue;
				lseek(f, ((long)u)-((long)utmp), L_SET);
				SCPYN(u->ut_name, "");
				SCPYN(u->ut_host, "");
				time(&u->ut_time);
				write(f, (char *)u, sizeof (wtmp));
				found++;
			}
		}
		close(f);
	}
	if (found) {
		f = open(wtmpf, O_WRONLY|O_APPEND);
		if (f >= 0) {
			SCPYN(wtmp.ut_line, line+5);
			SCPYN(wtmp.ut_name, "");
			SCPYN(wtmp.ut_host, "");
			time(&wtmp.ut_time);
			write(f, (char *)&wtmp, sizeof (wtmp));
			close(f);
		}
	}
	chmod(line, 0666);
	chown(line, 0, 0);
	line[strlen("/dev/")] = 'p';
	chmod(line, 0666);
	chown(line, 0, 0);
}
#endif /* SYSV */


static int
readstream(fd, buf, offset)
	int	fd;
	char	*buf;
	int	offset;
{
	struct strbuf ctlbuf, datbuf;
	union T_primitives tpi;
	int	ret = 0;
	int	flags = 0;
	int	bytes_avail, count;


	bzero((char *)&ctlbuf, sizeof (ctlbuf));
	bzero((char *)&datbuf, sizeof (datbuf));

	ctlbuf.buf = (char *)&tpi;
	ctlbuf.maxlen = sizeof(tpi);

	if (ioctl(fd, I_NREAD, &bytes_avail) < 0) {
		syslog(LOG_ERR, "I_NREAD returned error %m");
		return (-1);
	}
	if (bytes_avail > netibufsize - offset) {
		count = netip - netibuf;
		netibuf = (char *)realloc(netibuf,
		    (unsigned) (netibufsize + bytes_avail)) ;

		if (netibuf == NULL) {
			fatal(net, "netibuf realloc failed\n");
		}
		netibufsize += bytes_avail;
		netip = netibuf + count;
		buf = netibuf;
	}
	datbuf.buf = buf + offset;
	datbuf.maxlen = netibufsize;
	ret = getmsg(fd, &ctlbuf, &datbuf, &flags);

	if (ret < 0) {
		syslog(LOG_ERR, "getmsg returned -1, errno %d\n",
			errno);
		return (-1);
	}
	if (ctlbuf.len <= 0) {
		return (datbuf.len);
	}

	if (tpi.type == T_DATA_REQ) {
		return (0);
	}

	if ((tpi.type == T_ORDREL_IND) || (tpi.type == T_DISCON_IND))
			cleanup();
	fatal(fd, "no data or protocol element recognized");
	/*NOTREACHED*/
}

static void
drainstream(size)
	int	size;
{
	int	nbytes;
	int	tsize;


	tsize = netip - netibuf;

	if ((tsize + ncc + size) > netibufsize) {
		if (!(netibuf = (char *)realloc(netibuf,
		    (unsigned)tsize + ncc + size)))
			fatalperror(net, "netibuf realloc failed\n", errno);
		netibufsize = tsize + ncc + size;

		netip = netibuf + tsize;
	}

	if ((nbytes = read(net, (char *)netip + ncc, size)) != size)
		syslog(LOG_ERR, "read %d bytes\n", nbytes);

}

/*
 * TPI style replacement for socket send() primitive, so we don't require
 * sockmod to be on the stream.
 */
static int send_oob(int fd, char *ptr, int count)
{
	struct T_exdata_req exd_req;
	struct strbuf hdr, dat;
	int ret;


	exd_req.PRIM_type = T_EXDATA_REQ;
	exd_req.MORE_flag = 0;

	hdr.buf = (char *)&exd_req;
	hdr.len = sizeof (exd_req);

	dat.buf = ptr;
	dat.len = count;

	ret = putmsg(fd, &hdr, &dat, 0);
	if (ret == 0) {
		ret = count;
	}
	return (ret);
}
/* end subroutine (send_oob) */


#ifndef __STDC__
#define	const
#endif


#ifndef	__P
#define	__P(x)	()
#endif

static char *__findenv __P((const char *, int *));

/*
 * setenv --
 *	Set the value of the environmental variable "name" to be
 *	"value".  If rdebugwrite is set, replace any current value.
 */
setenv(name, value, rdebugwrite)
	const char *name;
	const char *value;
	int rdebugwrite;
{
	extern char **environ;
	static int alloced;			/* if allocated space before */
	register char *c;
	int l_value, offset;


	/*
	 * Do not allow environment variables which begin with LD_ to be
	 * inserted into the environment.  While normally the dynamic linker
	 * protects the login program, that is based on the assumption hostile
	 * invocation of login are from non-root users.  However, since telnetd
	 * runs as root, this cannot be utilized.  So instead we simply
	 * prevent LD_* from being inserted into the environment.
	 */
	if (strncmp(name, "LD_", 3) == 0) {
		return (-1);
	}


/* this variable is security hole for '/bin/login' on Solaris */

	if (strcmp(name, "TTYPROMPT") == 0)
		return (-1);

/* continue */

	if (*value == '=')			/* no `=' in value */
		++value;
	l_value = strlen(value);
	if ((c = __findenv(name, &offset))) {	/* find if already exists */
		if (!rdebugwrite)
			return (0);
		if ((int)strlen(c) >= l_value) { /* old larger; copy over */
			while (*c++ = *value++);
			return (0);
		}
	} else {					/* create new slot */
		register int cnt;
		register char **p;

		for (p = environ, cnt = 0; *p; ++p, ++cnt);
		if (alloced) {			/* just increase size */
			environ = (char **)realloc((char *)environ,
			    (size_t)(sizeof (char *) * (cnt + 2)));
			if (!environ)
				return (-1);
		} else {				/* get new space */
			alloced = 1;		/* copy old entries into it */
			p = (char **)malloc((size_t)(sizeof (char *)*
			    (cnt + 2)));
			if (!p)
				return (-1);
			memcpy(p, environ, cnt * sizeof (char *));
			environ = p;
		}
		environ[cnt + 1] = NULL;
		offset = cnt;
	}
	for (c = (char *)name; *c && *c != '='; ++c);	/* no `=' in name */
	if (!(environ[offset] =			/* name + `=' + value */
	    malloc((size_t)((int)(c - name) + l_value + 2))))
		return (-1);
	for (c = environ[offset]; ((*c = *name++) != 0) && (*c != '='); ++c);
	for (*c++ = '='; *c++ = *value++; );
	return (0);
}

/*
 * unsetenv(name) --
 *	Delete environmental variable "name".
 */
void
unsetenv(name)
	const char *name;
{
	extern char **environ;
	register char **p;
	int offset;

	while (__findenv(name, &offset))	/* if set multiple times */
		for (p = &environ[offset]; ; ++p)
			if ((*p = *(p + 1)) == 0)
				break;
}

/*
 * getenv --
 *	Returns ptr to value associated with name, if any, else NULL.
 */
char *
getenv(name)
	const char *name;
{
	int offset;

	return (__findenv(name, &offset));
}

/*
 * __findenv --
 *	Returns pointer to value associated with name, if any, else NULL.
 *	Sets offset to be the offset of the name/value combination in the
 *	environmental array, for use by setenv(3) and unsetenv(3).
 *	Explicitly removes '=' in argument name.
 */
static char *
__findenv(name, offset)
	register const char *name;
	int *offset;
{
	extern char **environ;
	register int len;
	register const char *np;
	register char **p, *c;

	if (name == NULL || environ == NULL)
		return (NULL);
	for (np = name; *np && *np != '='; ++np)
		continue;
	len = np - name;
	for (p = environ; (c = *p) != NULL; ++p)
		if (strncmp(c, name, len) == 0 && c[len] == '=') {
			*offset = p - environ;
			return (c + len + 1);
		}
	return (NULL);
}

#include	<deflt.h>
#include	"libcmd.h"	/* for defcntl */

void
showbanner()
{
	char	*cp;
	char	buf[BUFSIZ];
	char	evalbuf[BUFSIZ];
	extern void	map_banner(), defbanner();


	if (defopen(defaultfile) == 0) {
		int	flags;

		/*
		 * ignore case
		 */
		flags = defcntl(DC_GETFLAGS, 0);
		TURNOFF(flags, DC_CASE);
		defcntl(DC_SETFLAGS, flags);
		if (cp = defread(bannervar)) {
			FILE	*fp;
			char	*p;
			char	*curshell, *oldshell;

			if (strlen(cp) + strlen("eval echo '") + strlen("'\n")
			    + 1 < sizeof (evalbuf)) {
				strcpy(evalbuf, "eval echo '");
				strcat(evalbuf, cp);
				strcat(evalbuf, "'\n");

				/*
				 *	We need to ensure that popen uses a
				 *	Bourne shell, even if a different value
				 *	for SHELL is in our current environment
				 *	so we look, change it if necessary, then
				 *	restore the old value when we're done.
				 */
				if (curshell = getenv("SHELL")) {
					oldshell = strdup(curshell);
					(void) putenv(binshellvar);
				} else
					oldshell = (char *)NULL;
				if (fp = popen(evalbuf, "r")) {
					/*
					 *	Pipe I/O atomicity guarantees we
					 *	need only one read.
					 */
					if (fread(buf, 1, sizeof (buf), fp)) {
						p = strrchr(buf, '\n');
						if (p)
							*p = '\0';
						map_banner(buf);
						netflush();
					}
					(void) pclose(fp);
					return;
				}
				if (oldshell)
					(void) putenv(oldshell);
			}
		}
	}

	defbanner();
	netflush();
}
/* end subroutine (showbanner) */


void
map_banner(p)
char	*p;
{
	char	*q;

	/*
	 *	Map the banner:  "\n" -> "\r\n" and "\r" -> "\r\0"
	 */
	for (q = nfrontp; p && *p && q < (nfrontp + sizeof(netobuf) - 1) ; ) {

		if (*p == '\n') {
			*q++ = '\r';
			*q++ = '\n';
			p++;
		} else if (*p == '\r') {
				*q++ = '\r';
				*q++ = '\0';
				p++;
			} else
				*q++ = *p++;

	} /* end for */

	nfrontp += (q - netobuf) ;
}
/* end subroutine (map_banner) */


/*
 * Show banner that getty never gave.  By default, this is `uname -sr`.
 *
 * The banner includes some null's (for TELNET CR disambiguation),
 * so we have to be somewhat complicated.
 */
void
defbanner()
{
	struct utsname u;

	if (uname(&u) == -1)
		return;

	bcopy(BANNER1, nfrontp, sizeof (BANNER1) -1);
	nfrontp += sizeof (BANNER1) - 1;

	bcopy(u.sysname, nfrontp, strlen(u.sysname));
	nfrontp += strlen(u.sysname);

	bcopy(" ", nfrontp, 1);
	nfrontp++;

	bcopy(u.release, nfrontp, strlen(u.release));
	nfrontp += strlen(u.release);

	bcopy(BANNER2, nfrontp, sizeof (BANNER2) -1);
	nfrontp += sizeof (BANNER2) - 1;
}
/* end subroutine (defbanner) */


/*
 * Verify that the named module is at the top of the stream
 * and then pop it off.
 */
static int removemod(int f, char *modname)
{
	char topmodname[BUFSIZ];


	if (ioctl(f, I_LOOK, topmodname) < 0)
		return (-1);
	if (strcmp(modname, topmodname) != 0) {
		errno = ENXIO;
		return (-1);
	}
	if (ioctl(f, I_POP, 0) < 0)
		return (-1);
	return (0);
}
/* end subroutine (removemod) */


#if	CF_LOCALSVC

/* check if a terminal/service specification is allowed */
static int termsecure(terminaltype,svc)
const char	terminaltype[] ;
char		svc[] ;
{
	int	svcoff, i, j ;
	int	f_secure = FALSE ;

	char	*cp, *termsvc = NULL ;


	i = 0 ;
	if ((cp = strchr(terminaltype,'_')) != NULL) {

	    j = cp - terminaltype ;
	    for (i = 0 ; termtypes[i] != NULL ; i += 1) {

	        if ((strncmp(terminaltype,termtypes[i],j) == 0) &&
	            (termtypes[i][j] == '\0'))
	            break ;

	    } /* end for */

	    cp = (char *) termtypes[i] ;

	}

	if (cp != NULL) {

	    svcoff = strlen(termtypes[i]) ;

	    if (terminaltype[svcoff] == '_') {

	        termsvc = (char *) (terminaltype + svcoff + 1) ;

	        for (i = 0 ; services[i] != NULL ; i += 1) {

	            if (strcmp(termsvc,services[i]) == 0)
	                break ;

	        } /* end for */

	        f_secure = (services[i] != NULL) ;

		if (! f_secure)
		    f_secure = telserv_service(TELSERV_SERVICES,termsvc) ;

		strwcpy(svc,termsvc,SVCLEN) ;

	    } /* end if (has our service marker) */
	}

	return f_secure ;
}
/* end subroutine (termsecure) */


/* see if this service is allowed from the "services" file */
static int telserv_service(fname,service)
const char	fname[] ;
const char	service[] ;
{
	FILE	*fp ;

	int	len ;
	int	c = 0 ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	*sp, *cp ;


	if ((fname == NULL) || (fname[0] == '\0'))
	    goto ret0 ;

	c = 0 ;
	fp = fopen(fname,"r") ;
	if (fp == NULL)
	    goto ret0 ;

	while ((len = freadline(fp,linebuf,LINEBUFLEN)) > 0) {

	    if (linebuf[len - 1] == '\n')
	        linebuf[--len] = '\0' ;

	    if (len == 0)
	        continue ;

	    sp = linebuf ;
	    while (isspace(*sp))
	        sp += 1 ;

	    if ((*sp == '\0') || (*sp == '#'))
	        continue ;

	    cp = sp ;
	    while (*cp && (! isspace(*cp)))
	        cp += 1 ;

	    *cp = '\0' ;
	    if (strcmp(sp,service) == 0) {

	        c += 1 ;
	        break ;
	    }

	} /* end while */

	fclose(fp) ;

ret0:
	return c ;
}
/* end subroutine (telserv_service) */

#endif /* CF_LOCALSVC */


static int tcsetdefault(fd)
int	fd ;
{
	struct termios	ts ;

	speed_t	speed ;

	int	rs ;


	rs = uc_tcgetattr(fd,&ts) ;
	if (rs < 0)
	    goto ret0 ;

/* control stuff */

	ts.c_iflag &= 
	    (~ (INLCR | ICRNL | IUCLC | IXANY | ISTRIP | INPCK | PARMRK)) ;
	ts.c_iflag |= IXON ;

	ts.c_cflag &= (~ (CSIZE)) ;
	ts.c_cflag |= CS8 ;

	ts.c_lflag &= (~ (ICANON | ECHO | ECHOE | ECHOK | ECHONL)) ;
	ts.c_lflag &= (~ (ISIG | IEXTEN)) ;

	ts.c_oflag &= (~ (OCRNL | ONOCR | ONLRET)) ;

/* check speed */

	speed = cfgetispeed(&ts) ;

	if (speed == 0)
		uc_cfsetispeed(&ts,B38400) ;

	speed = cfgetospeed(&ts) ;

	if (speed == 0)
		uc_cfsetospeed(&ts,B38400) ;

	rs = uc_tcsetattr(fd,TCSANOW,&ts) ;

ret0:
	return rs ;
}
/* end subroutine (tcsetdefault) */


static int tcspeednonzero(fd)
int	fd ;
{
		struct termios	ts ;

	speed_t	speed ;

	int	rs ;
	int	f_needpop = FALSE ;


		rs = uc_tcgetattr(fd,&ts) ;
		if (rs >= 0) {

/* check speed */

	speed = cfgetispeed(&ts) ;

	if (speed == 0) {

		f_needpop = TRUE ;
		uc_cfsetispeed(&ts,B38400) ;

	}

	speed = cfgetospeed(&ts) ;

	if (speed == 0) {

		f_needpop = TRUE ;
		uc_cfsetospeed(&ts,B38400) ;

	}

/* do we need to set it? */

			if (f_needpop)
			rs = uc_tcsetattr(fd,TCSANOW,&ts) ;

		}

	return (rs >= 0) ? f_needpop : rs ;
}
/* end subroutine (tcspeednonzero) */


static int tcnoecho(fd)
int	fd ;
{
	struct termios	ts ;

	int	rs = SR_OK ;
	int	newval ;
	int	f_needpop = FALSE ;


	rs = uc_tcgetattr(fd,&ts) ;
	if (rs >= 0) {
		newval = ts.c_lflag &= (~ (ECHO | ECHOE | ECHOK | ECHONL)) ;
		if (newval != ts.c_lflag) {
			f_needpop = TRUE ;
			rs = uc_tcsetattr(fd,TCSANOW,&ts) ;
		}
	}

	return (rs >= 0) ? f_needpop : rs ;
}
/* end subroutine (tcnoecho) */


static int mkpam(pip,rhost,slavename)
struct proginfo	*pip ;
const char	rhost[] ;
const char	slavename[] ;
{
	pam_handle_t    *pamh;
	int		rs1 ;
	const char	*cp ;

#if	P_TELSERV
	cp = "telserv" ;
#else
	cp = "telnet" ;
#endif

	rs1 = pam_start(cp, pip->username, NULL, &pamh) ;
	if (rs1 == PAM_SUCCESS) {
				pam_set_item(pamh, PAM_TTY, slavename);
				pam_set_item(pamh, PAM_RHOST, rhost);
				pam_open_session(pamh, 0);
				pam_end(pamh, PAM_SUCCESS);
	} /* end if */

	return 0 ;
}
/* end subroutine (mkpam) */


static int fd_allzero(fd_set *sp)
{
	int		i ;

	for (i = 0 ; i < FD_SETSIZE ; i += 1) {
	    if (FD_ISSET(i,sp))
		break ;
	} /* end for */

	return (i == FD_SETSIZE) ;
}
/* end subroutine (fd_allzero) */


#if	CF_DEBUG || CF_DEBUGS
static int debugfstat(cchar *s,int fd)
{
	struct ustat	sb ;
	int	rs1 = SR_BADF ;
	if (fd >= 0)
	rs1 = u_fstat(fd,&sb) ;
	debugprintf("proghandle: %s fd=%d rs=%d mode=%08X s_issock=%u\n",
	    ((s != NULL) ? s : ""),fd,rs1,sb.st_mode,S_ISSOCK(sb.st_mode)) ;
	return rs1 ;
}
#endif /* CF_DEBUG */


