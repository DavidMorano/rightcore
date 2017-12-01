/* config */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	MSU_INCLUDE
#define	MSU_INCLUDE	1


#define	VERSION		"0a"
#define	WHATINFO	"@(#)msu "
#define	BANNER		"Machine Status Update"
#define	SEARCHNAME	"msu"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MSU_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MSU_BANNER"
#define	VARSEARCHNAME	"MSU_NAME"
#define	VAROPTS		"MSU_OPTS"
#define	VARQUIET	"MSU_QUIET"
#define	VARCONFIG	"MSU_CONF"
#define	VARFILEROOT	"MSU_FILEROOT"
#define	VARLOGTAB	"MSU_LOGTAB"
#define	VARPIDFNAME	"MSU_PIDFILE"
#define	VARREQFNAME	"MSU_REQFILE"
#define	VARMSFNAME	"MSU_MSFILE"
#define	VARMSNODE	"MSU_MSNODE"
#define	VARCFNAME	"MSU_CF"
#define	VARLFNAME	"MSU_LF"
#define	VARAFNAME	"MSU_AF"
#define	VAREFNAME	"MSU_EF"
#define	VAROFNAME	"MSU_OF"
#define	VARIFNAME	"MSU_IF"

#define	VARDEBUGLEVEL	"MSU_DEBUGLEVEL"
#define	VARDEBUGFNAME	"MSU_DEBUGFILE"
#define	VARDEBUGFD1	"MSU_DEBUGFD"
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
#define	PIDFNAME	"msu"		/* mutex PID file */
#define	LOGFNAME	"msu"		/* activity log */
#define	LOCKFNAME	"%N.%S"		/* lock mutex file */
#define	SERIALFNAME	"serial"

#define	LOGSIZE		(80*1024)

#define	TO_RUN		(5 * 60)
#define	TO_MARK		(8 * 3600)
#define	TO_SPEED	(24 * 3600)
#define	TO_POLL		7
#define	TO_IDLE		5
#define	TO_LOCK		4
#define	TO_CONFIG	5		/* configuration changed */

#define	OPT_LOGPROG	TRUE


#endif /* MSU_INCLUDE */


