/* config - header defaults */


#define	VERSION		"0"
#define	WHATINFO	"@(#)mbo "
#define	BANNER		"Mailbox Out"
#define	SEARCHNAME	"mbo"

#define	VARPROGRAMROOT1	"MBO_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PCS"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"MBO_NAME"
#define	VAROPTS		"MBO_OPTS"
#define	VARERRORFNAME	"MBO_ERRORFILE"

#define	VARDEBUGFNAME	"MBO_DEBUGFILE"
#define	VARDEBUGFD1	"MBO_DEBUGFD"
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
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	TMPDNAME	"/tmp"
#define SPOOLDNAME	"/var/mail"
#define	MSGIDDBNAME	"var/mbo"

#define	HELPFNAME	"help"
#define	SERIALFNAME	"var/serial"
#define	COMSATFNAME	"etc/mbo.nodes"
#define	SPAMFNAME	"etc/mbo.spam"
#define	LOGFNAME	"log/mbo"
#define	USERFNAME	"log/mbo.users"
#define	LOGENVFNAME	"log/mbo.env"
#define	LOGZONEFNAME	"log/mbo.zones"

#define	MAILGROUP	"mail"

#define	MAILGID		6

#define	DIVERTUSER	"adm"

#define	FIELDLEN	4096
#define	DEFTIMEOUT	20
#define	MAILLOCKAGE	(5 * 60)

#define	MAXMSGID	490



