/* config */


#define	VERSION		"b"
#define	WHATINFO	"@(#)teststrtab "
#define	BANNER		"Test STRTAB"
#define	SEARCHNAME	"teststrtab"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTSTRTAB_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTSTRTAB_BANNER"
#define	VARSEARCHNAME	"TESTSTRTAB_NAME"
#define	VAROPTS		"TESTSTRTAB_OPTS"
#define	VARFILEROOT	"TESTSTRTAB_FILEROOT"
#define	VARLFNAME	"TESTSTRTAB_LF"
#define	VARAFNAME	"TESTSTRTAB_AF"
#define	VAREFNAME	"TESTSTRTAB_EF"
#define	VARERRORFNAME	"TESTSTRTAB_ERRORFILE"

#define	VARDEBUGFNAME	"TESTSTRTAB_DEBUGFILE"
#define	VARDEBUGFD1	"TESTSTRTAB_DEBUGFD"
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

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/teststrtab"		/* mutex PID file */
#define	LOGFNAME	"var/log/teststrtab"		/* activity log */
#define	LOCKFNAME	"spool/locks/teststrtab"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */


