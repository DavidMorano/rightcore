/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)summary "
#define	BANNER		"Summary"
#define	SEARCHNAME	"summary"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"SUMMARY_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SUMMARY_BANNER"
#define	VARSEARCHNAME	"SUMMARY_NAME"
#define	VARFILEROOT	"SUMMARY_FILEROOT"
#define	VARLOGTAB	"SUMMARY_LOGTAB"
#define	VAREFNAME	"SUMMARY_EF"
#define	VARDEBUGFD1	"SUMMARY_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	NODEVAR		"NODE"
#define	CLUSTERVAR	"CLUSTER"
#define	SYSTEMVAR	"SYSTEM"
#define	PRINTERVAR	"PRINTER"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/summary"		/* mutex PID file */
#define	LOGFNAME	"var/log/summary"	/* activity log */
#define	LOCKFNAME	"spool/locks/summary"	/* lock mutex file */

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */


