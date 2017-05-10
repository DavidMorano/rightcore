/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)obmetar "

#define	VARPROGRAMROOT1	"OBMETAR_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"OBMETAR_BANNER"
#define	VARSEARCHNAME	"OBMETAR_NAME"

#define	VARFILEROOT	"OBMETAR_FILEROOT"
#define	VARLOGTAB	"OBMETAR_LOGTAB"

#define	VARDEBUGFD1	"OBMETAR_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"obmetar"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/obmetar"		/* mutex PID file */
#define	LOGFNAME	"var/log/obmetar"	/* activity log */
#define	LOCKFNAME	"spool/locks/obmetar"	/* lock mutex file */

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Login Name"

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PROG_MKPWI	"mkpwi"



