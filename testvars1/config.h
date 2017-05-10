/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testvars "
#define	BANNER		"Test Variables"
#define	SEARCHNAME	"testvars"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTVARS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTVARS_BANNER"
#define	VARSEARCHNAME	"TESTVARS_NAME"
#define	VARFILEROOT	"TESTVARS_FILEROOT"
#define	VARLOGTAB	"TESTVARS_LOGTAB"
#define	VARMSFNAME	"TESTVARS_MSFILE"
#define	VAROPTS		"TESTVARS_OPTS"
#define	VARERRORFNAME	"TESTVARS_ERRORFILE"

#define	VARDEBUGFNAME	"TESTVARS_DEBUGFILE"
#define	VARDEBUGFD1	"TESTVARS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARSYSNAME	"SYSNAME"
#define	VARRELEASE	"RELEASE"
#define	VARMACHINE	"MACHINE"
#define	VARARCHITECTURE	"ARCHITECTURE"
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
#define	VARHZ		"HZ"
#define	VARTZ		"TZ"
#define	VARUSERNAME	"USERNAME"
#define	VARLOGNAME	"LOGNAME"

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
#define	DEVDNAME	"/dev"
#define	DBDNAME		"/var/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/testvars"		/* mutex PID file */
#define	LOGFNAME	"var/log/testvars"	/* activity log */
#define	LOCKFNAME	"spool/locks/testvars"	/* lock mutex file */
#define	MSFNAME		"ms"

#define	DBNAME		"testvars"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */
#define	KEYBUFLEN	100



