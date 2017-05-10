/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)unlinkd "
#define	BANNER		"Unlink Daemon"
#define	SEARCHNAME	"unlinkd"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"UNLINKD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"UNLINKD_BANNER"
#define	VARSEARCHNAME	"UNLINKD_NAME"
#define	VARQUIET	"UNLINKD_QUIET"
#define	VAROPTS		"UNLINKD_OPTS"
#define	VARFILEROOT	"UNLINKD_FILEROOT"
#define	VARLOGTAB	"UNLINKD_LOGTAB"
#define	VARPIDFNAME	"UNLINKD_PIDFILE"
#define	VARREQFNAME	"UNLINKD_REQFILE"
#define	VARMSFNAME	"UNLINKD_MSFILE"
#define	VARMSNODE	"UNLINKD_MSNODE"
#define	VAREFNAME	"UNLINKD_EF"
#define	VARCFNAME	"UNLINKD_CF"
#define	VARLFNAME	"UNLINKD_LF"

#define	VARCONFIG	"UNLINKD_CONF"
#define	VARLOGFNAME	"UNLINKD_LOGFILE"
#define	VARDEBUGFNAME	"UNLINKD_DEBUGFILE"
#define	VARDEBUGFD1	"UNLINKD_DEBUGFD"
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
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	LISTENFNAME	"/tmp/local/unlinkd/req"
#define	MSDNAME		"var"
#define	VCNAME		"var"
#define	VARDNAME	"var"
#define	LOGCNAME	"log"
#define	LOGDNAME	"log"
#define	RUNDNAME	"var/run"
#define	PIDDNAME	"var/run/%S"
#define	LOCKDNAME	"var/spool/locks"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	REQCNAME	"req"
#define	PIDCNAME	"pid"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	MSFNAME		"ms"
#define	PIDFNAME	"unlinkd"			/* mutex PID file */
#define	LOGFNAME	"unlinkd"			/* activity log */
#define	LOCKFNAME	"%N.%S"			/* lock mutex file */
#define	SERIALFNAME	"serial"

#define	LOGSIZE		(80*1024)

#define	TO_RUN		(5 * 60)
#define	TO_MARK		(8 * 3600)
#define	TO_SPEED	(24 * 3600)
#define	TO_POLL		7
#define	TO_LOCK		4

#define	INTRUN		5
#define	INTPOLL		1
#define	INTMARK		(60*60)


