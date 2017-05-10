/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testinsolation "
#define	SEARCHNAME	"testinsolation"
#define	BANNER		"Test ISPROC"

#define	VARPROGRAMROOT1	"TESTINSOLATION_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTINSOLATION_BANNER"
#define	VARSEARCHNAME	"TESTINSOLATION_NAME"
#define	VARFILEROOT	"TESTINSOLATION_FILEROOT"
#define	VARLOGTAB	"TESTINSOLATION_LOGTAB"
#define	VARERRORFNAME	"TESTINSOLATION_ERRORFILE"

#define	VARDEBUGFNAME	"TESTINSOLATION_DEBUGFILE"
#define	VARDEBUGFD1	"TESTINSOLATION_DEBUGFD"
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

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/testinsolation"
#define	LOGFNAME	"var/log/testinsolation"
#define	LOCKFNAME	"spool/locks/testinsolation"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */



