/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)SHOWMEMCONF "
#define	BANNER		"Show Memory Configuration"

#define	VARPROGRAMROOT1	"SHOWMEMCONF_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SHOWMEMCONF_BANNER"
#define	VARSEARCHNAME	"SHOWMEMCONF_NAME"
#define	VARFILEROOT	"SHOWMEMCONF_FILEROOT"
#define	VARLOGTAB	"SHOWMEMCONF_LOGTAB"
#define	VARERRORFNAME	"SHOWMEMCONF_ERRORFILE"

#define	VARDEBUGFNAME	"SHOWMEMCONF_DEBUGFILE"
#define	VARDEBUGFD1	"SHOWMEMCONF_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"showmemconf"

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

#define	PIDFNAME	"run/showmemconf"		/* mutex PID file */
#define	LOGFNAME	"var/log/showmemconf"	/* activity log */
#define	LOCKFNAME	"spool/locks/showmemconf"	/* lock mutex file */



