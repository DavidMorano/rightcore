/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testermesc "
#define	BANNER		"Test Term Escape"
#define	SEARCHNAME	"testermesc"

#define	VARPROGRAMROOT1	"TESTTERMOUT_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"
#define	VARDEBUGFD1	"TESTTERMOUT_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	HELPFNAME	"help"
#define	LOGFNAME	"log/testermesc"		/* activity log */
#define	PIDFNAME	"var/run/testermesc"	/* mutex PID file */
#define	PSFNAME		"spool/testermesc/testermesc"
#define	LOCKFNAME	"spool/locks/testermesc"	/* lock mutex file */

#define	PTDIR		"etc/testermesc"
#define	PSSPOOLDIR	"spool/testermesc"
#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	PTFNAME		"pingtab"

#define	LOGSIZE		(160*1024)

#define	PINGTIMEOUT	15
#define	MINPINGINT	30		/* minimum interval between pings */
#define	MINUPDTIME	4


