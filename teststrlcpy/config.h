/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)teststrlcpy "

#define	VARPROGRAMROOT1	"TESTSTRLCPY_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTSTRLCPY_BANNER"
#define	VARSEARCHNAME	"TESTSTRLCPY_NAME"

#define	VARFILEROOT	"TESTSTRLCPY_FILEROOT"
#define	VARLOGTAB	"TESTSTRLCPY_LOGTAB"

#define	VARDEBUGFD1	"TESTSTRLCPY_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"teststrlcpy"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/teststrlcpy"		/* mutex PID file */
#define	LOGFNAME	"var/log/teststrlcpy"	/* activity log */
#define	LOCKFNAME	"spool/locks/teststrlcpy"	/* lock mutex file */

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Test STRLCPY"

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */




