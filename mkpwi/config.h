/* config */


/* revision history:

	= 2000-05-14, David A­D­ Morano

	Originally written for Rightcore Network Services.


*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)mkpwi "
#define	BANNER		"Make Password Name Index"
#define	SEARCHNAME	"mkpwi"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MKPWI_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MKPWI_BANNER"
#define	VARSEARCHNAME	"MKPWI_NAME"
#define	VAROPTS		"MKPWI_OPTS"
#define	VARFILEROOT	"MKPWI_FILEROOT"
#define	VARLOGTAB	"MKPWI_LOGTAB"
#define	VARDBNAME	"MKPWI_DBNAME"
#define	VARAFNAME	"MKPWI_AF"
#define	VAREFNAME	"MKPWI_EF"
#define	VARLFNAME	"MKPWI_LF"
#define	VARPFNAME	"MKPWI_PF"
#define	VARPIDFNAME	"MKPWI_PID"
#define	VARERRORFNAME	"MKPWI_ERRORFILE"

#define	VARDEBUGFNAME	"MKPWI_DEBUGFILE"
#define	VARDEBUGFD1	"MKPWI_DEBUGFD"
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

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	PRVDNAME	"var"
#define	PWIDNAME	"pwi"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	LOGFNAME	"var/log/mkpwi"		/* activity log */
#define	PIDFNAME	"run/mkpwi"		/* mutex PID file */
#define	LOCKFNAME	"spool/locks/mkpwi"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	GROUPNAME_TOOLS	"tools"

#define	GID_SYS		3
#define	GID_TOOLS	61

#define	DBSUF		"pwi"

#define	OPT_LOGPROG	TRUE


