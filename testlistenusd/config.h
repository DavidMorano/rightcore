/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testlistenusd "

#define	VARPROGRAMROOT1	"TESTLISTENUSD_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	CONFIGFNAME1	"etc/testlistenusd/testlistenusd.conf"
#define	CONFIGFNAME2	"etc/testlistenusd/conf"
#define	CONFIGFNAME3	"etc/testlistenusd.conf"

#define	LOGFNAME	"log/testlistenusd"
#define	HELPFNAME	"lib/testlistenusd/help"
#define	LOCKDIR		"/tmp/locks/testlistenusd/"
#define	PIDDIR		"spool/run/testlistenusd/"

#define	BANNER		"Test USD"

#define	DEF_OFFSET	(5*60)		/* default offset 5 minutes */
#define	DEF_TIMEOUT	(8 * 60)	/* default screen blanking timeout */
#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */
#define	DEF_REFRESH	(1 * 60)	/* automatic refresh interval */
#define	DEF_MAILTIME	(1 * 60)	/* active mail display time */

#define	VARDEBUGFD	"DEBUGFD"



