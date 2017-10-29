/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)syslogd "
#define	BANNER		"System Logger"
#define	SEARCHNAME	"syslogd"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"SYSLOFD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SYSLOFD_BANNER"
#define	VARSEARCHNAME	"SYSLOFD_NAME"
#define	VAROPTS		"SYSLOFD_OPTS"
#define	VARFILEROOT	"SYSLOFD_FILEROOT"
#define	VARLOGTAB	"SYSLOFD_LOGTAB"
#define	VARAFNAME	"SYSLOFD_AF"
#define	VAREFNAME	"SYSLOFD_EF"
#define	VARERRORFNAME	"SYSLOFD_ERRORFILE"

#define	VARDEBUGFNAME	"SYSLOFD_DEBUGFILE"
#define	VARDEBUGFD1	"SYSLOFD_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	INITFNAME	"/etc/default/init"
#define	LOGSTDINFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/syslogd"		/* mutex PID file */
#define	LOGFNAME	"var/log/syslogd"	/* activity log */
#define	LOCKFNAME	"spool/locks/syslogd"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */


