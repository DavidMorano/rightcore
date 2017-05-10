/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)mailforward "
#define	BANNER		"Mail Forward"
#define	SEARCHNAME	"mailforward"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MAILFORWARD_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MAILFORWARD_BANNER"
#define	VARSEARCHNAME	"MAILFORWARD_NAME"
#define	VAROPTS		"MAILFORWARD_OPTS"
#define	VARFILEROOT	"MAILFORWARD_FILEROOT"
#define	VARLOGTAB	"MAILFORWARD_LOGTAB"
#define	VARMSFNAME	"MAILFORWARD_MSFILE"
#define	VARAFNAME	"MAILFORWARD_AF"
#define	VAREFNAME	"MAILFORWARD_EF"

#define	VARDEBUGFD1	"MAILFORWARD_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	MAILDNAME	"/var/mail"
#define	MSDNAME		"var"
#define	LOGDNAME	"var/log"
#define	RUNDNAME	"var/run"
#define	PIDDNAME	"var/run/%S"
#define	LOCKDNAME	"var/spool/locks"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	MSFNAME		"ms"
#define	PIDFNAME	"mailforward"		/* mutex PID file */
#define	LOGFNAME	"mailforward"		/* activity log */
#define	LOCKFNAME	"%N.%S"			/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	RUNINT		60
#define	POLLINT		8
#define	MARKINT		(1 * 24 * 3600)

#define	TO_RUN		60
#define	TO_POLL		8
#define	TO_MARK		(1 * 24 * 3600)
#define	TO_MAILLOCK	(5 * 60)
#define	TO_SPEED	(24 * 3600)


