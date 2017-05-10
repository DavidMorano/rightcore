/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)sysexec "
#define	BANNER		"System Execute"
#define	SEARCHNAME	"sysexec"
#define	VARPRNAME	"EXTRA"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/extra"
#endif

#define	VARPROGRAMROOT1	"SYSEXEC_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SYSEXEC_BANNER"
#define	VARSEARCHNAME	"SYSEXEC_NAME"
#define	VAROPTS		"SYSEXEC_OPTS"
#define	VARFILEROOT	"SYSEXEC_FILEROOT"
#define	VARLOGTAB	"SYSEXEC_LOGTAB"
#define	VARAFNAME	"SYSEXEC_AF"
#define	VAREFNAME	"SYSEXEC_EF"
#define	VARERRORFNAME	"SYSEXEC_ERRORFILE"

#define	VARDEBUGFNAME	"SYSEXEC_DEBUGFILE"
#define	VARDEBUGFD1	"SYSEXEC_DEBUGFD"
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

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	MAPFNAME	"map"
#define	DEFSFNAME	"defs"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/progswitch"		/* mutex PID file */
#define	LOGFNAME	"var/log/progswitch"		/* activity log */
#define	LOCKFNAME	"spool/locks/progswitch"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */


