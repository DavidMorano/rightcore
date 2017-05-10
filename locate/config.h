/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)LOCATE "
#define	BANNER		"Locate File"

#define	VARPROGRAMROOT1	"LOCATE_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LOCATE_BANNER"
#define	VARSEARCHNAME	"LOCATE_NAME"
#define	VARERRFILE	"LOCATE_ERRFILE"
#define	VARFILEROOT	"LOCATE_FILEROOT"
#define	VARLOGTAB	"LOCATE_LOGTAB"

#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"

#define	VARDEBUGFD1	"LOCATE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"locate"

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

#define	PIDFNAME	"run/locate"		/* mutex PID file */
#define	LOGFNAME	"var/log/locate"	/* activity log */
#define	LOCKFNAME	"spool/locks/locate"	/* lock mutex file */



