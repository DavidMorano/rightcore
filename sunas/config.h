/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0"
#define	WHATINFO	"@(#)sunas "
#define	BANNER		"Execute SUN AS"
#define	SEARCHNAME	"sunas"
#define	VARPRNAME	"EXTRA"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/extra"
#endif

#define	VARPROGRAMROOT1	"SUNAS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SUNAS_BANNER"
#define	VARSEARCHNAME	"SUNAS_NAME"
#define	VAROPTS		"SUNAS_OPTS"
#define	VARFILEROOT	"SUNAS_FILEROOT"
#define	VARLOGTAB	"SUNAS_LOGTAB"
#define	VARAFNAME	"SUNAS_AF"
#define	VAREFNAME	"SUNAS_EF"
#define	VARERRORFNAME	"SUNAS_ERRORFILE"

#define	VARDEBUGFNAME	"SUNAS_DEBUGFILE"
#define	VARDEBUGFD1	"SUNAS_DEBUGFD"
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
#define	COPYDNAME	"/var/tmp/as"

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

#define	PROG_PREFIX	"SUN"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */


