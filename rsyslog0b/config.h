/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)rsyslog "
#define	SEARCHNAME	"rsyslog"
#define	BANNER		"Remote System Log"

#define	VARPROGRAMROOT1	"RSYSLOG_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"RSYSLOG_NAME"
#define	VAROPTS		"RSYSLOG_OPTS"
#define	VARLOGHOST	"RSYSLOG_LOGHOST"
#define	VARSVC		"RSYSLOG_SVC"
#define	VARERRORFNAME	"RSYSLOG_ERRORFILE"

#define	VARDEBUGFNAME	"RSYSLOG_DEBUGFILE"
#define	VARDEBUGFD1	"RSYSLOG_DEBUGFD"
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

#define	SYSFNAME1	"sn"
#define	SYSFNAME2	"systems"

#define	CONFFNAME1	"etc/rsyslog/rsyslog.conf"
#define	CONFFNAME2	"etc/rsyslog/conf"
#define	CONFFNAME3	"etc/rsyslog.conf"

#define	LOGFNAME	"log/rsyslog"
#define	HELPFNAME	"help"

#define	SVCSPEC_RSYSLOG	"rsyslog"

#define	MAILHOST	"www.rightcore.com"

#define	LOGHOST		"www.rightcore.com"		/* default LOGHOST */
#define	LOGPRIORITY	"user.info"
#define	LOGTAG		""

#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */



