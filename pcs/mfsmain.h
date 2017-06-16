/* config */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	MFSMAIN_INCLUDE
#define	MFSMAIN_INCLUDE	1


#define	VERSION		"0a"
#define	WHATINFO	"@(#)mfserve "
#define	BANNER		"Multi-Function Server"
#define	SEARCHNAME	"mfserve"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MFSERVE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MFSERVE_BANNER"
#define	VARSEARCHNAME	"MFSERVE_NAME"
#define	VAROPTS		"MFSERVE_OPTS"
#define	VARQUIET	"MFSERVE_QUIET"
#define	VARCONFIG	"MFSERVE_CONF"
#define	VARFILEROOT	"MFSERVE_FILEROOT"
#define	VARLOGTAB	"MFSERVE_LOGTAB"
#define	VARPIDFNAME	"MFSERVE_PIDFILE"
#define	VARREQFNAME	"MFSERVE_REQFILE"
#define	VARMSFNAME	"MFSERVE_MSFILE"
#define	VARMSNODE	"MFSERVE_MSNODE"
#define	VARCFNAME	"MFSERVE_CF"
#define	VARLFNAME	"MFSERVE_LF"
#define	VARAFNAME	"MFSERVE_AF"
#define	VAREFNAME	"MFSERVE_EF"

#define	VARDEBUGFNAME	"MFSERVE_DEBUGFILE"
#define	VARDEBUGFD1	"MFSERVE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
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
#define	VARHZ		"HZ"
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRMFS	"MFS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	MSDNAME		"var"
#define	VCNAME		"var"
#define	LOGCNAME	"log"
#define	LOGDNAME	"var/log"
#define	RUNDNAME	"var/run"
#define	PIDDNAME	"var/run/%S"
#define	LOCKDNAME	"var/spool/locks"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	PIDCNAME	"pid"
#define	REQCNAME	"req"
#define	SVCCNAME	"svc"
#define	ACCCNAME	"acc"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	MSFNAME		"ms"
#define	PIDFNAME	"mfs"		/* mutex PID file */
#define	LOGFNAME	"mfs"		/* activity log */
#define	SVCFNAME	"svc"		/* svctab file */
#define	ACCFNAME	"acc"		/* acctab file */
#define	LOCKFNAME	"%N.%S"		/* lock mutex file */
#define	SERIALFNAME	"serial"

#define	LOGSIZE		(80*1024)

#define	TO_RUN		(5 * 60)
#define	TO_MARK		(8 * 3600)
#define	TO_SPEED	(24 * 3600)
#define	TO_DIRMAINT	(4*3600)
#define	TO_DIRCLIENTS	(24*3600)
#define	TO_POLL		7
#define	TO_IDLE		5
#define	TO_LOCK		4
#define	TO_CONFIG	5		/* configuration changed */
#define	TO_MAINT	10		/* maintenance */
#define	TO_OPENSERVE	5		/* time-out to connect to server */

#define	OPT_LOGPROG	TRUE

#endif /* MFSMAIN_INCLUDE */


