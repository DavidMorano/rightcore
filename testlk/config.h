/* config */


#define	VERSION		"0b"
#define	WHATINFO	"@(#)TESTLK "

#define	VARPROGRAMROOT1	"TESTLK_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"TESTLK_NAME"
#define	TESTLK_NAMEVAR	"TESTLK_FNAME"

#define	VARDEBUGFD1	"TESTLK_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"lkfile"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	LOGFNAME	"log/lkfile"		/* activity log */
#define	PIDFNAME	"spool/run/lkfile"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/lkfile"	/* lock mutex file */

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	LOGSIZE		(80*1024)

#define	BANNER		"LockFile"


#define	TESTLK_DEFTIMEOUT	(5 * 60)		/* seconds */
#define	TESTLK_DEFREMOVE	(5 * 60)		/* seconds */
#define	TESTLK_MULREMOVE	10




