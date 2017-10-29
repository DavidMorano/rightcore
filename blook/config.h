/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)look "
#define	BANNER		"Look"
#define	SEARCHNAME	"look"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"LOOK_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LOOK_BANNER"
#define	VARSEARCHNAME	"LOOK_NAME"
#define	VAROPTS		"LOOK_OPTS"
#define	WORDSVAR	"LOOK_WORDS"
#define	VARFILEROOT	"LOOK_FILEROOT"
#define	VARLOGTAB	"LOOK_LOGTAB"
#define	VAREFNAME	"LOOK_EF"

#define	VARDEBUGFNAME	"LOOK_DEBUGFILE"
#define	VARDEBUGFD1	"LOOK_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARNODE		"NODE"
#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARNISDOMAIN	"NISDOMAIN"
#define	VARTERM		"TERM"
#define	VARPRINTER	"PRINTER"
#define	VARLPDEST	"LPDEST"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"
#define	VARLINES	"LINES"
#define	VARCOLUMNS	"COLUMNS"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

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
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/look"		/* mutex PID file */
#define	LOGFNAME	"var/log/look"		/* activity log */
#define	LOCKFNAME	"spool/locks/look"	/* lock mutex file */
#define	WORDSFNAME	"/usr/add-on/ncmp/share/dict/words"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */



