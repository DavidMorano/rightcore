/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)TESTFTPSESSION "

#define	VARPROGRAMROOT1	"TESTFTPSESSION_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testftpsession"

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"
#define	MSGQFNAME	"msgq"

#define	LOGFNAME	"log/testftpsession"		/* activity log */
#define	PIDFNAME	"spool/run/testftpsession"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/testftpsession"	/* lock mutex file */

#define	STAMPDIR	"spool/timestamps"	/* timestamp directory */
#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	BANNER		"Test Echo"

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)

#define	PORTSPEC_ECHO	"echo"
#define	SVCSPEC_ECHO	"echo"

#define	PORT_ECHO	7

#define	DIALTIME	20

#define	VARDEBUGFD	"DEBUG_FD"



