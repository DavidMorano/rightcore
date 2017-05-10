/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)isuser "
#define	BANNER		"Is User"
#define	SEARCHNAME	"isuser"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"ISUSER_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"ISUSER_BANNER"
#define	VARSEARCHNAME	"ISUSER_NAME"
#define	VAROPTS		"ISUSER_OPTS"
#define	VARFILEROOT	"ISUSER_FILEROOT"
#define	VARLOGTAB	"ISUSER_LOGTAB"
#define	VAREFNAME	"ISUSER_EF"
#define	VARERRORFNAME	"ISUSER_ERRORFILE"

#define	VARDEBUGFNAME	"ISUSER_DEBUGFILE"
#define	VARDEBUGFD1	"ISUSER_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	NODEVAR		"NODE"
#define	CLUSTERVAR	"CLUSTER"
#define	SYSTEMVAR	"SYSTEM"
#define	PRINTERVAR	"PRINTER"

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

#define	PIDFNAME	"run/isuser"		/* mutex PID file */
#define	LOGFNAME	"var/log/isuser"	/* activity log */
#define	LOCKFNAME	"spool/locks/isuser"	/* lock mutex file */

#define	LOGSIZE		(80*1024)


