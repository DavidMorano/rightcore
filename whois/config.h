/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)whois "
#define	BANNER		"Who Is"
#define	SEARCHNAME	"whois"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"WHOIS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"WHOIS_BANNER"
#define	VARSEARCHNAME	"WHOIS_NAME"
#define	VAROPTS		"WHOIS_OPTS"
#define	VARFILEROOT	"WHOIS_FILEROOT"
#define	VARLOGTAB	"WHOIS_LOGTAB"
#define	VARAFNAME	"WHOIS_AF"
#define	VAREFNAME	"WHOIS_EF"
#define	VARERRORFNAME	"WHOIS_ERRORFILE"

#define	VARDEBUGFNAME	"WHOIS_DEBUGFILE"
#define	VARDEBUGFD1	"WHOIS_DEBUGFD"
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
#define	VARNAME		"NAME"
#define	VARFULLNAME	"FULLNAME"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/whois"		/* mutex PID file */
#define	LOGFNAME	"var/log/whois"		/* activity log */
#define	LOCKFNAME	"spool/locks/whois"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	TO_CONNECT	20			/* seconds */
#define	TO_READ		8			/* seconds */

#ifndef	COLUMNS
#define	COLUMNS		80
#endif


