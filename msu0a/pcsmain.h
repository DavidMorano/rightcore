/* config */


/* revision history:

	= 2004-03-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	PCS_INCLUDE
#define	PCS_INCLUDE	1


#define	VERSION		"0a"
#define	WHATINFO	"@(#)pcs "
#define	BANNER		"Personal Communication Services"
#define	SEARCHNAME	"pcs"
#define	VARPRNAME	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"PCS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PCS_BANNER"
#define	VARSEARCHNAME	"PCS_NAME"
#define	VAROPTS		"PCS_OPTS"
#define	VARQUIET	"PCS_QUIET"
#define	VARCONFIG	"PCS_CONF"
#define	VARFILEROOT	"PCS_FILEROOT"
#define	VARLOGTAB	"PCS_LOGTAB"
#define	VARPIDFNAME	"PCS_PIDFILE"
#define	VARREQFNAME	"PCS_REQFILE"
#define	VARMSFNAME	"PCS_MSFILE"
#define	VARMSNODE	"PCS_MSNODE"
#define	VARCFNAME	"PCS_CF"
#define	VARLFNAME	"PCS_LF"
#define	VARAFNAME	"PCS_AF"
#define	VAREFNAME	"PCS_EF"
#define	VAROFNAME	"PCS_OF"
#define	VARIFNAME	"PCS_IF"

#define	VARDEBUGLEVEL	"PCS_DEBUGLEVEL"
#define	VARDEBUGFNAME	"PCS_DEBUGFILE"
#define	VARDEBUGFD1	"PCS_DEBUGFD"
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

#define	REQCNAME	"req"
#define	PIDCNAME	"pid"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	MSFNAME		"ms"
#define	PIDFNAME	"pid"		/* mutex PID file */
#define	LOGFNAME	"pcs"		/* activity log */
#define	LOCKFNAME	"%N.%S"		/* lock mutex file */
#define	SERIALFNAME	"serial"

#define	LOGSIZE		(80*1024)

#define	TO_RUN		(5 * 60)
#define	TO_MARK		(8 * 3600)
#define	TO_SPEED	(24 * 3600)
#define	TO_DIRMAINT	(1*3600)	/* interval between maintenance */
#define	TO_DIRCLIENT	(12*3600)	/* age of client files */
#define	TO_POLL		7
#define	TO_IDLE		5
#define	TO_LOCK		4
#define	TO_CONFIG	5		/* configuration changed */
#define	TO_OPENSERVE	5		/* time-out to connect to server */

#define	OPT_LOGPROG	TRUE

#endif /* PCS_INCLUDE */


