/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)audiolevel "

#define	VARPROGRAMROOT1	"AUDIOLEVEL_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"AUDIOLEVEL_BANNER"
#define	VARSEARCHNAME	"AUDIOLEVEL_NAME"

#define	VARFILEROOT	"AUDIOLEVEL_FILEROOT"
#define	VARLOGTAB	"AUDIOLEVEL_LOGTAB"

#define	VARDEBUGFD1	"AUDIOLEVEL_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	AUDIODEVVAR	"AUDIODEV"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"audiolevel"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	PIDFNAME	"run/audiolevel"		/* mutex PID file */
#define	LOGFNAME	"var/log/audiolevel"		/* activity log */
#define	LOCKFNAME	"spool/locks/audiolevel"	/* lock mutex file */

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"
#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	LOGSIZE		(80*1024)

#define	BANNER		"Audio Level"

#define	DEVDNAME	"/dev"
#define	AUDIODEV	"/dev/audio"




