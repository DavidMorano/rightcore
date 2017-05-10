/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)modlist "
#define	BANNER		"Module List"
#define	SEARCHNAME	"modlist"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"MODLIST_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MODLIST_BANNER"
#define	VARSEARCHNAME	"MODLIST_NAME"
#define	VAROPTS		"MODLIST_OPTS"
#define	VARFILEROOT	"MODLIST_FILEROOT"
#define	VARLOGTAB	"MODLIST_LOGTAB"
#define	VARAFNAME	"MODLIST_AF"
#define	VAREFNAME	"MODLIST_EF"
#define	VAROFNAME	"MODLIST_OF"
#define	VARIFNAME	"MODLIST_IF"

#define	VARDEBUGFD1	"MODLIST_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

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

#define	PIDFNAME	"run/modlist"		/* mutex PID file */
#define	LOGFNAME	"var/log/modlist"	/* activity log */
#define	LOCKFNAME	"spool/locks/modlist"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEVBASE		"/dev"

#define	PO_SUFFIX	"suffix"
#define	PO_OPTION	"option"


