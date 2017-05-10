/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)TESTCEXECER "
#define	BANNER		"Test CEXECER"
#define	SEARCHNAME	"testcexecer"

#define	VARPROGRAMROOT1	"TESTCEXECER_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARDEBUGFD1	"TESTCEXECER_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDIR		"/tmp"
#define	WORKDIR		"/tmp"

#define	LOGFNAME	"log/testcexecer"		/* activity log */
#define	PIDFNAME	"var/run/testcexecer"		/* mutex PID file */
#define	LOCKFNAME	"spool/locks/testcexecer"	/* lock mutex file */

#define	CONFFNAME	"conf"
#define	HELPFNAME	"help"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHFNAME	"path"
#define	REQFNAME	"req"

#define	LOGSIZE		(80*1024)

#define	PORTSPEC_ECHO	"echo"
#define	SVCSPEC_ECHO	"echo"

#define	PORT_ECHO	7

#define	DIALTIME	20
#define	DEFKEEPTIME	(3 * 60)	/* sanity check timeout */

#define	SANITYFAILURES	5		/* something w/ sanity ? */


