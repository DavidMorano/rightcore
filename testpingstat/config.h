/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)TESTPINGSTAT "

#define	VARPROGRAMROOT1	"TESTPINGSTAT_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testpingstat"

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"

#define	LOGFNAME	"log/testpingstat"		/* activity log */
#define	PIDFNAME	"spool/run/testpingstat"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/testpingstat"	/* lock mutex file */

#define	STAMPDIR	"spool/timestamps"	/* timestamp directory */
#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	BANNER		"Test Echo"

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)

#define	PORTSPEC_ECHO	"echo"
#define	SVCSPEC_ECHO	"echo"

#define	PORT_ECHO	7

#define	DIALTIME	20

#define	VARDEBUGFD1	"TESTPINGSTAT_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"



