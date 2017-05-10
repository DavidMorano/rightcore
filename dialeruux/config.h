/* config */


/* revision history:

	- 1998-02-01, David A­D­ Morano

	This subroutine was originally written.


*/


#define	VERSION		"0"
#define	WHATINFO	"@(#)testdialuux "
#define	SEARCHNAME	"testdialuux"
#define	BANNER		"Test Dial Program"

#define	VARPROGRAMROOT1	"TESTDIALUUX_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"TESTDIALUUX_NAME"
#define	VAROPTS		"TESTDIALUUX_OPTS"
#define	LOGHOSTVAR	"TESTDIALUUX_LOGHOST"
#define	VARSVC		"TESTDIALUUX_SVC"

#define	VARERRORFNAME	"TESTDIALUUX_ERRORFD"
#define	VARDEBUGFNAME	"TESTDIALUUX_DEBUGFD"
#define	VARDEBUGFD1	"TESTDIALUUX_DEBUGFD"
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

#define	CONFFNAME1	"etc/testdialuux/testdialuux.conf"
#define	CONFFNAME2	"etc/testdialuux/conf"
#define	CONFFNAME3	"etc/testdialuux.conf"

#define	LOGFNAME	"log/testdialuux"

#define	HELPFNAME	"help"

#define	SVCSPEC_TESTDIALUUX	"testdialuux"

#define	MAILHOST	"www.rightcore.com"

#define	LOGHOST		"www.rightcore.com"		/* default LOGHOST */
#define	LOGPRIORITY	"user.info"
#define	LOGTAG		""

#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */



