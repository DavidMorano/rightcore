/* config */


#define	VERSION		"0b"
#define	WHATINFO	"@(#)EXECNAME "
#define	BANNER		"Execute Name"
#define	SEARCHNAME	"execname"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"EXECNAME_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"EXECNAME_BANNER"
#define	VARSEARCHNAME	"EXECNAME_NAME"
#define	VAROPTS		"EXECNAME_OPTS"
#define	VARFILEROOT	"EXECNAME_FILEROOT"
#define	VARLOGTAB	"EXECNAME_LOGTAB"
#define	VARAFNAME	"EXECNAME_AF"
#define	VAREFNAME	"EXECNAME_EF"

#define	VARDEBUGLEVEL	"EXECNAME_DEBUGLEVEL"
#define	VARDEBUGFNAME	"EXECNAME_DEBUGFILE"
#define	VARDEBUGFD1	"EXECNAME_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"
#define	VARPRNCMP	"NCMP"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	MAPFNAME	"map"
#define	DEFSFNAME	"defs"
#define	ENVFNAME	"environ"
#define	PATHFNAME	"path"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/execname"		/* mutex PID file */
#define	LOGFNAME	"var/log/execname"	/* activity log */
#define	LOCKFNAME	"spool/locks/execname"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	PO_OPTION	"option"


