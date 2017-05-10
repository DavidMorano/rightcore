/* config */


#define	VERSION		"0c"
#define	WHATINFO	"@(#)logname "

#define	VARPROGRAMROOT1	"LOGNAME_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LOGNAME_BANNER"
#define	VARSEARCHNAME	"LOGNAME_NAME"

#define	VARFILEROOT	"LOGNAME_FILEROOT"
#define	VARLOGTAB	"LOGNAME_LOGTAB"

#define	NODEVAR		"NODE"
#define	CLUSTERVAR	"CLUSTER"
#define	SYSTEMVAR	"SYSTEM"
#define	PRINTERVAR	"PRINTER"

#define	VARDEBUGFD1	"LOGNAME_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"logname"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/logname"		/* mutex PID file */
#define	LOGFNAME	"var/log/logname"	/* activity log */
#define	LOCKFNAME	"spool/locks/logname"	/* lock mutex file */

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Login Name"

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PROG_MKPWI	"mkpwi"



