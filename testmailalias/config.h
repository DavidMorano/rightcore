/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testmailalias "
#define	BANNER		"Test MAILALIAS"

#define	VARPROGRAMROOT1	"TESTMAILALIAS_PROGRAMROOT"
#define	VARPROGRAMROOT2	"HOME"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTMAILALIAS_BANNER"
#define	VARSEARCHNAME	"TESTMAILALIAS_NAME"

#define	VARFILEROOT	"TESTMAILALIAS_FILEROOT"
#define	VARLOGTAB	"TESTMAILALIAS_LOGTAB"
#define	VARERRORFNAME	"TESTMAILALIAS_ERRORFILE"

#define	VARDEBUGFNAME	"TESTMAILALIAS_DEBUGFILE"
#define	VARDEBUGFD1	"TESTMAILALIAS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

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

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"testmailalias"

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

#define	PIDFNAME	"run/testmailalias"		/* mutex PID file */
#define	LOGFNAME	"var/log/testmailalias"		/* activity log */
#define	LOCKFNAME	"spool/locks/testmailalias"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */



