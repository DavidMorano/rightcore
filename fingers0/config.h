/* config */

/* last modified %G% version %I% */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	P_FINGERS	1

#define	VERSION		"0"
#define	WHATINFO	"@(#)FINGERS "
#define	SEARCHNAME	"fingers"
#define	BANNER		"Finger Server"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"FINGERS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"FINGERS_BANNER"
#define	VARSEARCHNAME	"FINGERS_NAME"
#define	VAROPTS		"FINGERS_OPTS"
#define	VARCONF		"FINGERS_CONF"
#define	VARLINELEN	"FINGERS_LINELEN"
#define	VARAFNAME	"FINGERS_AF"
#define	VAREFNAME	"FINGERS_EF"
#define	VARERRORFNAME	"FINGERS_ERRORFILE"

#define	VARDEBUGFNAME	"FINGERS_DEBUGFILE"
#define	VARDEBUGFD1	"FINGERS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARHZ		"HZ"
#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARDOMAIN	"DOMAIN"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARTERM		"TERM"
#define	VARPRINTER	"PRINTER"
#define	VARLPDEST	"LPDEST"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"
#define	VARNCPU		"NCPU"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"
#define	VARPATH		"PATH"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	SVCSPECLEN
#define	SVCSPECLEN	120
#endif

#ifndef	SVCARGSLEN
#define	SVCARGSLEN	LINEBUFLEN
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	ETCDNAME	"etc"
#define	LOGDNAME	"log"
#define	MSDNAME		"var"
#define	VARDNAME	"var"
#define	SPOOLDNAME	"var/spool"
#define	RUNDNAME	"var/run"
#define	STAMPDNAME	"var/timestamps"	/* timestamp directory */
#define	LOCKDNAME	"spool/locks"

#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	HELPFNAME	"help"
#define	CONFFNAME	"conf"
#define	DEFSFNAME	"def"
#define	XEFNAME		"xe"
#define	PVARSFNAME	"pvar"
#define	STAMPFNAME	"fingers"
#define	REQFNAME	"req"			/* "request" (IPC) file */
#define	PASSFNAME	"pass"
#define	SVCFNAME	"svc"
#define	ACCFNAME	"acc"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	XENVFNAME	"xenv"
#define	XPATHFNAME	"xpath"
#define	PASSWDFNAME	"passwd"
#define	SERIALFNAME	"serial"		/* serial file */
#define	PROJFNAME	".project"
#define	PLANFNAME	".plan"
#define	MSFNAME		"var/spool/ms"

#define	LOGFNAME	"log/fingers"		/* activity log */
#define	PIDFNAME	"var/run/fingers"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/fingers"	/* lock mutex file */

#define	SHMFNAME	"var/spool/%S/shareinfo"
#define	SERIALFNAME1	"var/serial"
#define	SERIALFNAME2	"/tmp/serial"
#define	USERSRV		"var/svc/ufinger"

#define	PVARFEXT	"pvar"
#define	USERFEXT	"users"
#define	REQFEXT		"req"
#define	SVCFEXT		"svc"
#define	ACCFEXT		"acc"

#define	DEFPATH		"/usr/xpg4/bin:/usr/bin:/usr/sbin"

#define	PORTNAME	"fingerserv"
#define	PORTNUM		"5112"		/* default TCP port */

#ifndef	COLUMNS
#define	COLUMNS		80		/* output columns (should be 80) */
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(2 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#define	LOGSIZE		(80*1024)
#define	MAXJOBS		4		/* maximum jobs at once */
#define	DEFNDEFS	20
#define	DEFNXENVS	200

#define	ORGCODE		"RC"
#define	DEFSVC		"default"

#define	TO_LOCK		(5*60)		/* lock time-out */
#define	TO_POLL		60		/* ? */
#define	TO_POLLSVC	(5*60)		/* default interval (minutes) */
#define	TO_RUN		(60*60)		/* default run interval */
#define	TO_MINCHECK	(1*60)		/* minimal check interval */
#define	TO_STANDCHECK	5		/* default standing-server check */
#define	TO_MAINT	5		/* general maintenance */
#define	TO_RECVMSG	3		/* receiving a message */
#define	TO_READSVC	3		/* reading a service-code */
#define	TO_READ		3		/* reading data */
#define	TO_SVC		60		/* service acquire timeout */
#define	TO_SPEED	(24*3600)	/* interval between updates */
#define	TO_MARKTIME	(12*3600)	/* mark-time interval */
#define	TO_BROKEN	(1*3600)	/* broken listener re-activation */
#define	TO_RECVFD	10		/* receiving a file-desriptor */
#define	TO_SENDFD	10		/* sending a file-desriptor */


