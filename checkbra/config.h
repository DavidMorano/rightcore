/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)checkbra "
#define	SEARCHNAME	"checkbra"
#define	BANNER		"Check Braces"

#define	VARPROGRAMROOT1	"CHECKBRA_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"CHECKBRA_BANNER"
#define	VARSEARCHNAME	"CHECKBRA_NAME"

#define	VARFILEROOT	"CHECKBRA_FILEROOT"
#define	VARLOGTAB	"CHECKBRA_LOGTAB"

#define	NODEVAR		"NODE"
#define	CLUSTERVAR	"CLUSTER"
#define	SYSTEMVAR	"SYSTEM"
#define	PRINTERVAR	"PRINTER"

#define	VARDEBUGFD1	"CHECKBRA_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

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

#define	PIDFNAME	"run/checkbra"		/* mutex PID file */
#define	LOGFNAME	"var/log/checkbra"	/* activity log */
#define	LOCKFNAME	"spool/locks/checkbra"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PO_OPTION	"option"
#define	PO_SUFFIX	"suffix"


