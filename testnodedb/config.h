/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testnodedb "

#define	VARPROGRAMROOT1	"TESTNODEDB_PROGRAMROOT"
#define	VARPROGRAMROOT2	"HOME"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTNODEDB_BANNER"
#define	VARSEARCHNAME	"TESTNODEDB_NAME"

#define	VARFILEROOT	"TESTNODEDB_FILEROOT"
#define	VARLOGTAB	"TESTNODEDB_LOGTAB"

#define	VARDEBUGFD1	"TESTNODEDB_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testnodedb"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/testnodedb"		/* mutex PID file */
#define	LOGFNAME	"var/log/testnodedb"		/* activity log */
#define	LOCKFNAME	"spool/locks/testnodedb"	/* lock mutex file */

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Test NODEDB"

#define	DEFSIZESPEC	"100000"		/* default target log size */



