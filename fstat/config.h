/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)fstat "
#define	BANNER		"File Status"
#define	SEARCHNAME	"fstat"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"FSTAT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"FSTAT_BANNER"
#define	VARSEARCHNAME	"FSTAT_NAME"
#define	VAROPTS		"FSTAT_OPTS"
#define	VARFILEROOT	"FSTAT_FILEROOT"
#define	VARLOGTAB	"FSTAT_LOGTAB"
#define	VARAFNAME	"FSTAT_AF"
#define	VAREFNAME	"FSTAT_EF"
#define	VARERRORFNAME	"FSTAT_ERRORFILE"

#define	VARDEBUGFNAME	"FSTAT_DEBUGFILE"
#define	VARDEBUGFD1	"FSTAT_DEBUGFD"
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

#define	PIDFNAME	"run/fstat"		/* mutex PID file */
#define	LOGFNAME	"var/log/fstat"		/* activity log */
#define	LOCKFNAME	"spool/locks/fstat"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PO_OPTION	"option"
#define	PO_OTYPE	"otype"

/* output types */

#define	OTYPE_INFO	0
#define	OTYPE_INT	1
#define	OTYPE_TOUCH	2
#define	OTYPE_TOUCHT	3
#define	OTYPE_TTOUCH	4
#define	OTYPE_ACCESS	5
#define	OTYPE_DEC	6
#define	OTYPE_DECIMAL	7
#define	OTYPE_LOG	8
#define	OTYPE_LOGZ	9
#define	OTYPE_STD	10



