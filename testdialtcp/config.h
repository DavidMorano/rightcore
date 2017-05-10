/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)testdialtcp "

#define	VARPROGRAMROOT1	"TESTDIALTCP_PROGRAMROOT"
#define	VARPROGRAMROOT2	"PROGRAMROOT"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	CONFIGFNAME1	"etc/testdialtcp/testdialtcp.conf"
#define	CONFIGFNAME2	"etc/testdialtcp/conf"
#define	CONFIGFNAME3	"etc/testdialtcp.conf"

#define	LOGFNAME	"log/testdialtcp"
#define	HELPFNAME	"lib/testdialtcp/help"
#define	LOCKDIR		"/tmp/locks/testdialtcp/"
#define	PIDDIR		"spool/run/testdialtcp/"

#define	MAILDNAME		"/var/spool/mail"
#define	BANNER		"Test DialTCP"

#define	DEF_OFFSET	(5*60)		/* default offset 5 minutes */
#define	DEF_TIMEOUT	(8 * 60)	/* default screen blanking timeout */
#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */
#define	DEF_REFRESH	(1 * 60)	/* automatic refresh interval */
#define	DEF_MAILTIME	(1 * 60)	/* active mail display time */

#define	VARDEBUGFD	"DEBUG_FD"



