/* config */


#define	VERSION		"1f"
#define	WHATINFO	"@(#)daytimer "

#define	PROGRAMROOTVAR1	"PROGRAMROOT_DAYTIMER"
#define	PROGRAMROOTVAR2	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	CONFIGFILE1	"etc/daytimer/daytimer.conf"
#define	CONFIGFILE2	"etc/daytimer/conf"
#define	CONFIGFILE3	"etc/daytimer.conf"

#define	LOGFNAME	"log/daytimer"
#define	SUMFNAME	"log/daytimer.sum"
#define	HELPFNAME	"lib/daytimer/help"
#define	LOCKDIR		"/tmp/locks/daytimer/"
#define	PIDDIR		"spool/run/daytimer/"

#define	MAILDIR		"/var/spool/mail"
#define	BANNER		"Login Daytime"

#define	DEF_OFFSET	(5*60)		/* default offset 5 minutes */
#define	DEF_TIMEOUT	(8 * 60)	/* default screen blanking timeout */
#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */
#define	DEF_REFRESH	(1 * 60)	/* automatic refresh interval */
#define	DEF_MAILTIME	(1 * 60)	/* active mail display time */

#define	DEBUGFDVAR1	"DAYTIMER_DEBUGFD"
#define	DEBUGFDVAR2	"DEBUGFD"



