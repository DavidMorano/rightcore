/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)LKTEST "
#define	BANNER		"LockTest"
#define	SEARCHNAME	"lktest"

#define	VARPROGRAMROOT1	"LKTEST_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"LKTEST_BANNER"
#define	VARSEARCHNAME	"LKTEST_NAME"
#define	VARFNAME	"LKTEST_FNAME"
#define	LKTEST_NAMEVAR	"LKTEST_FNAME"
#define	VARERRORFNAME	"LKTEST_ERRORFILE"

#define	VARDEBUGFNAME	"LKTEST_DEBUGFILE"
#define	VARDEBUGFD1	"LKTEST_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

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

#define	TMPDIR		"/tmp"
#define	WORKDIR		"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	LOGFNAME	"log/lktest"		/* activity log */
#define	PIDFNAME	"spool/run/lktest"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/lktest"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	LKTEST_DEFTIMEOUT	(5 * 60)		/* seconds */
#define	LKTEST_DEFREMOVE	(5 * 60)		/* seconds */
#define	LKTEST_MULREMOVE	10



