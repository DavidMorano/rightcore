/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0b"
#define	WHATINFO	"@(#)progswitch "
#define	BANNER		"Program Switch"
#define	SEARCHNAME	"progswitch"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"PROGSWITCH_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PROGSWITCH_BANNER"
#define	VARSEARCHNAME	"PROGSWITCH_NAME"
#define	VAROPTS		"PROGSWITCH_OPTS"
#define	VARFILEROOT	"PROGSWITCH_FILEROOT"
#define	VARLOGTAB	"PROGSWITCH_LOGTAB"
#define	VARAFNAME	"PROGSWITCH_AF"
#define	VARLFNAME	"PROGSWITCH_LF"
#define	VAREFNAME	"PROGSWITCH_EF"
#define	VARERRORFNAME	"PROGSWITCH_ERRORFILE"

#define	VARDEBUGFNAME	"PROGSWITCH_DEBUGFILE"
#define	VARDEBUGFD1	"PROGSWITCH_DEBUGFD"
#define	VARDEBUGLEVEL	"PROGSWITCH_DEBUGLEVEL"
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
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	MAPFNAME	"dir"
#define	DEFSFNAME	"defs"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/progswitch"		/* mutex PID file */
#define	LOGFNAME	"var/log/progswitch"		/* activity log */
#define	LOCKFNAME	"spool/locks/progswitch"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	OPT_LOGPROG	TRUE


