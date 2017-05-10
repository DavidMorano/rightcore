/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)palive "

#define	VARPROGRAMROOT1	"PALIVE_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PALIVE_BANNER"
#define	VARSEARCHNAME	"PALIVE_NAME"

#define	VARFILEROOT	"PALIVE_FILEROOT"
#define	VARLOGTAB	"PALIVE_LOGTAB"

#define	NODEVAR		"NODE"
#define	CLUSTERVAR	"CLUSTER"
#define	SYSTEMVAR	"SYSTEM"
#define	PRINTERVAR	"PRINTER"

#define	VARDEBUGFD1	"PALIVE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"palive"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/palive"		/* mutex PID file */
#define	LOGFNAME	"var/log/palive"	/* activity log */
#define	LOCKFNAME	"spool/locks/palive"	/* lock mutex file */

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Program Alive"

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */



