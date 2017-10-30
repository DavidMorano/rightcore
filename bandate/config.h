/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)bandate "
#define	BANNER		"Is User"
#define	SEARCHNAME	"bandate"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"BANDATE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"BANDATE_BANNER"
#define	VARSEARCHNAME	"BANDATE_NAME"
#define	VAROPTS		"BANDATE_OPTS"
#define	VARFILEROOT	"BANDATE_FILEROOT"
#define	VARLOGTAB	"BANDATE_LOGTAB"
#define	VARAFNAME	"BANDATE_AF"
#define	VAREFNAME	"BANDATE_EF"
#define	VARERRORFNAME	"BANDATE_ERRORFILE"
#define	VARDEBUGLEVEL	"BANDATE_DEBUGLEVEL"

#define	VARDEBUGFNAME	"BANDATE_DEBUGFILE"
#define	VARDEBUGFD1	"BANDATE_DEBUGFD"
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

#define	PIDFNAME	"run/bandate"		/* mutex PID file */
#define	LOGFNAME	"var/log/bandate"	/* activity log */
#define	LOCKFNAME	"spool/locks/bandate"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	PO_SUFFIX	"suffix"
#define	PO_OPTION	"option"


