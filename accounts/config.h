/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)accounts "
#define	BANNER		"Accounts"

#define	VARPROGRAMROOT1	"ACCOUNTS_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"ACCOUNTS_BANNER"
#define	VARSEARCHNAME	"ACCOUNTS_NAME"

#define	VARFILEROOT	"ACCOUNTS_FILEROOT"
#define	VARLOGTAB	"ACCOUNTS_LOGTAB"

#define	NODEVAR		"NODE"
#define	CLUSTERVAR	"CLUSTER"
#define	SYSTEMVAR	"SYSTEM"
#define	PRINTERVAR	"PRINTER"

#define	VARDEBUGFD1	"ACCOUNTS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"accounts"

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

#define	PIDFNAME	"run/accounts"		/* mutex PID file */
#define	LOGFNAME	"var/log/accounts"	/* activity log */
#define	LOCKFNAME	"spool/locks/accounts"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PO_SUFFIX	"suffix"
#define	PO_OPTION	"option"


