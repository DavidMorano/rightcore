/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)noticed "
#define	BANNER		"Notice Server"
#define	SEARCHNAME	"noticed"

#define	VARPROGRAMROOT1	"NOTICED_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PROGRAMROOT"

#define	VARBANNER	"NOTICED_BANNER"
#define	VARSEARCHNAME	"NOTICED_NAME"
#define	VARERRORFNAME	"NOTIVED_ERRORFILE"

#define	VARDEBUGFNAME	"NOTIVED_DEBUGFILE"
#define	VARDEBUGFD1	"NOTIVED_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	TMPDNAME	"/tmp"
#define	LOCKDNAME	"/tmp/locks/noticed/"
#define	PIDDNAME	"var/run/noticed/"

#define	CONFIGFNAME1	"etc/noticed/noticed.conf"
#define	CONFIGFNAME2	"etc/noticed/conf"
#define	CONFIGFNAME3	"etc/noticed.conf"

#define	LOGFNAME	"log/noticed"
#define	HELPFNAME	"lib/noticed/help"

#define	DEF_OFFSET	(5*60)		/* default offset 5 minutes */
#define	DEF_TIMEOUT	(8 * 60)	/* default screen blanking timeout */
#define	DEF_REFRESH	(1 * 60)	/* automatic refresh interval */
#define	DEF_MAILTIME	(1 * 60)	/* active mail display time */
#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */



