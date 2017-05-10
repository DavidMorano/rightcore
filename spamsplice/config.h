/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)spamsplice "

#define	VARPROGRAMROOT1	"SPAMSPLICE_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"SPAMSPLICE_BANNER"
#define	VARSEARCHNAME	"SPAMSPLICE_NAME"

#define	VARFILEROOT	"SPAMSPLICE_FILEROOT"
#define	VARLOGTAB	"SPAMSPLICE_LOGTAB"

#define	VARDEBUGFD1	"SPAMSPLICE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"spamsplice"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/spamsplice"		/* mutex PID file */
#define	LOGFNAME	"var/log/spamsplice"		/* activity log */
#define	LOCKFNAME	"spool/locks/spamsplice"	/* lock mutex file */

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Spam Splice"

#define	DEFSIZESPEC	"100000"		/* default target log size */



