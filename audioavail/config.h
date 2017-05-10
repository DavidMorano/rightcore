/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)audioavail "

#define	VARPROGRAMROOT1	"AUDIOAVAIL_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"AUDIOAVAIL_BANNER"
#define	VARSEARCHNAME	"AUDIOAVAIL_NAME"

#define	VARFILEROOT	"AUDIOAVAIL_FILEROOT"
#define	VARLOGTAB	"AUDIOAVAIL_LOGTAB"
#define	AUDIODEVVAR	"AUDIODEV"

#define	NODEVAR		"NODE"
#define	CLUSTERVAR	"CLUSTER"
#define	SYSTEMVAR	"SYSTEM"
#define	PRINTERVAR	"PRINTER"

#define	VARDEBUGFD1	"AUDIOAVAIL_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"audioavail"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"
#define	FULLFNAME	".fullname"

#define	PIDFNAME	"run/audioavail"		/* mutex PID file */
#define	LOGFNAME	"var/log/audioavail"	/* activity log */
#define	LOCKFNAME	"spool/locks/audioavail"	/* lock mutex file */

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	AUDIODEV	"/dev/audio"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Audio Avail"

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */




