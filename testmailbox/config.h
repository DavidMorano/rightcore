/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testmailbox "
#define	SEARCHNAME	"testmailbox"
#define	BANNER		"Test Mailbox"

#define	VARPROGRAMROOT1	"TESTMAILBOX_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTMAILBOX_BANNER"
#define	VARSEARCHNAME	"TESTMAILBOX_NAME"
#define	VAROPTS		"TESTMAILBOX_OPTS"
#define	VARFILEROOT	"TESTMAILBOX_FILEROOT"
#define	VARLOGTAB	"TESTMAILBOX_LOGTAB"
#define	VARAFNAME	"TESTMAILBOX_AF"
#define	VAREFNAME	"TESTMAILBOX_EF"
#define	VARERRORFNAME	"TESTMAILBOX_ERRORFILE"

#define	VARDEBUGFNAME	"TESTMAILBOX_DEBUGFILE"
#define	VARDEBUGFD1	"TESTMAILBOX_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

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
#define	WORKDNAME	"/tmp"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/testmailbox"		/* mutex PID file */
#define	LOGFNAME	"var/log/testmailbox"		/* activity log */
#define	LOCKFNAME	"spool/locks/testmailbox"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */



