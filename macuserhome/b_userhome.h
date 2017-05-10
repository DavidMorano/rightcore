/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)USERHOME "
#define	BANNER		"User Hone"
#define	SEARCHNAME	"userhome"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"USERHOME_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"USERHOME_BANNER"
#define	VARSEARCHNAME	"USERHOME_NAME"
#define	VARLOGTAB	"USERHOME_LOGTAB"
#define	VAREFNAME	"USERHOME_EF"

#define	VARDEBFNAME	"USERHOME_DEBUGFILE"
#define	VARDEBUGFD1	"USERHOME_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	INITFNAME	"/etc/default/init"
#define	LOGSTDINFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	HELPDNAME	"share/help"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/userhome"		/* mutex PID file */
#define	LOGFNAME	"var/log/userhome"	/* activity log */
#define	LOCKFNAME	"spool/locks/userhome"	/* lock mutex file */

#define	LOGLEN		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */



