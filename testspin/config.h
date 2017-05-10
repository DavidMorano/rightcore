/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)TESTSPIN "

#define	VARPROGRAMROOT1	"TESTSPIN_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"TESTSPIN_NAME"
#define	TESTSPIN_NAMEVAR	"TESTSPIN_FNAME"

#define	VARDEBUGFD1	"TESTSPIN_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testspin"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	LOGFNAME	"log/testspin"		/* activity log */
#define	PIDFNAME	"spool/run/testspin"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/testspin"	/* lock mutex file */

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFLOGINFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	LOGSIZE		(80*1024)

#define	BANNER		"LockFile"

#define	TESTSPIN_DEFTIMEOUT	(5 * 60)		/* seconds */
#define	TESTSPIN_DEFREMOVE	(5 * 60)		/* seconds */
#define	TESTSPIN_MULREMOVE	10

#define	LKFILE_DEFTIMEOUT	(5 * 60)		/* seconds */
#define	LKFILE_DEFREMOVE	(5 * 60)		/* seconds */
#define	LKFILE_MULREMOVE	10




