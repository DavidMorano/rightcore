/* config */

/* last modified %G% version %I% */


#define	VERSION		"0e"
#define	WHATINFO	"@(#)DWD "

#define	PROGRAMROOTVAR1	"DWD_PROGRAMROOT"
#define	PROGRAMROOTVAR2	"PROGRAMROOT"

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
#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	POLLTIME	250		/* in seconds */
#define	SRVIDLETIME	7		/* seconds */
#define	JOBIDLETIME	20		/* seconds */
#define	JOBRETRYTIME	120		/* seconds */

#define	INTERNALSUFFIX	"dwd"		/* internal function service */
#define	LOGID		"*"

#define	BANNER		"Directory Watcher Daemon (DWD)"

#define	MAXJOBS		10		/* maximum simultaneous jobs */
#define	NFORKS		30		/* maximum forks before failure */

#define	DEBUGFDVAR1	"DWD_DEBUGFD"
#define	DEBUGFDVAR2	"DEBUGFD"

#define	POLLMODETIME	360		/* seconds */



