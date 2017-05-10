/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testopenport "
#define	SEARCHNAME	"testopenport"
#define	BANNER		"Test OpenPort"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"TESTOPENPORT_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TESTOPENPORT_BANNER"
#define	VARSEARCHNAME	"TESTOPENPORT_NAME"
#define	VARDEBUGLEVEL	"TESTOPENPORT_DEBUGLEVEL"
#define	VARFILEROOT	"TESTOPENPORT_FILEROOT"
#define	VARLOGFNAME	"TESTOPENPORT_LOGFILE"
#define	VARDBFNAME	"TESTOPENPORT_DBFILE"
#define	VARAFNAME	"TESTOPENPORT_AF"
#define	VAREFNAME	"TESTOPENPORT_EF"
#define	VARERRORFNAME	"TESTOPENPORT_ERRORFILE"

#define	VARDEBUGFNAME	"TESTOPENPORT_DEBUGFILE"
#define	VARDEBUGFD1	"TESTOPENPORT_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

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

#define	LOGDNAME	"log"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/openport"		/* mutex PID file */
#define	LOGFNAME	"var/log/openport"	/* activity log */
#define	LOCKFNAME	"spool/locks/openport"	/* lock mutex file */
#define	USERPORTSFNAME	"/etc/userports"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	INTRUN		30



