/* config */

/* last modified %G% version %I% */


#define	P_TCPMUXD	1

#define	VERSION		"0c"
#define	WHATINFO	"@(#)TCPMUXD "

#define	VARPROGRAMROOT1	"TCPMUXD_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	CONFVAR		"TCPMUXD_CONF"
#define	VARSEARCHNAME	"TCPMUXD_NAME"


#define	SEARCHNAME	"tcpmuxd"

/* search for these */

#define	CONFFNAME	"conf"
#define	SRVFNAME	"srvtab"
#define	ACCFNAME	"acctab"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	XENVFNAME	"xenv"
#define	XPATHFNAME	"xpath"
#define	PASSWDFNAME	"passwd"

/* search for and optionally create these */

#define	LOGFNAME	"log/%S"		/* activity log */
#define	REQFNAME	"var/%S/req"
#define	PASSFNAME	"var/%S/pass"		/* pass-FD file */
#define	PIDFNAME	"var/run/%S"		/* mutex PID file */
#define	LOCKFNAME	"var/spool/locks/%S"

/* create these ? */

#define	SHMFNAME	"var/spool/%S/shareinfo"
#define	SERIALFNAME1	"var/spool/serial"
#define	SERIALFNAME2	"/tmp/serial"

#define	VARDNAME	"var/%S"
#define	SPOOLDNAME	"var/spool/%S"
#define	WORKDIR		"/tmp"
#define	TMPDIR		"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	PORTNAME	"tcpmux"
#define	PORTNUM		"5108"			/* default TCP port */

#define	LOGSIZE		(80*1024)

#define	BANNER		"TCP Multiplexor Daemon (TCPMUXD)"
#define	ORGCODE		"RC"

#define	DEFPATH		"/bin:/usr/sbin"

#define	PROG_SENDMAIL	"/usr/lib/sendmail"

#define	TO_SVC		60			/* service acquire timeout */
#define	TI_MARKTIME	(3600*12)

#define	VARDEBUGFD1	"TCPMUXD_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"



