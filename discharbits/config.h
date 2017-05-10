/* config */


#define	VERSION		"c"
#define	WHATINFO	"@(#)discharbits "
#define	BANNER		"Display Character Bits"
#define	SEARCHNAME	"discharbits"

#define	VARPROGRAMROOT1	"DISCHARBITS_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"DISCHARBITS_BANNER"
#define	VARSEARCHNAME	"DISCHARBITS_NAME"

#define	VARFILEROOT	"DISCHARBITS_FILEROOT"
#define	VARLOGTAB	"DISCHARBITS_LOGTAB"

#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARPRINTER	"PRINTER"

#define	VARDEBUGFD1	"DISCHARBITS_DEBUGFD"
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

#define	PIDFNAME	"run/discharbits"		/* mutex PID file */
#define	LOGFNAME	"var/log/discharbits"	/* activity log */
#define	LOCKFNAME	"spool/locks/discharbits"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PROG_MKPWI	"mkpwi"



