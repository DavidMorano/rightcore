/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testutmpx "
#define	BANNER		"Test UTMPX"
#define	SEARCHNAME	"testutmpx"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTUTMPX_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTUTMPX_BANNER"
#define	VARSEARCHNAME	"TESTUTMPX_NAME"
#define	VAROPTS		"TESTUTMPX_OPTS"
#define	VARBASEDNAME	"TESTUTMPX_BASEDIR"
#define	VARQS		"TESTUTMPX_QS"
#define	VARDB		"TESTUTMPX_DB"
#define	VARDBFNAME	"TESTUTMPX_DBFILE"
#define	VARAFNAME	"TESTUTMPX_AF"
#define	VAREFNAME	"TESTUTMPX_EF"
#define	VARERRORFNAME	"TESTUTMPX_ERRORFILE"
#define	VARERRORFNAME	"TESTUTMPX_ERRORFILE"

#define	VARDEBUGFNAME	"TESTUTMPX_DEBUGFILE"
#define	VARDEBUGFD1	"TESTUTMPX_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
#define	VARHZ		"HZ"
#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARDOMAIN	"DOMAIN"
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
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"
#define	VARPATH		"PATH"
#define	VARMANPATH	"MANPATH"
#define	VARCDPATH	"CDPATH"
#define	VARLIBPATH	"LD_LIBRARY_PATH"
#define	VARQUERYSTRING	"QUERY_STRING"

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

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/testutmpx"		/* mutex PID file */
#define	LOGFNAME	"var/log/testutmpx"		/* activity log */
#define	LOCKFNAME	"spool/locks/testutmpx"	/* lock mutex file */
#define	UTMPXFNAME	"/var/adm/utmpx"
#define	WTMPXFNAME	"/var/adm/wtmpx"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */


