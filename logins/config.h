/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)logins "
#define	BANNER		"Login Names"

#define	VARPROGRAMROOT1	"LOGINS_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LOGINS_BANNER"
#define	VARSEARCHNAME	"LOGINS_NAME"

#define	VARFILEROOT	"LOGINS_FILEROOT"
#define	VARLOGTAB	"LOGINS_LOGTAB"

#define	NODEVAR		"NODE"
#define	CLUSTERVAR	"CLUSTER"
#define	SYSTEMVAR	"SYSTEM"
#define	PRINTERVAR	"PRINTER"

#define	VARDEBUGFD1	"LOGINS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"logins"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/logins"		/* mutex PID file */
#define	LOGFNAME	"var/log/logins"	/* activity log */
#define	LOCKFNAME	"spool/locks/logins"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */



