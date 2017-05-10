/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)rshd "
#define	SEARCHNAME	"rshd"
#define	BANNER		"Remote Shell Daemon"
#define	VARPRNAME	"EXTRA"

#define	VARPROGRAMROOT1	"RSHD_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"RSHD_BANNER"
#define	VARSEARCHNAME	"RSHD_NAME"
#define	VAROPTS		"RSHD_OPTS"
#define	VARFILEROOT	"RSHD_FILEROOT"
#define	VARLOGTAB	"RSHD_LOGTAB"
#define	VARDEBUGFNAME	"RSHD_DEBUGFILE"

#define	VARDEBUGFD1	"RSHD_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/extra"
#endif

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"
#define	LOGCNAME	"log"

#define	INITFNAME	"/etc/default/init"
#define	LOGSTDINFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/rshd"		/* mutex PID file */
#define	LOGFNAME	"var/log/rshd"		/* activity log */
#define	LOCKFNAME	"spool/locks/rshd"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */



