/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testipasswd "
#define	BANNER		"Test IPasswd"
#define	SEARCHNAME	"testipasswd "
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTIPASSWD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTIPASSWD_BANNER"
#define	VARSEARCHNAME	"TESTIPASSWD_NAME"
#define	VAROPTS		"TESTIPASSWD_OPTS"
#define	VARFILEROOT	"TESTIPASSWD_FILEROOT"
#define	VARLOGTAB	"TESTIPASSWD_LOGTAB"
#define	VARMSFNAME	"TESTIPASSWD_MSFILE"
#define	VARAFNAME	"TESTIPASSWD_AF"
#define	VAREFNAME	"TESTIPASSWD_EF"

#define	VARDEBUGFNAME	"TESTIPASSWD_DEBUGFILE"
#define	VARDEBUGFD1	"TESTIPASSWD_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARPRINTER	"PRINTER"

#define	VARTMPDNAME	"TMPDIR"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/rest"		/* mutex PID file */
#define	LOGFNAME	"var/log/rest"		/* activity log */
#define	LOCKFNAME	"spool/locks/rest"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	IPASSWDDB	"/sys/realname"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_POLL		(8 * 60)

#define	USERFSUF	"user"



