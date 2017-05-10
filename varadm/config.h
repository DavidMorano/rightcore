/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)VarAdm"
#define	BANNER		"Variable Administration"
#define	SEARCHNAME	"varadm"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"VARADM_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"VARADM_BANNER"
#define	VARSEARCHNAME	"VARADM_NAME"
#define	VAROPTS		"VARADM_OPTS"
#define	VARFILEROOT	"VARADM_FILEROOT"
#define	VARLOGTAB	"VARADM_LOGTAB"
#define	VARDBNAME	"VARADM_DBNAME"
#define	VARERRORFNAME	"VARADM_ERRORFILE"

#define	VARDEBUGFNAME	"VARADM_DEBUGFILE"
#define	VARDEBUGFD1	"VARADM_DEBUGFD"
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
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/varadm"		/* mutex PID file */
#define	LOGFNAME	"var/log/varadm"	/* activity log */
#define	LOCKFNAME	"spool/locks/varadm"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	DBNAME		"varadm"



