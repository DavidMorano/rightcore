/* config */


#define	VERSION		"0b"
#define	WHATINFO	"@(#)LKFILE "
#define	SEARCHNAME	"lkfile"
#define	BANNER		"LockFile"
#define	VARPRNAME	"LOCAL"

#define	VARPROGRAMROOT1	"LKFILE_PROGRAMROOT"
#define	VARPROGRAMROOT2	VARPRNAME
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"LKFILE_NAME"

#define	VARFNAME	"LKFILE_FNAME"

#define	VARDEBUGFD1	"LKFILE_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDNAME	"/tmp"
#define	WORKDNAME	"/tmp"

#define	DEFINITFNAME	"/etc/default/init"
#define	DEFLOGFNAME	"/etc/default/login"

#define	CONFIGFNAME	"conf"
#define	ENVFNAME	"environ"
#define	PATHSFNAME	"paths"
#define	HELPFNAME	"help"

#define	LOGFNAME	"log/lkfile"		/* activity log */
#define	PIDFNAME	"spool/run/lkfile"	/* mutex PID file */
#define	LOCKFNAME	"spool/locks/lkfile"	/* lock mutex file */

#define	LOGSIZE		(80*1024)

#define	LKFILE_DEFTIMEOUT	(5 * 60)		/* seconds */
#define	LKFILE_DEFREMOVE	(5 * 60)		/* seconds */
#define	LKFILE_MULREMOVE	10




