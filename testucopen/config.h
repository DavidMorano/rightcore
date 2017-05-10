/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)TESTUCOPEN "
#define	BANNER		"Test UC_Open"
#define	SEARCHNAME	"testucopen"

#define	VARPROGRAMROOT1	"TESTUCOPEN_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VAREFNAME	"TESTUCOPEN_EF"
#define	VARERRORFNAME	"TESTUCOPEN_ERRORFILE"

#define	VARDEBUGFNAME	"TESTUCOPEN_DEBUGFILE"
#define	VARDEBUGFD1	"TESTUCOPEN_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDIR		"/tmp"
#define	WORKDIR		"/tmp"
#define	STAMPDIR	"spool/timestamps"	/* timestamp directory */

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"
#define	MSGQFNAME	"msgq"

#define	LOGFNAME	"log/testucopen"		/* activity log */
#define	PIDFNAME	"spool/run/testucopen"		/* mutex PID file */
#define	LOCKFNAME	"spool/locks/testucopen"	/* lock mutex file */

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)
#define	DEFINTERVAL	5			/* default interval (minutes) */
#define	MAXJOBS		5			/* maximum jobs at once */

#define	ECHOPORT	517



