/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)daytimed "
#define	BANNER		"Daytime Daemon"

#define	VARPROGRAMROOT1	"DAYTIMED_PROGRAMROOT"
#define	VARPROGRAMROOT2	"EXTRA"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"DAYTIMED_NAME"
#define	VARERRORFNAME	"DAYTIMED_ERRORFILE"

#define	VARDEBUGFNAME	"DAYTIMED_DEBUGFILE"
#define	VARDEBUGFD1	"DAYTIMED_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"
#define	VARDEBUGFD3	"ERROR_FD"

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
#define	VARPREXTRA	"EXTRA"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"daytimed"

#define	TMPDNAME	"/tmp"
#define	LOCKDNAME	"/tmp/locks/daytimed/"
#define	PIDDNAME	"spool/run/daytimed/"
#define	MAILDNAME	"/var/spool/mail"

#define	CONFIGFILE1	"etc/daytimed/daytimed.conf"
#define	CONFIGFILE2	"etc/daytimed/conf"
#define	CONFIGFILE3	"etc/daytimed.conf"

#define	HELPFNAME	"help"
#define	LOGFNAME	"log/daytimed"

#define	SVCSPEC_DAYTIMED	"daytimed"

#define	DEF_OFFSET	(5*60)		/* default offset 5 minutes */
#define	DEF_TIMEOUT	(8 * 60)	/* default screen blanking timeout */
#define	DEF_REFRESH	(1 * 60)	/* automatic refresh interval */
#define	DEF_MAILTIME	(1 * 60)	/* active mail display time */
#define	TO_LOCK		(5 * 60)	/* lockfile timeout */
#define	TO_READ		10

#ifndef	ORGANIZATION
#define	ORGANIZATION	"RC"
#endif



