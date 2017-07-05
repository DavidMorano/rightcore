/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0b"
#define	WHATINFO	"@(#)CHECKLOGS "
#define	BANNER		"Check Log-File Length"
#define	SEARCHNAME	"checklogs"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"CHECKLOGS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"CHECKLOGS_BANNER"
#define	VARSEARCHNAME	"CHECKLOGS_NAME"
#define	VAROPTS		"CHECKLOGS_OPTS"
#define	VARLOGTAB	"CHECKLOGS_LOGTAB"
#define	VARLOGROOT	"CHECKLOGS_LOGROOT"
#define	VARFILEROOT	"CHECKLOGS_FILEROOT"
#define	VARDEFSIZE	"CHECKLOGS_DEFSIZE"
#define	VARAFNAME	"CHECKLOGS_AF"
#define	VAREFNAME	"CHECKLOGS_EF"
#define	VARERRORFNAME	"CHECKLOGS_ERRORFILE"

#define	VARDEBUGFNAME	"CHECKLOGS_DEBUGFILE"
#define	VARDEBUGFD1	"CHECKLOGS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
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

#define	LOGTABFNAME	"logtab"
#define	CONFIGFNAME	"conf"
#define	DEFSFNAME	"defs"
#define	XPATHFNAME	"xpath"
#define	XENVFNAME	"xenv"
#define	HELPFNAME	"help"
#define	LOGFNAME	"log/checklogs"		/* activity log */
#define	PIDFNAME	"run/checklogs"		/* mutex PID file */
#define	LOCKFNAME	"spool/locks/checklogs"	/* lock mutex file */

#define	LOGSIZE		(80*1024)	/* my own! */

#define	DEFLOGSIZE	100000		/* default target log size */

#define	TO_PIDLOCK	5


