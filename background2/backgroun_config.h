/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)jobsub "
#define	BANNER		"Job Submit"

#define	VARPROGRAMROOT1	"JOBSUB_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"JOBSUB_BANNER"
#define	VARSEARCHNAME	"JOBSUB_NAME"
#define	VARSTDERRFNAME	"JOBSUB_ERRFILE"
#define	VARMSFNAME	"JOBSUB_MSFILE"

#define	VARFILEROOT	"JOBSUB_FILEROOT"
#define	VARLOGTAB	"JOBSUB_LOGTAB"

#define	VARNODE		"NODE"
#define	VARCLUSTER	"CLUSTER"
#define	VARSYSTEM	"SYSTEM"
#define	VARPRINTER	"PRINTER"

#define	VARDEBUGFD1	"JOBSUB_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"jobsub"

#define	INITFNAME	"/etc/default/init"
#define	LOGSTDINFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	MSDNAME		"var"
#define	LOGDNAME	"var/log"
#define	RUNDNAME	"var/run"
#define	PIDDNAME	"var/run/jobsub"
#define	LOCKDNAME	"var/spool/locks"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	MSFNAME		"msfname"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"%N"			/* mutex PID file */
#define	LOGFNAME	"var/log/jobsub"	/* activity log */
#define	LOCKFNAME	"%N.%S"			/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	QUEUENAME	"b"

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */
#define	TO_SPEED	3600
#define	TO_LOCK		(5 * 60)

#define	POLLINT		5
#define	RUNINT		3600
#define	MARKINT		(4 * 3600)



