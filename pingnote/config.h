/* config */

/* last modified %G% version %I% */


#define	VERSION		"0"
#define	WHATINFO	"@(#)pingnote "
#define	BANNER		"Ping Note"
#define	SEARCHNAME	"pingnote"

#define	VARPROGRAMROOT1	"PINGNOTE_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"PINGNOTE_NAME"
#define	VAROPTS		"PINGNOTE_OPTS"
#define	VARERRORFNAME	"PINGNOTE_ERRORFILE"
#define	VARDEBUGFNAME	"PINGNOTE_DEBUGFILE"
#define	VARDEBUGFD1	"PINGNOTE_DEBUGFD"
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
#define	STAMPDNAME	"spool/timestamps"	/* timestamp directory */

#define	HELPFNAME	"help"
#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	REQFNAME	"req"

#define	LOGFNAME	"log/pingnote"		/* activity log */
#define	PIDFNAME	"spool/run/pingnote"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/pingnote"	/* lock mutex file */

#define	DEFPATH		"/bin:/usr/sbin"

#define	LOGSIZE		(80*1024)

#define	PORTSPEC_PINGSTAT	"pingstat"
#define	SVCSPEC_PINGSTAT	"pingstat"

#define	PORT_PINGSTAT		5112

#define	TO_DIAL			20

#define	PO_PINGHOST	"pinghost"


