/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)jobsub "
#define	SEARCHNAME	"jobsub"
#define	BANNER		"Job Submit"

#define	VARPROGRAMROOT1	"JOBSUB_PROGRAMROOT"
#define	VARPROGRAMROOT2	"HOME"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"JOBSUB_BANNER"
#define	VARSEARCHNAME	"JOBSUB_NAME"
#define	VARQUEUE	"JOBSUB_QUEUE"
#define	VARSTDERRFNAME	"JOBSUB_ERRFILE"
#define	VARSPOOLDNAME	"JOBSUB_SPOOLDIR"
#define	VARPIDFNAME	"JOBSUB_PIDFILE"
#define	VARLOGFNAME	"JOBSUB_LOGFILE"
#define	VARMSFNAME	"JOBSUB_MSFILE"
#define	VARMAILADDR	"JOBSUB_MAILADDR"
#define	VARMAILER	"JOBSUB_MAILER"
#define	VARPOLLINT	"JOBSUB_POLLINT"

#define	VARFILEROOT	"JOBSUB_FILEROOT"

#define	VARDEBUGFD1	"JOBSUB_DEBUGFD"
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

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"
#define	JOBSUBSDNAME	"jobsubs"
#define	MSDNAME		"var"
#define	LOGDNAME	"var/log"
#define	RUNDNAME	"var/run"
#define	PIDDNAME	"var/run/jobsub"
#define	SPOOLDNAME	"var/spool"
#define	LOCKDNAME	"var/spool/locks"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	MSFNAME		"msfname"
#define	FULLFNAME	".fullname"

#define	PROGSERVER	"jobsubs"
#define	PROGMAILER	"rmail"

#define	PIDFNAME	"%N.%S"			/* mutex PID file */
#define	LOGFNAME	"jobsub"		/* activity log */
#define	LOCKFNAME	"%N.%S"			/* lock mutex file */

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGSIZE		(80*1024)

#define	QUEUENAME	"b"

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */
#define	TO_SPEED	3600
#define	TO_LOCK		(5 * 60)

#define	POLLINT		25
#define	RUNINT		3600
#define	MARKINT		(4 * 3600)



