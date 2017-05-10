/* config */


#define	VERSION		"0b"
#define	WHATINFO	"@(#)music "
#define	SEARCHNAME	"music"
#define	BANNER		"Music"
#define	VARPRNAME	"LOCAL"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	VARPROGRAMROOT1	"MUSIC_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARFILEROOT	"MUSIC_FILEROOT"
#define	VARLOGTAB	"MUSIC_LOGTAB"
#define	VARDAFNAME	"MUSIC_AF"
#define	VARDEFNAME	"MUSIC_EF"

#define	VARDEBUGFD1	"MUSIC_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"
#define	IPASSWDFNAME	"ipasswd"

#define	PIDFNAME	"run/music"		/* mutex PID file */
#define	LOGFNAME	"var/log/music"	/* activity log */
#define	LOCKFNAME	"spool/locks/music"	/* lock mutex file */

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	LOGSIZE		(80*1024)

#define	DEFSIZESPEC	"100000"		/* default target log size */

#define	TO_FILEMOD	(1 * 60 * 60)		/* IPASSWD timeout */

#define	PROG_MKPWI	"mkpwi"


