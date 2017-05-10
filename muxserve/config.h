/* config */

/* last modified %G% version %I% */


#define	P_MUXSERVE	1

#define	VERSION		"0"
#define	WHATINFO	"@(#)MUXSERVE "
#define	SEARCHNAME	"muxserve"
#define	BANNER		"Multiplexor Server"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MUXSERVE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MUXSERVE_BANNER"
#define	VARSEARCHNAME	"MUXSERVE_NAME"
#define	VAROPTS		"MUXSERVE_OPTS"
#define	VARCONF		"MUXSERVE_CONF"
#define	VARLINELEN	"MUXSERVE_LINELEN"
#define	VARAFNAME	"MUXSERVE_AF"
#define	VAREFNAME	"MUXSERVE_EF"
#define	VARERRORFNAME	"MUXSERVE_ERRORFILE"

#define	VARDEBUGFNAME	"MUXSERVE_DEBUGFILE"
#define	VARDEBUGFD1	"MUXSERVE_DEBUGFD"
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
#define	VARNCPU		"NCPU"

#define	VARHOMEDNAME	"HOME"
#define	VARTMPDNAME	"TMPDIR"
#define	VARMAILDNAME	"MAILDIR"
#define	VARMAILDNAMES	"MAILDIRS"
#define	VARPATH		"PATH"

#define	VARPRLOCAL	"LOCAL"
#define	VARPRPCS	"PCS"

#ifndef	SVCSPECLEN
#define	SVCSPECLEN	120
#endif

#ifndef	SVCARGSLEN
#define	SVCARGSLEN	LINEBUFLEN
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	ETCDNAME	"etc"
#define	MSDNAME		"var"
#define	VARDNAME	"var"
#define	SPOOLDNAME	"var/spool"
#define	RUNDNAME	"var/run"
#define	STAMPDNAME	"var/timestamps"	/* timestamp directory */
#define	LOCKDNAME	"spool/locks"
#define	LOGDNAME	"log"
#define	LOGCNAME	"log"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	HELPFNAME	"help"
#define	CONFFNAME	"conf"
#define	DEFSFNAME	"def"
#define	XEFNAME		"xe"
#define	PVARSFNAME	"pvar"
#define	STAMPFNAME	"muxserve"
#define	REQFNAME	"req"			/* "request" (IPC) file */
#define	PASSFNAME	"pass"
#define	SVCFNAME	"svc"
#define	ACCFNAME	"acc"
#define	ENVFNAME	"env"
#define	PATHFNAME	"path"
#define	XENVFNAME	"xenv"
#define	XPATHFNAME	"xpath"
#define	PASSWDFNAME	"passwd"
#define	SERIALFNAME	"serial"		/* serial file */
#define	PROJFNAME	".project"
#define	PLANFNAME	".plan"
#define	MSFNAME		"var/spool/ms"

#define	LOGFNAME	"log/muxserve"		/* activity log */
#define	PIDFNAME	"var/run/muxserve"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/muxserve"	/* lock mutex file */

#define	SHMFNAME	"var/spool/%S/shareinfo"
#define	SERIALFNAME1	"var/serial"
#define	SERIALFNAME2	"/tmp/serial"
#define	USERSRV		"var/svc/ufinger"

#define	PVARFEXT	"pvar"
#define	USERFEXT	"users"
#define	REQFEXT		"req"
#define	SVCFEXT		"svc"
#define	ACCFEXT		"acc"

#define	DEFPATH		"/usr/xpg4/bin:/usr/bin:/usr/sbin"

#define	PORTNAME	"tcpmux"
#define	PORTNUM		"5108"		/* default TCP port */

#ifndef	COLUMNS
#define	COLUMNS		80		/* output columns (should be 80) */
#endif

#define	LOGSIZE		(80*1024)
#define	MAXJOBS		4		/* maximum jobs at once */
#define	DEFNDEFS	20
#define	DEFNXENVS	200

#define	ORGCODE		"RC"
#define	DEFSVC		"default"

#define	PROG_SENDMAIL	"/usr/lib/sendmail"

#define	TO_LOCK		(5*60)		/* lock time-out */
#define	TO_POLLSVC	(5*60)		/* default interval (minutes) */
#define	TO_RUN		(60*60)		/* default run interval */
#define	TO_MINCHECK	(1*60)		/* minimal check interval */
#define	TO_STANDCHECK	5		/* default standing-server check */
#define	TO_MAINT	5		/* general maintenance */
#define	TO_RECVMSG	3		/* receiving a message */
#define	TO_READSVC	3		/* reading a service-code */
#define	TO_READ		3		/* reading data */
#define	TO_SVC		60		/* service acquire timeout */
#define	TO_SPEED	(24*3600)	/* interval between updates */
#define	TO_MARKTIME	(12*3600)	/* mark-time interval */
#define	TO_BROKEN	(1*3600)	/* broken listener re-activation */
#define	TO_RECVFD	10		/* receiving a file-desriptor */
#define	TO_SENDFD	10		/* sending a file-desriptor */

#ifndef	VBUFLEN
#define	VBUFLEN		(2 * MAXPATHLEN)
#endif

#ifndef	EBUFLEN
#define	EBUFLEN		(3 * MAXPATHLEN)
#endif

#define	OPT_LOGPROG	TRUE


