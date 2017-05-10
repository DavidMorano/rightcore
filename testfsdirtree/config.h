/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)TESTFSDIRTREE "
#define	BANNER		"Test FSDIRTREE"
#define	SEARCHNAME	"testfsdirtree"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTFSDIRTREE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTFSDIRTREE_BANNER"
#define	VARSEARCHNAME	"TESTFSDIRTREE_NAME"
#define	VAROPTS		"TESTFSDIRTREE_OPTS"
#define	VARNEWSDIR	"TESTFSDIRTREE_NEWSDIR"
#define	VARAFNAME	"TESTFSDIRTREE_AF"
#define	VAREFNAME	"TESTFSDIRTREE_EF"
#define	VARCFNAME	"TESTFSDIRTREE_CF"
#define	VARLFNAME	"TESTFSDIRTREE_LF"
#define	VARERRORFNAME	"TESTFSDIRTREE_ERRORFILE"

#define	VARDEBUGFNAME	"TESTFSDIRTREE_DEBUGFILE"
#define	VARDEBUGFD1	"TESTFSDIRTREE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"
#define	VARTERM		"TERM"
#define	VARCOLUMNS	"COLUMNS"

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	NEWSDNAME	"var/spool/boards"		/* news directory */
#define	STAMPDNAME	"var/timestamps"		/* timestamps */

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	HELPFNAME	"help"
#define	CONFFNAME	"conf"
#define	STAMPFNAME	"testfsdirtree"
#define	SRVFNAME	"srvtab"
#define	ACCFNAME	"acctab"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	XENVFNAME	"xenv"
#define	XPATHFNAME	"xpath"
#define	USERFNAME	"testfsdirtree.users"
#define	DIRCACHEFNAME	".dircache"

#define	LOGFNAME	"log/testfsdirtree"		/* activity log */
#define	PIDFNAME	"var/run/testfsdirtree"		/* mutex PID file */
#define	SERIALFNAME	"var/serial"			/* serial file */
#define	LOCKFNAME	"var/spool/locks/testfsdirtree"	/* lock mutex file */

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)	/* nominal log file length */
#define	MAXJOBS		4		/* maximum jobs at once */

#define	TI_POLLSVC	(5 * 60)	/* default interval (minutes) */
#define	TI_MINCHECK	(1 * 60)	/* minimal check interval */

#define	TO_PIDLOCK	5



