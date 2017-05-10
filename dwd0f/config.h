/* config */

/* last modified %G% version %I% */


#define	VERSION		"0g"
#define	WHATINFO	"@(#)DWD "
#define	BANNER		"Directory Watcher Daemon (DWD)"
#define	SEARCHNAME	"dwd"
#define	VARPR		"PCS"

#define	VARPROGRAMROOT1	"DWD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPR
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"DWD_BANNER"
#define	VARSEARCHNAME	"DWD_SEARCHNAME"
#define	VAROPTS		"DWD_OPTS"
#define	VARCONF		"DWD_CONF"
#define	VARAFNAME	"DWD_AF"
#define	VAREFNAME	"DWD_EF"
#define	VARERRORFNAME	"DWD_ERRORFILE"

#define	VARDEBUGFNAME	"DWD_DEBUGFILE"
#define	VARDEBUGFD1	"DWD_DEBUGFD"
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

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	CONFIGDIR1	"etc/dwd"
#define	CONFIGDIR2	"etc"

#define	CONFIGFNAME1	"dwd.conf"
#define	CONFIGFNAME2	"conf"
#define	DIRECTORY	"q"
#define	INTERRUPT	"i"
#define	SRVFNAME1	"dwd.srvtab"
#define	SRVFNAME2	"srvtab"
#define	LOGFNAME	"dwd.log"	/* activity log */
#define	LOCKFNAME	"dwd.lock"	/* mutex lock file */
#define	PIDFNAME	"pid"		/* mutex lock file */
#define	HELPFNAME	"help"

#define	POLLTIME	250		/* in seconds */
#define	SRVIDLETIME	7		/* seconds */
#define	JOBIDLETIME	20		/* seconds */
#define	JOBRETRYTIME	120		/* seconds */

#define	INTERNALSUFFIX	"dwd"		/* internal function service */
#define	LOGIDNIL	"*"

#define	MAXJOBS		10		/* maximum simultaneous jobs */
#define	NFORKS		30		/* maximum forks before failure */

#define	POLLMODETIME	360		/* seconds */


