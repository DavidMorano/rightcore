/* config */

/* last modified %G% version %I% */


#define	P_PCSUUCPD	1

#define	VERSION		"0a"
#define	WHATINFO	"@(#)PCSUUCPD "
#define	BANNER		"PCS UUCP Daemon (PCSUUCPD)"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"PCSUUCPD_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PCS"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"PCSUUCPD_BANNER"
#define	VARSEARCHNAME	"PCSUUCPD_NAME"
#define	VAROPTS		"PCSUUCPD_OPTS"
#define	VARCONF		"PCSUUCPD_CONF"
#define	VARTMPDNAME	"TMPDIR"
#define	VARERRORFNAME	"PCSUUCPD_ERRORFILE"

#define	VARDEBUGFNAME	"PCSUUCPD_DEBUGFILE"
#define	VARDEBUGFD1	"PCSUUCPD_DEBUGFD"
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

#define	SEARCHNAME	"pcsuucpd"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	VARDNAME	"var/%S"
#define	SPOOLDNAME	"var/spool/%S"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

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
#define	PIDFNAME	"var/run/%S"		/* mutex PID file */
#define	MSFNAME		"var/spool/ms"
#define	LOCKFNAME	"var/spool/locks/%S"

/* create these? */

#define	SHMFNAME	"var/spool/%S/shareinfo"
#define	SERIALFNAME1	"var/spool/serial"
#define	SERIALFNAME2	"/tmp/serial"

#define	PORTNAME	"pcsuucp"
#define	PORTNUM		"5111"			/* default TCP port */

#define	LOGSIZE		(80*1024)

#define	ORGCODE		"RC"

#define	DEFPATH		"/bin:/usr/sbin"

#define	PROG_SENDMAIL	"/usr/lib/sendmail"

#define	TI_MARKTIME	(3600*12)
#define	TI_POLLINT	60

#define	TO_LOCK		(5*60)
#define	TO_SVC		60			/* service acquire timeout */
#define	TO_SPEED	(24 * 3600)

#define	LOGIN_TEXT	"RightCore network services\n"
#define	LOGIN_PROMPT	"UUCP login: "
#define	PASSWORD_PROMPT	"password: "



