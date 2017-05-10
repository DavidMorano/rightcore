/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)isinteractive "
#define	BANNER		"Is Interactive"
#define	SEARCHNAME	"isinteractive"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"ISINTERACTIVE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"ISINTERACTIVE_BANNER"
#define	VARSEARCHNAME	"ISINTERACTIVE_NAME"
#define	VAROPTS		"ISINTERACTIVE_OPTS"
#define	VARFILEROOT	"ISINTERACTIVE_FILEROOT"
#define	VARLOGTAB	"ISINTERACTIVE_LOGTAB"
#define	VAREFNAME	"ISINTERACTIVE_EF"
#define	VARERRORFNAME	"ISINTERACTIVE_ERRORFILE"

#define	VARDEBUGFNAME	"ISINTERACTIVE_DEBUGFILE"
#define	VARDEBUGFD1	"ISINTERACTIVE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	NODEVAR		"NODE"
#define	CLUSTERVAR	"CLUSTER"
#define	SYSTEMVAR	"SYSTEM"
#define	PRINTERVAR	"PRINTER"

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

#define	PIDFNAME	"run/isinteractive"
#define	LOGFNAME	"var/log/isinteractive"
#define	LOCKFNAME	"spool/locks/isinteractive"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	PO_SUFFIX	"suffix"
#define	PO_OPTION	"option"


