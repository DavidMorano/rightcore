/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)cbvan "
#define	BANNER		"C-Language Beautifier"

#define	VARPROGRAMROOT1	"CBVAN_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"CBVAN_BANNER"
#define	VARSEARCHNAME	"CBVAN_NAME"

#define	VARFILEROOT	"CBVAN_FILEROOT"
#define	VARLOGTAB	"CBVAN_LOGTAB"

#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARPRINTER	"PRINTER"

#define	VARDEBUGFD1	"CBVAN_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"cbvan"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/cbvan"		/* mutex PID file */
#define	LOGFNAME	"var/log/cbvan"	/* activity log */
#define	LOCKFNAME	"spool/locks/cbvan"	/* lock mutex file */

#define	LOGSIZE		(80*1024)



