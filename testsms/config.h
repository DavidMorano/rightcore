/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)TESTSMS "
#define	BANNER		"Test SMS"
#define	SEARCHNAME	"testsms"

#define	VARPROGRAMROOT1	"TESTSMS_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARDEBUGFD1	"TESTSMS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"
#define	MSGQFNAME	"msgq"

#define	LOGFNAME	"log/testsms"		/* activity log */
#define	PIDFNAME	"spool/run/testsms"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/testsms"	/* lock mutex file */

#define	STAMPDIR	"spool/timestamps"	/* timestamp directory */
#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)

#define	PORTSPEC_ECHO	"echo"
#define	SVCSPEC_ECHO	"echo"

#define	PORT_ECHO	7

#define	DIALTIME	20



