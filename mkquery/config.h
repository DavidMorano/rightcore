/* config */


#define	VERSION		"0"
#define	WHATINFO	"@(#)MKQUERY "
#define	SEARCHNAME	"mkquery"
#define	BANNER		"Make Query"

#define	VARPROGRAMROOT1	"MKQUERY_PROGRAMROOT"
#define	VARPROGRAMROOT2	"LOCAL"
#define	VARPROGRAMROOT3	"PROGRAMROOT"

#define	VARBANNER	"MKQUERY_BANNER"
#define	VARSEARCHNAME	"MKQUERY_NAME"

#define	VARDEBUGFD1	"MKQUERY_DEBUGFD"
#define	VARDEBUGFD2	"DEBUGFD"

#ifndef	PROGRAMROOT
#define	PROGRAMROOT	"/usr/add-on/local"
#endif

#define	ETCDIR1		"etc/mkquery"
#define	ETCDIR2		"etc"
#define	CONFIGFILE1	"mkquery.conf"
#define	CONFIGFILE2	"conf"
#define	LOGFNAME	"log/mkquery"
#define	HELPFNAME	"help"
#define	INDEXNAME	"index"

#define	TMPDNAME	"/tmp"
#define	LOGSIZE		(80*1024)

/* tuning defaults */

#define	NHASH		(64 * 1024)



