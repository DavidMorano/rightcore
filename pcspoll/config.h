/* config */

/* last modified %G% version %I% */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#define	P_PCSPOLL	1

#define	VERSION		"0b"
#define	WHATINFO	"@(#)PCSPOLL "
#define	BANNER		"PCS Poll"
#define	SEARCHNAME	"pcspoll"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"PCSPOLL_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PCSPOLL_BANNER"
#define	VARSEARCHNAME	"PCSPOLL_NAME"
#define	VAROPTS		"PCSPOLL_OPTS"
#define	VARCONF		"PCSPOLL_CONF"
#define	VARLINELEN	"FINGERS_LINELEN"
#define	VARAFNAME	"PCSPOLL_AF"
#define	VAREFNAME	"PCSPOLL_EF"
#define	VARERRORFNAME	"PCSPOLL_ERRORFILE"

#define	VARDEBUGFNAME	"PCSPOLL_DEBUGFILE"
#define	VARDEBUGFD1	"PCSPOLL_DEBUGFD"
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

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	ETCDNAME	"etc"
#define	VARDNAME	"var"
#define	LOGDNAME	"var/log"
#define	RUNDNAME	"var/run"
#define	LOCKDNAME	"spool/locks"
#define	STAMPDNAME	"var/timestamps"	/* timestamp directory */

#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	HELPFNAME	"help"
#define	CONFFNAME	"conf"
#define	SVCFNAME	"svc"
#define	ACCFNAME	"acc"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	XPATHFNAME	"xpath"
#define	XLIBPATHFNAME	"xlibpath"
#define	XENVFNAME	"xenv"
#define	DEFSFNAME	"def"
#define	XEFNAME		"xe"
#define	PVARSFNAME	"pvar"

#define	LOGFNAME	"log/pcspoll"		/* activity log */
#define	PIDFNAME	"var/run/pcspoll"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/pcspoll"	/* lock mutex file */
#define	SERIALFNAME	"serial"		/* serial file */

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

#define	TO_PIDLOCK	0		/* PID lock */
#define	TO_LOCK		4		/* lock acquisition */
#define	TO_POLL		5		/* maximum polling interval */
#define	TO_POLLSVC	(5*60)		/* service polling */
#define	TO_RUN		(60*60)		/* default run interval */
#define	TO_MINCHECK	(2*60)		/* minimal check interval */
#define	TO_STANDCHECK	5		/* default standing-server check */
#define	TO_MAINT	(3*60)		/* general maintenance */
#define	TO_STAMP	(3*60)		/* stamp (lock) update */
#define	TO_RECVMSG	3		/* read message */
#define	TO_READSVC	3		/* read service-code */
#define	TO_READ		3		/* read data */
#define	TO_LOGJOBS	60
#define	TO_DBDUMP	(8 * 3600)
#define	TO_SVC		60		/* service acquire timeout */
#define	TO_SPEED	(24*3600)	/* interval between updates */
#define	TO_MARKTIME	(12*3600)	/* mark-time interval */
#define	TO_BROKEN	(1*3600)	/* broken listener re-activation */
#define	TO_RECVFD	10		/* receiving a file-desriptor */
#define	TO_SENDFD	10		/* sending a file-desriptor */
#define	TO_JOBDIR	(3*3600)	/* job-dir check */
#define	TO_JOBFILE	(8*3600)	/* job-file age */


