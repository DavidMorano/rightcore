/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)mailbridge "
#define	BANNER		"Mail Bridge"
#define	SEARCHNAME	"mailbridge"

#define	VARPROGRAMROOT1	"MAILBRIDGE_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PCS"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"MAILBRIDGE_NAME"
#define	VAROPTS		"MAILBRIDGE_OPTS"
#define	VARMAILHOST	"MAILBRIDGE_MAILHOST"
#define	VARMAILSVC	"MAILBRIDGE_MAILSVC"
#define	VARERRORFNAME	"MAILBRIDGE_ERRORFILE"

#define	VARDEBUGFNAME	"MAILBRIDGE_DEBUGFILE"
#define	VARDEBUGFD1	"MAILBRIDGE_DEBUGFD"
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
#define	VARPRINTER	"PRINTER"
#define	VARPAGER	"PAGER"
#define	VARMAIL		"MAIL"
#define	VARORGANIZATION	"ORGANIZATION"

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

#define	SYSFNAME1	"sn"
#define	SYSFNAME2	"systems"
#define	CONFFNAME1	"etc/mailbridge/mailbridge.conf"
#define	CONFFNAME2	"etc/mailbridge/conf"
#define	CONFFNAME3	"etc/mailbridge.conf"
#define	LOGFNAME	"log/mailbridge"
#define	HELPFNAME	"help"

#define	SVCSPEC_MAILBRIDGE	"mailbridge"

#define	MAILHOST	"mailhost"	/* default MAILHOST */

#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */



