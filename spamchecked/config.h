/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)spamchecked "

#define	VARPROGRAMROOT1	"SPAMCHECKED_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SPAMCHECKED_BANNER"
#define	VARSEARCHNAME	"SPAMCHECKED_NAME"

#define	VARFILEROOT	"SPAMCHECKED_FILEROOT"
#define	VARLOGTAB	"SPAMCHECKED_LOGTAB"

#define	VARDEBUGFD1	"SPAMCHECKED_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"spamchecked"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/spamchecked"		/* mutex PID file */
#define	LOGFNAME	"var/log/spamchecked"		/* activity log */
#define	LOCKFNAME	"spool/locks/spamchecked"	/* lock mutex file */

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Spam Checked"

#define	DEFSIZESPEC	"100000"		/* default target log size */



