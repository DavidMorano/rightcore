/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)theses "
#define	BANNER		"Theses"
#define	SEARCHNAME	"theses"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"THESES_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"THESES_BANNER"
#define	VARSEARCHNAME	"THESES_NAME"
#define	VAROPTS		"THESES_OPTS"
#define	VARFILEROOT	"THESES_FILEROOT"
#define	VARLOGTAB	"THESES_LOGTAB"
#define	VARLFNAME	"THESES_LF"
#define	VAREFNAME	"THESES_EF"
#define	VARDEBUGFNAME	"THESES_DEBUGFILE"
#define	VARDEBUGFD1	"THESES_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARPRINTER	"PRINTER"

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
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/theses"		/* mutex PID file */
#define	LOGFNAME	"var/log/theses"	/* activity log */
#define	LOCKFNAME	"spool/locks/theses"	/* lock mutex file */

#define	LOGLEN		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	MAXBLANKLINES	100
#define	DEFBLANKLINES	2


