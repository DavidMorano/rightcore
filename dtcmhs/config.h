/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)dtcmhs "
#define	BANNER		"Desktop Calendar Have Server"
#define	SEARCHNAME	"dtcmhs"
#deifne	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/pcs"
#endif

#define	VARPROGRAMROOT1	"DTCMHS_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"DTCMHS_NAME"
#define	VARFILEROOT	"DTCMHS_FILEROOT"
#define	VARLOGTAB	"DTCMHS_LOGTAB"

#define	VARDEBUGFD1	"DTCMHS_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	VARTMPDNAME	"TMPDIR"

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"
#define	CALDNAME	"/var/spool/calendar"

#define	NISDOMAINNAME	"/etc/defaultdomain"
#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/dtcmhs"		/* mutex PID file */
#define	LOGFNAME	"var/log/dtcmhs"	/* activity log */
#define	LOCKFNAME	"spool/locks/dtcmhs"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	RUNINT		60
#define MAXIDLE		120
#define	CHECKLOGINT	(24 * 3600)

#define	CALLOG		"callog"

#define	SVCSPEC_DTCMHAVE	"dtcmhave"

#ifndef	IPPORT_DTCMHAVE	
#define	IPPORT_DTCMHAVE		5115
#endif


