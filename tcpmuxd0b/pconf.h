/* config */

/* last modified %G% version %I% */


#define	P_TCPMUXD	1

#define	VERSION		"0d"
#define	WHATINFO	"@(#)TCPMUXD "
#define	SEARCHNAME	"tcpmuxd"
#define	BANNER		"TCP Multiplexor Daemon (TCPMUXD)"

#define	VARPROGRAMROOT1	"TCPMUXD_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"TCPMUXD_BANNER"
#define	VARSEARCHNAME	"TCPMUXD_NAME"
#define	VAROPTS		"TCPMUXD_OPTS"
#define	VARCONF		"TCPMUXD_CONF"
#define	VARERRORFNAME	"TCPMUXD_ERRORFILE"

#define	VARDEBUGFNAME	"TCPMUXD_DEBUGFILE"
#define	VARDEBUGFD1	"TCPMUXD_DEBUGFD"
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

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	VARDNAME	"var/%S"
#define	MSDNAME		"var"
#define	PIDDNAME	"var/run"
#define	RUNDNAME	"var/run"
#define	LOGDNAME	"var"
#define	SPOOLDNAME	"var/spool/%S"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

/* search for these */

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ACCFNAME	"acctab"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	XENVFNAME	"xenv"
#define	XPATHFNAME	"xpath"
#define	PASSWDFNAME	"passwd"
#define	HELPFNAME	"help"

/* search for and optionally create these */

#define	LOGFNAME	"log/%S"		/* activity log */
#define	REQFNAME	"var/%S/req"
#define	PASSFNAME	"var/%S/pass"		/* pass-FD file */
#define	MSFNAME		"var/ms"
#define	PIDFNAME	"var/run/%S"		/* mutex PID file */
#define	LOCKFNAME	"var/spool/locks/%S"

/* create these? */

#define	SHMFNAME	"var/spool/%S/info"
#define	SERIALFNAME1	"var/spool/serial"
#define	SERIALFNAME2	"/tmp/serial"

/* other stuff */

#define	PORTNAME	"tcpmux"
#define	PORTNUM		"5108"			/* default TCP port */

#define	LOGSIZE		(80*1024)

#define	ORGCODE		"RC"

#define	DEFPATH		"/bin:/usr/sbin"

#define	PROG_SENDMAIL	"/usr/lib/sendmail"

#define	TI_MARKTIME	(3600*12)
#define	TI_POLLINT	60

#define	TO_LOCK		(5*60)
#define	TO_SVC		60			/* service acquire timeout */
#define	TO_SPEED	(24 * 3600)



