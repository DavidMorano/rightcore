/* config */


#define	VERSION		"0a"
#define	WHATINFO	"@(#)sieve "

#define	VARPROGRAMROOT1	"SIEVE_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARFILEROOT	"SIEVE_FILEROOT"
#define	VARLOGTAB	"SIEVE_LOGTAB"

#define	VARDEBUGFD1	"SIEVE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"sieve"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/sieve"		/* mutex PID file */
#define	LOGFNAME	"var/log/sieve"	/* activity log */
#define	LOCKFNAME	"spool/locks/sieve"	/* lock mutex file */

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Login Name"

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(60 * 60)		/* IPASSWD timeout */

#define	PROG_MKPWI	"mkpwi"



