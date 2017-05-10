/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)rsend "

#define	VARPROGRAMROOT1	"RSEND_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARSEARCHNAME	"RSEND_NAME"

#define	VARDEBUGFD1	"RSEND_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"
#define	VARDEBUGFD3	"ERROR_FD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	SEARCHNAME	"rsend"

#define	SYSFNAME1	"sn"
#define	SYSFNAME2	"systems"

#define	CONFFNAME1	"etc/rsend/rsend.conf"
#define	CONFFNAME2	"etc/rsend/conf"
#define	CONFFNAME3	"etc/rsend.conf"

#define	LOGFNAME	"log/rsend"
#define	HELPFNAME	"lib/rsend/help"

#define	BANNER		"Remote Send"

#define	SVCSPEC_RSEND	"rsend"

#define	DEF_OFFSET	(5*60)		/* default offset 5 minutes */
#define	DEF_TIMEOUT	(8 * 60)	/* default screen blanking timeout */
#define	LOCKTIMEOUT	(5 * 60)	/* lockfile timeout */
#define	DEF_REFRESH	(1 * 60)	/* automatic refresh interval */
#define	DEF_MAILTIME	(1 * 60)	/* active mail display time */



