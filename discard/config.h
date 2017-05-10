/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)DISCARD "

#define	VARPROGRAMROOT1	"DISCARD_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARDEBUGFD1	"DISCARD_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"discard"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"

#define	HELPFNAME	"help"
#define	LOGFNAME	"log/discard"		/* activity log */
#define	PIDFNAME	"var/run/discard"	/* mutex PID file */
#define	PSFNAME		"spool/discard/discard"
#define	LOCKFNAME	"spool/locks/discard"	/* lock mutex file */

#define	PTDNAME		"etc/discard"
#define	PSSPOOLDNAME	"spool/discard"
#define	WORKDNAME	"/tmp"
#define	TMPDNAME	"/tmp"

#define	DEFLOGFNAME	"/etc/default/login"
#define	DEFINITFNAME	"/etc/default/init"

#define	LOGSIZE		(160*1024)

#define	BANNER		"Discard"

#define	RUNTIMEOUT	60
#define	MININPUTINT	120		/* minimum input interval (timeout) */


