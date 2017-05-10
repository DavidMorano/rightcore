/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)id-a "

#define	VARPROGRAMROOT1	"IDA_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARFILEROOT	"IDA_FILEROOT"
#define	VARLOGTAB	"IDA_LOGTAB"

#define	VARDEBUGFD1	"IDA_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"id-a"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/ida"		/* mutex PID file */
#define	LOGFNAME	"var/log/ida"		/* activity log */
#define	LOCKFNAME	"spool/locks/ida"	/* lock mutex file */

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	LOGSIZE		(80*1024)

#define	BANNER		"ID Alternate"

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */




