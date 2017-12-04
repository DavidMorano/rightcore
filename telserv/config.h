/* config */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	P_TELSERV	1

#define	VERSION		"0b"
#define	WHATINFO	"@(#)telserv "
#define	BANNER		"Telnet Server"
#define	SEARCHNAME	"telserv"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"TELSERV_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TELSERV_BANNER"
#define	VARSEARCHNAME	"TELSERV_NAME"
#define	VAROPTS		"TELSERV_OPTS"
#define	VARCONF		"TELSERV_CONF"
#define	VARLINELEN	"TELSERV_LINELEN"
#define	VARAFNAME	"TELSERV_AF"
#define	VAREFNAME	"TELSERV_EF"
#define	VARERRORFNAME	"TELSERV_ERRORFILE"

#define	VARDEBUGFNAME	"TELSERV_DEBUGFILE"
#define	VARDEBUGFD1	"TELSERV_DEBUGFD"
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
#define	SVCARGSLEN	120
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	ETCDNAME	"etc"
#define	VARDNAME	"var"
#define	MSDNAME		"var"
#define	RUNDNAME	"var/run"
#define	STAMPDNAME	"var/timestamps"	/* timestamp directory */
#define	LOCKDNAME	"spool/locks"
#define	LOGDNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	LOGCNAME	"log"

#define	HELPFNAME	"help"
#define	CONFFNAME	"conf"
#define	DEFSFNAME	"def"
#define	XEFNAME		"xe"
#define	PVARSFNAME	"pvar"
#define	STAMPFNAME	"telserv"
#define	REQFNAME	"req"			/* "request" (IPC) file */
#define	PASSFNAME	"pass"
#define	SVCFNAME	"svc"
#define	ACCFNAME	"acc"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	XPATHFNAME	"xpath"
#define	XLIBPATHFNAME	"xlibpath"
#define	XENVFNAME	"xenv"
#define	SERIALFNAME	"serial"		/* serial file */
#define	PROJFNAME	".project"
#define	PLANFNAME	".plan"

#define	LOGFNAME	"log/telserv"		/* activity log */
#define	PIDFNAME	"var/run/telserv"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/telserv"	/* lock mutex file */

#define	PVARFEXT	"pvar"
#define	USERFEXT	"users"
#define	REQFEXT		"req"
#define	SVCFEXT		"svc"
#define	ACCFEXT		"acc"

#define	DEFPATH		"/usr/xpg4/bin:/usr/bin:/usr/sbin"

#ifndef	COLUMNS
#define	COLUMNS		80		/* output columns (should be 80) */
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(2 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#define	LOGSIZE		(80*1024)	/* nominal log file length */
#define	MAXJOBS		4		/* maximum jobs at once */
#define	DEFNDEFS	20
#define	DEFNXENVS	200

#define	ORGCODE		"RC"
#define	DEFSVC		"default"

#define	PROG_SENDMAIL	"/usr/lib/sendmail"

#define	TO_LOCK		4		/* lock acquisition timeout */
#define	TO_POLLSVC	(5 * 60)	/* default interval (minutes) */
#define	TO_RUN		(60 * 60)	/* default run interval */
#define	TO_MINCHECK	(1 * 60)	/* minimal check interval */
#define	TO_STANDCHECK	5		/* default standing-server check */
#define	TO_MAINT	5		/* general maintenance */
#define	TO_RECVMSG	3		/* read message */
#define	TO_READSVC	3		/* read service-code */
#define	TO_READ		3		/* read data */
#define	TO_SVC		60		/* service acquire timeout */
#define	TO_SPEED	(24*3600)	/* interval between updates */
#define	TO_MARKTIME	(12*3600)	/* mark-time interval */
#define	TO_BROKEN	(1*3600)	/* broken listener re-activation */
#define	TO_RECVFD	30		/* receiving a file-desriptor */
#define	TO_SENDFD	30		/* sending a file-desriptor */


