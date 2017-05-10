/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testvechand "

#define	VARPROGRAMROOT1	"TESTVECHAND_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTVECHAND_BANNER"
#define	VARSEARCHNAME	"TESTVECHAND_NAME"

#define	VARFILEROOT	"TESTVECHAND_FILEROOT"
#define	VARLOGTAB	"TESTVECHAND_LOGTAB"

#define	VARDEBUGFD1	"TESTVECHAND_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testvechand"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/testvechand"		/* mutex PID file */
#define	LOGFNAME	"var/log/testvechand"	/* activity log */
#define	LOCKFNAME	"spool/locks/testvechand"	/* lock mutex file */

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Test VECHAND"

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */




