/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)POSTQUERY "
#define	SEARCHNAME	"postquery"
#define	BANNER		"Make Query"

#define	VARPROGRAMROOT1	"POSTQUERY_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"POSTQUERY_BANNER"
#define	VARSEARCHNAME	"POSTQUERY_NAME"

#define	VARDEBUGFD1	"POSTQUERY_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	ETCDIR1		"etc/postquery"
#define	ETCDIR2		"etc"
#define	CONFIGFILE1	"postquery.conf"
#define	CONFIGFILE2	"conf"
#define	LOGFNAME	"log/postquery"
#define	HELPFNAME	"help"
#define	INDEXNAME	"index"

#define	TMPDNAME	"/tmp"
#define	LOGSIZE		(80*1024)

/* tuning defaults */

#define	NHASH		(64 * 1024)



