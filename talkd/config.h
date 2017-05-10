/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)talkd "


#define	VARPROGRAMROOT1	"TALKD_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"talkd"


#define	CONFVAR		"TALKD_CONF"
#define	VARSEARCHNAME	"TALKD_NAME"
#define	LOGFILEVAR	"TALKD_LOGFILE"


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

#define	LOGFNAME	"/var/adm/log/talkd"
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

#define	PORTNAME	"talk"
#define	PORTNUM		"517"			/* default TALK port */

#define	LOGSIZE		(80*1024)

#define	BANNER		"Talk Daemon"
#define	ORGCODE		"RC"

#define	DEFPATH		"/bin:/usr/sbin"

#define	VARDEBUGFD1	"TALKD_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	NISDOMAINNAME	"/etc/defaultdomain"



