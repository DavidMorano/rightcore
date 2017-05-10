/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testisproc "
#define	SEARCHNAME	"testisproc"
#define	BANNER		"Test ISPROC"

#define	VARPROGRAMROOT1	"TESTISPROC_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTISPROC_BANNER"
#define	VARSEARCHNAME	"TESTISPROC_NAME"
#define	VARFILEROOT	"TESTISPROC_FILEROOT"
#define	VARLOGTAB	"TESTISPROC_LOGTAB"
#define	VARERRORFNAME	"TESTISPROC_ERRORFILE"

#define	VARDEBUGFNAME	"TESTISPROC_DEBUGFILE"
#define	VARDEBUGFD1	"TESTISPROC_DEBUGFD"
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

#define	PIDFNAME	"run/testisproc"		/* mutex PID file */
#define	LOGFNAME	"var/log/testisproc"		/* activity log */
#define	LOCKFNAME	"spool/locks/testisproc"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */



