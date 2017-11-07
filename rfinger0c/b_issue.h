/* config */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	P_ISSUE	1

#define	VERSION		"0"
#define	WHATINFO	"@(#)ISSUE "
#define	SEARCHNAME	"issue"
#define	BANNER		"Issue Server"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"ISSUE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"ISSUE_BANNER"
#define	VARSEARCHNAME	"ISSUE_NAME"
#define	VAROPTS		"ISSUE_OPTS"
#define	VARCONF		"ISSUE_CONF"
#define	VARLINELEN	"ISSUE_LINELEN"
#define	VARTARUSER	"ISSUE_USER"
#define	VARMNTFNAME	"ISSUE_MNTFILE"
#define	VARADMINS	"ISSUE_ADMINS"
#define	VARKEYNAME	"ISSUE_KEYNAME"
#define	VARDEBUGLEVEL	"ISSUE_DEBUGLEVEL"
#define	VARAFNAME	"ISSUE_AF"
#define	VAREFNAME	"ISSUE_EF"
#define	VAROFNAME	"ISSUE_OF"
#define	VARIFNAME	"ISSUE_IF"

#define	VARDEBUGFNAME	"ISSUE_DEBUGFILE"
#define	VARDEBUGFD1	"ISSUE_DEBUGFD"
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
#define	VARGROUPNAME	"GROUPNAME"
#define	VARLOGNAME	"LOGNAME"

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

#ifndef	VBUFLEN
#define	VBUFLEN		(2 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	ETCDNAME	"etc"
#define	MSDNAME		"var"
#define	VARDNAME	"var"
#define	SPOOLDNAME	"var/spool"
#define	RUNDNAME	"var/run"
#define	STAMPDNAME	"var/timestamps"	/* timestamp directory */
#define	LOCKDNAME	"spool/locks"
#define	LOGDNAME	"log"
#define	LOGCNAME	"log"
#define	MDNAME		"issue"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	PIDFNAME	"issue"			/* mutex PID file */
#define	HELPFNAME	"help"
#define	CONFFNAME	"conf"
#define	DEFSFNAME	"def"
#define	XEFNAME		"xe"
#define	PVARSFNAME	"pvar"
#define	STAMPFNAME	"issue"
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
#define	ISSUEFNAME	"issue"
#define	TSFNAME		".lastmaint"

#define	LOGFNAME	"log/issue"		/* activity log */
#define	LOCKFNAME	"spool/locks/issue"	/* lock mutex file */
#define	MNTFNAME	"/etc/issue"

#define	SHMFNAME	"var/spool/%S/shareinfo"
#define	SERIALFNAME1	"var/serial"
#define	SERIALFNAME2	"/tmp/serial"

#define	PVARFEXT	"pvar"
#define	USERFEXT	"users"
#define	REQFEXT		"req"
#define	SVCFEXT		"svc"
#define	ACCFEXT		"acc"

#define	DEFPATH		"/usr/xpg4/bin:/usr/bin:/usr/sbin"

#define	PORTNAME	"issue"
#define	PORTNUM		"5112"		/* default TCP port */

#define	ORGCODE		"RC"
#define	DEFSVC		"default"

#define	LOGSIZE		(80*1024)
#define	MAXJOBS		4		/* maximum jobs at once */
#define	DEFNDEFS	20
#define	DEFNXENVS	200

#define	PROG_SENDMAIL	"/usr/lib/sendmail"

#define	HOUR_MAINT	18

#define	TO_RUN		(60*60)		/* default run interval */
#define	TO_POLL		5		/* poll-interval */
#define	TO_MARK		(24 * 3600)
#define	TO_PID		(2 * 60)
#define	TO_LOCK		(5*60)		/* lock time-out */
#define	TO_CACHE	60
#define	TO_POLLSVC	(5*60)		/* default interval (minutes) */
#define	TO_MINCHECK	(1*60)		/* minimal check interval */
#define	TO_STANDCHECK	5		/* default standing-server check */
#define	TO_MAINT	5		/* general maintenance */
#define	TO_RECVMSG	3		/* receiving a message */
#define	TO_READSVC	3		/* reading a service-code */
#define	TO_READ		3		/* reading data */
#define	TO_SVC		60		/* service acquire timeout */
#define	TO_SPEED	(24*3600)	/* interval between updates */
#define	TO_MARKTIME	(12*3600)	/* interval between updates */


